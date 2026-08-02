# Repository Guidelines

## Project Structure & Module Organization

`3D-GPolylla` is a C++20 tetrahedral-mesh processing project. Public API headers belong in `include/gpolylla/`; keep implementations in `src/`. The command-line executable is defined under `src/exe/`, while the OpenGL renderer and its GLSL shaders live in `src/renderer/`. Add GoogleTest coverage in `test/`, and place reusable sample meshes in `data/`. Treat `old/` and `old2/` as archived implementations: do not extend them unless a task explicitly targets legacy code. CMake configuration is split between the root and component-level `CMakeLists.txt` files.

## Build, Test, and Development Commands

Run these commands from the repository root:

- `cmake -S . -B build -DGPOLYLLA_TEST=ON -DGPOLYLLA_EXE=ON` configures the library, tests, and CLI.
- `cmake -S . -B build -DGPOLYLLA_RENDERER=ON` additionally enables the renderer; OpenGL development libraries are required.
- `cmake --build build` compiles configured targets.
- `ctest --test-dir build --output-on-failure` runs all discovered GoogleTest cases.
- `clang-format -i src/file.cpp include/gpolylla/file.h` formats changed C++ files.

Configuration uses `FetchContent`, so the first configure requires network access for Eigen, QuickHull, polyhedron-kernel, and optionally GoogleTest and renderer dependencies. Avoid the current `Makefile`: it references nonexistent `default` presets. `CMakePresets.json` contains a Windows/MSVC-specific `Debug` preset, so use the portable commands above on other platforms.

## Coding Style & Naming Conventions

Format C++ with the repository `.clang-format` (`BasedOnStyle: Microsoft`) and four-space indentation. Follow existing conventions: `PascalCase` for types (`PolyMesh`, `CavityAlgorithm`), `camelCase` for methods (`readMesh`), and trailing underscores for private members (`cavities_`). Put stable interfaces in `include/gpolylla/`, implementation-only helpers in `src/`, and shaders beside the renderer.

## Testing Guidelines

Tests use GoogleTest and CMake's `gtest_discover_tests`. Name files `*_test.cpp` and fixtures `*Test`. Add focused cases for normal meshes, degenerate geometry, adjacency, readers/writers, and file-format failures. Tests may use `DATA_DIR` for fixtures and `TEMP_DIR` for generated output. Run the full CTest command before submitting changes.

## Commit & Pull Request Guidelines

History primarily uses concise Conventional Commit subjects, such as `feat: stats added`, `fix: kernel calculations`, and scoped forms like `feat(lib): ...`. Use an imperative, specific subject and an optional scope. Pull requests should explain motivation and algorithmic impact, list build/test commands run, link relevant issues, and include screenshots for renderer changes or representative mesh inputs/outputs for geometry changes.
