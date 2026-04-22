# wsjcpp-logger

<p align="center">
    <a href="https://github.com/wsjcpp/wsjcpp-logger" alt="Activity">
        <img src="https://img.shields.io/github/commit-activity/m/wsjcpp/wsjcpp-logger" />
    </a>
</p>

[![Github Stars](https://img.shields.io/github/stars/wsjcpp/wsjcpp-logger.svg?label=github%20%E2%98%85)](https://github.com/wsjcpp/wsjcpp-logger/stargazers) [![Github Stars](https://img.shields.io/github/contributors/wsjcpp/wsjcpp-logger.svg)](https://github.com/wsjcpp/wsjcpp-logger/) [![Github Forks](https://img.shields.io/github/forks/wsjcpp/wsjcpp-logger.svg?label=github%20forks)](https://github.com/wsjcpp/wsjcpp-logger/network/members)

C++ Logger Implementation

## Integrate to your project

```
$ wsjcpp install https://github.com/wsjcpp/wsjcpp-logger:master
```

Or include files:

* `src/wsjcpp-logger/wsjcpp_logger.h`
* `src/wsjcpp-logger/wsjcpp_logger.cpp`

## Logger (wsjcpp::Log)

* Output will be colored for console, but color will be missing for files.
* Functions are safe thread.
* Logger supports a log rotation (every 51000 seconds / every day)
* wsjcpp::Log::throw_err - will be generate `throw std::runtime_error(sMessage);`
* std::vector<std::string> wsjcpp::Log::getLastLogMessages() - last 50 records from log

To init logger first and enable.

```
#include <wsjcpp_logger.h>

int main(int argc, char* argv[]) {
    std::string TAG = "MAIN";
    wsjcpp::Log::setLogDirectory(".logs");
    wsjcpp::Log::setPrefixLogFile("app");
    wsjcpp::Log::setEnableLogFile(true);

    wsjcpp::Log::debug(TAG, "Hello info");
    wsjcpp::Log::info(TAG, "Hello info");
    wsjcpp::Log::err(TAG, "Hello err");
    wsjcpp::Log::warn(TAG, "Hello warn");
    wsjcpp::Log::ok(TAG, "Hello ok");

    return 0;
}
```

Example output
```
2026-04-22 14:40:07.395, 0x00007aa3e8c98740 [DEBUG] MAIN: Hello info
2026-04-22 14:40:07.395, 0x00007aa3e8c98740 [INFO] MAIN: Hello info
2026-04-22 14:40:07.396, 0x00007aa3e8c98740 [ERR] MAIN: Hello err
2026-04-22 14:40:07.396, 0x00007aa3e8c98740 [WARN] MAIN: Hello warn
2026-04-22 14:40:07.396, 0x00007aa3e8c98740 [OK] MAIN: Hello ok
```
