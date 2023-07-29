#include <niLang.h>
#include <stdio.h>

using namespace ni;

int main(int argc, char** argv) {
  tUUID uuid = UUIDGenGlobal();
  cString r;
  if (argc == 2 && argv[1][0] == '2') {
    r.Format(
      "0x%08x,0x%04x,0x%04x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
      (unsigned int)uuid.nData1,(unsigned int)uuid.nData2,(unsigned int)uuid.nData3,
      uuid.nData4[0],uuid.nData4[1],uuid.nData4[2],uuid.nData4[3],
      uuid.nData4[4],uuid.nData4[5],uuid.nData4[6],uuid.nData4[7]);
  }
  else if (argc == 2 && argv[1][0] == '3') {
    r.Format(
      "%08X_%04X_%04X_%02X%02X_%02X%02X%02X%02X%02X%02X",
      (unsigned int)uuid.nData1,(unsigned int)uuid.nData2,(unsigned int)uuid.nData3,
      uuid.nData4[0],uuid.nData4[1],uuid.nData4[2],uuid.nData4[3],
      uuid.nData4[4],uuid.nData4[5],uuid.nData4[6],uuid.nData4[7]);
  }
  else {
    r.Format(
      "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      (unsigned int)uuid.nData1,(unsigned int)uuid.nData2,(unsigned int)uuid.nData3,
      uuid.nData4[0],uuid.nData4[1],uuid.nData4[2],uuid.nData4[3],
      uuid.nData4[4],uuid.nData4[5],uuid.nData4[6],uuid.nData4[7]);
  }
  ni::GetStdOut()->WriteString(r.Chars());
  return 0;
}
