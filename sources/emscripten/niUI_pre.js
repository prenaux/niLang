console.log("I/js: niLang/niUI_pre");
niAssert(NIAPP_FSMODULES);
niAssert(NIAPP_FSMODULES['niUI'] == undefined, 'FSModule "niUI" already defined');

// dirs: find "$WORK/niLang/data/niUI" -type d | sed "s~$WORK/niLang/data/niUI/~~g" | sort | awk '{print "\"" $0 "\","}'
// files: find "$WORK/niLang/data/niUI" -type f | sed "s~$WORK/niLang/data/niUI/~~g" | sort | awk '{print "\"" $0 "\","}'
NIAPP_FSMODULES['niUI'] = function FS_niUI(params) {
  console.log("FSModule: niUI");
  var dir = "/Work/niLang/data/niUI";
  var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/niUI");
  niFS_MakeDir(dir);
  niFS_AddDirs(dir, [
    "skins",
    "shaders",
    "fonts",
  ]);

  var files = [
    "error.dds",
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
    "fonts/Roboto-Regular.ttf",
  ]

  try {
    var cn = params["cn"];
    if (cn) {
      files.push("fonts/NotoSansCJKsc-Regular.otf");
    }
  } catch (e) {
    console.log(e);
  }

  niFS_AddFiles(dir, url, files);
};
