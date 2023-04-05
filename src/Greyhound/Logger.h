#pragma once
class Logger
{
public:
	void InitializeLogFile();
	void Info(const char* fmt, ...);
	void Info(std::string msg);

	void Warning(const char* fmt, ...);
	void Warning(std::string msg);
private:
	std::ofstream m_LogFileStream;
};

extern Logger g_Logger;