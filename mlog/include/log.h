#ifndef LOG_H
#define LOG_H

#include <pthread.h>
#include <stdint.h>

#include <string>
#include <sstream>
#include <map>
#include <initializer_list>
#include <memory>


namespace mlog {

#define HOUR_SECONDS                            3600ul
#define DAY_SECONDS                             86400ul

enum LogLevel {
	kTrace,
	kDebug,
	kInfo,
	kWarn,
	kError,
	kFatal,
	kMaxLevel
};

extern LogLevel work_level;
extern pthread_mutex_t mlock;

class Log {
public:
	Log(const LogLevel level);
  ~Log();

	std::stringstream& strm() {
		return strm_;
	}

private:
	std::stringstream strm_;
	LogLevel self_level_;
};

void Init(const LogLevel level = kInfo, const std::string &log_dir = "./log", const std::string &file_prefix = "", const bool screen_out = true);
bool SetLogLevel(const std::string &level_str);
bool SetLogDir(const std::string &level_dir);

std::string GetLevelStr();
int32_t Write(const LogLevel level, const std::string& str);
int32_t Write(const LogLevel level, const char* format, ...);
void BackupAndSwitchLog(const std::string& d); 

int32_t Log(const mlog::LogLevel level, const std::initializer_list<std::string>& args); 
int32_t LogTrace(const std::initializer_list<std::string>& args); 
int32_t LogDebug(const std::initializer_list<std::string>& args); 
int32_t LogInfo(const std::initializer_list<std::string>& args); 
int32_t LogWarn(const std::initializer_list<std::string>& args); 
int32_t LogError(const std::initializer_list<std::string>& args); 
int32_t LogFatal(const std::initializer_list<std::string>& args); 
int32_t LogMaxLevel(const std::initializer_list<std::string>& args); 
}


#define TRACE   mlog::kTrace
#define DEBUG   mlog::kDebug
#define INFO    mlog::kInfo
#define WARN    mlog::kWarn
#define ERROR   mlog::kError
#define FATAL   mlog::kFatal

#define LOG(level) \
	if (level >= mlog::work_level) \
		mlog::Log(level).strm()

using mlog::Log;
using mlog::LogTrace;
using mlog::LogDebug;
using mlog::LogInfo;
using mlog::LogWarn;
using mlog::LogError;
using mlog::LogFatal;
using mlog::LogMaxLevel;

#endif
