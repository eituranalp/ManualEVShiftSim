#include "clutch.hpp"
#include <algorithm>

namespace ev_sim {

Clutch::Clutch(float stiffness)
    : stiffness_(stiffness)
    , engagement_level_(0.0f)
{
}

void Clutch::update(float& engine_rpm, float& transmission_rpm, 
                    float clutch_engaged, float dt) {
    // Clamp engagement level
    clutch_engaged = std::clamp(clutch_engaged, 0.0f, 1.0f);
    engagement_level_ = clutch_engaged;
    
    if (clutch_engaged == 0.0f) {
        // Disengaged: decay transmission RPM from internal friction
        float decay_rate = 0.03f; // ~3% per second decay rate
        transmission_rpm *= (1.0f - decay_rate * dt);
        transmission_rpm = std::max(transmission_rpm, 0.0f); // prevent reversal
        // Engine RPM remains unchanged (runs independently)
        return;
    }
    else if (clutch_engaged == 1.0f) {
        // Fully engaged: fast convergence toward average RPM
        float avg_rpm = (engine_rpm + transmission_rpm) * 0.5f;
        float fast_convergence_rate = 0.8f; // 80% convergence per timestep (much faster than partial engagement)
        
        // Move both RPMs toward their average quickly but not instantly
        engine_rpm += (avg_rpm - engine_rpm) * fast_convergence_rate;
        transmission_rpm += (avg_rpm - transmission_rpm) * fast_convergence_rate;
    }
    else {
        // Partial engagement: gradual convergence by engagement, stiffness, dt
        
        float avg_rpm = (engine_rpm + transmission_rpm) * 0.5f;
        float convergence_rate = clutch_engaged * stiffness_ * dt;
        
        // Clamp to prevent overshoot
        convergence_rate = std::clamp(convergence_rate, 0.0f, 1.0f);
        
        // Move both RPMs toward average
        engine_rpm += (avg_rpm - engine_rpm) * convergence_rate;
        transmission_rpm += (avg_rpm - transmission_rpm) * convergence_rate;
    }
}

} // namespace ev_sim