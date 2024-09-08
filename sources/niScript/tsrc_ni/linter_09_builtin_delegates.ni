::Import("niLang")

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
  {
    local v2 = ::Vec2();
    ::LINT_CHECK_TYPE("resolved_type<Vec2>", v2);
  }

  {
    local v3 = ::Vec3();
    ::LINT_CHECK_TYPE("resolved_type<Vec3>", v3);
  }

  {
    local rgb = ::RGB();
    ::LINT_CHECK_TYPE("resolved_type<Vec3>", rgb);
  }

  {
    local v4 = ::Vec4();
    ::LINT_CHECK_TYPE("resolved_type<Vec4>", v4);
  }

  {
    local r = ::Rect();
    ::LINT_CHECK_TYPE("resolved_type<Vec4>", r);
  }

  {
    local p = ::Plane();
    ::LINT_CHECK_TYPE("resolved_type<Vec4>", p);
  }

  {
    local m = ::Matrix();
    ::LINT_CHECK_TYPE("resolved_type<Matrix>", m);
  }

  {
    local uuid = ::UUID();
    ::LINT_CHECK_TYPE("resolved_type<UUID>", uuid);
  }
}
