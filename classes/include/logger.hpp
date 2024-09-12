#pragma once

#define ERROR 0
#define WARNING 1
#define INFO 5
#define DEBUG 9

#define VALIDATE_LOG_LEVEL(lv) ((lv == ERROR || lv == WARNING || lv == INFO || lv == DEBUG) ? lv : INFO)

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <filesystem>

class EventMessage {
public:
    int level;
    std::string message;
    std::chrono::system_clock::time_point timeEvent;
};

class AppLogs {
private:
    static std::unique_ptr<AppLogs> _instance;
    std::vector<EventMessage> messages;

    AppLogs() = default;

public:
    std::string path = "";
    std::string fileName = "logs";
    std::string fileExtension = "txt";
    bool filenamedWithTimestamp = false;
    bool display = false;
    int currentLevel = 1;

    static AppLogs& Instance();
    void EventLog(int lv, const std::string& m);
    int GetNumberForLevel(int lv);
    std::string GenerateFile();   
};