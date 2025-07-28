#pragma once

namespace ev_sim {

class Engine {
private:
    // Engine State
    float rpm_;                    // Current engine RPM
    float torque_output_;         // Current output torque (Nm)
    float temperature_;           // Engine temperature (°C)
    
    // Engine Parameters
    const float idle_rpm_;        // Idle RPM
    const float max_rpm_;         // Maximum RPM (redline)
    const float flywheel_inertia_; // kg⋅m²
    const float max_torque_;      // Maximum torque output (Nm)
    const float drag_coefficient_; // Nm/(1000 RPM)²
    
    // Internal calculations
    float calculateTorque(float throttle_percent) const;
    float calculateRPMChange(float load_torque, float dt) const;
    float calculateDragTorque() const;  // New method for drag calculation
    void limitRPM();

public:
    Engine(float idle_rpm, float max_rpm, float flywheel_inertia, float max_torque, float drag_coefficient = 0.1f);
    
    // Core methods
    void update(float throttle_percent, float load_torque, float dt);
    
    // Getters
    float getRPM() const { return rpm_; }
    float getTorque() const { return torque_output_; }
    float getTemperature() const { return temperature_; }
    
    // Engine characteristics
    float getIdleRPM() const { return idle_rpm_; }
    float getMaxRPM() const { return max_rpm_; }
    float getInertia() const { return flywheel_inertia_; }
    float getDragCoefficient() const { return drag_coefficient_; }
};

} // namespace ev_sim 