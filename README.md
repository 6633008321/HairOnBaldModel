# HairOnBaldModel

An Interactive Tool for Placing and Saving 3D Hair Models on Bald Heads using OpenGL and ImGui.

## Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/6633008321/HairOnBaldModel.git
   cd HairOnBaldModel
   ```

2. Generate Visual Studio solution:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. Open and configure Visual Studio:
   - Open `HairOnBaldModel.sln` in `build/` with Visual Studio 2022.
   - Right-click `HairOnBaldModel` in Solution Explorer > **Set as Startup Project**.
   - Right-click `HairOnBaldModel` > **Properties** > **Configuration Properties** > **Debugging** > Set **Working Directory** to `$(ProjectDir)` (points to `HairOnBaldModel/`).

4. Build and run

## Usage

- Press `O` or click "Select Hair Model" to choose a hair model (`.obj` or `.ply`).
- Use `WASD` to move camera, mouse to rotate, scroll to zoom.
- Press `1` (bald head), `2` (hair), or `3` (both) to toggle rendering.
- Press `F` to toggle wireframe mode.
- Press `Tab` to lock/unlock mouse.
- Adjust hair position, scale, rotation, and color via ImGui panel.
