#include "stdafx.h"

// The class we are implementing
#include "Console.h"
#include "VectorMath.h"

#define FOREGROUND_MASK 0xf
#define BACKGROUND_MASK 0xf0

void Console::Write(const std::string& Value, ...)
{
    // Prepare to write the value to the console
    va_list va;
    va_start(va, Value);
    vprintf(Value.c_str(), va);
    va_end(va);
}

void Console::WriteLine(const std::string& Value, ...)
{
    // Prepare to write the value to the console
    va_list va;
    va_start(va, Value);
    vprintf(Value.c_str(), va);
    printf("\n");
    va_end(va);
}

void Console::SetTitle(const std::string& Title)
{
    // Set the title window
    SetConsoleTitleA(Title.c_str());
}

std::string Console::GetTitle()
{
    // Build the title
    char Title[MAX_PATH + 1];
    std::memset(&Title[0], 0, MAX_PATH + 1);

    // Get it
    GetConsoleTitleA(&Title[0], MAX_PATH);

    // Return it
    return std::string(Title);
}

std::string Console::ReadLine()
{
    // Read the entire next line
    std::string Result = "";
    std::getline(std::cin, Result);
    return Result;
}

int Console::ReadKey()
{
    return _getch();
}

void Console::SetBackgroundColor(ConsoleColor Color)
{
    // Convert to native color
    auto NativeColor = ConsoleColorToNative(Color, true);
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get current configuration
    CONSOLE_SCREEN_BUFFER_INFO cBuffer;
    GetConsoleScreenBufferInfo(hStdOut, &cBuffer);

    // Mask off the current background and apply our own
    int16_t Attrs = cBuffer.wAttributes;
    Attrs &= ~((int16_t)BACKGROUND_MASK);
    Attrs = (int16_t)(((uint32_t)(uint16_t)Attrs) | ((uint32_t)(uint16_t)NativeColor));

    // Set the new configuration
    SetConsoleTextAttribute(hStdOut, Attrs);
}

void Console::SetForegroundColor(ConsoleColor Color)
{
    // Convert to native color
    auto NativeColor = ConsoleColorToNative(Color, false);
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get current configuration
    CONSOLE_SCREEN_BUFFER_INFO cBuffer;
    GetConsoleScreenBufferInfo(hStdOut, &cBuffer);

    // Mask off the current foreground and apply our own
    int16_t Attrs = cBuffer.wAttributes;
    Attrs &= ~((int16_t)FOREGROUND_MASK);
    Attrs = (int16_t)(((uint32_t)(uint16_t)Attrs) | ((uint32_t)(uint16_t)NativeColor));

    // Set the new configuration
    SetConsoleTextAttribute(hStdOut, Attrs);
}

ConsoleColor Console::GetBackgroundColor()
{
    // Get handle
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get current configuration
    CONSOLE_SCREEN_BUFFER_INFO cBuffer;
    GetConsoleScreenBufferInfo(hStdOut, &cBuffer);

    // Apply mask to get the color value
    auto Attr = (int16_t)(cBuffer.wAttributes & (int16_t)BACKGROUND_MASK);

    // Convert and return
    return NativeToConsoleColor(Attr);
}

ConsoleColor Console::GetForegroundColor()
{
    // Get handle
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get current configuration
    CONSOLE_SCREEN_BUFFER_INFO cBuffer;
    GetConsoleScreenBufferInfo(hStdOut, &cBuffer);

    // Apply mask to get the color value
    auto Attr = (int16_t)(cBuffer.wAttributes & (int16_t)FOREGROUND_MASK);

    // Convert and return
    return NativeToConsoleColor(Attr);
}

void Console::RemapConsoleColor(ConsoleColor ToRemap, COLORREF NewColor)
{
    // Get the current extended info
    CONSOLE_SCREEN_BUFFER_INFOEX cBuffer;
    cBuffer.cbSize = sizeof(cBuffer);
    // Standard output
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get it
    GetConsoleScreenBufferInfoEx(hStdOut, &cBuffer);

    // Change color per type
    cBuffer.ColorTable[(int32_t)ToRemap] = NewColor;

    // Set the new information
    SetConsoleScreenBufferInfoEx(hStdOut, &cBuffer);
}

void Console::SetConsoleTextSize(int Width, int Height, int Weight)
{
    // Get font info
    CONSOLE_FONT_INFOEX cFont;
    cFont.cbSize = sizeof(cFont);
    // Standard output
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Fetch it
    GetCurrentConsoleFontEx(hStdOut, false, &cFont);

    // Apply new data
    cFont.dwFontSize.X = Width;
    cFont.dwFontSize.Y = Height;
    cFont.FontWeight = Weight;

    // Set it
    SetCurrentConsoleFontEx(hStdOut, false, &cFont);
}

void Console::SetConsoleWindowSize(int Width, int Height)
{
    // Get the current extended info
    CONSOLE_SCREEN_BUFFER_INFOEX cBuffer;
    cBuffer.cbSize = sizeof(cBuffer);
    // Standard output
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get it
    GetConsoleScreenBufferInfoEx(hStdOut, &cBuffer);

    // Store buffer size
    COORD dwCoord;
    dwCoord.X = cBuffer.dwSize.X;
    dwCoord.Y = cBuffer.dwSize.Y;
    bool resizeBuffer = false;

    // Perform buffer resize
    if (cBuffer.dwSize.X < cBuffer.srWindow.Left + Width)
    {
        dwCoord.X = (int16_t)(cBuffer.srWindow.Left + Width);
        resizeBuffer = true;
    }
    if (cBuffer.dwSize.Y < cBuffer.srWindow.Top + Height)
    {
        dwCoord.Y = (int16_t)(cBuffer.srWindow.Top + Height);
        resizeBuffer = true;
    }
    if (resizeBuffer)
    {
        SetConsoleScreenBufferSize(hStdOut, dwCoord);
    }

    // Perform window resize
    SMALL_RECT srWindow = cBuffer.srWindow;
    srWindow.Bottom = (int16_t)(srWindow.Top + Height - 1);
    srWindow.Right = (int16_t)(srWindow.Left + Width - 1);

    // Set the size
    SetConsoleWindowInfo(hStdOut, true, &srWindow);
}

void Console::InitializeWraithColorPallete()
{
    // Prepare color map
    RemapConsoleColor(ConsoleColor::Gray, RGB(0x20, 0x20, 0x20));
    RemapConsoleColor(ConsoleColor::Blue, RGB(0x1E, 0x90, 0xFF));
    RemapConsoleColor(ConsoleColor::Red, RGB(0xFF, 0x0, 0x0));
    RemapConsoleColor(ConsoleColor::Yellow, RGB(0xF1, 0x95, 0x0A));
    RemapConsoleColor(ConsoleColor::Green, RGB(0x32, 0xCD, 0x32));

    // Standard output
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    auto LargestSize = GetLargestConsoleWindowSize(hStdOut);

    // Set it up, clamp between largest size!
    SetConsoleWindowSize(std::min<int>(112, LargestSize.X), std::min<int>(30, LargestSize.Y));

    // Make the text default to white!
    Console::SetForegroundColor(ConsoleColor::White);
    Console::SetBackgroundColor(ConsoleColor::Black);

    // Clear the console!
    Clear();
}

void Console::Clear()
{
    COORD coordScreen = { 0 };
    CONSOLE_SCREEN_BUFFER_INFO cBuffer;
    int conSize;

    // Standard output
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(hStdOut, &cBuffer);
    conSize = cBuffer.dwSize.X * cBuffer.dwSize.Y;

    // Fill the console with blanks
    DWORD resultSize = 0;
    FillConsoleOutputCharacterA(hStdOut, ' ', conSize, coordScreen, &resultSize);

    // Set buffer attributes
    resultSize = 0;
    FillConsoleOutputAttribute(hStdOut, cBuffer.wAttributes, conSize, coordScreen, &resultSize);

    // Reset cursor
    SetConsoleCursorPosition(hStdOut, coordScreen);
}

void Console::WriteHeader(const std::string& Header, const std::string& Value, ...)
{
    // Prepare to write the text to the console, formatting the header
    auto CurrentForeground = GetForegroundColor();
    auto CurrentBackground = GetBackgroundColor();

    // Apply header color scheme
    SetForegroundColor(ConsoleColor::Green);
    SetBackgroundColor(ConsoleColor::Gray);

    printf("[%-14s]", Header.c_str());

    // Set standard
    SetForegroundColor(ConsoleColor::White);
    SetBackgroundColor(ConsoleColor::Black);
    
    // Divider for content
    printf(": ");

    // Reset it
    SetForegroundColor(CurrentForeground);
    SetBackgroundColor(CurrentBackground);

    // Prepare the va args
    va_list va;
    va_start(va, Value);
    vprintf(Value.c_str(), va);
    va_end(va);
}

void Console::WriteLineHeader(const std::string& Header, const std::string& Value, ...)
{
    // Prepare to write the text to the console, formatting the header
    auto CurrentForeground = GetForegroundColor();
    auto CurrentBackground = GetBackgroundColor();

    // Apply header color scheme
    SetForegroundColor(ConsoleColor::Green);
    SetBackgroundColor(ConsoleColor::Gray);

    printf("[%-14s]", Header.c_str());

    // Set standard
    SetForegroundColor(ConsoleColor::White);
    SetBackgroundColor(ConsoleColor::Black);

    // Divider for content
    printf(": ");

    // Reset it
    SetForegroundColor(CurrentForeground);
    SetBackgroundColor(CurrentBackground);

    // Prepare the va args
    va_list va;
    va_start(va, Value);
    vprintf(Value.c_str(), va);
    printf("\n");
    va_end(va);
}

int16_t Console::ConsoleColorToNative(ConsoleColor Color, bool isBackground)
{
    int16_t Result = (int16_t)Color;

    // Shift off the foreground
    if (isBackground)
        Result = (int16_t)((int32_t)Color << 4);

    return Result;
}

ConsoleColor Console::NativeToConsoleColor(int16_t Native)
{
    // Turn background colors into foreground colors
    if ((Native & BACKGROUND_MASK) != 0)
        Native = (int32_t)(((int32_t)Native) >> 4);

    return (ConsoleColor)Native;
}