#pragma once

#define _USE_MATH_DEFINES

#include <curl/curl.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <unicode/locid.h>
#include <unicode/normlzr.h>
#include <unicode/translit.h>
#include <unicode/unistr.h>

#ifndef LV_SIMULATOR

#include <sys/unistd.h>

#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <vector>

#include "lib/cJSON/cJSON.h"
#include "lvgl/lvgl.h"

using namespace std;

#include "Callback.h"
#include "Messages.h"
#include "lvgl_events.h"
#include "result.h"
#include "support.h"
#include "fonts.h"
#include "RefCounted.h"

#ifdef LV_SIMULATOR

constexpr char const* BOARD_ID = "5c693da426bbbc593b624654";

#endif

constexpr char const* SRVMAIN = "192.168.178.3";
constexpr char const* FAVORITE_LABEL = "FAVORIET";
constexpr char const* ICU_LOCALE = "nl-NL";
