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

Build the `rd-strawberry` project will generate a shared library prefixed with `rd-strawberry_`.

### Building SPIR-V Shaders
The Vulkan API only accepts shaders in the *SPIR-V format*, which is a binary shader format. These shaders are created by compiling the GLSL shaders in the `glsl/` directory with the shader compiler provided in the Vulkan SDK.

```
$ mkdir -p spv
$ glslc -O -o spv/render.vert.spv glsl/render.vert
$ glslc -O -o spv/render.frag.spv glsl/render.frag
$ glslc -O -DUSE_MULTITEXTURE -o spv/render.multitexture.vert.spv glsl/render.vert
$ glslc -O -DUSE_MULTITEXTURE=1 -o spv/render.multitexture.add.frag.spv glsl/render.frag
$ glslc -O -DUSE_MULTITEXTURE=2 -o spv/render.multitexture.multiply.frag.spv glsl/render.frag
```

The `spv` directory should be compressed into a `.pk3` file and copied into the game's `base/` folder. Alternatively, if the game is started with `sv_pure 0` then the directory can be copied directory in the game's `base/` folder.

## New Cvars
Name            | Default Value | Description
----------------|---------------|---------------------------------
r_debugApi      | 0             | Toggles the Vulkan validation layer. Useful for debugging renderer changes

## Architectural Overview
Strawberry started as a copy of the Vanilla renderer so that JKA features could not be accidentally missed, or would need to be re-implemented.

In its current state, Strawberry does not attempt to move any of the existing renderer processing from the CPU to the GPU. In this regard, it is a very straightforward port.

All possible rendering states are generated at shader load time and compiled into graphics pipelines.
