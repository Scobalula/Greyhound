#pragma once

// Pre-processor defines
#define WIN32_LEAN_AND_MEAN // Prevent extra windows functions
#define _AFXDLL // AFX Shared DLL
#define _WIN32_WINNT 0x0601 // Windows 7+

// Libraries to link
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Gdiplus.lib")
// MFC
#include <afxwin.h>
#include <afxdisp.h>
#include <afxglobals.h>

// Global includes
#include <Gdiplus.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <io.h>
#include <fcntl.h>
#include <comutil.h>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <conio.h>
#include <stack>
#include <exception>
#include <stdio.h>
#include <string>
#include <sstream>
#include <istream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <immintrin.h>
#include <cstdint>
#include <functional>
#include <locale>
#include <thread>
#include <cctype>
#include <stdarg.h>
#include <array>
#include <list>
#include <mutex>
#include <algorithm>
#include <IPHlpApi.h>
#include <locale>
#include <codecvt>

// Remove several windows problems
#undef max
#undef min