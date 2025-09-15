## Manual EV Shift Simulator

Real‑time manual driveline simulator for an electric vehicle in C++ with SDL3 gamepad input, OpenGL rendering, and an ImGui dashboard. Shows engine RPM, transmission RPM, throttle/clutch inputs, and history graphs with a fixed‑timestep physics loop.

### Features
- Engine model with throttle, torque curve, and internal drag
- Clutch engagement model with smooth synchronization behavior
- Transmission RPM inertia and decay when disconnected
- SDL3 gamepad input (PS5/compatible): R2 throttle, L2 clutch, Start to exit
- ImGui dashboard with gauges, input bars, and time‑series plots
- Deterministic fixed‑timestep physics (100 ms)

### Screenshots / Video
- Add a screenshot of the dashboard to `img/` and link here

### Build and Run

Prereqs:
- CMake 3.10+
- Ninja or your preferred generator
- A C++17 compiler
- SDL3 and OpenGL available (SDL3 via vcpkg recommended)

Windows (example with vcpkg and Ninja):
```bash
cmake -G Ninja -S . -B build \
  -DCMAKE_BUILD_TYPE=Release
ninja -C build
./build/ManualEVShiftSim.exe
```

Notes:
- `CMakeLists.txt` uses SDL3 CONFIG mode. If needed, install SDL3 with vcpkg and ensure the triplet is integrated. The project currently includes a direct include path; adjust to your environment as needed.
- The app is silent in the terminal; all feedback is via the ImGui window.

### Controls
- Right Trigger (R2): Throttle (0–100%)
- Left Trigger (L2): Clutch pedal (0–100%, 100 = fully pressed/disengaged)
- Start: Exit simulation

<!-- Documentation (Doxygen) section removed at user request -->

### Project Layout
- `include/` public headers (`engine.hpp`, `clutch.hpp`)
- `src/` implementation files
- `main.cpp` application entry with SDL3 + ImGui UI
- `imgui_backends/` vendored ImGui and backends for SDL3/OpenGL3
<!-- docs/ directory removed at user request -->

### Roadmap
- Vehicle speed and wheel dynamics
- Gearbox and gear shifting
- Road load (aero drag, grade, rolling resistance)
- Unit tests (GoogleTest) and CI

### License
MIT — see `LICENSE`.

### Packaging (ready-to-run ZIP)

Build and package a redistributable ZIP with the executable and required runtime files:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --prefix "package"
# Create ZIP (via CPack)
(cd build && cpack -G ZIP)
# The ZIP will be in build/ as ManualEVShiftSim-<version>.zip
```

The ZIP places `ManualEVShiftSim.exe`, `SDL3.dll` (if found), `LICENSE`, and `README.md` at the archive root for a straightforward double‑click run.

### Ready-to-run download

For convenience, prebuilt archives are located under `releases/`:
- `releases/ManualEVShiftSim-0.1.0-win64.zip` (MSVC build)
- `releases/ManualEVShiftSim-0.1.0-win64-mingw.zip` (MinGW build)

Unzip and double‑click `ManualEVShiftSim.exe`. If SmartScreen warns, click “More info” → “Run anyway,” or right‑click the EXE → Properties → “Unblock.”
