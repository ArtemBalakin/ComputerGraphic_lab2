#pragma once
#include <fstream>
#include <string>
#include <mutex>

class Logger {
public:
    Logger() {
        logFile.open("C:\\Users\\Stalker\\Desktop\\labalog.txt", std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Не удалось открыть файл логов: C:\\Users\\Stalker\\Desktop\\labalog.txt");
        }
    }

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    template<typename T>
    Logger& operator<<(const T& value) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << value;
            logFile.flush();
        }
        return *this;
    }

    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << manip;
            logFile.flush();
        }
        return *this;
    }

private:
    std::ofstream logFile;
    std::mutex logMutex;
};

extern Logger logger;