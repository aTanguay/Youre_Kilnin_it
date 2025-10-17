# Kilnin' It - Software Tools & Development Environment

This document lists all approved software tools, programs, and libraries used in the Kilnin' It ESP32 Kiln Controller project.

---

## Core Development Environment

### Required Tools

| Tool | Version | Purpose | Installation |
|------|---------|---------|--------------|
| **Python** | 3.7+ | PlatformIO dependency | [python.org](https://www.python.org/downloads/) |
| **PlatformIO Core** | Latest | ESP32 build system | `pip install platformio` |
| **Git** | 2.x+ | Version control | [git-scm.com](https://git-scm.com/) |

### Recommended Tools

| Tool | Purpose | Installation |
|------|---------|--------------|
| **Visual Studio Code** | Code editor with PlatformIO IDE extension | [code.visualstudio.com](https://code.visualstudio.com/) |
| **PlatformIO IDE Extension** | Visual interface for PlatformIO | Install from VSCode extensions |

### Alternative Code Editors (Optional)

- **Atom** with PlatformIO IDE plugin
- **Vim/Neovim** with appropriate plugins
- **Sublime Text** with PlatformIO plugin
- **Any text editor** (CLI-only workflow supported)

---

## Firmware Development

### Platform & Framework

| Component | Version | Purpose |
|-----------|---------|---------|
| **Espressif 32 Platform** | Latest | ESP32 toolchain and SDK |
| **Arduino Framework** | Latest | ESP32 Arduino core |
| **GCC ARM Embedded** | Included | C/C++ compiler for ESP32 |
| **esptool.py** | Included | ESP32 flash tool |

### Build Commands

```bash
pio run                    # Build firmware
pio run --target upload    # Upload to ESP32
pio run --target monitor   # Serial monitor
pio run --target uploadfs  # Upload SPIFFS filesystem
pio test                   # Run unit tests
```

---

## Required Libraries (Embedded)

### Communication Libraries

| Library | Version | Purpose | Repository |
|---------|---------|---------|------------|
| **AsyncTCP** | Latest | Asynchronous TCP for ESP32 | [me-no-dev/AsyncTCP](https://github.com/me-no-dev/AsyncTCP) |
| **ESPAsyncWebServer** | Latest | Non-blocking web server | [me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) |
| **ArduinoJson** | 6.x | JSON parsing/serialization | [bblanchon/ArduinoJson](https://arduinojson.org/) |

### Hardware Interface Libraries

| Library | Version | Purpose | Repository |
|---------|---------|---------|------------|
| **Adafruit MAX31855** | Latest | Thermocouple amplifier interface | [adafruit/Adafruit-MAX31855-library](https://github.com/adafruit/Adafruit-MAX31855-library) |
| **U8g2** | Latest | Universal graphics library (LCD) | [olikraus/u8g2](https://github.com/olikraus/u8g2) |
| **SPI** | Built-in | SPI hardware interface | Arduino core |
| **Wire** | Built-in | I2C/TWI interface | Arduino core |

### Control & Logic Libraries

| Library | Version | Purpose | Repository |
|---------|---------|---------|------------|
| **PID_v1** | Latest | PID controller with auto-tune | [br3ttb/Arduino-PID-Library](https://github.com/br3ttb/Arduino-PID-Library) |

### ESP32 Built-In Libraries

| Library | Purpose |
|---------|---------|
| **WiFi** | ESP32 WiFi stack (Station + AP modes) |
| **Preferences** | Non-volatile storage (NVS) wrapper |
| **SPIFFS** | SPI Flash File System |
| **LittleFS** | Alternative filesystem (more robust than SPIFFS) |
| **esp_task_wdt** | Watchdog timer for safety |
| **FreeRTOS** | Real-time operating system (built into ESP32 Arduino) |

### Optional Libraries

| Library | Purpose | Use Case |
|---------|---------|----------|
| **Encoder** | Rotary encoder handling with debouncing | Simplifies encoder code (optional) |
| **ESP32Time** | Software RTC for timestamps | Alternative to NTP time sync |
| **ESPmDNS** | Multicast DNS (discovery) | Access controller via `kiln.local` |

---

## Programming & Debugging Tools

### Serial Communication

| Tool | Purpose | Command/Access |
|------|---------|----------------|
| **PlatformIO Serial Monitor** | Serial debugging (recommended) | `pio device monitor` |
| **Arduino Serial Monitor** | Alternative serial monitor | Arduino IDE |
| **screen** | Unix terminal emulator | `screen /dev/ttyUSB0 115200` |
| **minicom** | Linux serial terminal | `minicom -D /dev/ttyUSB0 -b 115200` |
| **PuTTY** | Windows serial terminal | GUI application |

### USB Drivers

| Driver | Chip | Operating System | Download |
|--------|------|------------------|----------|
| **CP210x** | Silicon Labs | Windows/Mac/Linux | [silabs.com](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |
| **CH340** | WCH | Windows/Mac/Linux | [wch-ic.com](https://www.wch-ic.com/downloads/CH341SER_ZIP.html) |

**Note**: Most modern operating systems include these drivers. Check your ESP32 board's USB chip.

### Flashing & Upload Tools

| Tool | Purpose | Included With |
|------|---------|---------------|
| **esptool.py** | ESP32 flash read/write | PlatformIO |
| **ESP32 Flash Download Tool** | Official Espressif GUI flasher | [espressif.com](https://www.espressif.com/en/support/download/other-tools) |

---

## Web Development

### Frontend Technologies

| Technology | Purpose | Type |
|------------|---------|------|
| **HTML5** | Web interface structure | Markup |
| **CSS3** | Styling and responsive design | Stylesheet |
| **Vanilla JavaScript** | Client-side logic | Programming language |
| **WebSocket API** | Real-time data streaming | Browser API |
| **Fetch API** | REST API communication | Browser API |

### JavaScript Libraries (Client-Side)

| Library | Version | Purpose | CDN/Local |
|---------|---------|---------|-----------|
| **Chart.js** | 3.x+ | Temperature/cost graphing | CDN or local |

**Note**: Keep web UI lightweight (< 500KB total) - ESP32 serves from SPIFFS

### Development Tools

| Tool | Purpose |
|------|---------|
| **Any text editor** | HTML/CSS/JS editing |
| **Chrome DevTools** | Web interface debugging |
| **Firefox Developer Tools** | Alternative debugging |
| **Safari Web Inspector** | macOS debugging |
| **Lighthouse** | Performance auditing (optional) |

### Supported Browsers

- **Chrome/Chromium** 90+
- **Firefox** 88+
- **Safari** 14+
- **Edge** 90+
- **Mobile browsers** (iOS Safari, Chrome Mobile)

---

## Testing & Validation

### Unit Testing

| Tool | Purpose | Command |
|------|---------|---------|
| **PlatformIO Unit Testing** | Firmware module testing | `pio test` |
| **Unity Test Framework** | C/C++ test framework | Included with PlatformIO |

### Integration Testing

| Tool | Purpose |
|------|---------|
| **Hardware setup** | Physical component testing |
| **Multimeter** | Voltage/continuity verification |
| **Logic analyzer** | SPI bus debugging (optional) |
| **Oscilloscope** | Signal timing analysis (optional) |

### Web Testing

| Tool | Purpose |
|------|---------|
| **Browser DevTools** | JavaScript debugging, network analysis |
| **Postman** | REST API testing (optional) |
| **cURL** | Command-line API testing |
| **WebSocket King** | WebSocket connection testing (optional) |

### Network Tools (Optional)

| Tool | Purpose |
|------|---------|
| **Wireshark** | Packet capture and analysis |
| **nmap** | Network scanning and discovery |
| **ping** | Connectivity testing |
| **netcat (nc)** | Raw TCP/UDP testing |

---

## Schematic & PCB Design Tools (Optional)

### Breadboard & Schematic Design

| Tool | License | Platform | Purpose |
|------|---------|----------|---------|
| **Fritzing** | Open-source (paid for executable) | Windows/Mac/Linux | Breadboard diagrams, simple schematics |
| **EasyEDA** | Free (online) | Web-based | Schematic capture, PCB layout |
| **KiCad** | Open-source | Windows/Mac/Linux | Professional schematic and PCB design |

### Enclosure Design

| Tool | License | Platform | Purpose |
|------|---------|----------|---------|
| **Inkscape** | Open-source | Windows/Mac/Linux | 2D vector graphics (panel cutouts) |
| **FreeCAD** | Open-source | Windows/Mac/Linux | 3D CAD for enclosures |
| **Fusion 360** | Free (hobbyist) | Windows/Mac | 3D modeling and CAD |
| **Blender** | Open-source | Windows/Mac/Linux | 3D modeling (advanced) |

---

## Documentation Tools

### Markdown Editors

| Tool | Platform | Features |
|------|----------|----------|
| **Visual Studio Code** | All | Live preview, syntax highlighting |
| **Typora** | Windows/Mac/Linux | WYSIWYG markdown editor |
| **MacDown** | macOS | Live preview, GitHub-flavored |
| **MarkText** | All | Open-source, cross-platform |
| **Any text editor** | All | Basic markdown editing |

### Documentation Viewers

| Tool | Purpose |
|------|---------|
| **GitHub** | Online markdown rendering |
| **Web browser** | Local markdown preview (with extensions) |
| **grip** | Render GitHub-flavored markdown locally |

---

## Version Control & Collaboration

### Git Hosting

| Platform | Features | Access |
|----------|----------|--------|
| **GitHub** | Repository hosting, issues, discussions | [github.com](https://github.com) |
| **GitLab** | Alternative hosting, CI/CD | [gitlab.com](https://gitlab.com) |
| **Bitbucket** | Private repositories | [bitbucket.org](https://bitbucket.org) |

### Git Tools

| Tool | Purpose |
|------|---------|
| **Git CLI** | Command-line version control |
| **GitHub Desktop** | GUI for Git (beginner-friendly) |
| **GitKraken** | Visual Git client |
| **Sourcetree** | Free Git GUI client |

---

## File Transfer & Management

### Filesystem Upload

| Tool | Purpose | Command |
|------|---------|---------|
| **PlatformIO SPIFFS Uploader** | Upload web files to ESP32 | `pio run --target uploadfs` |
| **ESP32 Sketch Data Upload** | Arduino IDE alternative | Arduino IDE menu |

### File Management

| Tool | Purpose |
|------|---------|
| **Any file manager** | Organize project files |
| **Terminal/Command Prompt** | CLI file operations |

---

## Additional Utilities

### JSON Validation

| Tool | Purpose | Access |
|------|---------|--------|
| **JSONLint** | Validate firing profile JSON | [jsonlint.com](https://jsonlint.com) |
| **jq** | Command-line JSON processor | CLI tool |

### Terminal Multiplexers (Optional)

| Tool | Purpose |
|------|---------|
| **tmux** | Multiple terminal sessions |
| **screen** | Alternative terminal multiplexer |

### Text Processing

| Tool | Purpose |
|------|---------|
| **grep** | Search files for patterns |
| **sed** | Stream editor |
| **awk** | Text processing |

---

## Hardware Measurement Tools (Software)

### Multimeter Software

| Software | Purpose | Hardware |
|----------|---------|----------|
| **DMM Software** | USB multimeter logging | Compatible USB multimeters |
| **Fluke Connect** | Bluetooth multimeter app | Fluke meters |

### Logic Analyzer Software

| Software | Purpose | Hardware |
|----------|---------|----------|
| **PulseView** | Logic analyzer GUI | Sigrok-compatible analyzers |
| **Saleae Logic** | Professional logic analysis | Saleae hardware |

---

## Operating System Compatibility

### Supported Platforms

| OS | PlatformIO | Development | Notes |
|----|------------|-------------|-------|
| **Windows** | ✅ Yes | ✅ Fully supported | Requires USB drivers |
| **macOS** | ✅ Yes | ✅ Fully supported | Native USB support |
| **Linux** | ✅ Yes | ✅ Fully supported | May need udev rules for USB |
| **Raspberry Pi OS** | ✅ Yes | ✅ Supported | ARM-based development |

### USB Serial Device Paths

| OS | Device Path |
|----|-------------|
| **Windows** | `COM3`, `COM4`, etc. |
| **macOS** | `/dev/tty.usbserial-*` or `/dev/cu.usbserial-*` |
| **Linux** | `/dev/ttyUSB0`, `/dev/ttyACM0`, etc. |

---

## Installation Quick Reference

### One-Time Setup

```bash
# Install Python (if not already installed)
# Download from python.org or use system package manager

# Install PlatformIO Core
pip install platformio

# Verify installation
pio --version

# Install USB drivers (if needed)
# Download from manufacturer website (see USB Drivers section)

# Clone repository
git clone https://github.com/yourusername/kilnin-it.git
cd kilnin-it

# Install project dependencies (automatic)
pio run
```

### Daily Development Workflow

```bash
# Build firmware
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio run --target monitor

# Upload web files to SPIFFS
pio run --target uploadfs
```

---

## Security & Safety Tools

### Code Analysis (Optional)

| Tool | Purpose |
|------|---------|
| **cppcheck** | Static analysis for C/C++ |
| **clang-tidy** | Linting and static analysis |
| **PlatformIO Check** | Built-in code analysis |

### Network Security (Optional)

| Tool | Purpose |
|------|---------|
| **nmap** | Port scanning and security auditing |
| **SSL Labs** | HTTPS configuration testing (if using SSL) |

---

## Prohibited Tools/Libraries

**Do NOT use the following** (compatibility or safety issues):

- ❌ **Blocking web server libraries** (e.g., `WebServer.h`) - use AsyncWebServer instead
- ❌ **delay()** in critical paths - use non-blocking timing
- ❌ **Dynamic memory allocation** in ISRs or time-critical tasks
- ❌ **String class** for parsing (use char arrays) - memory fragmentation
- ❌ **Software serial** on ESP32 - use hardware UART
- ❌ **Third-party WiFi managers** - use native ESP32 WiFi

---

## Learning Resources

### Official Documentation

- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core Docs](https://docs.espressif.com/projects/arduino-esp32/)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)

### Tutorials & Guides

- [Random Nerd Tutorials (ESP32)](https://randomnerdtutorials.com/projects-esp32/)
- [Adafruit Learn (MAX31855)](https://learn.adafruit.com/thermocouple/)
- [U8g2 Reference Manual](https://github.com/olikraus/u8g2/wiki)

### Community Support

- [PlatformIO Community](https://community.platformio.org/)
- [ESP32 Forum](https://www.esp32.com/)
- [Arduino Forum](https://forum.arduino.cc/)

---

## Tool Updates & Maintenance

### Keeping Tools Updated

```bash
# Update PlatformIO Core
pip install --upgrade platformio

# Update PlatformIO platforms
pio platform update

# Update project libraries
pio lib update

# Update VSCode extensions
# Use VSCode extension manager
```

### Recommended Update Frequency

- **PlatformIO Core**: Monthly or when new ESP32 features needed
- **Libraries**: Before major milestones, test thoroughly after updates
- **Firmware**: Test after library updates before deploying to hardware

---

## Troubleshooting Tools

### Common Issues & Diagnostic Tools

| Issue | Diagnostic Tool | Command/Action |
|-------|----------------|----------------|
| **ESP32 not detected** | Device manager / lsusb | Check USB connection, drivers |
| **Upload fails** | esptool.py | `esptool.py --port [PORT] chip_id` |
| **Library conflicts** | PlatformIO | `pio lib list` and check versions |
| **Build errors** | GCC output | Read error messages carefully |
| **Web not loading** | Browser DevTools | Check network tab, console |
| **SPI not working** | Logic analyzer | Capture SPI signals |

---

## Summary

**Essential Tools** (minimum to get started):
- Python 3.x
- PlatformIO Core
- Git
- Text editor
- USB drivers (if needed)
- Web browser

**Recommended Tools** (better experience):
- Visual Studio Code + PlatformIO IDE
- Serial monitor (built into PlatformIO)
- Multimeter (hardware debugging)

**Optional Tools** (advanced users):
- Schematic design software (Fritzing, KiCad)
- Logic analyzer software
- Network analysis tools (Wireshark)

---

**Last Updated**: 2025-10-13
**Project Version**: Development (M1.1 Complete)

For questions about tool selection or setup, see [CLAUDE.md](CLAUDE.md) or open an issue on GitHub.
