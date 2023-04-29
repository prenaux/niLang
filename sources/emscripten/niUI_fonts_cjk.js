console.log("I/js: niLang/niUI_fonts_cjk");
niAssert(NIAPP_FSMODULES && NIAPP_FSMODULES['niUI'] !== undefined, 'FSModule "niUI" must be defined first');
niAssert(NIAPP_FSMODULES['niUI_fonts_cjk'] == undefined, 'FSModule "niUI_fonts_cjk" already defined');

NIAPP_FSMODULES['niUI_fonts_cjk'] = function FS_niUI_fonts_cjk() {
  console.log("FSModule: niUI_fonts_cjk");
  var dir = "/Work/niLang/data/niUI";
  var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/niUI");
  niFS_AddFiles(dir, url, [
    // for Unicode characters, ~18MB :(
    "fonts/NotoSansCJKsc-Regular.otf",
  ]);
};
