#include "stdafx.h"

struct FStartPath {
};

TEST_FIXTURE(FStartPath,OpenUrl) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    ni::GetLang()->StartPath("https://bitscollider.com/");
  }
}
