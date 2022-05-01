#include <niLang.h>
#include <stdio.h>

using namespace ni;

int main(int argc, char** argv) {
	tUUID uuid = UUIDGenGlobal();
  cString hello/* = "Hello World"*/;
  if (argc == 2 && argv[1][0] == '2') {
    printf(
        "0x%08x,0x%04x,0x%04x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
        (unsigned int)uuid.nData1,(unsigned int)uuid.nData2,(unsigned int)uuid.nData3,
        uuid.nData4[0],uuid.nData4[1],uuid.nData4[2],uuid.nData4[3],
        uuid.nData4[4],uuid.nData4[5],uuid.nData4[6],uuid.nData4[7]);
  }
  else if (argc == 2 && argv[1][0] == '3') {
    printf(
        "%08X_%04X_%04X_%02X%02X_%02X%02X%02X%02X%02X%02X",
        (unsigned int)uuid.nData1,(unsigned int)uuid.nData2,(unsigned int)uuid.nData3,
        uuid.nData4[0],uuid.nData4[1],uuid.nData4[2],uuid.nData4[3],
        uuid.nData4[4],uuid.nData4[5],uuid.nData4[6],uuid.nData4[7]);
  }
  else {
    printf(
        "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        (unsigned int)uuid.nData1,(unsigned int)uuid.nData2,(unsigned int)uuid.nData3,
        uuid.nData4[0],uuid.nData4[1],uuid.nData4[2],uuid.nData4[3],
        uuid.nData4[4],uuid.nData4[5],uuid.nData4[6],uuid.nData4[7]);
  }
  fputs(hello.Chars(),stdout);
  fflush(stdout);
	return 0;
}
