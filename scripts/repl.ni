// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")
::Import("algo.ni")
::Import("concurrent.ni")

::REPL_SetRunCallback(::concurrent.newCallback(#(toRun,code) {
  // ::dbg("... REPL Run:" code)
  toRun.Run()
}))

function ::REPL_PrintResult(aResult) {
  // ::dbg("... REPL PrintResult:" aResult)
  ::println(aResult)
}
