# SALD Sandbox 3D

A 3D sandbox built using [SALD](https://github.com/mayuso/SALD).

![](otherResources/sald-sandbox3D.gif)

## Usage

### Build from source

Ensure you have **CMake** and a **C++ compiler** installed.

**1. Clone the repository**

```bash
git clone https://github.com/mayuso/SALD-Sandbox3D.git
cd SALD-Sandbox3D
```

**2. Update submodules**

This project depends on the **SALD** engine, which is included as a submodule.

```bash
git submodule update --init --recursive
```

**3. Build and Run (Debug)**

For quick testing or development:

```bash
mkdir build
cd build
cmake ..
make
./sald-sandbox3d
```

### Release Build

To create a standalone `release` folder containing the optimized executable and all necessary assets (Textures, Shaders, Models):

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make install
```

This will generate a `release/` directory in the project root. You can then run the game directly from there:

```bash
cd ../release
./sald-sandbox3d
```

## About

A 3D sandbox created to demonstrate and test the capabilities of the **SALD** engine (OpenGL wrapper).

## Resources

*   [SALD Engine](https://github.com/mayuso/SALD)
