# HaloTAS
Halo: Combat Evolved PC Speedrunning Tools

Blog stuff: https://scaless.github.io/HaloTAS/

Development has halted for the TAS of the 2003 version of the game. All new efforts will be on the Master Chief Collection (MCCTAS).

Video Demo: https://www.youtube.com/watch?v=AEBJUeXLTlo

[Engine Documentation](https://docs.google.com/document/d/1ED6EnvpQ_c7rSdS5oY5-EKQLF_58TptMr9A5uNV8aSs/edit?usp=sharing)

#### Goals:
- Have fun
- Go fast
- Beat [human world record](https://haloruns.com/records/solo/hce) time

### MCCTAS vs. TASDLL

MCCTAS is the latest version of the project aimed to create a TAS for the Master Chief Collection version of the game.

TASDLL was the original project used to develop a TAS for the 2003 Gearbox port of the game. It is no longer being actively maintained.

#### Compiling MCCTAS requires:
- Visual Studio 2019 with the following components installed:
  - "Desktop development with C++"
  - Windows 10 SDK
  - .NET desktop development
- [Boost](https://www.boost.org/users/download/) v1.77 or higher (sorry)
  - Default location: C:\boost_1_77_0
- [.NET Core 3.1 SDK](https://dotnet.microsoft.com/download/dotnet-core/3.1)

#### Compiling TASDLL requires:
- Windows 10 (Might still work on 7/8 I just haven't tried)
- Visual Studio 2019
- Retail Halo:CE Installed
    - Updated to [01.00.10.0621](http://halo.bungie.net/images/games/halopc/patch/110/halopc-patch-1.0.10.exe)
- Graphics card/driver capable of OpenGL 3.2 or higher
    - NVIDIA or AMD, integrated Intel has problems
- [Boost](https://www.boost.org/users/download/) v1.70 or higher
    - Default location: C:\boost_1_70_0
	- Several binaries are required to be built. To build boost, unzip the boost folder and run bootstrap.bat followed by b2.exe in a command prompt in the boost directory.
- [DirectX 9 SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
	- Default location: C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)

#### Made possible with the following projects:

- [GLFW - A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input ][glfw]
- [GL3W - Simple OpenGL core profile loading][gl3w]
- [Dear ImGui - Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies][imgui]
- [GLM - OpenGL Mathematics][glm]

 [glfw]: <https://github.com/glfw/glfw>
 [gl3w]: <https://github.com/skaslev/gl3w>
 [imgui]: <https://github.com/ocornut/imgui>
 [glm]: <https://github.com/g-truc/glm>
