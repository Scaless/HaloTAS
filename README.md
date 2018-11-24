# HaloTAS
Halo: Combat Evolved PC Speedrunning Tools

#### Goals:
- Have fun
- Go fast
- Beat [human world record](https://haloruns.com/records?lb=0100) time

![Screenshot][screenshot1]

#### Requirements to compile yourself:
- Windows 10
    - [Build 15063, aka version 1703, aka Creator's Update](https://en.wikipedia.org/wiki/Windows_10_version_history#Rings) or higher
- Visual Studio 2017
    - ".NET desktop development" component installed
    - .NET 4.6.1 or later
- Retail Halo:CE Installed
    - Updated to [01.00.10.0621](http://halo.bungie.net/images/games/halopc/patch/110/halopc-patch-1.0.10.exe)
- Graphics card/driver capable of OpenGL 3.2 or higher
- [Boost](https://www.boost.org/users/download/) v1.68 or higher
    - Default location: C:\boost_1_68_0
	- If installing boost elsewhere, must change project include paths to point to boost install location

#### Made possible with the following projects:

- [GLFW - A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input ][glfw]
- [GL3W - Simple OpenGL core profile loading][gl3w]
- [Dear ImGui - Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies][imgui]
- [GLM - OpenGL Mathematics][glm]

 [glfw]: <https://github.com/glfw/glfw>
 [gl3w]: <https://github.com/skaslev/gl3w>
 [imgui]: <https://github.com/ocornut/imgui>
 [glm]: <https://github.com/g-truc/glm>
 
 [screenshot1]: https://i.imgur.com/Fj2gOWp.png
