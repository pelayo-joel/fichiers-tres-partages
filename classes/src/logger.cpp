#include "../include/logger.hpp"


std::unique_ptr<AppLogs> AppLogs::_instance = nullptr;

AppLogs& AppLogs::Instance() {
    if (!_instance) {
        _instance = std::unique_ptr<AppLogs>(new AppLogs());
    }
    return *_instance;
}

void AppLogs::EventLog(int lv, const std::string& m) {

    lv = VALIDATE_LOG_LEVEL(lv);


    if (lv <= this->currentLevel) {
        EventMessage ev;
        ev.level = lv;
        ev.message = m;
        ev.timeEvent = std::chrono::system_clock::now();
        this->messages.push_back(ev);
    }
}

int AppLogs::GetNumberForLevel(int lv) {
    return std::count_if(messages.begin(), messages.end(),
                         [lv](const EventMessage& obj) { return obj.level == lv; });
}

std::string AppLogs::GenerateFile() {
    std::string pathFile = this->path + this->fileName;
    try {
        // Ensure the directory exists before creating the file
        std::filesystem::create_directories(this->path);

        std::string fileContent = "ERROR : " + std::to_string(this->GetNumberForLevel(0)) + "\t\t";
        if (this->currentLevel >= ERROR) fileContent += "WARNING : " + std::to_string(this->GetNumberForLevel(1)) + "\t\t";
        if (this->currentLevel >= INFO) fileContent += "INFO : " + std::to_string(this->GetNumberForLevel(5)) + "\t\t";
        if (this->currentLevel >= DEBUG) fileContent += "DEBUG : " + std::to_string(this->GetNumberForLevel(9));
        fileContent += "\n\n";

        auto appendMessages = [&](int level, const std::string& header) {
            if (this->GetNumberForLevel(level) > 0) {
                fileContent += "******* " + header + " *******\n";
                for (const auto& m : this->messages) {
                    if (m.level == level) {
                        auto timeStr = std::chrono::system_clock::to_time_t(m.timeEvent);
                        std::ostringstream oss;
                        oss << std::put_time(std::localtime(&timeStr), "%Y-%m-%d %H:%M:%S");
                        fileContent += oss.str() + std::string("\t\t") + m.message + "\n";
                    }
                }
                fileContent += "\n\n";
            }
        };

        appendMessages(0, "ERROR");
        if (this->currentLevel > ERROR) appendMessages(1, "WARNING");
        if (this->currentLevel >= INFO) appendMessages(5, "INFO");
        if (this->currentLevel >= DEBUG) {
            fileContent += "******* DEBUG *******\n";
            for (const auto& m : this->messages) {
                std::string txtLevel;
                switch (m.level) {
                    case 0: txtLevel = "ERROR"; break;
                    case 1: txtLevel = "WARNING"; break;
                    case 5: txtLevel = "INFO"; break;
                    case 9: txtLevel = "DEBUG"; break;
                    default: txtLevel = "UNKNOWN";
                }
                auto timeStr = std::chrono::system_clock::to_time_t(m.timeEvent);
                std::ostringstream oss;
                oss << std::put_time(std::localtime(&timeStr), "%Y-%m-%d %H:%M:%S");
                fileContent += oss.str() + std::string("\t\t") + txtLevel + "\t\t" + m.message + "\n";
            }
        }

        if (this->filenamedWithTimestamp) {
            auto now = std::chrono::system_clock::now();
            auto timeStr = std::chrono::system_clock::to_time_t(now);
            char buffer[20];
            std::strftime(buffer, sizeof(buffer), "_%Y%m%d%H%M%S", std::localtime(&timeStr));
            pathFile += buffer;
        }
        pathFile += "." + this->fileExtension;

        std::ofstream outFile(pathFile);
        if (!outFile) {
            throw std::runtime_error("Unable to open file for writing");
        }
        outFile << fileContent;
        outFile.close();
    }
    catch (const std::exception& e) {
        auto now = std::chrono::system_clock::now();
        auto timeStr = std::chrono::system_clock::to_time_t(now);
        std::cerr << std::put_time(std::localtime(&timeStr), "%d/%m/%y %H:%M")
                  << "   Erreur de création du fichier de Log : " << e.what() << std::endl;
        return "";
    }
    return pathFile;
}