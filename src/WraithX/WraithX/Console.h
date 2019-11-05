#pragma once

#include <cstdint>
#include <string>

// The built-in system console colors
enum class ConsoleColor : int32_t
{
    Black = 0,
    DarkBlue = 1,
    DarkGreen = 2,
    DarkCyan = 3,
    DarkRed = 4,
    DarkMagenta = 5,
    DarkYellow = 6,
    Gray = 7,
    DarkGray = 8,
    Blue = 9,
    Green = 10,
    Cyan = 11,
    Red = 12,
    Magenta = 13,
    Yellow = 14,
    White = 15
};

// A class that handles reading and writing to the console
class Console
{
public:

    // -- Standard console utilities

    // Writes a value to the console with optional formatting
    static void Write(const std::string& Value, ...);
    // Writes a line to the console with optional formatting
    static void WriteLine(const std::string& Value, ...);

    // Sets the title of the console window
    static void SetTitle(const std::string& Title);
    // Gets the title of the console window
    static std::string GetTitle();

    // Reads the next 'line' of input from the console
    static std::string ReadLine();

    // Reads the next keystroke from the console
    static int ReadKey();

    // Change the background text color of the console
    static void SetBackgroundColor(ConsoleColor Color);
    // Change the foreground text color of the console
    static void SetForegroundColor(ConsoleColor Color);

    // Get the current background text color of the console
    static ConsoleColor GetBackgroundColor();
    // Get the current foreground text color of the console
    static ConsoleColor GetForegroundColor();

    // Remaps an existing console color to the provided color
    static void RemapConsoleColor(ConsoleColor ToRemap, COLORREF NewColor);
    // Change the font size of the console window
    static void SetConsoleTextSize(int Width, int Height, int Weight);
    // Change the window buffer size
    static void SetConsoleWindowSize(int Width, int Height);

    // Clears the console window
    static void Clear();

    // Writes text to the console with optional formatting and a header
    static void WriteHeader(const std::string& Header, const std::string& Value, ...);
    // Writes a line to the console with optional formatting and a header
    static void WriteLineHeader(const std::string& Header, const std::string& Value, ...);

    // -- Wraith specific console utilities

    // Setup the custom Wraith color scheme
    static void InitializeWraithColorPallete();

private:

    // Converts a console color to native color code
    static int16_t ConsoleColorToNative(ConsoleColor Color, bool isBackground);
    // Converts a native color code to a console color
    static ConsoleColor NativeToConsoleColor(int16_t Native);
};