console.log("I/js: niLang/niUI_pre");
niAssert(NIAPP_FSMODULES);
niAssert(NIAPP_FSMODULES['niUI'] == undefined, 'FSModule "niUI" already defined');

// dirs: find "$WORK/niLang/data/niUI" -type d | sed "s~$WORK/niLang/data/niUI/~~g" | sort | awk '{print "\"" $0 "\","}'
// files: find "$WORK/niLang/data/niUI" -type f | sed "s~$WORK/niLang/data/niUI/~~g" | sort | awk '{print "\"" $0 "\","}'
NIAPP_FSMODULES['niUI'] = function FS_niUI() {
  console.log("FSModule: niUI");
  var dir = "/Work/niLang/data/niUI";
  var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/niUI");
  FS.mkdir(dir);
  niFS_AddDirs(dir, [
    "skins",
    "shaders",
    "fonts",
  ]);
  niFS_AddFiles(dir, url, [
    "error.dds",
    "fonts/NotoMono-Regular.ttf",
    // "fonts/NotoSansCJKsc-Regular.otf", // for Unicode characters, ~18MB :(
    "fonts/PlayfairDisplay-Bold.ttf",
    "fonts/PlayfairDisplay-BoldItalic.ttf",
    "fonts/PlayfairDisplay-Italic.ttf",
    "fonts/PlayfairDisplay-Regular.ttf",
    "fonts/Roboto-Bold.ttf",
    "fonts/Roboto-BoldItalic.ttf",
    "fonts/Roboto-Italic.ttf",
    "fonts/Roboto-Regular.ttf",
    "fonts/fab-400.otf",
    "fonts/far-400.otf",
    "fonts/fas-900.otf",
    "loading.tga",
    "shaders/fixed_ps.cgo",
    "shaders/fixed_ps_alphatest.cgo",
    "shaders/fixed_ps_base.cgo",
    "shaders/fixed_ps_base_alphatest.cgo",
    "shaders/fixed_ps_base_distancefield.cgo",
    "shaders/fixed_ps_base_opacity_alphatest.cgo",
    "shaders/fixed_ps_base_opacity_translucent.cgo",
    "shaders/fixed_ps_base_translucent.cgo",
    "shaders/fixed_ps_decal.cgo",
    "shaders/fixed_ps_decal_alphatest.cgo",
    "shaders/fixed_ps_decal_base.cgo",
    "shaders/fixed_ps_decal_base_alphatest.cgo",
    "shaders/fixed_ps_decal_base_distancefield.cgo",
    "shaders/fixed_ps_decal_base_opacity_alphatest.cgo",
    "shaders/fixed_ps_decal_base_opacity_translucent.cgo",
    "shaders/fixed_ps_decal_base_translucent.cgo",
    "shaders/fixed_ps_decal_distancefield.cgo",
    "shaders/fixed_ps_decal_opacity_alphatest.cgo",
    "shaders/fixed_ps_decal_opacity_translucent.cgo",
    "shaders/fixed_ps_decal_translucent.cgo",
    "shaders/fixed_ps_distancefield.cgo",
    "shaders/fixed_ps_opacity_alphatest.cgo",
    "shaders/fixed_ps_opacity_translucent.cgo",
    "shaders/fixed_ps_translucent.cgo",
    "shaders/fixed_vs_p.cgo",
    "shaders/fixed_vs_pa.cgo",
    "shaders/fixed_vs_pb4inat1.cgo",
    "shaders/fixed_vs_pb4int1.cgo",
    "shaders/fixed_vs_pn.cgo",
    "shaders/fixed_vs_pna.cgo",
    "shaders/fixed_vs_pnat1.cgo",
    "shaders/fixed_vs_pnat2.cgo",
    "shaders/fixed_vs_pnt1.cgo",
    "shaders/fixed_vs_pnt2.cgo",
    "skins/UIDark.png",
    "skins/default.uiskin.xml",
  ]);
};
