# Strawberry
Strawberry is a Vulkan renderer for OpenJK.

## Additional Build Dependencies
* Vulkan SDK (can be downloaded from https://vulkan.lunarg.com)

## Build Instructions
Generate the project/makefiles for your compiler:

```
$ mkdir build
$ cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DVULKAN_SDK=/path/to/vulkan/sdk ..
```

Build the `rd-strawberry` project will generate a shared library prefixed
with `rd-strawberry_`.

### Building SPIR-V Shaders
The Vulkan API only accepts shaders in the *SPIR-V format*, which is a binary
shader format. These shaders are created by compiling the GLSL shaders in the
`glsl/` directory with the shader compiler provided in the Vulkan SDK.

On Linux/macOS, you can run the provided `compile_shaders.sh` script to
compile the GLSL shaders to SPIR-V:

```
$ cd glsl/
$ VULKAN_SDK=/path/to/vulkan/sdk ./compile_shaders.sh
```

The output `rd-shaders.pk3` file should be copied into the game's `base/`
folder.

## New Cvars
Name            | Default Value | Description
----------------|---------------|---------------------------------
r_debugApi      | 0             | Toggles the Vulkan validation layer. Useful for debugging renderer changes

## Architectural Overview
Strawberry started as a copy of the Vanilla renderer so that JKA features
could not be accidentally missed, or would need to be re-implemented.

In its current state, Strawberry does not attempt to move any of the existing
renderer processing from the CPU to the GPU. In this regard, it is a very
straightforward port.

All possible rendering states are generated at shader load time and compiled
into graphics pipelines.