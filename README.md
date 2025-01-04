# Overview

niLang is a C++ library that abstracts a variety of operating system
functionalities, encompassing window management, input handling for keyboards,
mice, and gamepads, filesystem operations, threading, socket communication,
HTTP requests, 2D & 3D rendering, user interfaces, and audio processing.

In addition, niLang incorporates an automated introspection system for C++
modules, interfaces, and enums, facilitating seamless interoperability with
other programming languages. By default, niLang offers integration with
niScript (a Squirrel scripting language derivative) and the Java Virtual
Machine (JVM).

# Setup the environment

```
cd niLang
. hat
```

# Build

## Bootstrap

```
./_build_bootstrap.sh
```

## Module and libraries

```
ham pass1 modlibs
```

## A particular module

```
ham MODULENAME
```

## Build in debug mode

```
ham BUILD=da MODULENAME
```

# WebJS / Emscripten version

Build Test\_niLang:
```
BUILD_TARGET=web-js . hat
ham pass1 && ham Test_niLang
```

Run Test\_niLang:
```
# Serve the $WORK folder (niLang's parent folder)
cd "$WORK"
simple-http-server
# Open the test case
open http://localhost:8123/niLang/bin/web-js/Test_niLang_ra.html
```

# Build & edit shaders

All shaders are precompiled in .cgo files which are binary dependencies of the libraries. Building shaders is an explicit build step because its really its own thing and most will not need to touch them. A lot of shaders can also take a long time to build.

On Windows the shader compiler is enabled by default. On Linux & macOS it is disabled by default because it needs Wine to run which is a fairly big dependency.

To enable the shader compiler set the envvar `SHADER_COMPILE=1` before importing the toolset. For example:
```
cd niLang ; export SHADER_COMPILE=1 ; . hat
```

To build the shaders build the shader target:
```
ham shaders
````

You can add the shader target before other targets to build the shaders first:
```
hamx :niLang shaders Run_HelloUI_cpp
```

You can build single shaders while iterating:
```
# Build all the texture sampling shaders
ham niUI_shader_fixed_ps_base niUI_shader_fixed_ps_base_translucent Run_HelloUI_cpp
```

You can list the shader targets with:
```
ham-ls-targets | grep shader
```

# Tips

## Working on a UI testcase script

- Add a UI script in `niUI/tsrc_ni/TESTNAME.niui`
- Run the UI test with `ham Run_Test_niUI_TESTNAME`
- Press F9 while the test is running to reload the code

Or

- Add a UI script in `scripts/forms/TESTNAME.niui`
- Run the UI test with `ham Run_Test_niUI_forms_TESTNAME`
- Press F9 while the test is running to reload the code

# License

niLang is distributed under the MIT License; the full license text can be
found in `LICENSE.md`. This project incorporates code from other projects, some
of which are licensed under different terms than MIT. Generally, these
licenses are similar to the MIT License in terms of practical
implications. For more information on third-party licenses, refer to the
`docs/thirdparty` folder.
