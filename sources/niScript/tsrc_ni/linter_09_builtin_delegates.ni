local __lint = {
  _all = 1
  _pedantic = 0
  _explicit = 1
  ret_type_is_null = 0
}

function del_string(string v) {
  v.thehamster(1,2,3)
  return v.split(":")
}

local lang = ::CreateInstance("niLang.Lang").QueryInterface("iLang")

function del_string_resolved():(lang) string {
  local v = lang.GetProperty("bla");
  if (!(v.?len()))
    return "narf";
  return v;
}

function del_int(int v) {
  v.thehamster(1,2,3)
  return v.ToFloat()
}

function del_float(float v) {
  v.thehamster(1,2,3)
  return v.ToInt()
}

function del_array(array v) {
  v.thehamster(1,2,3)
  return v.append(123)
}

function del_table(table v) {
  v.thehamster(1,2,3)
  return v.DeepClone()
}

function del_closure(closure v) {
  v.thehamster(1,2,3)
  v.GetNumParams()
  v.ACall([])
}

function del_vec2(Vec2 v) {
  v.thehamster(1,2,3)
  v.normalize()
}

function del_vec3(Vec3 v) {
  v.thehamster(1,2,3)
  v.normalize()
}

function del_vec4(Vec4 v) {
  v.thehamster(1,2,3)
  v.normalize()
}

function del_matrix(Matrix v) {
  v.thehamster(1,2,3)
  v.transpose()
}

function del_uuid(UUID v) {
  v.thehamster(1,2,3)
  v._tostring()
}

function del_closure() {
  del_closure.GetNumParams()
  del_closure.SetRoot({})
  del_closure.GetNumFreeVars()
}

function main() {
}
