#include <niLang/Types.h>

#ifdef niPosix
#include <niLang/StringDef.h>
#include <niLang/STL/vector.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

struct sPosixREPL {
  astl::vector<ni::cString> history;

  void addToHistory(const ni::cString& aLine) {
    history.push_back(aLine);
  }

  ni::cString readLine()
  {
    ni::cString line;
    unsigned int cursor = 0;
    int hist_idx = history.size();

    auto replaceLine = [&](const ni::cString& aNewLine) {
      clearLineBuffer(line.size(),cursor);
      line = aNewLine;
      printf("%s",line.c_str());
      cursor = line.size();
    };

    // For every char
    while (true) {
      int c = getCh();
      // Handle backspace key
      if (c == 127) {
        if (cursor > 0) {
          clearLineBuffer(line.size(),cursor);
          line.erase(--cursor,1);
          printf("%s",line.c_str());
          backUpBuffer(line.size()-cursor);
        }
        continue;
      }
      // Handle arrow keys
      if (c == 27) {
        int c2 = getCh();
        if (c2 == 27) {
          // skip the second escape char, its something like Alt+Up/Down which
          // we don't distinguish atm
          c2 = getCh();
        }

        if (c2 == 91) {
          switch (getCh()) {
            //Down Arrow
            case 65:
              if (hist_idx > 0){
                replaceLine(history.at(--hist_idx));
              }
              break;
              // Up Arrow
            case 66:
              if (hist_idx+1 < history.size() ){
                replaceLine(history.at(++hist_idx));
              }
              break;
              // Right Arrow
            case 67:
              if (cursor < line.size())
                putchar(line.at(cursor++));
              break;
              // Left Arrow
            case 68:
              if (cursor > 0) {
                putchar('\b');
                cursor--;
              }
              break;
          }
          continue;
        }
        else if (c2 == 98) {
          // niDebugFmt(("... Alt-Left"));
          continue;
        }
        else if (c2 == 102) {
          // niDebugFmt(("... Alt-Right"));
          continue;
        }
        else if (c2 == 127) {
          // niDebugFmt(("... Alt-Backspace"));
          replaceLine("");
          continue;
        }
      }

      // Handle Ctrl+D
      if (c == 4) {
        replaceLine("");
        continue;
      }
      // Handle Enter key
      else if (c == 10){
        // niDebugFmt(("... ENTER KEY"));
        putchar(10);
        addToHistory(line);
        break;
      }

      // Everything else
      clearLineBuffer(line.size(),cursor);
      line.insert(cursor++, c);
      printf("%s",line.c_str());
      backUpBuffer(line.size()-cursor);
    }

    return line;
  }

  // Backs up n spaces
  void backUpBuffer(int n){
    for (int idx = 0; idx < n; idx++)
      putchar('\b');
  }

  // Print n spaces
  void whitespace(int n){
    for (int idx = 0; idx < n; idx++)
      putchar(' ');
  }

  // Clear line of length len
  void clearLineBuffer(int len){
    clearLineBuffer(len,len);
  }

  // Clear line of length len at position pos
  void clearLineBuffer(int len, int pos){
    backUpBuffer(pos);
    whitespace(len);
    backUpBuffer(len);
  }

  // Hack to get getch to work without cursors
  int getCh() {
    int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
    newt = oldt; /* copy old settings to new settings */
    newt.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediatly */
    ch = getchar(); /* standard getchar call */
    // niDebugFmt(("... c: %d (%x): %c", ch, ch, ch));
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */
    return ch; /*return received char */
  }
};

static ni::cString _currentLine;
static ni::StrCharIt _lineIt(AZEROSTR,(ni::tU32)0,(ni::tSize)0);
static sPosixREPL _posixRepl;

// NOT thread safe
niExportFunc(int) waitForNextTerminalChar() {
  if (_lineIt.is_end()) {
    _currentLine = _posixRepl.readLine();
    _currentLine.appendChar('\n');
    _lineIt = _currentLine.charIt();
  }
  if (_lineIt.is_end()) {
    return -1;
  }
  return _lineIt.next();
}
#endif
