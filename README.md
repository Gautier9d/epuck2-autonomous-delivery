# 🤖 E-PUCK2 Prison Meal Delivery Robot

An autonomous meal delivery system for correctional facilities using the E-PUCK2 robot platform. This embedded systems project implements color-based navigation and proximity detection for safe and efficient meal distribution in prison corridors.

## 📋 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Architecture](#software-architecture)
- [Installation](#installation)
- [Usage](#usage)
- [Color Code System](#color-code-system)
- [Project Structure](#project-structure)
- [Technical Documentation](#technical-documentation)
- [Contributors](#contributors)
- [License](#license)

## 🎯 Overview

This project transforms an E-PUCK2 robot into an autonomous meal delivery system designed for prison environments. The robot navigates through corridors using color-coded floor markers, detects cells requiring service, delivers meals, and handles emergency situations—all while maintaining safety protocols.

### Key Capabilities
- Autonomous navigation through prison corridors
- Cell-by-cell meal delivery
- Emergency response mode
- Row changing for multi-corridor coverage
- Collision avoidance using proximity sensors

## ✨ Features

### Navigation System
- **Line Following**: Follows main corridor path
- **Color Detection**: Camera-based floor marker recognition
- **Proximity Detection**: IR sensors (IR4 & IR5) for wall detection
- **Rotation Control**: Precise 90° and 180° turns

### State Machine
The robot operates through 5 distinct states:
1. **GO_STRAIGHT** - Standard corridor navigation
2. **DELIVERY** - Meal delivery to cell
3. **RETURN** - Return to main path after delivery
4. **CHANGE_ROW** - Transition to next corridor
5. **EMERGENCY** - Safety protocol activation

### Safety Features
- Emergency stop on white light detection
- Wall collision prevention
- Automatic path correction
- State recovery after emergency

## 🔧 Hardware Requirements

### Base Platform
- E-PUCK2 robot
- Camera module
- IR proximity sensors (IR4, IR5)
- Motors with step counting

### Custom 3D-Printed Extensions
- **Mirror Holder Ring**: PLA-printed support for 45° mirror
- **Mounting Cone**: Secures mirror assembly to robot
- **Mirror**: Angled at π/4 for ground scanning

## 🏗️ Software Architecture

### Threading Model
The system uses ChibiOS RTOS with three concurrent threads:

| Thread | Priority | Stack Size | Function |
|--------|----------|------------|----------|
| ColorDetection | NormalPrio | 1024 bytes | Camera processing & color analysis |
| Movement | NormalPrio+1 | 512 bytes | State machine & motor control |
| ProxDetection | NormalPrio | 128 bytes | IR sensor monitoring |

### Communication
- **MessageBus**: Color detection → Movement (color topics)
- **Semaphore**: Proximity detection → Movement (wall detection)

### Memory Usage
- **Flash**: 8.80% (88,036 bytes / 1MB)
- **RAM**: 13.16% (25,264 bytes / 192KB)

## 📦 Installation

### Prerequisites
- ARM GCC Toolchain
- ChibiOS 16.x
- E-PUCK2 development environment
- Make build system

### Building the Project

1. Clone the repository:
```bash
git clone https://github.com/Gautier9d/epuck2-prison-delivery.git
cd epuck2-prison-delivery
```

2. Configure the hardware settings:
   - Adjust thresholds in `src/color_detection.h` based on lighting
   - Modify `PROXIMITY_THRESHOLD` in `src/prox_detection.c`

3. Build the project:
```bash
make clean
make
```

4. Flash to E-PUCK2:
```bash
make flash
```

## 🚀 Usage

### Initial Setup

1. **Install floor markers**: Place colored tape/LEDs according to layout
2. **Mount mirror assembly**: Attach 3D-printed extensions to robot
3. **Calibrate sensors**:
   - Adjust `BLACK_THRESHOLD` and `WHITE_THRESHOLD` in `src/color_detection.c`
   - Set `PROXIMITY_THRESHOLD` in `src/prox_detection.c`

### Operation

1. **Power on** the E-PUCK2
2. **Set selector to position 1** to start the program
3. Robot begins in `GO_STRAIGHT` state
4. Monitor operation via USB serial (115200 baud)

### Emergency Mode
- Activate by shining bright white light
- Robot moves to wall and stops
- Resume by restarting the robot

## 🎨 Color Code System

| Color | RGB Threshold | Action |
|-------|--------------|--------|
| 🔴 **Red** | R > G, R > B | Cell delivery required |
| 🔵 **Blue** | B > G+10 | Return path marker |
| 🟢 **Green** | G dominant | Change row |
| ⚪ **White** | All > 180 | Emergency stop |
| ⚫ **Black** | Sum < 200 | Navigation reference |

## 📁 Project Structure

```
epuck2-prison-delivery/
├── config/              # Configuration files
│   ├── chconf.h        # ChibiOS configuration
│   ├── halconf.h       # HAL configuration
│   └── mcuconf.h       # MCU-specific settings
├── src/                # Source code
│   ├── main.c          # Entry point & initialization
│   ├── main.h
│   ├── color_detection.c   # Camera processing & color analysis
│   ├── color_detection.h
│   ├── move.c          # State machine & motor control
│   ├── move.h
│   ├── prox_detection.c   # IR sensor handling
│   └── prox_detection.h
├── Makefile            # Build configuration
├── Report.pdf          # Detailed project report (French)
├── LICENSE             # MIT License
└── README.md          # This file
```

## 📚 Technical Documentation

### Image Processing
- Resolution: 640 pixels per line
- Format: RGB565
- Processing: Single line scanning (line 200)
- AWB: Disabled for consistent color detection

### Motor Control
- Steps per revolution: 1000
- Wheel perimeter: 13.0 cm
- Robot diameter: 5.35 cm
- 90° turn: ~209 steps
- 180° turn: ~418 steps
- Running speed: 500 steps/s

### Timing
- Color detection: 100ms intervals
- Proximity check: 50ms intervals
- Movement update: 50ms intervals

### Constants & Thresholds
```c
#define USED_LINE 200           // Camera scanning line
#define IMAGE_BUFFER_SIZE 640   // Pixels per line
#define BLACK_THRESHOLD 200     // Calibrate based on lighting
#define WHITE_THRESHOLD 180     // Calibrate based on lighting
#define PROXIMITY_THRESHOLD 120 // IR sensor threshold
#define SPEED_RUN 500          // Motor speed
```

## 🔬 Detailed Features

### Color Detection Algorithm
1. Capture RGB565 image from camera
2. Extract R, G, B components for each pixel
3. Calculate mean intensity for each color channel
4. Compare means to determine dominant color
5. Publish color to message bus

### Movement State Transitions
- `GO_STRAIGHT` → `DELIVERY` (on RED detection)
- `GO_STRAIGHT` → `CHANGE_ROW` (on GREEN detection)
- `GO_STRAIGHT` → `EMERGENCY` (on WHITE detection)
- `DELIVERY` → `RETURN` (after wall detection)
- `RETURN` → `GO_STRAIGHT` (on BLUE detection)
- `CHANGE_ROW` → `GO_STRAIGHT` (after row change)

## 🐛 Known Issues & Limitations

- Color thresholds require manual calibration for different lighting conditions
- Emergency mode requires manual restart
- Maximum operational distance depends on battery level
- Mirror alignment critical for proper floor scanning

## 🔮 Future Improvements

- [ ] Dynamic threshold calibration
- [ ] Battery level monitoring
- [ ] Multiple meal tray support
- [ ] RFID cell identification
- [ ] Remote monitoring interface
- [ ] Path optimization algorithms
- [ ] Automatic emergency recovery

## 👥 Contributors

- **COUYOUMTZELIS Romain Nicolas Paul** (340933)
- **DEMIERRE Gautier** (340423)

**Course**: Micro-315 - Embedded Systems and Robotics  
**Professor**: Francesco Mondada  
**Institution**: École Polytechnique Fédérale de Lausanne (EPFL)  
**Group**: 33, Section MT-BA6  
**Year**: 2024

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Prof. Francesco Mondada for course supervision
- EPFL Robotics Systems Laboratory (LSRO)
- E-PUCK2 development team

---

*This project was developed as part of the EPFL Micro-315 course on Embedded Systems and Robotics. It demonstrates practical applications of real-time operating systems, sensor integration, and autonomous navigation in robotics.*
