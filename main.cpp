#include <iostream>
#include <iomanip>
#include <cmath>
#include "include/engine.hpp"

int main() {
    // Create engine with test parameters
    // idle_rpm, max_rpm, flywheel_inertia, max_torque, drag_coefficient
    ev_sim::Engine engine(800.0f, 7000.0f, 0.1f, 200.0f, 0.25f);
    
    // Simulation parameters
    const float dt = 0.016f;  // 16ms timestep (~60Hz)
    const float sim_duration = 12.0f;  // Extended to 12 seconds for more scenarios
    float current_time = 0.0f;
    
    // Test scenarios: 
    // 0-1s:   Idle behavior
    // 1-2s:   Quick throttle blips (0->50->0)
    // 2-4s:   Gradual ramp to 40%
    // 4-5s:   Hold at 40%
    // 5-6s:   Quick drop to 20%
    // 6-7s:   Hold at 20%
    // 7-8s:   Aggressive blip to 100%
    // 8-9s:   Quick oscillations (100->0->100->0)
    // 9-12s:  Cool down, return to idle
    
    std::cout << "Time(s) | Throttle(%) | RPM    | Torque(Nm)\n";
    std::cout << "----------------------------------------\n";
    
    while (current_time < sim_duration) {
        float throttle_percent = 0.0f;
        
        // Idle period
        if (current_time < 1.0f) {
            throttle_percent = 0.0f;
        }
        // Quick blips
        else if (current_time < 2.0f) {
            float cycle = std::fmod(current_time - 1.0f, 0.4f);  // 0.4s per blip cycle
            if (cycle < 0.1f) {  // 0.1s at 50% throttle
                throttle_percent = 50.0f;
            }
        }
        // Gradual ramp to 40%
        else if (current_time < 4.0f) {
            throttle_percent = (current_time - 2.0f) * 20.0f;  // 0->40 over 2s
        }
        // Hold at 40%
        else if (current_time < 5.0f) {
            throttle_percent = 40.0f;
        }
        // Quick drop to 20%
        else if (current_time < 6.0f) {
            throttle_percent = 20.0f;
        }
        // Hold at 20%
        else if (current_time < 7.0f) {
            throttle_percent = 20.0f;
        }
        // Aggressive blip to 100%
        else if (current_time < 8.0f) {
            throttle_percent = 100.0f;
        }
        // Quick oscillations
        else if (current_time < 9.0f) {
            float cycle = std::fmod(current_time - 8.0f, 0.2f);  // 0.2s per oscillation
            throttle_percent = (cycle < 0.1f) ? 100.0f : 0.0f;
        }
        // Cool down
        else {
            throttle_percent = 0.0f;
        }
        
        // Simulate basic drivetrain load (temporary until clutch/transmission implemented)
        // This represents: transmission losses + rolling resistance + accessories
        const float base_load = 20.0f;  // Nm - reduced to allow higher RPM at full throttle
        const float rpm_dependent_load = 0.005f * engine.getRPM();  // Reduced RPM dependency
        const float load_torque = base_load + rpm_dependent_load;
        
        // Update engine
        engine.update(throttle_percent, load_torque, dt);
        
        // Print results every 0.1 seconds
        if (std::fmod(current_time, 0.1f) < dt) {
            std::cout << std::fixed << std::setprecision(1)
                     << std::setw(7) << current_time << " | "
                     << std::setw(11) << throttle_percent << " | "
                     << std::setw(6) << engine.getRPM() << " | "
                     << std::setw(9) << engine.getTorque() << "\n";
        }
        
        current_time += dt;
    }
    
    return 0;
}