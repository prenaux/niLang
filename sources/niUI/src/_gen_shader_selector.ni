//
// ni _gen_shader_selector.ni
//
// This is meant to be called after you added & compiled new light shader types
//
::Import("lang.ni");
::Import("tools/gen_shader_selector.ni")

local shadersDir = ::fs.getAbsolutePath("../../../data/niUI/shaders/");
::gen_shader_selector.genShaderSelector(shadersDir, "fixed_ps", false);
