local __lint = {
  _all = 1
  _experimental = 1
}

::Import("niLang")

function main() void {
  local dtType1 = ::eDataTablePropertyType.Int;
  local dtType2 = ::eDataTablePropertyType.FluffyHamster; // should fail

  ::Import("niCURL")
  local curl = ::CreateInstance("niCURL.CURL")
  local curlMsg = ::eCURLMessage
  local curlMsgYey = curlMsg.Started
  local curlMsgNey = curlMsg.StartedHamster // should fail
  ::LINT_CHECK_TYPE("enum_def<eCURLMessage>", curlMsg)
}
