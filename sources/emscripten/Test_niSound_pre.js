console.log("I/js: niLang/Test_niSound_pre");
niAssert(NIAPP_FSMODULES);
niAssert(NIAPP_FSMODULES['Test_niSound'] == undefined, 'FSModule "Test_niSound" already defined');

// dirs: find "$WORK/niLang/data/Test_niSound" -type d | sed "s~$WORK/niLang/data/Test_niSound/~~g"
// files: find "$WORK/niLang/data/Test_niSound" -type f | sed "s~$WORK/niLang/data/Test_niSound/~~g"
NIAPP_FSMODULES['Test_niSound'] = function FS_Test_niSound() {
  console.log("FSModule: Test_niSound");
  var dir = "/Work/niLang/data/Test_niSound";
  var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/Test_niSound/");
  niFS_MakeDir(dir);
  niFS_AddFiles(dir, url, [
    "atmo_loop_01.wav",
    "step_stone5.wav",
    "step_stone4.wav",
    "step_stone1.wav",
    "step_stone3.wav",
    "atmo_loop_01.ogg",
    "step_stone2.wav",
    "music_sketch8.ogg",
    "lobby_chat.wav",
    "lobby_chat.ogg",
    "click.wav",
    "music_punch.ogg",
    "click.ogg",
  ]);
};
