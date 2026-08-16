<div align="center">
  <b>Personal Portfolio:</b>
  <a href="https://github.com/bhung-chung/Bangla-Fake-News-Detection">Bangla Fake News Detection</a> |
  <a href="https://github.com/bhung-chung/Drone-Object-Detection">Drone Object Detection</a> |
  <a href="https://github.com/bhung-chung/Heart-Disease-Prediction">Heart Disease Prediction</a> |
  <a href="https://github.com/bhung-chung/Wooden-Planetary-Gear">Planetary Gear System</a> |
  <a href="https://github.com/bhung-chung/Autonomous-Obstacle-Avoiding-Robot">Obstacle Avoiding Robot</a>
</div><br>

# Autonomous Obstacle Avoiding Mobile Robot

[![Platform: Arduino](https://img.shields.io/badge/Platform-Arduino%20Uno%20R3-00979C?style=flat&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language: C++](https://img.shields.io/badge/Language-Embedded%20C%2B%2B-00599C?style=flat&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Course: ME-376](https://img.shields.io/badge/Course-Control%20Engineering%20Sessional-blue)](https://mist.ac.bd/)
[![Grade: A+](https://img.shields.io/badge/Grade-A%2B%20(GPA%205.00)-brightgreen)](https://mist.ac.bd/)

An autonomous, differential-drive mobile robotics platform engineered to perform real-time environmental sensing, obstacle avoidance, and dynamic path correction using optical infrared telemetry and Pulse-Width Modulation (PWM) velocity control.

Developed as a capstone laboratory project for **ME-376: Control Engineering Sessional** (1.50 Credit, Grade: **A+ / 5.00**) at the **Department of Mechanical Engineering, Military Institute of Science and Technology (MIST)**.

---

## 📸 Physical Prototype Showcase

<div align="center">
  <img src="media/top view.jpg" width="48%" alt="Robot Top View" />
  <img src="media/isometric view.jpg" width="48%" alt="Robot Isometric View" />
</div>

---

## 📌 Project Overview & Engineering Objectives

The objective was to design, fabricate, and program an autonomous wheeled mobile robot (WMR) capable of collision-free navigation in unstructured environments using low-cost embedded hardware. The design integrates electromechanical propulsion, closed-loop state logic, and power conditioning to achieve robust real-time autonomy.

### Key Engineering Features
* **Real-Time Obstacle Sensing:** Dual optical Infrared (IR) reflective sensors continuously scan the navigation corridor for immediate threat detection.
* **Differential Drive Skid Steering:** Dynamic PWM modulation enables zero-radius turning, smooth trajectory adjustments, and active deceleration.
* **Dual-Rail Power Architecture:** Step-down voltage regulation via an **LM2596 DC-DC Buck Converter** isolates logic electronics (5V) from high-current inductive motor transients (12V).
* **Fault-Tolerant Deadlock Recovery:** Multi-sensor conflict resolution with an automated reverse-pivot maneuver to escape tight corners and frontal dead ends.

---

## 🛠️ Hardware Architecture & Bill of Materials (BOM)

| Component | Specification / Model | Function in System | Reference Photo |
| :--- | :--- | :--- | :---: |
| **Microcontroller** | Arduino Uno R3 (ATmega328P @ 16 MHz) | Central control unit, state machine execution & PWM generation | [View](media/components/arduino_uno.jpg) |
| **Motor Driver** | L298N Dual H-Bridge Driver Module | Bi-directional high-current DC motor actuation | [View](media/components/l298n_driver.jpg) |
| **Sensors** | 2× Optical IR Obstacle Avoidance Modules | Proximity detection (active-low digital threshold output) | [View](media/components/ir_sensor.jpg) |
| **Power Converter** | LM2596 DC-DC Step-Down Buck Module | High-efficiency voltage step-down (12V $\rightarrow$ 5V logic rail) | [View](media/components/lm2596_buck.jpg) |
| **Actuators** | 2× TT DC Dual-Shaft Gearbox Motors + Wheels | Differential skid-steer locomotion & torque delivery | [View](media/components/tt_gear_motor.jpg) |
| **Power Source** | 12V Rechargeable Li-ion / Multi-cell Battery Pack | Primary DC bus for motors and onboard electronics | [View](media/components/battery_pack.jpg) |
| **Chassis** | Laser-cut acrylic dual-plate robot chassis | Structural mounting & caster wheel balance | — |

---

## 🔌 Circuit Interfacing & Pinout Mapping

```text
               +---------------------------------------------------+
               |              12V Main Battery Pack                |
               +-------------------------+-------------------------+
                                         |
                       +-----------------+-----------------+
                       | 12V DC                            | 12V DC
                       v                                   v
             +-------------------+               +-------------------+
             | LM2596 Buck Module|               |  L298N VMS (12V)  |
             +---------+---------+               +---------+---------+
                       | 5V Regulated (Logic)              | Motor Drive
                       v                                   v
       +---------------+---------------+         +---------+---------+
       |       Arduino Uno R3          |         | 2x TT Gear Motors |
       |                               |         +-------------------+
       |  [D5]  PWM ENA -------------->| (L298N Left Speed)
       |  [D8]  IN1 ------------------>| (Left Forward)
       |  [D7]  IN2 ------------------>| (Left Reverse)
       |  [D4]  IN3 ------------------>| (Right Forward)
       |  [D2]  IN4 ------------------>| (Right Reverse)
       |  [D6]  PWM ENB -------------->| (L298N Right Speed)
       |                               |
       |  [D11] <--- Left IR Sensor    |
       |  [D12] <--- Right IR Sensor   |
       +-------------------------------+
```

### Complete Pin Connections

| Arduino Uno Pin | Connected Module | Function Description |
| :--- | :--- | :--- |
| **D5 (PWM)** | L298N `ENA` | Left Motor Speed Control via PWM |
| **D8** | L298N `IN1` | Left Motor Forward Logic |
| **D7** | L298N `IN2` | Left Motor Reverse Logic |
| **D4** | L298N `IN3` | Right Motor Forward Logic |
| **D2** | L298N `IN4` | Right Motor Reverse Logic |
| **D6 (PWM)** | L298N `ENB` | Right Motor Speed Control via PWM |
| **D11** | Left IR Sensor `OUT` | Left Obstacle Signal (`LOW` = Obstacle detected) |
| **D12** | Right IR Sensor `OUT` | Right Obstacle Signal (`LOW` = Obstacle detected) |
| **5V / GND** | LM2596 / Sensors | System Common Ground and 5V Regulated Bus |

---

## ⚙️ Control Logic & Finite State Machine (FSM)

The control algorithm operates as a non-blocking discrete finite state machine running inside the microcontroller's main `loop()` routine at ~50 Hz update rates:

```
                      +-------------------+
                      |   STATE_FORWARD   | <----+ (Path Clear)
                      | Cruise: 175 PWM   |      |
                      +---------+---------+      |
                                |                |
             +------------------+------------------+
             |                                     |
   (Obstacle Left)                       (Obstacle Right)
             v                                     v
+------------------------+            +------------------------+
|    STATE_TURN_RIGHT    |            |    STATE_TURN_LEFT     |
| Pivot Right (300ms)    |            | Pivot Left (300ms)     |
+------------+-----------+            +------------+-----------+
             |                                     |
             +------------------+------------------+
                                |
                    (Frontal Wall / Both Active)
                                v
                   +-------------------------+
                   |  STATE_AVOID_DEADLOCK   |
                   |  1. Reverse (350ms)     |
                   |  2. Pivot 90° (450ms)   |
                   +-------------------------+
```

### Decision Truth Table

| Left IR State | Right IR State | Detected Condition | Control Action | Differential Drive State |
| :---: | :---: | :--- | :--- | :--- |
| `HIGH` (Clear) | `HIGH` (Clear) | No obstacles in path | **Drive Forward** | Left: Forward (175), Right: Forward (175) |
| `LOW` (Blocked) | `HIGH` (Clear) | Obstacle on Left | **Pivot Right** | Left: Forward (190), Right: Reverse (190) |
| `HIGH` (Clear) | `LOW` (Blocked) | Obstacle on Right | **Pivot Left** | Left: Reverse (190), Right: Forward (190) |
| `LOW` (Blocked) | `LOW` (Blocked) | Frontal / Corner trap | **Reverse & Turn** | Step 1: Full Reverse $\rightarrow$ Step 2: Extended Right Pivot |

---

## 🔬 Challenges Tackled & Engineering Solutions

1. **Optical Noise & IR Sensing Constraints:**
   * *Problem:* Ambient fluorescent lighting and variable surface reflectivity caused intermittent false triggers on optical IR receivers.
   * *Solution:* Implemented multi-turn comparator potentiometer calibration alongside a software state-settling buffer ($50\text{ ms}$) to debounce transient noise without sacrificing obstacle reaction latency.

2. **Inductive Transients & Voltage Sag:**
   * *Problem:* DC motor stall current surges caused brownout resets on the ATmega328P microcontroller when driven directly from a shared unregulated battery line.
   * *Solution:* Introduced an **LM2596 switching buck regulator** with bulk decoupling capacitors, isolating the 5V logic supply from inductive motor voltage spikes.

3. **Inertial Momentum in Differential Turns:**
   * *Problem:* Abrupt transitions between forward drive and pivoting induced mechanical chassis tipping and motor stall wear.
   * *Solution:* Implemented intermediate braking intervals (`stopMotors()`) with brief mechanical settling delays prior to executing skid-steer direction reversals.

---

## 🚀 Firmware Compilation & Setup

### Requirements
* **IDE:** Arduino IDE (v1.8.x or v2.x) / PlatformIO
* **Core:** Arduino AVR Boards Core

### Flashing Procedure
1. Clone this repository:
   ```bash
   git clone https://github.com/bhung-chung/Autonomous-Obstacle-Avoiding-Robot.git
   cd Autonomous-Obstacle-Avoiding-Robot
   ```
2. Open [`src/obstacle_avoiding_robot.ino`](src/obstacle_avoiding_robot.ino) in the Arduino IDE.
3. Select **Board:** `Arduino Uno` and choose your corresponding serial **COM Port**.
4. Click **Verify (Compile)**, then **Upload**.
5. Disconnect the USB cable, power the chassis via the 12V battery switch, and observe autonomous navigation!

---

## 📁 Repository Structure

```text
.
├── src/
│   └── obstacle_avoiding_robot.ino     # Embedded C++ control firmware
├── media/
│   ├── top view.jpg                    # High-resolution physical prototype top photo
│   ├── isometric view.jpg              # Prototype isometric perspective photo
│   └── components/                     # Reference hardware component photos
│       ├── arduino_uno.jpg
│       ├── l298n_driver.jpg
│       ├── ir_sensor.jpg
│       ├── lm2596_buck.jpg
│       ├── tt_gear_motor.jpg
│       └── battery_pack.jpg
├── docs/                               # Schematics and technical documentation
└── README.md                           # Comprehensive engineering documentation
```
