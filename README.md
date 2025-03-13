# TrayImGui #

Template for raylib, imGui and others.

Clone the repository with recurse submodules option from a terminal:
``` shell
git clone --recurse-submodules https://github.com/nephewtom/trayimg
```

## Building ##

### Windows ###

#### Install compiler infrastructure ####

Install [mingw-w64](https://www.mingw-w64.org).
For example via [WinLibs](https://winlibs.com/):
```
winget install BrechtSanders.WinLibs.POSIX.UCRT

or

winget install BrechtSanders.WinLibs.POSIX.UCRT.LLVM 
```
which also includes other useful tools.
If you pick LLVM package you will also get LLVM Compiler infrastructure.

Add the compiler bin/ directory to you Windows PATH.

#### Compile raylib & rlImGui ####

``` shell
.\preBuild.bat
```

#### Compile and run the basic example ####

``` shell
.\build.bat
```

### Linux ###

TODO

### MacOS ###

TODO

