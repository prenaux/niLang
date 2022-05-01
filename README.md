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

# Tips

## Working on a UI testcase script

- Add a UI script in `niUI/tsrc_ni/TESTNAME.niui`
- Run the UI test with `ham Run_Test_niUI_TESTNAME`
- Press F9 while the test is running to reload the code

Or

- Add a UI script in `scripts/forms/TESTNAME.niui`
- Run the UI test with `ham Run_Test_niUI_forms_TESTNAME`
- Press F9 while the test is running to reload the code
