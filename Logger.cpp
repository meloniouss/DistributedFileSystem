#include "Logger.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>

Logger::Logger(std::string nodetype) 
:
_nodetype(nodetype)
{
    std::cout << "Creating logger" << std::endl;
}

void Logger::Log(std::string msg){
	auto now = std::chrono::system_clock::now();
    	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &now_c); 
#else
    localtime_r(&now_c, &local_tm);  
#endif

	std::cout << "[" << std::put_time(&local_tm, "%F %T") << "] [INFO] [" << _nodetype << "] "<< msg << std::endl;
}

void Logger::LogWarning(std::string msg){
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &now_c); 
#else
    localtime_r(&now_c, &local_tm);  
#endif
    std::cout << "[" << std::put_time(&local_tm, "%F %T") << "] [WARNING] [" << _nodetype<< "] " << msg << std::endl;
}

void Logger::LogError(std::string msg){
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
#ifdef _WIN32
    localtime_s(&local_tm, &now_c); 
#else
    localtime_r(&now_c, &local_tm);  
#endif
    std::cout << "[" << std::put_time(&local_tm, "%F %T") << "] [ERROR] [" << _nodetype << "] " << msg << std::endl;
}

