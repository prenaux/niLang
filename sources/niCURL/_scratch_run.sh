#!/bin/bash -ex
export BUILD=da
# hamx :niLang Test_niCURL
# hamx :niLang codegen Run_Test_niCURL FIXTURE=FCURLGet
# hamx :niLang codegen Debug_Test_niCURL FIXTURE=FCURLGet,ChunkedTransfer
hamx :niLang codegen Run_Test_niCURL FIXTURE=FCURLGet,ChunkedTransfer

