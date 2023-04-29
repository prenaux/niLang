console.log("I/js: niLang/Test_niLang_pre");
niAssert(NIAPP_FSMODULES);
niAssert(NIAPP_FSMODULES['Test_niLang'] == undefined, 'FSModule "Test_niLang" already defined');

// dirs: find "$WORK/niLang/data/Test_niLang" -type d | sed "s~$WORK/niLang/data/Test_niLang/~~g" | sort
// files: find "$WORK/niLang/data/Test_niLang" -type f | sed "s~$WORK/niLang/data/Test_niLang/~~g" | sort
NIAPP_FSMODULES['Test_niLang'] = function FS_Test_niLang() {
  console.log("FSModule: Test_niLang");
  var dir = "/Work/niLang/data/Test_niLang";
  var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/Test_niLang/");
  niFS_MakeDir(dir);
  niFS_AddFiles(dir, url, [
    "hello-file-system.txt"
  ]);
}
