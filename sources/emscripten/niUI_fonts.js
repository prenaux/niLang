console.log("I/js: niLang/niUI_fonts");
niAssert(NIAPP_FSMODULES && NIAPP_FSMODULES['niUI'] !== undefined, 'FSModule "niUI" must be defined first');
niAssert(NIAPP_FSMODULES['niUI_fonts'] == undefined, 'FSModule "niUI_fonts" already defined');

NIAPP_FSMODULES['niUI_fonts'] = function FS_niUI_fonts() {
  console.log("FSModule: niUI_fonts");
  var dir = "/Work/niLang/data/niUI";
  var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/niUI");
  niFS_AddFiles(dir, url, [
    "fonts/NotoMono-Regular.ttf",
    "fonts/PlayfairDisplay-Bold.ttf",
    "fonts/PlayfairDisplay-BoldItalic.ttf",
    "fonts/PlayfairDisplay-Italic.ttf",
    "fonts/PlayfairDisplay-Regular.ttf",
    "fonts/Roboto-Bold.ttf",
    "fonts/Roboto-BoldItalic.ttf",
    "fonts/Roboto-Italic.ttf",
    "fonts/fab-400.otf",
    "fonts/far-400.otf",
    "fonts/fas-900.otf",
  ]);
};
