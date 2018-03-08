// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdbool>
#include <algorithm>
#include <limits>

#include "rapidjson\document.h"
#include "rapidjson\filereadstream.h"
#include "SonicRModLoader.h"
#include "MiscFuncts.h"
#include "SonicRFuncts.h"