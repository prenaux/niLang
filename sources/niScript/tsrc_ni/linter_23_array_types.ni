local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

function array_cloned() {
  local b = [1,2,3]
  local a = [b,4,5,6]
  a[0][0] = 123 // should be ok before clone
  a[0][10] // should fail, out of bound
  a[3][10] // should fail, not found in int
  local c = a.DeepClone()
  c[0][0] = 123 // should be ok after clone
  c[0][10] // should fail, out of bound
  c[3][10] // should fail, not found in int
}
