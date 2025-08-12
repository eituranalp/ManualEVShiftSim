#include "engine.hpp"
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>

namespace ev_sim {

Engine::Engine(float idle_rpm, float max_rpm, float flywheel_inertia, float max_torque, float drag_coefficient)
    : rpm_(idle_rpm)
    , torque_output_(0.0f)
    , temperature_(80.0f)  // Normal operating temperature
    , idle_rpm_(idle_rpm)
    , max_rpm_(max_rpm)
    , flywheel_inertia_(flywheel_inertia)
    , max_torque_(max_torque)
    , drag_coefficient_(drag_coefficient)
{
}

float Engine::calculateTorque(float throttle_percent) const {
    // Simulates a typical engine torque curve:
    // - Torque builds up smoothly from idle
    // - Peaks in the mid-range (around 4-6k RPM)
    // - Starts falling off as we approach redline
    
    const float rpm_ratio = rpm_ / max_rpm_;
    float torque_curve;
    
    if (rpm_ratio < 0.6f) {
        // Build up to peak torque - quadratic curve
        torque_curve = (rpm_ratio / 0.6f) * (2.0f - rpm_ratio / 0.6f) * max_torque_;
    } else if (rpm_ratio < 0.85f) {
        // Maintain high torque in mid-range
        torque_curve = max_torque_ * (1.0f - 0.1f * (rpm_ratio - 0.6f) / 0.25f);
    } else {
        // Gradual falloff toward redline
        torque_curve = max_torque_ * 0.9f * (1.0f - rpm_ratio) / 0.15f;
    }
    
    // Add engine braking when throttle is low
    const float min_throttle = 0.1f;  // Minimum throttle to overcome engine braking
    float engine_braking = 0.0f;
    
    if (throttle_percent < min_throttle) {
        // Engine braking increases with RPM
        engine_braking = -20.0f * rpm_ratio;  // Simple linear model
    }
    
    // Calculate base torque
    float base_torque = torque_curve * throttle_percent + engine_braking;
    
    // Apply rev limiter
    const float rev_limit_start = 0.98f * max_rpm_;  // Start limiting at 98% max RPM
    if (rpm_ >= rev_limit_start) {
        // Linear reduction from rev_limit_start to max_rpm_
        float limit_factor = (max_rpm_ - rpm_) / (max_rpm_ - rev_limit_start);
        limit_factor = std::clamp(limit_factor, 0.0f, 1.0f);
        base_torque *= limit_factor;
    }
    
    return base_torque;
}

float Engine::calculateDragTorque() const {
    // Engine drag increases with the square of RPM
    // Uses a simple quadratic model: drag = -k * (rpm/1000)²
    // (negative because drag always opposes rotation)
    const float rpm_thousands = rpm_ / 1000.0f;
    return -drag_coefficient_ * rpm_thousands * rpm_thousands;
}

float Engine::calculateRPMChange(float load_torque, float dt) const {
    // Calculate drag torque
    const float drag_torque = calculateDragTorque();
    
    // Net torque = engine output - load - drag
    const float net_torque = torque_output_ - load_torque + drag_torque;
    
    // Angular acceleration = torque / inertia
    const float angular_accel = net_torque / flywheel_inertia_;
    
    // Convert to RPM/s (rad/s² → RPM/s)
    const float rpm_change = angular_accel * (60.0f / (2.0f * M_PI)) * dt;
    
    return rpm_change;
}

void Engine::limitRPM() {
    // Ensure RPM stays within valid range
    if (rpm_ < idle_rpm_) {
        // Apply additional torque to maintain idle
        const float idle_error = idle_rpm_ - rpm_;
        const float idle_correction = idle_error * 0.1f;  // Simple proportional control
        rpm_ = std::max(rpm_ + idle_correction, 0.0f);
    }
    
    rpm_ = std::min(rpm_, max_rpm_);
}

void Engine::update(float throttle_percent, float load_torque, float dt) {
    // Clamp throttle input
    throttle_percent = std::clamp(throttle_percent, 0.0f, 1.0f);
    
    // Calculate target torque based on current state
    float target_torque = calculateTorque(throttle_percent);
    
    // Apply exponential smoothing to torque changes
    float smoothing_factor = std::clamp(5.0f * dt, 0.0f, 1.0f);
    torque_output_ = torque_output_ + smoothing_factor * (target_torque - torque_output_);
    
    // Update RPM based on smoothed torque
    rpm_ += calculateRPMChange(load_torque, dt);
    
    // Apply RPM limits
    limitRPM();
    
    // Simple temperature model (future enhancement)
    // temperature_ = ...
}

} // namespace ev_sim 