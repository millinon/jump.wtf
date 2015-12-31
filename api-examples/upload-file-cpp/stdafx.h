// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include <tchar.h>
#include <windows.h>
#include "targetver.h"
#endif

#include <fstream> // for std::ifstream
#ifdef _DEBUG
#include <iostream>
#endif

#include "curl/curl.h"

#include "json.hpp"
using json = nlohmann::json;

