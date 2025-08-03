#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>
#include <fstream>
#include "include/engine.hpp"
#include "include/clutch.hpp"

// Helper function for clamping values
inline float clamp(float value, float min_val, float max_val) {
    return std::max(min_val, std::min(value, max_val));
}

// Test data structure
struct TestData {
    float time;
    float throttle;
    float clutch_pedal;
};

int main() {
    // Open output CSV file
    std::ofstream csv_file("../output/clutch_engine_test_results.csv");
    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not open output CSV file\n";
        return 1;
    }
    
    // Write CSV header
    csv_file << "Time,Throttle,ClutchPedal,EngageLevel,EngineRPM,TransRPM,RPMDiff,EngineTorque\n";
    
    std::cout << "=== INTEGRATED ENGINE & CLUTCH TEST ===\n";
    std::cout << "Using realistic test scenario with engine and clutch dynamics\n";
    std::cout << "Results will be saved to: output/clutch_engine_test_results.csv\n\n";
    
    // Create engine and clutch
    ev_sim::Engine engine(800.0f, 7000.0f, 0.1f, 200.0f, 0.25f);
    ev_sim::Clutch clutch(10.0f);  // 10 Hz stiffness
    
    // Test data for clutch engagement effects on engine acceleration
    std::vector<TestData> test_data = {
        // Phase 1: Idle throttle (0%) tests
        // Start at idle, clutch out
        {0.0f, 0.0f, 100.0f}, {1.0f, 0.0f, 100.0f},
        // Quick clutch engagement
        {1.1f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f},
        // Quick clutch disengagement
        {2.1f, 0.0f, 100.0f}, {3.0f, 0.0f, 100.0f},
        // Gradual clutch engagement
        {3.1f, 0.0f, 75.0f}, {3.5f, 0.0f, 50.0f}, 
        {4.0f, 0.0f, 25.0f}, {4.5f, 0.0f, 0.0f},
        // Gradual clutch disengagement
        {5.0f, 0.0f, 25.0f}, {5.5f, 0.0f, 50.0f},
        {6.0f, 0.0f, 75.0f}, {6.5f, 0.0f, 100.0f},

        // Phase 2: High throttle (80%) tests
        // Start with clutch out, rev up
        {7.0f, 80.0f, 100.0f}, {8.0f, 80.0f, 100.0f},
        // Quick clutch engagement
        {8.1f, 80.0f, 0.0f}, {9.0f, 80.0f, 0.0f},
        // Quick clutch disengagement
        {9.1f, 80.0f, 100.0f}, {10.0f, 80.0f, 100.0f},
        // Gradual clutch engagement
        {10.1f, 80.0f, 75.0f}, {10.5f, 80.0f, 50.0f},
        {11.0f, 80.0f, 25.0f}, {11.5f, 80.0f, 0.0f},
        // Gradual clutch disengagement
        {12.0f, 80.0f, 25.0f}, {12.5f, 80.0f, 50.0f},
        {13.0f, 80.0f, 75.0f}, {13.5f, 80.0f, 100.0f},

        // Phase 3: Rapid clutch pumping
        // Idle throttle pumping
        {14.0f, 0.0f, 100.0f}, {14.2f, 0.0f, 0.0f},
        {14.4f, 0.0f, 100.0f}, {14.6f, 0.0f, 0.0f},
        {14.8f, 0.0f, 100.0f}, {15.0f, 0.0f, 0.0f},
        // High throttle pumping
        {15.5f, 80.0f, 100.0f}, {15.7f, 80.0f, 0.0f},
        {15.9f, 80.0f, 100.0f}, {16.1f, 80.0f, 0.0f},
        {16.3f, 80.0f, 100.0f}, {16.5f, 80.0f, 0.0f}
    };
    
    // Simulation parameters
    const float dt = 0.1f;  // 100ms timestep
    float current_time = 0.0f;
    size_t data_index = 0;
    
    // Initialize transmission RPM (starts from rest)
    float transmission_rpm = 0.0f;
    
    std::cout << "Time(s) | Throttle | ClutchPed | EngageLevel | Engine RPM | Trans RPM | RPM Diff | Engine Torque\n";
    std::cout << "--------|----------|-----------|-------------|------------|-----------|----------|-------------\n";
    
    while (current_time <= 20.0f && data_index < test_data.size()) {
        // Interpolate or use closest test data point
        TestData current_data = test_data[data_index];
        if (data_index < test_data.size() - 1 && current_time >= test_data[data_index + 1].time) {
            data_index++;
            current_data = test_data[data_index];
        }
        
        // Convert clutch pedal position to engagement level
        // Clutch pedal: 100 = fully pressed (disengaged), 0 = released (engaged)  
        float clutch_engagement = 1.0f - (current_data.clutch_pedal / 100.0f);
        clutch_engagement = clamp(clutch_engagement, 0.0f, 1.0f);
        
        // Convert throttle percentage (handle negative values as engine braking)
        float throttle_percent = clamp(current_data.throttle, 0.0f, 100.0f);
        
        // Simulate drivetrain load (when clutch is engaged)
        float base_load = 15.0f;  // Base drivetrain losses
        float speed_load = 0.01f * transmission_rpm;  // Speed-dependent load
        float load_torque = (clutch_engagement > 0.1f) ? (base_load + speed_load) : 0.0f;
        
        // Update engine
        engine.update(throttle_percent, load_torque, dt);
        
        // Get current engine RPM
        float engine_rpm = engine.getRPM();
        
        // Update clutch (modifies RPMs by reference)
        clutch.update(engine_rpm, transmission_rpm, clutch_engagement, dt);
        
        // Calculate values for output
        float rpm_difference = engine_rpm - transmission_rpm;
        float engine_torque = engine.getTorque();
        
        // Save data to CSV (every timestep)
        csv_file << std::fixed << std::setprecision(3)
                 << current_time << ","
                 << current_data.throttle << ","
                 << current_data.clutch_pedal << ","
                 << clutch_engagement << ","
                 << engine_rpm << ","
                 << transmission_rpm << ","
                 << rpm_difference << ","
                 << engine_torque << "\n";
        
        // Print results to console (every 0.2 seconds for readability)
        if (std::fmod(current_time, 0.2f) < dt/2) {
            std::cout << std::fixed << std::setprecision(1)
                     << std::setw(7) << current_time << " | "
                     << std::setw(8) << current_data.throttle << " | "
                     << std::setw(9) << current_data.clutch_pedal << " | "
                     << std::setw(11) << clutch_engagement << " | "
                     << std::setw(10) << engine_rpm << " | "
                     << std::setw(9) << transmission_rpm << " | "
                     << std::setw(8) << rpm_difference << " | "
                     << std::setw(11) << engine_torque << "\n";
        }
        
        current_time += dt;
    }
    
    std::cout << "\n=== TEST PHASES SUMMARY ===\n";
    std::cout << "Phase 1 (0-7s):   Idle Throttle (0%) Tests\n";
    std::cout << "  0-1s:  Clutch out, establish baseline\n";
    std::cout << "  1-2s:  Quick clutch engagement\n";
    std::cout << "  2-3s:  Quick clutch disengagement\n";
    std::cout << "  3-4.5s: Gradual clutch engagement\n";
    std::cout << "  5-6.5s: Gradual clutch disengagement\n";
    std::cout << "\nPhase 2 (7-14s):  High Throttle (80%) Tests\n";
    std::cout << "  7-8s:   Clutch out, rev up\n";
    std::cout << "  8-9s:   Quick clutch engagement\n";
    std::cout << "  9-10s:  Quick clutch disengagement\n";
    std::cout << "  10-11.5s: Gradual clutch engagement\n";
    std::cout << "  12-13.5s: Gradual clutch disengagement\n";
    std::cout << "\nPhase 3 (14-17s): Rapid Clutch Pumping\n";
    std::cout << "  14-15s: Low throttle pumping\n";
    std::cout << "  15.5-16.5s: High throttle pumping\n";
    std::cout << "\nFinal State: Engine=" << engine.getRPM() 
              << " RPM, Transmission=" << transmission_rpm << " RPM\n";
    
    // Close CSV file
    csv_file.close();
    std::cout << "\nResults saved to: output/clutch_engine_test_results.csv\n";
    
    return 0;
}