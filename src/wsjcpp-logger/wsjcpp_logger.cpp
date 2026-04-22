/*
 * MIT License
 *
 * Copyright (c) 2019-2026 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

// original source-code: https://github.com/wsjcpp/wsjcpp-logger

#include "wsjcpp_logger.h"

#ifndef _MSC_VER
    #include <dirent.h>
    #include <sys/time.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#else
    #include <direct.h>
    #define PATH_MAX 256
#endif

// #include <filesystem> // in c++ 17
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <iomanip> // std::setw

namespace wsjcpp {

long getCurrentTimeInMilliseconds() {
    long nTimeStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return nTimeStart;
}

long getCurrentTimeInSeconds() {
    long nTimeStart = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return nTimeStart;
}

std::string getCurrentTimeForLogFormat() {
    long nTimeStart = getCurrentTimeInMilliseconds();
    std::string sMilliseconds = std::to_string(int(nTimeStart % 1000));
    nTimeStart = nTimeStart / 1000;

    std::time_t tm_ = long(nTimeStart);
    struct tm tstruct;
    gmtime_r(&tm_, &tstruct);

    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    return std::string(buf) + "." + std::string( 3 - sMilliseconds.length(), '0').append(sMilliseconds);
}

std::string getThreadId() {

    static_assert(sizeof(std::thread::id)==sizeof(uint64_t),"this function only works if size of thead::id is equal to the size of uint_64");
    std::thread::id this_id = std::this_thread::get_id();
    uint64_t val = *((uint64_t*) &this_id);
    std::stringstream stream;
    stream << "0x" << std::setw(16) << std::setfill('0') << std::hex << val;
    return std::string(stream.str());
}

std::string formatTimeForFilename(long nTimeInSec) {
    std::time_t tm_ = long(nTimeInSec);
    // struct tm tstruct = *localtime(&tm_);
    struct tm tstruct;
    gmtime_r(&tm_, &tstruct);

    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    char buf[80];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tstruct);
    return std::string(buf);
}

bool dirExists(const std::string &sDirname) {
    struct stat st;
    bool bExists = (stat(sDirname.c_str(), &st) == 0);
    if (bExists) {
        return (st.st_mode & S_IFDIR) != 0;
    }
    return false;
}

bool makeDir(const std::string &sDirname) {
    struct stat st;

    // const std::filesystem::path dir{sDirname};
    // std::filesystem::create_directory(dir);

    int nStatus = mkdir(sDirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (nStatus == 0) {
        return true;
    }
    if (nStatus == EACCES) {
        std::cout << "FAILED create folder " << sDirname << std::endl;
        return false;
    }
    // std::cout << "nStatus: " << nStatus << std::endl;
    return true;
}

enum ColorCode {
  FG_RED      = 31,
  FG_GREEN    = 32,
  FG_YELLOW   = 93,
  FG_BLUE     = 34,
  FG_CYAN     = 36,
  FG_DEFAULT  = 39,
  BG_RED      = 41,
  BG_GREEN    = 42,
  BG_BLUE     = 44,
  BG_CYAN     = 46,
  BG_DEFAULT  = 49
};

class ColorModifier {
public:
  ColorModifier(ColorCode pCode) : code(pCode) {}
  friend std::ostream&
  operator<<(std::ostream& os, const ColorModifier& mod) {
      return os << "\033[" << mod.code << "m";
  }
  // ColorModifier green = ColorModifier(ColorCode::FG_GREEN);
private:
  ColorCode code;
};

// ---------------------------------------------------------------------
// LogGlobalConf

LogGlobalConf::LogGlobalConf() {
    logDir = "./";
    logPrefixFile = "";
    logFile = "";
    enableLogFile = false;
    logStartTime = 0;
    logRotationPeriodInSeconds = 51000;
}

void LogGlobalConf::doLogRotateUpdateFilename(bool bForce) {
    long t = getCurrentTimeInSeconds();
    long nEverySeconds = logRotationPeriodInSeconds; // rotate log if started now or if time left more then 1 day
    if (logStartTime == 0 || t - logStartTime > nEverySeconds || bForce) {
        logStartTime = t;
        logFile = logDir + "/"
            + logPrefixFile + "_"
            + formatTimeForFilename(logStartTime) + ".log";
    }
}

void Log_add(ColorModifier &clr, const std::string &sType, const std::string &sTag, const std::string &sMessage) {
  Log::g_WSJCPP_LOG_GLOBAL_CONF.doLogRotateUpdateFilename();

    std::lock_guard<std::mutex> lock(Log::g_WSJCPP_LOG_GLOBAL_CONF.logMutex);
    ColorModifier def(ColorCode::FG_DEFAULT);

    std::string sLogMessage = getCurrentTimeForLogFormat() + ", " + getThreadId()
         + " [" + sType + "] " + sTag + ": " + sMessage;
    std::cout << clr << sLogMessage << def << std::endl;

    Log::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.push_front(sLogMessage);


    while (Log::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.size() > 50) {
        Log::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.pop_back();
    }

    // log file
    if (Log::g_WSJCPP_LOG_GLOBAL_CONF.enableLogFile) {
        std::ofstream logFile(Log::g_WSJCPP_LOG_GLOBAL_CONF.logFile, std::ios::app);
        if (!logFile) {
            std::cout << "Error Opening File" << std::endl;
            return;
        }

        logFile << sLogMessage << std::endl;
        logFile.close();
    }
}

// ---------------------------------------------------------------------
// Log

LogGlobalConf Log::g_WSJCPP_LOG_GLOBAL_CONF;

void Log::debug(const std::string &sTag, const std::string &sMessage) {
    ColorModifier cyan(ColorCode::FG_CYAN);
    Log_add(cyan, "DEBUG", sTag, sMessage);
}

void Log::info(const std::string & sTag, const std::string &sMessage) {
    ColorModifier def(ColorCode::FG_DEFAULT);
    Log_add(def, "INFO", sTag, sMessage);
}

void Log::err(const std::string & sTag, const std::string &sMessage) {
    ColorModifier red(ColorCode::FG_RED);
    Log_add(red, "ERR", sTag, sMessage);
}

void Log::throw_err(const std::string &sTag, const std::string &sMessage) {
    ColorModifier red(ColorCode::FG_RED);
    Log_add(red, "ERR", sTag, sMessage);
    throw std::runtime_error(sMessage);
}

void Log::warn(const std::string & sTag, const std::string &sMessage) {
    ColorModifier yellow(ColorCode::FG_YELLOW);
    Log_add(yellow, "WARN",sTag, sMessage);
}

void Log::ok(const std::string &sTag, const std::string &sMessage) {
    ColorModifier green(ColorCode::FG_GREEN);
    Log_add(green, "OK", sTag, sMessage);
}

std::vector<std::string> Log::getLastLogMessages() {
    std::lock_guard<std::mutex> lock(Log::g_WSJCPP_LOG_GLOBAL_CONF.logMutex);
    std::vector<std::string> vRet;
    for (int i = 0; i < Log::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages.size(); i++) {
        vRet.push_back(Log::g_WSJCPP_LOG_GLOBAL_CONF.logLastMessages[i]);
    }
    return vRet;
}

void Log::setLogDirectory(const std::string &sDirectoryPath) {
    Log::g_WSJCPP_LOG_GLOBAL_CONF.logDir = sDirectoryPath;
    if (!dirExists(sDirectoryPath)) {
        if (!makeDir(sDirectoryPath)) {
            Log::err("setLogDirectory", "Could not create log directory '" + sDirectoryPath + "'");
        }
    }
    Log::g_WSJCPP_LOG_GLOBAL_CONF.doLogRotateUpdateFilename(true);
}

void Log::setPrefixLogFile(const std::string &sPrefixLogFile) {
    Log::g_WSJCPP_LOG_GLOBAL_CONF.logPrefixFile = sPrefixLogFile;
    Log::g_WSJCPP_LOG_GLOBAL_CONF.doLogRotateUpdateFilename(true);
}

void Log::setEnableLogFile(bool bEnable) {
    Log::g_WSJCPP_LOG_GLOBAL_CONF.enableLogFile = bEnable;
}

void Log::setRotationPeriodInSec(long nRotationPeriodInSec) {
    Log::g_WSJCPP_LOG_GLOBAL_CONF.logRotationPeriodInSeconds = nRotationPeriodInSec;
}

} // namespace wsjcpp
