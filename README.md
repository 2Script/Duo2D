<div align="center">
    <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/d/d7/SMPTE-C_RGB_color_cube.png/250px-SMPTE-C_RGB_color_cube.png" alt="Placeholder Logo"/>
    <h1>Duo2D</h1>
</div>

> Duo2D is in a very early development stage and may be subject to significant changes, including to its API
> 
> Any features *italicized* are not currently implemented but are planned to be before the full release

Duo2D is a cross-platform, comprehensive general-purpose Vulkan graphics engine built heavily around **performance**, **efficiency**, and **ease of use**, and designed based on the [zero-overhead principle](https://en.cppreference.com/w/cpp/language/Zero-overhead_principle.html).
<br/>
It handles windowing, UI, 2D and 3D graphics, 2D animations, inputs, and graphical arithmetic computations.
<br/>
It's written in C++ and it natively supports Windows, Linux, BSD, *Android* and Nintendo Switch 1 & 2.

Duo2D was created due to a lack of a similarly-comprehensive graphics engine - other graphics engines tend to lack significant performance/efficiency improvements or not give enough control to the developer; Duo2D uses a variety of optimizations, while still offering a rich, well-documented API.

<h2>Features</h2>

**Performance & Efficiency**

- **Runtime** SIMD dispatching wherever practical
- No use of exceptions or RTTI - compile with `-fno-exceptions` and `-fno-rtti`
- No use of virtual functions - all polymorphism is static
- Use of indirect rendering - **only 1 draw call for the entire window**
- No dependence on any platform-specific libraries, SDL, GLM, VMA, ImGui, FreeType, *or even the standard C++ library*.
- Multithreaded input handling *and image loading*
- Async rendering and event handling
- Optimized manual management of GPU memory with custom memory models and buffer sub-allocation
- O(1) space complexity for GPU buffers - constant number of memory allocations for all graphical objects
- Use of memory-mapped files for I/O
- Thread-safe async GPU memory modification
- Compile-time generation of draw functions for zero runtime overhead

**Ease of Use**

- API consistent with that of the C++ standard library
- Pre-compiled shaders - no shader compilation necessary for the end-user
- *Built-in native modding/plugin API*
- *CSS-like UI styling system*
- Custom shader/graphical object creation system with developer-chosen memory models
- Built-in shaders for common UI elements
- Single function calls for adding graphical objects, adding assets, rendering, event polling
- Fully customizable event-based input system
- Free choice over which graphical device to render on
- Support for virtually any image format, including compressed [ktx2][ktx-software]
- On-the-fly MTSDF font generation - turn any font into a arbitrarily scalable set of glyphs
- Built-in arithmetic types and related functions commonly used for graphics (e.g. vectors, matrices)
- Robust result and error code system used globally
- *In-depth, robust documentation similar to that of the standard library*


<h2>Example</h2>

A minimal example which opens a window, loads some assets, and renders to the window.

> The `RESULT_XXX` macros verify if the function was successful and proceed with any specified operation if they were. If not, it returns the error code and thus stops execution.

```cpp
#include <Duo2D.hpp>

int main(){
    //Create an application instance, verifying it was successful
    d2d::application<> app;
    RESULT_TRY_MOVE(app, d2d::make<d2d::application<>>("My App"));


    //Generate a list of graphical devices, verifying the generation was successful and the list is not empty
    std::set<d2d::vk::physical_device> device_list;
    RESULT_TRY_COPY(device_list, app.devices());
    if(device_list.empty()) return d2d::error::no_vulkan_devices;

    //Select the first device
    app.selected_device() = *device_list.begin();
    
    //Initialize the device, verifying it was successful
    RESULT_VERIFY(app.initialize_device());

    //Create a window, verifying the creation was successful
    d2d::window* win;
    RESULT_TRY_COPY(win, app.add_window());


    const std::filesystem::path assets_path = std::filesystem::canonical(std::filesystem::path("../assets"));

    //Add a texture
    win->try_emplace<d2d::texture>("characters", assets_path / "my_characters.ktx2");

    //Add a font
    win->try_emplace<d2d::font>("main_font", assets_path / "my_font.ttf");

    //Apply the changes, making sure it was successfully applied
    RESULT_VERIFY(win->apply_changes<d2d::texture>());
    RESULT_VERIFY(win->apply_changes<d2d::font>());


    //Begin async rendering
    std::future<d2d::result<void>> render = app.start_async_render();

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

Alternatively, here's the same example without the result verification macros:

```cpp
#include <Duo2D.hpp>

int main(){
    //Create an application instance, verifying it was successful
    auto _a = d2d::make<d2d::application<>>("My App");
    if(!_a.has_value()) return _a.error();
    d2d::application<> app = *std::move(a);


    //Generate a list of graphical devices, verifying the generation was successful and the list is not empty
    auto _d = app.devices();
    if(!_d.has_value()) return d.error();
    std::set<d2d::vk::physical_device> device_list = *d;
    if(device_list.empty()) return d2d::error::no_vulkan_devices;

    //Select the first device
    app.selected_device() = *device_list.begin();
    
    //Initialize the device, verifying it was successful
    auto _i = app.initialize_device();
    if(!_i.has_value()) return _i.error(); 

    //Create a window, verifying the creation was successful
    auto _w = app.add_window();
    if(!_w.has_value()) return w.error();
    d2d::window* win = *_w;


    const std::filesystem::path assets_path = std::filesystem::canonical(std::filesystem::path("../assets"));

    //Add a texture
    win->try_emplace<d2d::texture>("characters", assets_path / "my_characters.ktx2");

    //Add a font
    win->try_emplace<d2d::font>("main_font", assets_path / "my_font.ttf");

    //Apply the changes, making sure it was successfully applied
    if(auto r = win->apply_changes<d2d::texture>(); !r.has_value())
        return r.error();
    if(auto r = win->apply_changes<d2d::font>(); !r.has_value())
        return r.error();


    //Begin async rendering
    std::future<d2d::result<void>> render = app.start_async_render();

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

<h2>Dependencies</h2>


<h3>Runtime</h3>

| Name | Minimum Version | Purpose | License | Primary Author(s) |
| ---- | --------------- | ------- | ------- | ----------------- |
| Vulkan | 1.3 | Graphics API | N/A (graphics driver dependent) | [Khronos Group](https://www.khronos.org/) |
| [GLFW](https://github.com/glfw/glfw)              | 3.4.0  | Window system | `Zlib` | [GLFW Team](https://www.glfw.org/) |
| [DuoDecode](https://github.com/2Script/DuoDecode) | master branch | Image loading (except KTX2) | `LGPL-3.0-or-later` | [2Script](https://github.com/2Script) [Artin Alavi ([Arastais](https://github.com/Arastais))] |
| [FFmpeg](https://github.com/FFmpeg/FFmpeg)        | -      | Image decoding (except KTX2) | `LGPL-2.1-or-later` | [FFmpeg Team](https://ffmpeg.org/) |
| [llfio](https://github.com/ned14/llfio)           | develop branch (c94d2e0) | File I/O | `Apache-2.0 OR BSL-1.0` | Niall Douglas ([ned14](https://github.com/ned14)) |
| [msdfgen](https://github.com/Chlumsky/msdfgen)    | 1.12.1 | MTSDF font generation | `MIT` | Viktor Chlumsky ([Chlumsky](https://github.com/Chlumsky)) | 
| [harfbuzz](https://github.com/harfbuzz/harfbuzz)  | 12.0.0 | Font file loading | `MIT-Modern-Variant` | Behdad Esfahbod ([behdad](https://github.com/behdad)) |
| [libktx][ktx-software]                            | 1.3    | Image loading and decoding (KTX2 only) | `Apache-2.0` | [Khronos Group](https://www.khronos.org/) [Mark Callow ([MarkCallow](https://github.com/MarkCallow))] |

<h3>Compile Time</h3>

In addition to the above dependencies, the following are also needed at compile time.

> Only **direct** dependencies are listed; They may require dependencies of their own not listed here.

| Name | Minimum Version | Purpose | License | Primary Author(s) |
| ---- | --------------- | ------- | ------- | ----------------- |
| [zsimd](https://github.com/open-lite/zsimd)   | master branch | Runtime SIMD functions | `MPL-2.0` | [OpenLite](https://github.com/open-lite) [Artin Alavi ([Arastais](https://github.com/Arastais))]
| [result](https://github.com/open-lite/result) | master branch | Rich result            | `MPL-2.0` | [OpenLite](https://github.com/open-lite) [Artin Alavi ([Arastais](https://github.com/Arastais))]

<h2>Supported Platforms</h2>

<h3>Operating Systems</h3>

| Name | Minimum Version Name |
| ---- | -------------------- |
| Windows | 7 |
| Linux | N/A (graphics driver dependent) |
| BSD | N/A (graphics driver dependent) |
| Horizon (Nintendo Switch) | 15.0.0<sup>1</sup> |
| Android | 13 |

<h3>Instruction Set Architectures</h3>

In general, all modern 64-bit ISAs are supported.

The following additionally have optional support for runtime dispatching of SIMD extensions/vector intrinsics and are tested and verified:

| ISA Name    | Required Microarchitecture | Optional Extensions | 
| ----------- | ------------------- | ------------------- |
| x86         | x86-64-v1              | POPCNT, SSE4.2, AVX, AVX2, FMA, BMI1, BMI2, AVX512F, AVX512BW, AVX512DQ, AVX10 |
| ARM AArch64 | -                      | SVE, SVE2 |
| RISC-V      | RV64IMAFD or RV64EMAFD | B, P, C |



> <sup>1</sup>This is an educated guess made based on the date of the official [Vulkan Conformant Products submission][switch-vulkan-conformance] and the [system update history for the Nintendo Switch][switch-update-history]

<h3>Devices</h3>

The client's graphics device must at least support Vulkan 1.3.

The minimum microarchitecture (along with their associated devices) of each vendor that meets these requirements is listed below.
> This table assumes that the client is using the latest possible graphics driver.
>
> On Linux, the Mesa driver is assumed to be used (RADV for AMD and NVK for Nvidia). However, even if using the proprietary Nvidia driver on Linux, the table below is still accurate.

| GPU Vendor  | Platforms              | Microarchitecture Name         | Desktop Devices | Mobile Devices |
| ----------  | ---------------------- | -------------------            | --------------- | -------------- |
| Nvidia      | Windows, Linux, Switch | Maxwell                        | GeForce GTX 745, GTX 750 [Ti]; GeForce 900 series | GeForce 830M, 840M, 845M, GTX 850M; GeForce 900M series |
| AMD         | Linux                  | GCN 1 (Sea Islands)            | Radeon HD 7000 series | Radeon HD 7000M series |
| AMD         | Windows                | GCN 4 (Arctic Islands/Polaris) | Radeon 400 series | Radeon M400 series | 
| Qualcomm    | Android, Linux         | N/A                            | - | Adreno 710, 720, 722, 732, 735, 740, 750; Adreno 800 series |
| Arm Limited | Android                | Valhall 4th gen                | - | Mali G615, G715; Immortalis G715 |
| Arm Limited | Linux                  | Bifrost 1st Gen                | - | Mali G31, G51, G71 |
| Broadcomm   | Linux                  | VideoCore VI                   | - | BCM2711 | 


<h3>Compilers</h3>

Only GCC and Clang (and its derivatives) are officially supported; The MSVC compiler is not.

The compiler must support at least C++20, OpenMP, and gnu-style attributes.

| Compiler | Minimum Version |
| -------- | --------------- |
| GCC      | 10 |
| Clang    | 10 |
| ICX      | 2022.1 |
| AOCC     | 2.3.0 |

<h2>License</h2>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 

[ktx-software]: https://github.com/KhronosGroup/KTX-Software
[switch-vulkan-conformance]: https://www.khronos.org/conformance/adopters/conformant-products#submission_693
[switch-update-history]: https://en-americas-support.nintendo.com/app/answers/detail/a_id/43314/session/L2F2LzEvdGltZS8xNzU5NTM0MTY4L2dlbi8xNzU5NTM0MTY4L3NpZC9mVTJzU3RlY1pQRmljb244ZUdDQkUlN0VnM1A1UnBubUpvX0IlN0VXeUhoV05RNWNqWlVSSkVHcFFSWVFsOUlkaiU3RW1nS3N4U055aGNLMUhqblNGazlibFZFOThibzJBWkU2SkJTdndZUmYzS0QwayU3RUpMYl9wQlFhS0ZydyUyMSUyMQ%3D%3D
