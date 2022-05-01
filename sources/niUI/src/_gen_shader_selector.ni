//
// ni _gen_shader_selector.ni
//
// This is meant to be called after you added & compiled new light shader types
//
::Import("lang.ni");
local genShaderSelector = ::ImportModule("tools/gen_shader_selector.ni").genShaderSelector;

local shadersDir = ::fs.getAbsolutePath("../../../data/niUI/shaders/");
genShaderSelector(shadersDir, "fixed_ps");
