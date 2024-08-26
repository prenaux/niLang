local __lint = {
  _all = 1
  _pedantic = 1
  _explicit = 1
}

function del_string(string v) {
  v.thehamster(1,2,3)
  return v.split(":")
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

function del_matrix(UUID v) {
  v.thehamster(1,2,3)
  v._tostring()
}

function main() {
}
