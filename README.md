# Overview

niLang a C++ library to abstract the OS: Window management, Input -
Keyboard/Mouse/Gamepads, Filesystem, Threading, Socket, Http requests,
Graphics & Audio. It includes an automated way to do introspection on C++
modules/interfaces/enums which is notably leveraged to automate interop with
other languages, out of the box with niScript (a derivative of the squirrel
scripting language) & the JVM.

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

niLang is available under the MIT License, see the full license text in
`LICENSE.md`. This project redistributes code from other projects, some of
which have other licenses besides MIT. Such licenses are generally similar to
the MIT license for practical purposes. See the `docs/thirdparty/` folder for
more details.
