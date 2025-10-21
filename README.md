# mimik - Mini Linux Shell for ESP32-CAM

This repository contains the source code, technical documentation, and auxiliary files for **mimik**, a system that tries to mimic a Linux-like shell designed for ESP32-CAM microcontrollers. The project was developed to practice Linux command-line operations and explore interactive ways to interface with IoT devices, making them behave more like traditional Linux systems or networking equipment.

## 📁 Repository Structure

```
mimik/
├── src/                          # Main source code
│   ├── mimik.ino                # Main Arduino sketch
│   ├── shell.h                  # Shell core definitions
│   ├── shell.cpp                # Shell core implementation
│   ├── shellCommands.cpp        # File system commands
│   ├── shellTasks.cpp           # FreeRTOS task management
│   ├── monitorCommands.cpp      # System monitoring commands
│   ├── networkCommands.cpp      # Networking commands
│   ├── networkConfig.h          # Network configuration manager
│   ├── networkConfig.cpp        # Network persistence implementation
│   ├── sshServer.h              # Telnet server definitions
│   └── sshServer.cpp            # Telnet server implementation
├── docs/                        # Auxiliary files (if any)
├── README.md                    # This file
└── .gitignore                   # Git exclusions
```

## 🔧 Requirements

- **ESP32-CAM** board (AI-Thinker or compatible)
- **microSD card** (for persistent storage)
- Arduino IDE 1.8.19 or later / Arduino IDE 2.x
- Board: "AI Thinker ESP32-CAM" or "ESP32 Dev Module"
- Libraries:
  - `SD_MMC.h`
  - `WiFi.h`
  - `ESPmDNS.h`
  - `ESPping.h`
  - FreeRTOS (included with ESP32 core)

## ⚙️ System Description

**mimik** is a lightweight command-line shell running on ESP32-CAM that tries to mimic Linux shell behavior, providing:

- **File System Management**: Navigate, create, delete, copy, and edit files on the SD card
- **Network Operations**: WiFi scanning, connection management, ping, and network configuration
- **System Monitoring**: Real-time resource usage (CPU, RAM, PSRAM, Flash, SD card)
- **Remote Access**: Telnet server for remote shell access over the network
- **Persistent Configuration**: Network settings saved to SD card with auto-connect on boot

### Command Categories

#### File System Commands
- `ls` - List directory contents
- `cd` - Change directory
- `pwd` - Print working directory
- `mkdir` - Create directory
- `touch` - Create empty file
- `rm` - Remove files/directories
- `mv` - Move/rename files
- `cp` - Copy files
- `cat` - Display file contents
- `nano` - Simple text editor

#### Network Commands
- `ifconfig` - Display network interface information
- `wifiscan` - Scan for available WiFi networks
- `wificonnect` - Connect to a WiFi network
- `wifidisconnect` - Disconnect from WiFi
- `ping` - Send ICMP ping to a host
- `ipset` - Configure static IP address
- `netconfig` - Show saved network configuration
- `netclear` - Clear saved network configuration

#### System Commands
- `top` - Display system resource usage
- `help` - Show available commands

## 🧠 Architecture

The system is composed of:

- **Hardware**: ESP32-CAM module with microSD card slot
- **Storage**: SD card (MMC 1-bit mode) for persistent file system
- **Networking**: WiFi STA mode with Telnet server for remote access
- **RTOS**: FreeRTOS task management for concurrent operations
- **Software**: Modular C++ architecture using Arduino framework

### Key Features

- **Multi-output System**: Commands can output to both Serial and Telnet clients
- **Persistent Configuration**: WiFi credentials and network settings stored on SD card
- **Auto-connect**: Automatic WiFi connection on boot using saved configuration
- **Resource Monitoring**: Real-time tracking of memory, storage, and system resources
- **Remote Shell**: Full command-line access via Telnet (port 23)

## 🧪 Usage

### Initial Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/mimik.git
   ```

2. Insert a formatted microSD card into the ESP32-CAM

3. Open `mimik.ino` in Arduino IDE

4. Select the correct board and port:
   - Board: "AI Thinker ESP32-CAM" or "ESP32 Dev Module"
   - Upload Speed: 115200

5. Compile and upload to ESP32-CAM

### Local Access (Serial)

1. Open Serial Monitor at 115200 baud
2. Type commands at the `mimik:/$` prompt
3. Type `help` to see available commands

### Remote Access (Telnet)

1. Connect ESP32-CAM to WiFi:
   ```
   mimik:/$ wifiscan
   mimik:/$ wificonnect "YourSSID" "YourPassword"
   ```

2. Note the IP address displayed

3. From another device on the same network:
   ```bash
   telnet <ESP32-CAM-IP-ADDRESS>
   ```

### Example Session

```
mimik:/$ wifiscan
Scanning WiFi networks...

=== Available WiFi Networks ===

SSID                             | RSSI | Channel | Encryption
----------------------------------------------------------------
MyHomeWiFi                       |  -45 |       6 | WPA2-PSK
OfficeNetwork                    |  -67 |      11 | WPA2-PSK

mimik:/$ wificonnect "MyHomeWiFi" "mypassword"
Connecting to 'MyHomeWiFi'...
Connected successfully!
IP Address: 192.168.1.100
WiFi credentials saved to SD card

mimik:/$ top

=== System Resources Monitor ===

CPU:
-------------------------------
  Cores:      2
  Frequency:  240 MHz
  Uptime:     00:05:32

Memory (RAM):
-------------------------------
  Total:      520 KB
  Used:       156 KB
  Free:       364 KB
  Usage:      30%
  Bar:        [######--------------]

mimik:/$ mkdir projects
Directory created

mimik:/$ cd projects
mimik:/projects$ touch readme.txt
File created
```

## 📝 Configuration Files

### Network Configuration
Location: `/networkConfig.cfg` (on SD card)

Format:
```
# mimik Network Configuration
SSID=YourWiFiName
PASSWORD=YourPassword
USE_STATIC_IP=false
STATIC_IP=192.168.1.100
NETMASK=255.255.255.0
GATEWAY=192.168.1.1
```

This file is automatically created when using `wificonnect` or `ipset` commands.

## 🔌 Pin Configuration

ESP32-CAM SD Card (MMC 1-bit mode):
- CMD: GPIO 15
- CLK: GPIO 14
- D0: GPIO 2

## 🛠️ Development

### Adding New Commands

1. Declare the command function in `shell.h`:
   ```cpp
   static ShellError cmd_mycommand(CommandArgs args);
   ```

2. Implement the command in an appropriate `.cpp` file:
   ```cpp
   ShellError MiniShell::cmd_mycommand(CommandArgs args) {
       ShellOutput::println("My command output");
       return SHELL_OK;
   }
   ```

3. Register the command in `shell.cpp` `init()`:
   ```cpp
   registerCommand("mycommand", "Description", cmd_mycommand, minArgs, maxArgs);
   ```

### Output Abstraction

Use `ShellOutput` class for all output to support both Serial and Telnet:

```cpp
ShellOutput::print("Text");      // Print without newline
ShellOutput::println("Text");    // Print with newline
ShellOutput::printf("Value: %d\n", value);  // Formatted output
```

## 📚 Credits

Developed by Danilo Arregui as a personal project to explore Linux-like interfaces on embedded IoT devices - 2025.

## 📄 License

This project is open source and available for educational and personal use, under GPL-2.0 license.

---

**Note**: This is an educational project that tries to mimic Linux shell behavior on embedded systems. For production IoT deployments, consider additional security measures such as authentication, encryption, and access control.
