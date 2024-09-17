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
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.xy);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.rg);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[0]);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[1]);
  v[2];
  v.xyz;

  // should succeed
  v.x = 1;
  v.y = 2;
  // should fail
  v.z = 3;
  v.xy = 4;
}

function del_vec3(Vec3 v) {
  v.thehamster(1,2,3)
  v.normalize()
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.xy);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.rg);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",v.xyz);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",v.rgb);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[0]);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[1]);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[2]);
  v[3];
  v.xyzw;

  // should succeed
  v.x = 1;
  v.y = 2;
  v.z = 3;
  // should fail
  v.w = 3;
  v.xy = 4;
}

function del_vec4(Vec4 v) {
  v.thehamster(1,2,3)
  v.normalize()
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.xy);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.rg);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",v.xyz);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",v.rgb);
  ::LINT_CHECK_TYPE("resolved_type<Vec4>",v.xyzw);
  ::LINT_CHECK_TYPE("resolved_type<Vec4>",v.rgba);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[0]);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[1]);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[2]);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[3]);
  ::LINT_CHECK_TYPE("resolved_type<float>",v[3]);
  v[4];
  v.xyzww;

  // should succeed
  v.x = 1;
  v.y = 2;
  v.z = 3;
  v.w = 4;
  v.width = 1;
  v.height = 1;
  v.size = Vec2(1);
  v.center = Vec2(1);
  v.top_left = Vec2(1);
  v.top_right = Vec2(1);
  v.bottom_left = Vec2(1);
  v.bottom_right = Vec2(1);
  v.top = 1;
  v.bottom = 1;
  v.left = 1;
  v.right = 1;
  ::LINT_CHECK_TYPE("resolved_type<float>",v.width);
  ::LINT_CHECK_TYPE("resolved_type<float>",v.height);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.size);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.center);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.top_left);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.top_right);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.bottom_left);
  ::LINT_CHECK_TYPE("resolved_type<Vec2>",v.bottom_right);
  ::LINT_CHECK_TYPE("resolved_type<float>",v.top);
  ::LINT_CHECK_TYPE("resolved_type<float>",v.bottom);
  ::LINT_CHECK_TYPE("resolved_type<float>",v.left);
  ::LINT_CHECK_TYPE("resolved_type<float>",v.right);

  // should fail
  v.xy = 4;
}

function del_matrix(Matrix m) {
  m.thehamster(1,2,3)
  m.transpose()
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.up);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.right);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.forward);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.translation);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._11);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._12);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._13);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._14);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._21);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._32);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._43);
  ::LINT_CHECK_TYPE("resolved_type<float>",m._44);
  ::LINT_CHECK_TYPE("resolved_type<float>",m[0]);
  ::LINT_CHECK_TYPE("resolved_type<float>",m[1]);
  ::LINT_CHECK_TYPE("resolved_type<float>",m[15]);
  ::LINT_CHECK_TYPE("resolved_type<float>",m[0]);
  m[16];
  m._111;
  m._01;
  m._15;

  // should succeed
  m._11 = 1;
  m._21 = 2;
  m._44 = 3;
  m.right = ::Vec3();
  m.up = ::Vec3();
  m.forward = ::Vec3();
  m.translation = ::Vec3();
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.up);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.right);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.forward);
  ::LINT_CHECK_TYPE("resolved_type<Vec3>",m.translation);
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

function del_intf_lang_delegate() {
  local dr = ::GetLangDelegate("iunknown:iDataTableReadStack")
  dr.GetEnumDefault = function() {
    ::LINT_CHECK_TYPE("table:iDataTableReadStack", this);
    return this.top;
  }
  dr.GetFlagsDefault <- function() {
    ::LINT_CHECK_TYPE("table:iDataTableReadStack", this);
    return this.top;
  }

  local dw = ::GetLangDelegate("iunknown:iDataTableWriteStack")
  dw.SetEnum = function() {
    ::LINT_CHECK_TYPE("table:iDataTableWriteStack", this);
    return this.top;
  }
  dw.SetFlagsWhatever <- function() {
    ::LINT_CHECK_TYPE("table:iDataTableWriteStack", this);
    return this.top;
  }

  local dt = ::LINT_AS_TYPE("iDataTable", null)
  dt.SetName("123")
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
