#ifndef CONFIGMANAGER_HPP
#define CONFIGMANAGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>

class ConfigManager {
private:
    std::map<std::string, std::string> configData;

    // 생성자를 private으로 은닉 (싱글톤 패턴)
    ConfigManager() {
        loadConfig("config.ini");
    }

    // 파일이 없을 경우 기본값으로 자동 생성
    void createDefaultConfig(const std::string& filename) {
        std::ofstream outfile(filename);
        if (outfile.is_open()) {
            outfile << "StartStamina=150\n";
            outfile << "MaxLevel=30\n";
            outfile << "ColdChance=2\n";
            outfile << "DroughtChance=5\n";
            outfile.close();
            std::cout << "⚙️ [시스템] 기본 config.ini 파일을 자동 생성했습니다.\n";
        }
    }

public:
    // 유일한 인스턴스를 반환하는 static 메서드
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    // ini 파일 읽기
    void loadConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            createDefaultConfig(filename);
            file.open(filename); // 생성 후 다시 열기
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream is_line(line);
            std::string key, value;
            // '='를 기준으로 Key와 Value 분리
            if (std::getline(is_line, key, '=') && std::getline(is_line, value)) {
                configData[key] = value;
            }
        }
        file.close();
    }

    // 템플릿을 활용하여 원하는 데이터 타입(int, float, string 등)으로 변환 후 반환
    template <typename T>
    T getValue(const std::string& key) {
        // 키가 존재하지 않으면 기본값 반환 (안전장치)
        if (configData.find(key) == configData.end()) {
            std::cerr << "⚠️ [경고] config.ini에 '" << key << "' 값이 없습니다!\n";
            return T(); 
        }
        std::stringstream ss(configData[key]);
        T value;
        ss >> value;
        return value;
    }
};

#endif // CONFIGMANAGER_HPP