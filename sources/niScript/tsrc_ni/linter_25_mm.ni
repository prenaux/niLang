::Import("niLang")

local __lint = {
  _all = 1
  _experimental = 1
  _pedantic = 0
}

/// Vec2 /////////////////////////////////////////////////////////////////
function mm_Vec2_add(Vec2 a, Vec2 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", b)
  local c = a + b;
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", c)
  local l = c.length();
  ::LINT_CHECK_TYPE("resolved_type<float>", l)
  local d = c.dot(a);
  ::LINT_CHECK_TYPE("resolved_type<float>", d)
}

function mm_Vec2_sub(Vec2 a, Vec2 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", b)
  local c = a - b;
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", c)
}

function mm_Vec2_mul(Vec2 a, Vec2 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", b)
  local c = a * b;
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", c)
}

function mm_Vec2_div(Vec2 a, Vec2 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", b)
  local c = a / b;
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", c)
}

function mm_Vec2_modulo(Vec2 a, Vec2 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", b)
  local c = a % b;
  ::LINT_CHECK_TYPE("resolved_type<Vec2>", c)
}

/// Vec3 /////////////////////////////////////////////////////////////////
function mm_Vec3_add(Vec3 a, Vec3 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", b)
  local c = a + b;
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", c)
  local l = c.length();
  ::LINT_CHECK_TYPE("resolved_type<float>", l)
  local d = c.dot(a);
  ::LINT_CHECK_TYPE("resolved_type<float>", d)
}

function mm_Vec3_sub(Vec3 a, Vec3 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", b)
  local c = a - b;
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", c)
}

function mm_Vec3_mul(Vec3 a, Vec3 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", b)
  local c = a * b;
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", c)
}

function mm_Vec3_div(Vec3 a, Vec3 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", b)
  local c = a / b;
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", c)
}

function mm_Vec3_modulo(Vec3 a, Vec3 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", b)
  local c = a % b;
  ::LINT_CHECK_TYPE("resolved_type<Vec3>", c)
}

/// Vec4 /////////////////////////////////////////////////////////////////
function mm_Vec4_add(Vec4 a, Vec4 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", b)
  local c = a + b;
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", c)
  local l = c.length();
  ::LINT_CHECK_TYPE("resolved_type<float>", l)
  local d = c.dot(a);
  ::LINT_CHECK_TYPE("resolved_type<float>", d)
}

function mm_Vec4_sub(Vec4 a, Vec4 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", b)
  local c = a - b;
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", c)
}

function mm_Vec4_mul(Vec4 a, Vec4 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", b)
  local c = a * b;
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", c)
}

function mm_Vec4_div(Vec4 a, Vec4 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", b)
  local c = a / b;
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", c)
}

function mm_Vec4_modulo(Vec4 a, Vec4 b) {
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", a)
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", b)
  local c = a % b;
  ::LINT_CHECK_TYPE("resolved_type<Vec4>", c)
}

/// Matrix /////////////////////////////////////////////////////////////////
function mm_Matrix_add(Matrix a, Matrix b) {
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", a)
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", b)
  local c = a + b;
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", c)
  local d = c.Determinant();
  ::LINT_CHECK_TYPE("resolved_type<float>", d)
  local m = c.Multiply(a);
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", m)
}

function mm_Matrix_sub(Matrix a, Matrix b) {
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", a)
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", b)
  local c = a - b;
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", c)
}

function mm_Matrix_mul(Matrix a, Matrix b) {
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", a)
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", b)
  local c = a * b;
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", c)
}

function mm_Matrix_div(Matrix a, Matrix b) {
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", a)
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", b)
  local c = a / b;
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", c)
}

function mm_Matrix_modulo(Matrix a, Matrix b) {
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", a)
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", b)
  local c = a % b;
  ::LINT_CHECK_TYPE("resolved_type<Matrix>", c)
}
