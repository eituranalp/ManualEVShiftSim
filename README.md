# Manual EV Clutch Simulation

This project simulates a simplified manual clutch system for an electric vehicle. The goal is to model realistic interactions between engine RPM, clutch engagement, and transmission behavior during different phases of clutch use.

---

## What’s Working

- Engine model with throttle input, torque curve, and internal drag
- Simplified clutch model with three states:
  - Disengaged: engine and transmission act independently
  - Engaged: RPMs lock together
  - Partially engaged: engine and trans RPMs converge gradually
- Transmission RPM decay when disconnected from the engine (coasting behavior)
- Predefined test sequence simulating idle, revving, engagement, slip, and release
- CSV output for plotting and analysis

---

## What's Planned

This is not a complete simulation. The following are not yet implemented:

- Vehicle speed and wheel dynamics
- Road load (aero drag, slope, rolling resistance)
- Gear shifting or gearbox logic
- Live user input (everything is scripted)

This project is in the clutch modeling phase.

---

## How to Use

1. Build the C++ simulation
2. Run the program — it saves results to `output/clutch_engine_test_results.csv`
3. Use a plotting script (e.g. in Python) to visualize RPMs, inputs, and system response

---
