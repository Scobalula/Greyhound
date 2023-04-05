#pragma once
#pragma message("Pre-compiling headers.\n")

#define WIN32_LEAN_AND_MEAN // Prevent winsock2 redefinition.
#include <windows.h>
#include <WinSock2.h>
#include <shellapi.h>

#define _CRT_USE_WINAPI_FAMILY_DESKTOP_APP
#include <crtdefs.h>
#include <process.h>

#include <stdio.h>
#include <Psapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <emmintrin.h>
#include <cmath>
#include <vector>
#include <cstdint>
#include <array>
#include <regex>
#include <map>
#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <filesystem>
#include <array>
#include <set>

#include "ExportManager.h"
#include "Utils.h"
#include "Logger.h"

typedef unsigned short uint16;