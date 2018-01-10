#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <utility>
#include <atomic>

#include "log.h"

namespace mlog {


struct LogMeta {
	LogMeta();
	~LogMeta();

	std::map<LogLevel, std::string> log_level_prompts;
	std::map<LogLevel, std::string> log_level_strs;
	std::map<LogLevel, FILE*> log_level_files;
	std::map<LogLevel, pthread_mutex_t*> log_level_mutexes_;
	pthread_mutex_t *screen_mutex_;
	
	std::string dir;
	std::string file_prefix;
	bool screen_out;
};

LogMeta::LogMeta() {

	log_level_prompts.insert(std::make_pair(kTrace,  "[TRACE]"));
	log_level_prompts.insert(std::make_pair(kDebug,  "[DEBUG]"));
	log_level_prompts.insert(std::make_pair(kInfo,   "[INFO ]"));
	log_level_prompts.insert(std::make_pair(kWarn,   "[WARN ]"));
	log_level_prompts.insert(std::make_pair(kError,  "[ERROR]"));
	log_level_prompts.insert(std::make_pair(kFatal,  "[FATAL]"));

	log_level_strs.insert(std::make_pair(kTrace, "trace" ));
	log_level_strs.insert(std::make_pair(kDebug, "debug" ));
	log_level_strs.insert(std::make_pair(kInfo,  "info" ));
	log_level_strs.insert(std::make_pair(kWarn,  "warn" ));
	log_level_strs.insert(std::make_pair(kError, "error" ));
	log_level_strs.insert(std::make_pair(kFatal, "fatal"));

	pthread_mutex_t* mutex_p = NULL;
	for (uint32_t level = kTrace; level < kMaxLevel; ++level) {
		mutex_p = new pthread_mutex_t;
		pthread_mutex_init(mutex_p, NULL);
		log_level_mutexes_.insert(std::make_pair(static_cast<LogLevel>(level), mutex_p));
	}
	screen_mutex_ = new pthread_mutex_t;
	pthread_mutex_init(screen_mutex_, NULL);
}

LogMeta::~LogMeta() {

	LogLevel level;
	for (uint32_t idx = kTrace; idx < kMaxLevel; ++idx) {	
		level = static_cast<LogLevel>(idx);
		if (log_level_files[level]) {
			fclose(log_level_files[level]);
		}
		pthread_mutex_destroy(log_level_mutexes_[level]);
		delete log_level_mutexes_[level];
	}
	pthread_mutex_destroy(screen_mutex_);

}

LogLevel work_level;
LogMeta log_meta;


static int DoCreatePath(const char *path, mode_t mode) {
  struct stat st;
  int status = 0;

  if (stat(path, &st) != 0) {
    /* Directory does not exist. EEXIST for race
     * condition */
    if (mkdir(path, mode) != 0 && errno != EEXIST)
      status = -1;
  } else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    status = -1;
  }

  return (status);
}

static int CreatePath(const std::string& path, mode_t mode) {
  char           *pp;
  char           *sp;
  int             status;
  char           *copypath = strdup(path.c_str());

  status = 0;
  pp = copypath;
  while (status == 0 && (sp = strchr(pp, '/')) != 0) {
    if (sp != pp) {
      /* Neither root nor double slash in path */
      *sp = '\0';
      status = DoCreatePath(copypath, mode);
      *sp = '/';
    }
    pp = sp + 1;
  }
  if (status == 0)
    status = DoCreatePath(path.c_str(), mode);
  free(copypath);
  return (status);		
}

void Init(const LogLevel level, const std::string &log_dir, const std::string &file_prefix, const bool screen_out) {
	log_meta.file_prefix = file_prefix;
	work_level = level;
	log_meta.screen_out = screen_out;

	if (access(log_dir.c_str(), F_OK)) {
		CreatePath(log_dir, 0775);	
	}
	std::string log_path = log_dir;
	if (!log_path.empty() && *log_path.rbegin() == '/') {
		log_path.erase(log_path.size()-1);
	}

	std::string file_name;
	FILE *file;
	for (int32_t idx = kTrace; idx < kMaxLevel; ++idx) {
		file_name = file_prefix.empty() ? "" : (file_prefix + "_");
		file_name = log_path + "/" + file_name; 
		file_name += log_meta.log_level_strs[static_cast<LogLevel>(idx)] + ".log";
		file = fopen(file_name.c_str(), "a+");
		if (file == NULL) {
			std::cerr << __FILE__ << ":"  << __LINE__ << ", Open " << file_name << " failed" << std::endl;
			exit(-1);
		}

		log_meta.log_level_files.insert(std::make_pair(static_cast<LogLevel>(idx), file));
	}
}

static LogLevel InterpretLogLevel(const std::string level_str) {
	for (int32_t idx = kInfo; idx < kMaxLevel; ++idx) {
		if (strcasecmp(log_meta.log_level_strs[static_cast<LogLevel>(idx)].c_str(), level_str.c_str()) == 0) {
			return static_cast<LogLevel>(idx);
		}
  }
  return kMaxLevel;
}

std::string GetLevelStr() {
	return log_meta.log_level_strs[work_level];
}

bool SetLogLevel(const std::string &level_str) {
	bool ret = true;
	LogLevel level = InterpretLogLevel(level_str);
	if (level == kMaxLevel) {
		ret = false;
		level = kInfo;
	}

	/*
	 * to be pretected
	 */

	work_level = level;
	return ret;
}

Log::Log(const LogLevel level) {
	self_level_ = level;
	strm_ << log_meta.log_level_prompts[self_level_] << "\t\t";
}

Log::~Log() {
	std::string str = strm_.str();
  if (str.empty()) {
    return;
  }

	if (log_meta.screen_out) {
		pthread_mutex_lock(log_meta.screen_mutex_);
		std::cerr << str;
		pthread_mutex_unlock(log_meta.screen_mutex_);
	}

	pthread_mutex_lock(log_meta.log_level_mutexes_[self_level_]);
	if (fwrite(str.c_str(), str.size(), 1, log_meta.log_level_files[self_level_]) < 1) {
		 std::cerr << __FILE__ << ":"  << __LINE__ << ", write " << log_meta.log_level_strs[self_level_] << " failed" << std::endl;
	}
	pthread_mutex_unlock(log_meta.log_level_mutexes_[self_level_]);
}

int32_t Write(const LogLevel level, const std::string& str) {
  int32_t ret = 0;
  if (level < work_level) {
    return ret;
  }
	if (log_meta.screen_out) {
		pthread_mutex_lock(log_meta.screen_mutex_);
		std::cerr << str;
		pthread_mutex_unlock(log_meta.screen_mutex_);
	}

	pthread_mutex_lock(log_meta.log_level_mutexes_[level]);
	ret = fwrite(str.c_str(), str.size(), 1, log_meta.log_level_files[level]);
	pthread_mutex_unlock(log_meta.log_level_mutexes_[level]); 
  if (ret < 1) {
		 std::cerr << __FILE__ << ":"  << __LINE__ << ", write " << log_meta.log_level_strs[level] << " failed" << std::endl;
	}
  return ret;
}

int32_t Write(const LogLevel level, const char* format, ...) {
  static char buf[1024];

  int32_t ret = 0;
  if (level < work_level) {
    return ret;
  }
  
  if (log_meta.screen_out) {
    va_list vl;
    va_start(vl, format);
		pthread_mutex_lock(log_meta.screen_mutex_);
    fprintf(stderr, format, vl); 
		pthread_mutex_unlock(log_meta.screen_mutex_);
    va_end(vl);
  }
  
  va_list vl;
  va_start(vl, format);
  int32_t len = vsnprintf(buf, sizeof(buf), format, vl);
  va_end(vl); 
  if (len < 0) {
    return -1;
  }

	pthread_mutex_lock(log_meta.log_level_mutexes_[level]);
	ret = fwrite(buf, len, 1, log_meta.log_level_files[level]);
	pthread_mutex_unlock(log_meta.log_level_mutexes_[level]); 
  if (ret < 1) {
		 std::cerr << __FILE__ << ":"  << __LINE__ << ", write " << log_meta.log_level_strs[level] << " failed" << std::endl;
	}
  
  return ret;
}

void BackupAndSwitchLog(const std::string& d) {
  std::string date = d;
  if (date.empty()) { 
    char buf[64];
    time_t now = time(NULL);
    strftime(buf, sizeof(buf), "%Y%m%d", localtime(&now));
    date.assign(buf, strlen(buf));
  }
  
  std::string backup_dir = log_meta.dir + "/backup";
  if (access(backup_dir.c_str(), F_OK)) {
    CreatePath(backup_dir.c_str(), 0775);
  }

  std::string file_path, back_file_path, file_name;
  std::string half_file_name = log_meta.file_prefix;
  if (!half_file_name.empty()) {
    half_file_name += "_";
  }
  
  for (LogLevel level = kTrace; level < kMaxLevel; level = (LogLevel)(level + 1)) {
    file_name = half_file_name + log_meta.log_level_strs[static_cast<LogLevel>(level)] + ".log";
    file_path = log_meta.dir + "/" + file_name;   
    back_file_path = backup_dir + "/" + file_name + "." + date;
    /*
     * not care the successity of rename
     */
    rename(file_path.c_str(), back_file_path.c_str());
    FILE* fp =fopen(file_path.c_str(), "a+");
    if (!fp) {
      continue;
    }
	  pthread_mutex_lock(log_meta.log_level_mutexes_[level]);
    fclose(log_meta.log_level_files[level]);
    log_meta.log_level_files[level] = fp; 
	  pthread_mutex_unlock(log_meta.log_level_mutexes_[level]);
  }
}

inline static void SwitchDateLog(const time_t now) {
  static std::atomic<uint32_t> cur_date((time(NULL)+8*HOUR_SECONDS) / DAY_SECONDS); /* 0s <---> 1970年 01月 01日 星期四 08:00:00 CST*/
  uint32_t now_date = (now+8*HOUR_SECONDS) / DAY_SECONDS;
  if (now_date == cur_date) {
    return;
  }
  uint32_t date_start = cur_date; 
  if (cur_date.compare_exchange_strong(date_start, now_date)) {
    char buf[16];
    time_t backup_time = static_cast<time_t>(date_start) * DAY_SECONDS + 1;
    strftime(buf, sizeof(buf), "%Y%m%d", localtime(&backup_time)); /* plus 1 is for safe */
    mlog::BackupAndSwitchLog(std::string(buf));
  }
}

int32_t Log(const mlog::LogLevel level, const std::initializer_list<std::string>& args) {
	static char time_buf[64];
  
	time_t now = time(NULL);
  SwitchDateLog(now);
  
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S    ", localtime(&now));
	std::string line(time_buf, strlen(time_buf));
	line.append("|");

	for (const std::string& arg : args) {
		line.append(arg);
		line.append("|");
	}
	line.append("\n");
	mlog::Write(level, line);
	return 0;
}

int32_t LogTrace(const std::initializer_list<std::string>& args) {
  return Log(kTrace, args);
}

int32_t LogDebug(const std::initializer_list<std::string>& args) {
  return Log(kDebug, args);
}

int32_t LogInfo(const std::initializer_list<std::string>& args) {
  return Log(kInfo, args);
}

int32_t LogWarn(const std::initializer_list<std::string>& args) {
  return Log(kWarn, args);
}

int32_t LogError(const std::initializer_list<std::string>& args) {
  return Log(kError, args);
}

int32_t LogFatal(const std::initializer_list<std::string>& args) {
  return Log(kFatal, args);
}

int32_t LogMaxLevel(const std::initializer_list<std::string>& args) {
  return Log(kMaxLevel, args);
}

}
