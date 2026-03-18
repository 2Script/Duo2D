<div align="center">
    <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/d/d7/SMPTE-C_RGB_color_cube.png/250px-SMPTE-C_RGB_color_cube.png" alt="Placeholder Logo"/>
    <h1>Sirius</h1>
</div>

> Sirius is in a early development stage and may be subject to significant changes, including to its API
> 
> Any features *italicized* are not currently implemented but are planned to be before the full release

Sirius is a cross-platform, comprehensive graphics engine library built using Vulkan 1.3 and designed to be **efficient**, **adaptable**, and **easy to use**.
<br/>

Sirius was developed as a **no-compromises** engine - it gives complete control of a modern and optimized rendering process to the developer while still having a straightforward, easy to use API.
<br/>

It supports windowed and window-less rendering, fully custom compute and graphics shaders, and contains utilities for graphical arithmetic computations. It's written in C++ and it natively supports Windows, Linux, BSD, *Android* and Nintendo Switch 1 & 2.


<h2>Features</h2>

<h3>Design Principles</h3>

- **WYSIWYG** - Anything written by the developer in Sirius is 1:1 with what the graphics API and/or other internal processes execute; There are no under the hood processes that the developer doesn't have control over.
- **[Zero-Overhead](https://en.cppreference.com/w/cpp/language/Zero-overhead_principle.html)** - You don't pay for what you don't use
- **Bindless** - There are as little pipelines and draw calls as possible; Large buffers for data, Runtime-sized arrays for textures, and no vertex attributes.
- **GPU Driven Rendering** - As little GPU-issued draw calls as possible and no CPU-issued draw calls at all; Multi and indirect rendering so that the CPU is free for other work and the GPU can do as much work as possible all at once.
- **Extensibility** - The developer and the end-user can extend/customize any built-in functionality

<h3>API</h3>

- *Built-in native modding/plugin API*
- Straightforward and fully customizable initialization process and render timeline declaration
- Developer selectable resource models - complete control over how your resources are used by the GPU
<br/> <br/>
- Familiar API consistent with that of the C++ standard library
- Rich event-based categorized input system
- Complete control over which graphics device to utilize
- Built-in arithmetic types (e.g. vectors, matricies)
- Simple, standardized API for image loading - use the built-in image decoder or create your own decoder that interfaces with a standardized texture API
- Built-in decoding of [ktx2][ktx-software] image files
- Rich result return types on all falliable functions instead of exceptions


<h3>Efficiency</h3>

- Compile-time offloading - almost all rendering logic happens at compile time
- No dependence on any platform-specific libraries, SDL, GLM, VMA, ImGui, FreeType, *or even the standard C++ library*.
- Pre-compiled shaders - no shader compilation necessary for the end-user
- Fully asynchronous rendering
- Multithreaded, asynchronous input handling
<br/> <br/>
- No use of exceptions or RTTI - compile with `-fno-exceptions` and `-fno-rtti`
- No use of virtual functions - all polymorphism is static
- Constant number of memory allocations for buffers - 0(1) space complexity thanks to buffer sub-allocation
- Memory-mapped files for I/O
- **Runtime** dispatched SIMD wherever practical


<h3>Documentation</h3>

- *In-depth API documentation formatted similar to that of the standard library*
- *Thorough guides for both high-level and low-level funtionality*
- *Comprehensive examples that showcase a wide range of objectives*


<h2>Showcase</h2>

(Under Construction)


<h2>Example</h2>

A minimal example which opens a window, loads some assets, and renders to the window.

> The `RESULT_XXX` macros verify if the function was successful and proceed with any specified operation if they were. If not, it returns the error code and thus stops execution.

```cpp
#include <sirius.hpp>

int main(){
    //Create an application instance, verifying it was successful
    acma::application<> app;
    RESULT_TRY_MOVE(app, acma::make<acma::application<>>("My App"));


    //Generate a list of graphical devices, verifying the generation was successful and the list is not empty
    std::set<acma::vk::physical_device> device_list;
    RESULT_TRY_COPY(device_list, app.devices());
    if(device_list.empty()) return acma::error::no_vulkan_devices;

    //Select the first device
    app.selected_device() = *device_list.begin();
    
    //Initialize the device, verifying it was successful
    RESULT_VERIFY(app.initialize_device());

    //Create a window, verifying the creation was successful
    acma::window* win;
    RESULT_TRY_COPY(win, app.add_window());


    const std::filesystem::path assets_path = std::filesystem::canonical(std::filesystem::path("../assets"));

    //Add a texture
    win->try_emplace<acma::texture>("characters", assets_path / "my_characters.ktx2");

    //Add a font
    win->try_emplace<acma::font>("main_font", assets_path / "my_font.ttf");

    //Apply the changes, making sure it was successfully applied
    RESULT_VERIFY(win->apply_changes<acma::texture>());
    RESULT_VERIFY(win->apply_changes<acma::font>());


    //Begin async rendering
    std::future<acma::result<void>> render = app.start_async_render();

    //Poll events
    while(app.open())
        app.poll_events();

    //Check the result of the rendering
    RESULT_VERIFY(render.get());

    //Check the result of the rest of the application
    RESULT_VERIFY(app.join());

    return 0;
}
```
<br/>

<details><summary>Alternatively, here's the same example without the result verification macros</summary>

```cpp
#include <sirius.hpp>

int main(){
    //Create an application instance, verifying it was successful
    auto _a = acma::make<acma::application<>>("My App");
    if(!_a.has_value()) return _a.error();
    acma::application<> app = *std::move(a);


    //Generate a list of graphical devices, verifying the generation was successful and the list is not empty
    auto _d = app.devices();
    if(!_d.has_value()) return d.error();
    std::set<acma::vk::physical_device> device_list = *d;
    if(device_list.empty()) return acma::error::no_vulkan_devices;

    //Select the first device
    app.selected_device() = *device_list.begin();
    
    //Initialize the device, verifying it was successful
    auto _i = app.initialize_device();
    if(!_i.has_value()) return _i.error(); 

    //Create a window, verifying the creation was successful
    auto _w = app.add_window();
    if(!_w.has_value()) return w.error();
    acma::window* win = *_w;


    const std::filesystem::path assets_path = std::filesystem::canonical(std::filesystem::path("../assets"));

    //Add a texture
    win->try_emplace<acma::texture>("characters", assets_path / "my_characters.ktx2");

    //Add a font
    win->try_emplace<acma::font>("main_font", assets_path / "my_font.ttf");

    //Apply the changes, making sure it was successfully applied
    if(auto r = win->apply_changes<acma::texture>(); !r.has_value())
        return r.error();
    if(auto r = win->apply_changes<acma::font>(); !r.has_value())
        return r.error();


    //Begin async rendering
    std::future<acma::result<void>> render = app.start_async_render();

    //Poll events
    while(app.open())
        app.poll_events();

    //Check the result of the rendering
    if(auto r = render.get(); !r.has_value())
        return r.error();

    //Check the result of the rest of the application
    if(auto r = app.join(); !r.has_value())
        return r.error();

    return 0;
}
```
</details>

<h2>Dependencies</h2>

| Name | Minimum Version | Purpose | License | Primary Author(s) |
| ---- | --------------- | ------- | ------- | ----------------- |
| Vulkan | 1.3 | Graphics API | N/A (graphics driver dependent) | [Khronos Group](https://www.khronos.org/) |
| [streamline](https://github.com/open-lite/streamline)            | master branch | General purpose | `GPL-3.0 WITH GCC-exception-3.1` | [OpenLite](https://github.com/open-lite) [Artin Alavi ([Arastais](https://github.com/Arastais))]
| [GLFW](https://github.com/glfw/glfw)                        | 3.4.0  | Window system | `Zlib` | [GLFW Team](https://www.glfw.org/) |
| [harfbuzz](https://github.com/harfbuzz/harfbuzz)            | 12.0.0 | Font file loading | `MIT-Modern-Variant` | Behdad Esfahbod ([behdad](https://github.com/behdad)) |
| [libktx][ktx-software]                                      | 1.3    | KTX2 Image loading and decoding | `Apache-2.0` | [Khronos Group](https://www.khronos.org/) [Mark Callow ([MarkCallow](https://github.com/MarkCallow))] |
| [BS::thread_pool](https://github.com/bshoshany/thread-pool) | 5.0.0  | Generalized multi-threading | `MIT` | Barak Shoshany ([bshoshany](https://github.com/bshoshany)) |


<h2>Supported Platforms</h2>

<h3>Operating Systems</h3>

| Name | Minimum Version Name |
| ---- | -------------------- |
| Windows | 7 |
| Linux<sup>1</sup> | N/A (graphics driver dependent) |
| BSD | N/A (graphics driver dependent) |
| Horizon (Nintendo Switch) | 15.0.0<sup>2</sup> |
| Android | 13 |

> <sup>1</sup>Only wayland is supported on Linux <br/>
> <sup>2</sup>This is an educated guess made based on the date of the official [Vulkan Conformant Products submission][switch-vulkan-conformance] and the [system update history for the Nintendo Switch][switch-update-history]


<h3>Devices & Drivers</h3>

The client's graphics device must at least support Vulkan 1.3 and, additionally, the `VK_KHR_push_descriptor` extension (included as part of Vulkan 1.4). <!--the `VK_KHR_maintenance5` (included as part of Vulkan 1.4) and `VK_EXT_descriptor_heap` extensions.-->

The minimum requirements for microarchitecture and graphics driver combination (along with its associated devices) that satisfy the above conditions are listed below.

| GPU Vendor  | Platform | Driver               | Microarchitecture        | Desktop Devices | Mobile Devices |
| ----------  | ---------| ------------------   | -----------------        | --------------- | -------------- |
| Nvidia      | Windows  | Nvidia Proprietary   | Maxwell                  | GeForce 900 series; GeForce GTX 745, GTX 750 [Ti] | GeForce 900M series; GeForce 830M, 840M, 845M, GTX 850M  |              
| Nvidia      | Linux    | Nvidia Proprietary   | Maxwell                  | GeForce 900 series; GeForce GTX 745, GTX 750 [Ti] | GeForce 900M series; GeForce 830M, 840M, 845M, GTX 850M  |              
| Nvidia      | Linux    | Mesa (NVK)           | Kepler                   | Geforce 600 series | GeForce 700M series; GeForce GT 640M, GT 645M, GT 650M, GTX 660M, GTX 670MX, GTX 675MX, GTX 680M, GTX 680MX |
| Nvidia      | Switch   | Nintendo Proprietary | Maxwell (Tegra)          | - | T210, T214 |
| AMD         | Windows  | AMD Proprietary      | GCN 4 (Arctic Islands)   | Radeon 400 series; Ryzen 2000 APU series | Radeon RX 5000M series | 
| AMD         | Linux    | Mesa (RADV)          | GCN 1 (Southern Islands) | Radeon HD 7000 series; Athlon 5000 APU series | Radeon HD 7000M series |
| Intel       | Windows  | Intel Proprietary    | GT Gen9 (Skylake)        | HD Graphics 500 series; Iris [Pro] Graphics 500 series | HD Graphics 500 series; Iris [Pro] Graphics 500 series |
| Intel       | Linux    | Mesa (ANV)           | GT Gen9 (Skylake)        | HD Graphics 500 series; Iris [Pro] Graphics 500 series | HD Graphics 500 series; Iris [Pro] Graphics 500 series |
| Qualcomm    | Android  | Qualcomm Proprietary | N/A                      | - | Adreno 710, 720, 722, 732, 735, 740, 750; Adreno 800 series |
| Qualcomm    | Linux    | Mesa (Turnip)        | N/A                      | - | Adreno 600 series |
| Arm Limited | Android  | Arm Proprietary      | Valhall 4th gen          | - | Mali G615, G715; Immortalis G715 |
| Arm Limited | Linux    | Mesa (PanVK)         | Valhall 3rd gen          | - | Mali G310, G510, G610, G710  |


<!--
| GPU Vendor  | Platform | Driver               | Driver Version                    | Microarchitecture        | Desktop Devices | Mobile Devices |
| ----------  | ---------| ------------------   | --------------------------------- | -----------------        | --------------- | -------------- |
| Nvidia      | Windows  | Nvidia Proprietary   | 582.30+ (except 590.00 to 595.59) | Maxwell                  | GeForce GTX 745, GTX 750 [Ti]; GeForce 900 series | GeForce 830M, 840M, 845M, GTX 850M; GeForce 900M series |              
| Nvidia      | Linux    | Nvidia Proprietary   | 580.94.16+                        | Maxwell                  | GeForce GTX 745, GTX 750 [Ti]; GeForce 900 series | GeForce 830M, 840M, 845M, GTX 850M; GeForce 900M series |              
| Nvidia      | Linux    | Mesa (NVK)           | 26.1+                             | Kepler                   | Geforce 600 series | GeForce GT 640M, GT 645M, GT 650M, GTX 660M, GTX 670MX, GTX 675MX, GTX 680M, GTX 680MX; GeForce 700M series |
| Nvidia      | Switch   | Nintendo Proprietary | N/A                               | Maxwell (Tegra)          | - | T210, T214 |
| AMD         | Windows  | AMD Proprietary      | 25.30.17.02+                      | GCN 4 (Arctic Islands)   | Radeon 400 series; Ryzen 2000 APU series | Radeon RX 5000M series | 
| AMD         | Linux    | Mesa (RADV)          | 26.1+                             | GCN 1 (Southern Islands) | Radeon HD 7000 series; Athlon 5000 APU series | Radeon HD 7000M series |
| Intel       | Windows  | Intel Proprietary    | 32.0.101.8509(?)+                 | Alchemist                | Arc A series; Core Ultra 100 APU series | Arc AM series; Core Ultra 100 APU series |
| Intel       | Linux    | Mesa (ANV)           | 26.1+                             | Skylake                  | Core 6000 APU series; Pentium G4000 APU series; Celeron G3000 APU series  | Core 6000 APU series; Pentium 4000 APU series; Celeron 3000 APU series |
| Qualcomm    | Android  | Qualcomm Proprietary | N/A                               | N/A                      | - | Adreno 710, 720, 722, 732, 735, 740, 750; Adreno 800 series |
| Qualcomm    | Linux    | Mesa (Turnip)        | TBD                               | N/A                      | - | Adreno 600 series |
| Arm Limited | Android  | Arm Proprietary      | N/A                               | Valhall 4th gen          | - | Mali G615, G715; Immortalis G715 |
| Arm Limited | Linux    | Mesa (PanVK)         | TBD                               | Valhall 3rd gen          | - | Mali G310, G510, G610, G710  |
-->

<h3>Instruction Set Architectures</h3>

In general, all modern 64-bit ISAs are supported.

The following additionally have optional support for runtime dispatching of SIMD extensions/vector intrinsics and are tested and verified:

| ISA Name    | Required Microarchitecture | Optional Extensions | 
| ----------- | -------------------------- | ------------------- |
| x86         | x86-64-v1                  | POPCNT, SSE4.2, AVX, AVX2, FMA, BMI1, BMI2, AVX512F, AVX512BW, AVX512DQ, AVX10 |
| ARM AArch64 | -                          | SVE, SVE2 |
| RISC-V      | RV64IMAFD or RV64EMAFD     | B, P, C |


<h3>Compilers</h3>

> This section is only relevant if you are building Sirius from source

Only GCC and Clang (and its derivatives) are officially supported; The MSVC compiler is not supported at all.

The compiler must support gnu-style attributes.

| Compiler | Minimum Version |
| -------- | --------------- |
| GCC      | 14 |
| Clang    | 21.1.0 |
| ICX      | 2025.3 |
| AOCC     | TBD |


<h2>License</h2>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 

[ktx-software]: https://github.com/KhronosGroup/KTX-Software
[switch-vulkan-conformance]: https://www.khronos.org/conformance/adopters/conformant-products#submission_693
[switch-update-history]: https://en-americas-support.nintendo.com/app/answers/detail/a_id/43314/session/L2F2LzEvdGltZS8xNzU5NTM0MTY4L2dlbi8xNzU5NTM0MTY4L3NpZC9mVTJzU3RlY1pQRmljb244ZUdDQkUlN0VnM1A1UnBubUpvX0IlN0VXeUhoV05RNWNqWlVSSkVHcFFSWVFsOUlkaiU3RW1nS3N4U055aGNLMUhqblNGazlibFZFOThibzJBWkU2SkJTdndZUmYzS0QwayU3RUpMYl9wQlFhS0ZydyUyMSUyMQ%3D%3D
