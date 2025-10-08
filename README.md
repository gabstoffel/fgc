# OpenGL Template

A simple OpenGL application template based on the laboratory code structure from INF01047 Computer Graphics Fundamentals.

## Features

- Basic OpenGL 3.3 Core setup
- Simple triangle rendering
- GLFW window management
- GLM for matrix operations
- Basic shader system
- Cross-platform build support

## Dependencies

- OpenGL 3.3+
- GLFW3
- GLM
- CMake 3.10+

## Building

### Windows
```bash
build.bat
```

### Linux/macOS
```bash
chmod +x build.sh
./build.sh
```

### Manual Build
```bash
mkdir build
cd build
cmake ..
make
```

## Running

After building, run the executable:
- Windows: `build/Release/OpenGLTemplate.exe`
- Linux/macOS: `build/OpenGLTemplate`

## Controls

- ESC: Exit application

## Project Structure

```
├── CMakeLists.txt          # CMake configuration
├── build.bat              # Windows build script
├── build.sh               # Linux/macOS build script
├── README.md              # This file
├── include/
│   ├── utils.h            # OpenGL error checking utilities
│   └── matrices.h         # Matrix operations and transformations
└── src/
    ├── main.cpp           # Main application code
    ├── glad.c             # GLAD OpenGL loader
    └── shaders/
        ├── shader_vertex.glsl    # Vertex shader
        └── shader_fragment.glsl  # Fragment shader
```

## Notes

This template provides a minimal working OpenGL application. For a complete GLAD implementation, visit https://glad.dav1d.de/ and generate the glad.c file with OpenGL 3.3 Core profile selected.
