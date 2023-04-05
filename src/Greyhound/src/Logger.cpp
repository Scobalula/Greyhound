#include "pch.h"
#include "Logger.h"

std::string AddTimestamp(std::string msg)
{
	return std::string("[" + Utils::GetTimestamp() + "] ") + msg;
}

void Logger::InitializeLogFile()
{
	if (!m_LogFileStream.is_open())
	{
		// make the logs directory if it doesn't already exist
		std::filesystem::create_directory("logs");

		string LogFileName = "logs/" + Utils::GetDate() + ".log";
		m_LogFileStream.open(LogFileName, std::ios::out | std::ios::app);

		m_LogFileStream << "\n-------------- [" << Utils::GetTimestamp() << "] --------------\n";
	}
}

void Logger::Info(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	std::string sFmt(fmt);

	sFmt = AddTimestamp("[I] " + sFmt);

	vprintf(sFmt.c_str(), args);
	
	string t = string::Format(string(sFmt), args);

	if (m_LogFileStream.is_open())
		m_LogFileStream << t.ToCString();

	va_end(args);
}

void Logger::Info(std::string msg)
{
	msg = AddTimestamp("[I] " + msg);

	printf(msg.c_str());

	if (m_LogFileStream.is_open())
		m_LogFileStream << msg;
}

void Logger::Warning(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	std::string sFmt(fmt);

	sFmt = AddTimestamp("[W] " + sFmt);

	vprintf(sFmt.c_str(), args);

	string t = string::Format(string(sFmt), args);

	if (m_LogFileStream.is_open())
		m_LogFileStream << t.ToCString();

	va_end(args);
}

void Logger::Warning(std::string msg)
{
	msg = AddTimestamp("[W] " + msg);

	printf(msg.c_str());

	if (m_LogFileStream.is_open())
		m_LogFileStream << msg;
}

Logger g_Logger;