#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <thread>
#include <vector>
#include "include/engine.hpp"
#include "include/clutch.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <GL/gl.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

// MSVC may treat double->float as warning-as-error; use a float PI constant
static constexpr float kPi = 3.14159265358979323846f;

// Draw a half-circle RPM gauge
void drawRPMGauge(const char* label, float value, float max_value, const ImVec4& color, float size = 120.0f) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = ImVec2(size, size * 0.6f);
    
    ImGui::InvisibleButton(label, canvas_size);
    
    ImVec2 center = ImVec2(canvas_pos.x + size * 0.5f, canvas_pos.y + size * 0.35f);
    float radius = size * 0.4f;
    
    const int segments = 32;
    const float start_angle = kPi;
    const float end_angle = 0.0f;
    
    for (int i = 0; i < segments; i++) {
        float a1 = start_angle + (end_angle - start_angle) * i / segments;
        float a2 = start_angle + (end_angle - start_angle) * (i + 1) / segments;
        
        ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);
        ImVec2 p2 = ImVec2(center.x + cosf(a2) * radius, center.y + sinf(a2) * radius);
        
        draw_list->AddLine(p1, p2, IM_COL32(60, 60, 60, 255), 8.0f);
    }
    
    float percentage = fminf(value / max_value, 1.0f);
    float value_angle = start_angle + (end_angle - start_angle) * percentage;
    
    int value_segments = (int)(segments * percentage);
    for (int i = 0; i < value_segments; i++) {
        float a1 = start_angle + (end_angle - start_angle) * i / segments;
        float a2 = start_angle + (end_angle - start_angle) * (i + 1) / segments;
        
        ImVec2 p1 = ImVec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);
        ImVec2 p2 = ImVec2(center.x + cosf(a2) * radius, center.y + sinf(a2) * radius);
        
        ImU32 col = ImGui::ColorConvertFloat4ToU32(color);
        draw_list->AddLine(p1, p2, col, 8.0f);
    }
    
    ImVec2 needle_end = ImVec2(center.x + cosf(value_angle) * (radius - 10), 
                               center.y + sinf(value_angle) * (radius - 10));
    draw_list->AddLine(center, needle_end, IM_COL32(255, 255, 255, 255), 3.0f);
    
    draw_list->AddCircleFilled(center, 6.0f, IM_COL32(80, 80, 80, 255));
    draw_list->AddCircleFilled(center, 4.0f, IM_COL32(200, 200, 200, 255));
    
    for (int i = 0; i <= 10; i++) {
        float mark_angle = start_angle + (end_angle - start_angle) * i / 10.0f;
        ImVec2 mark_inner = ImVec2(center.x + cosf(mark_angle) * (radius - 15), 
                                   center.y + sinf(mark_angle) * (radius - 15));
        ImVec2 mark_outer = ImVec2(center.x + cosf(mark_angle) * radius, 
                                   center.y + sinf(mark_angle) * radius);
        
        float thickness = (i % 5 == 0) ? 2.0f : 1.0f;
        draw_list->AddLine(mark_inner, mark_outer, IM_COL32(150, 150, 150, 255), thickness);
    }
    
    ImVec2 text_size = ImGui::CalcTextSize("00000");
    ImVec2 text_pos = ImVec2(center.x - text_size.x * 0.5f, center.y - 10);
    
    char value_text[32];
    snprintf(value_text, sizeof(value_text), "%.0f", value);
    draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), value_text);
    
    ImVec2 label_size = ImGui::CalcTextSize(label);
    ImVec2 label_pos = ImVec2(center.x - label_size.x * 0.5f, center.y + 15);
    draw_list->AddText(label_pos, IM_COL32(180, 180, 180, 255), label);
    
    char max_text[16];
    snprintf(max_text, sizeof(max_text), "%.0f", max_value);
    ImVec2 max_size = ImGui::CalcTextSize(max_text);
    ImVec2 max_pos = ImVec2(center.x + radius - max_size.x, center.y + 5);
    draw_list->AddText(max_pos, IM_COL32(120, 120, 120, 255), max_text);
    
    draw_list->AddText(ImVec2(center.x - radius, center.y + 5), IM_COL32(120, 120, 120, 255), "0");
}

// Helper function for clamping values
inline float clamp(float value, float min_val, float max_val) {
    return std::max(min_val, std::min(value, max_val));
}

// Convert SDL3 axis value (-32768 to 32767) to percentage (0-100)
inline float axisToPercent(int16_t axis_value) {
    // Clamp to positive range and convert to 0-100%
    float normalized = static_cast<float>(std::max(0, static_cast<int>(axis_value))) / 32767.0f;
    return clamp(normalized * 100.0f, 0.0f, 100.0f);
}

int main() {
    // Initialize SDL3 with video and gamepad support
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        return 1;
    }
    
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Manual EV Shift Simulator - Real-Time Dashboard",
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        SDL_Quit();
        return 1;
    }
    
    // Create OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Enable VSync
    SDL_GL_SetSwapInterval(1);
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Find and open the first available gamepad
    SDL_Gamepad* gamepad = nullptr;
    int num_joysticks = 0;
    SDL_JoystickID* joysticks = SDL_GetJoysticks(&num_joysticks);
    
    if (joysticks && num_joysticks > 0) {
        for (int i = 0; i < num_joysticks; i++) {
            if (SDL_IsGamepad(joysticks[i])) {
                gamepad = SDL_OpenGamepad(joysticks[i]);
                if (gamepad) {
                    break;
                }
            }
        }
        SDL_free(joysticks);
    }
    
    if (!gamepad) {
        // No gamepad found - simulation will continue with zero input
    }
    
    
    // Create engine and clutch
    ev_sim::Engine engine(800.0f, 7000.0f, 0.1f, 200.0f, 0.25f);
    ev_sim::Clutch clutch(10.0f);  // 10 Hz stiffness
    
    // Simulation parameters
    const float dt = 0.1f;  // 100ms timestep for physics consistency
    // Initialize transmission RPM (starts from rest)
    float transmission_rpm = 0.0f;
    
    // Timing variables
    auto last_physics_time = std::chrono::steady_clock::now();
    
    // Input variables
    float throttle_percent = 0.0f;
    float clutch_pedal_percent = 100.0f;  // Start with clutch fully pressed (disengaged)
    
    // History for graphing (circular buffers)
    const int history_size = 100;  // 10 seconds at 0.1s timestep
    std::vector<float> engine_rpm_history(history_size, 800.0f);  // Initialize with idle RPM
    std::vector<float> trans_rpm_history(history_size, 0.0f);
    std::vector<float> throttle_history(history_size, 0.0f);  // Throttle input history
    std::vector<float> clutch_pedal_history(history_size, 100.0f);  // Clutch pedal history (start disengaged)
    std::vector<float> time_history(history_size, 0.0f);
    int history_index = 0;
    
    
    bool running = true;
    float simulation_time = 0.0f;
    
    // Main loop
    while (running) {
        auto current_time = std::chrono::steady_clock::now();
        
        // Process SDL3 events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                    
                case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                    if (event.gbutton.button == SDL_GAMEPAD_BUTTON_START) {
                        running = false;
                    }
                    break;
                    
                case SDL_EVENT_GAMEPAD_ADDED:
                    if (!gamepad) {
                        gamepad = SDL_OpenGamepad(event.gdevice.which);
                        if (gamepad) {
                            // Gamepad connected
                        }
                    }
                    break;
                    
                case SDL_EVENT_GAMEPAD_REMOVED:
                    if (gamepad) {
                        SDL_CloseGamepad(gamepad);
                        gamepad = nullptr;
                    }
                    break;
            }
        }
        
        // Read controller input if available
        if (gamepad) {
            // Right trigger (R2) for throttle
            int16_t right_trigger = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
            throttle_percent = axisToPercent(right_trigger);
            
            // Left trigger (L2) for clutch pedal
            int16_t left_trigger = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
            clutch_pedal_percent = axisToPercent(left_trigger);
        } else {
            // No controller - use default values
            throttle_percent = 0.0f;
            clutch_pedal_percent = 100.0f;  // Clutch disengaged
        }
        
        // Update physics at fixed timestep
        auto physics_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_physics_time);
        if (physics_elapsed.count() >= static_cast<long>(dt * 1000)) {
            // Convert clutch pedal position to engagement level
            // Clutch pedal: 100 = fully pressed (disengaged), 0 = released (engaged)
            float clutch_engagement = 1.0f - (clutch_pedal_percent / 100.0f);
            clutch_engagement = clamp(clutch_engagement, 0.0f, 1.0f);
            
            // Simulate drivetrain load (gradual application based on engagement)
            float base_load = 15.0f;  // Base drivetrain losses
            float speed_load = 0.01f * transmission_rpm;  // Speed-dependent load
            
            // Apply load gradually - use smooth engagement curve instead of hard threshold
            float engagement_factor = std::max(0.0f, (clutch_engagement - 0.2f) / 0.8f); // Start at 20% engagement, full at 100%
            engagement_factor = engagement_factor * engagement_factor; // Square for smoother curve
            
            // When clutch is engaged, there's actually LESS resistance due to transmission "support"
            // The main effect should come from higher inertia, not higher load
            float engine_rpm = engine.getRPM();
            float rpm_ratio = engine_rpm / 7000.0f; // Normalize to max RPM
            
            // Aggressive engine braking when disengaged to make RPM fall quickly
            float disengaged_extra_braking = (1.0f - clutch_engagement) * 20.0f * rpm_ratio; // Strong braking when disengaged
            
            // Minimal load when engaged, extra braking when disengaged
            float base_resistance = engagement_factor * (base_load + speed_load) * 0.3f;
            
            float load_torque = base_resistance + disengaged_extra_braking;
            
            // Update engine with clutch engagement for variable inertia
            engine.update(throttle_percent, load_torque, clutch_engagement, dt);
            
            // Get updated engine RPM after update
            engine_rpm = engine.getRPM();
            
            // Update clutch (modifies RPMs by reference)
            clutch.update(engine_rpm, transmission_rpm, clutch_engagement, dt);
            
            // Feed clutch-modified engine RPM back to engine object
            // This ensures clutch synchronization affects the engine's internal state
            if (clutch_engagement > 0.1f) { // Only when clutch is significantly engaged
                engine.setRPM(engine_rpm);
            }
            
            // Update history for graphing
            engine_rpm_history[history_index] = engine_rpm;
            trans_rpm_history[history_index] = transmission_rpm;
            throttle_history[history_index] = throttle_percent;
            clutch_pedal_history[history_index] = clutch_pedal_percent;
            time_history[history_index] = simulation_time;
            history_index = (history_index + 1) % history_size;
            
            simulation_time += dt;
            last_physics_time = current_time;
        }
        
        // Console output removed - using ImGui dashboard for visualization
        
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        // Get current values for dashboard
        float engine_rpm = engine.getRPM();
        float clutch_engagement = 1.0f - (clutch_pedal_percent / 100.0f);
        
        // Get engine torque for display
        float engine_torque = engine.getTorque();
        
        // Create main dashboard window
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 580), ImGuiCond_FirstUseEver); // Increased height for new UI elements
        
        if (ImGui::Begin("Manual EV Shift Simulator", nullptr, ImGuiWindowFlags_NoResize)) {
            
            // === HEADER SECTION ===
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font, but we'll make it bold with styling
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "MANUAL EV SHIFT SIMULATOR");
            ImGui::PopFont();
            
            ImGui::Separator();
            ImGui::Spacing();
            
            // === RPM GAUGES SECTION ===
            ImGui::Text("RPM INDICATORS");
            
            // Create a child window for gauges with more height and custom positioning
            ImGui::BeginChild("GaugeArea", ImVec2(0, 140), true, ImGuiWindowFlags_NoScrollbar);
            {
                // Move cursor up to position gauges higher in the child window
                ImGui::SetCursorPosY(5.0f); // Start gauges near the top
                
                // Calculate center positioning for gauges
                float gauge_width = 120.0f; // Slightly smaller for better fit
                float total_width = gauge_width * 2 + 50.0f; // Two gauges + spacing
                float start_x = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;
                
                ImGui::SetCursorPosX(start_x);
                drawRPMGauge("ENGINE", engine_rpm, 7000.0f, ImVec4(1.0f, 0.3f, 0.3f, 1.0f), gauge_width);
                
                ImGui::SameLine();
                ImGui::SetCursorPosX(start_x + gauge_width + 50.0f);
                drawRPMGauge("TRANSMISSION", transmission_rpm, 7000.0f, ImVec4(0.3f, 0.7f, 1.0f, 1.0f), gauge_width);
            }
            ImGui::EndChild();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // === CONTROL INPUTS SECTION ===
            ImGui::Text("CONTROLLER INPUTS");
            ImGui::Spacing();
            
            // Two column layout for inputs
            ImGui::Columns(2, "InputColumns", false);
            ImGui::SetColumnWidth(0, 280.0f);
            
            // Left column - Input bars
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, 1.0f), "THROTTLE (R2)");
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.9f, 0.2f, 1.0f));
            ImGui::ProgressBar(throttle_percent / 100.0f, ImVec2(-1, 25), "");
            ImGui::SameLine(); ImGui::Text("%.1f%%", throttle_percent);
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "CLUTCH PEDAL (L2)");
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
            ImGui::ProgressBar(clutch_pedal_percent / 100.0f, ImVec2(-1, 25), "");
            ImGui::SameLine(); ImGui::Text("%.1f%%", clutch_pedal_percent);
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "CLUTCH ENGAGEMENT");
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
            ImGui::ProgressBar(clutch_engagement, ImVec2(-1, 25), "");
            ImGui::SameLine(); ImGui::Text("%.2f", clutch_engagement);
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            // Gear Position Display
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "GEAR POSITION");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::Button("1st", ImVec2(50, 30));
            ImGui::PopStyleColor(3);
            ImGui::SameLine(); ImGui::Text("First Gear");
            ImGui::Spacing();
            
            // Engine Torque Display
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "ENGINE TORQUE");
            ImGui::Text("%.1f Nm", engine_torque);
            
            ImGui::NextColumn();
            
            // Right column - Status and controls
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "STATUS");
            ImGui::Spacing();
            
            if (gamepad) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "🎮 CONNECTED");
                ImGui::TextWrapped("%s", SDL_GetGamepadName(gamepad));
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.0f, 1.0f), "🎮 DISCONNECTED");
                ImGui::Text("Connect PS5 Controller");
            }
            
            ImGui::Spacing();
            ImGui::Text("Time: %.1fs", simulation_time);
            ImGui::Spacing();
            
            // Exit button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            if (ImGui::Button("EXIT SIMULATION", ImVec2(-1, 35))) {
                running = false;
            }
            ImGui::PopStyleColor(2);
            
            ImGui::Columns(1);
        }
        ImGui::End();
        
        // === RPM GRAPH WINDOW ===
        ImGui::SetNextWindowPos(ImVec2(640, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 580), ImGuiCond_Always); // Force resize to match main dashboard window
        
        if (ImGui::Begin("Engine & Transmission RPM Over Time", nullptr, ImGuiWindowFlags_NoResize)) {
            
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "RPM HISTORY - LAST 10 SECONDS");
            ImGui::Separator();
            ImGui::Spacing();
            
            // Create ordered arrays for plotting (ImGui expects chronological order)
            std::vector<float> engine_plot_data(history_size);
            std::vector<float> trans_plot_data(history_size);
            std::vector<float> throttle_plot_data(history_size);
            std::vector<float> clutch_plot_data(history_size);
            
            for (int i = 0; i < history_size; i++) {
                int idx = (history_index + i) % history_size;
                engine_plot_data[i] = engine_rpm_history[idx];
                trans_plot_data[i] = trans_rpm_history[idx];
                throttle_plot_data[i] = throttle_history[idx];
                clutch_plot_data[i] = clutch_pedal_history[idx];
            }
            
            // Plot Engine RPM
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "ENGINE RPM");
            ImGui::PlotLines("##EngineRPM", engine_plot_data.data(), history_size, 0, nullptr, 0.0f, 7000.0f, ImVec2(-1, 80));
            
            ImGui::Spacing();
            
            // Plot Transmission RPM  
            ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "TRANSMISSION RPM");
            ImGui::PlotLines("##TransRPM", trans_plot_data.data(), history_size, 0, nullptr, 0.0f, 7000.0f, ImVec2(-1, 80));
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // === INPUT HISTORY SECTION ===
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "THROTTLE & CLUTCH INPUT HISTORY");
            ImGui::Spacing();
            
            // Combined throttle and clutch graph
            if (ImGui::BeginChild("InputGraphChild", ImVec2(0, 100), true)) {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
                ImVec2 canvas_size = ImGui::GetContentRegionAvail();
                
                // Draw background
                draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), 
                                       IM_COL32(20, 20, 20, 255));
                
                // Draw grid lines (horizontal)
                for (int i = 0; i <= 4; i++) {
                    float y = canvas_pos.y + (canvas_size.y * i / 4.0f);
                    draw_list->AddLine(ImVec2(canvas_pos.x, y), ImVec2(canvas_pos.x + canvas_size.x, y), 
                                     IM_COL32(60, 60, 60, 255));
                }
                
                // Draw throttle line (green)
                for (int i = 0; i < history_size - 1; i++) {
                    float x1 = canvas_pos.x + (canvas_size.x * i / (history_size - 1));
                    float x2 = canvas_pos.x + (canvas_size.x * (i + 1) / (history_size - 1));
                    float y1 = canvas_pos.y + canvas_size.y * (1.0f - throttle_plot_data[i] / 100.0f);
                    float y2 = canvas_pos.y + canvas_size.y * (1.0f - throttle_plot_data[i + 1] / 100.0f);
                    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(0, 255, 100, 255), 2.0f);
                }
                
                // Draw clutch line (orange)
                for (int i = 0; i < history_size - 1; i++) {
                    float x1 = canvas_pos.x + (canvas_size.x * i / (history_size - 1));
                    float x2 = canvas_pos.x + (canvas_size.x * (i + 1) / (history_size - 1));
                    float y1 = canvas_pos.y + canvas_size.y * (1.0f - clutch_plot_data[i] / 100.0f);
                    float y2 = canvas_pos.y + canvas_size.y * (1.0f - clutch_plot_data[i + 1] / 100.0f);
                    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(255, 150, 0, 255), 2.0f);
                }
                
                // Draw scale labels
                draw_list->AddText(ImVec2(canvas_pos.x + 5, canvas_pos.y + 2), IM_COL32(200, 200, 200, 255), "100%");
                draw_list->AddText(ImVec2(canvas_pos.x + 5, canvas_pos.y + canvas_size.y - 15), IM_COL32(200, 200, 200, 255), "0%");
            }
            ImGui::EndChild();
            
            // Legend
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "● THROTTLE");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "● CLUTCH PEDAL");
            
            ImGui::Spacing();
            ImGui::Separator();
            
            // Current values
            ImGui::Text("Current Engine RPM: %.0f", engine_rpm);
            ImGui::Text("Current Transmission RPM: %.0f", transmission_rpm);
            ImGui::Text("Current Throttle: %.1f%%", throttle_percent);
            ImGui::Text("Current Clutch Pedal: %.1f%%", clutch_pedal_percent);
            ImGui::Text("Time: %.1fs", simulation_time);
        }
        ImGui::End();
        
        // Rendering
        ImGui::Render();
        
        int display_w, display_h;
        SDL_GetWindowSizeInPixels(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        SDL_GL_SwapWindow(window);
        
        // Small sleep to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    
    // Cleanup SDL3
    if (gamepad) {
        SDL_CloseGamepad(gamepad);
    }
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}