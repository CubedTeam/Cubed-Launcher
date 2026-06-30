# Cubed Launcher

Official game launcher for Cubed.

## Overview

Cubed Launcher is the official client-side application for managing, updating, and launching the Cubed game. It provides a native graphical interface for game installation, version control, and runtime configuration.

## Requirements

### Build Dependencies

- **CMake**: version 3.21 or higher
- **Compiler**: C++23 conforming compiler (GCC 12+, Clang 16+, MSVC 19.38+)
- **Qt**: version 6.11 or higher (including Core, Widgets, Network, and Gui modules)
- **Ninja** (recommended) or any CMake-supported generator

### Development Environment

- Operating System: Windows 10/11, macOS 12+, or Linux (glibc 2.35+)
- Git (for source cloning)
- Qt toolchain (qmake, moc, rcc, uic) available in PATH

## Building from Source

### 1. Clone the Repository

```bash
git clone https://github.com/CubedTeam/Cubed-Launcher.git
cd Cubed-Launcher
```

### 2. Create Build Directory

```bash
mkdir build
cd build
```

### 3. Configure with CMake

Use Ninja as the generator (recommended for speed):

```bash
cmake -G "Ninja" ..
```

Alternatively, using the default generator:

```bash
cmake ..
```

You may specify additional options:

- `-DCMAKE_BUILD_TYPE=Release` – for optimized build
- `-DQt6_DIR=/path/to/Qt/lib/cmake/Qt6` – if Qt6 is not auto-detected

### 4. Build

```bash
cmake --build .
```

Or with Ninja directly:

```bash
ninja
```

### Compiler C++23 Support

Ensure your compiler is up to date. For GCC, use version 12 or later. For Clang, version 16 or later. For MSVC, Visual Studio 2022 version 17.6 or later.

## License

GPL-3.0 license

