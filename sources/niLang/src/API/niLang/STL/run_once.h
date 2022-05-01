#pragma once
#ifndef __RUN_ONCE_H_DC5BEB53_C5AD_C74F_8A51_9BC096C155DB__
#define __RUN_ONCE_H_DC5BEB53_C5AD_C74F_8A51_9BC096C155DB__

#include "atomic.h"

#define niRunOnce \
  for (static astl::atomic<int> _already_ran_(false); !_already_ran_.fetch_or(true);)

#endif // __RUN_ONCE_H_DC5BEB53_C5AD_C74F_8A51_9BC096C155DB__
