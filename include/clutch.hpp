#pragma once

namespace ev_sim {

/**
 * Simplified Clutch class focusing on RPM synchronization
 * 
 * Three modes:
 * - Disengaged (clutchEngaged = 0.0): Independent RPM updates
 * - Engaged (clutchEngaged = 1.0): Locked RPM synchronization  
 * - Partial (0.0 < clutchEngaged < 1.0): Gradual RPM convergence
 */
class Clutch {
private:
    // Clutch parameters
    const float stiffness_;           // RPM convergence rate (Hz or 1/s)
    
    // State tracking (for future torque modeling)
    float engagement_level_;          // Current engagement level [0.0, 1.0]
    
public:
    /**
     * Constructor
     * @param stiffness RPM convergence rate - higher values = faster sync (default: 10.0 Hz)
     */
    Clutch(float stiffness = 10.0f);
    
    /**
     * Update clutch state and modify RPMs based on engagement
     * @param engine_rpm Engine RPM (modified by reference)
     * @param transmission_rpm Transmission input shaft RPM (modified by reference)  
     * @param clutch_engaged Clutch engagement level [0.0 = disengaged, 1.0 = engaged]
     * @param dt Time step (seconds)
     */
    void update(float& engine_rpm, float& transmission_rpm, 
                float clutch_engaged, float dt);
    
    // Getters
    float getEngagementLevel() const { return engagement_level_; }
    float getStiffness() const { return stiffness_; }
    
    // For future torque modeling expansion
    float calculateClutchTorque() const { return 0.0f; } // Placeholder
};

} // namespace ev_sim