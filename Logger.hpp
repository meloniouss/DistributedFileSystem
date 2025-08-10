#pragma once
#include <string>

class Logger{
public:
	Logger(std::string nodetype);
	void Log(std::string msg);
	void LogWarning(std::string msg);
	void LogError(std::string msg);
private:
	std::string _nodetype;
};
