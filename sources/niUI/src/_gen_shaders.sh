#!/bin/bash
if [ "$1" == "clean" ]; then
    shift
    rm -f ../../../data/niUI/shaders/*.cgl
    rm -f ../../../data/niUI/shaders/*.cgo
fi
if [ "$1" != "noshader" ]; then
    shift
    . hat
    ham niUI_shaders
fi
ni -e _gen_shader_selector.ni
