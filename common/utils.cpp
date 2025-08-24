#include "utils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>

std::string FileUtils::readFormula(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Não foi possível abrir o arquivo: " + filename);
    }
    
    std::string formula;
    std::getline(file, formula);
    file.close();
    
    return trim(formula);
}

std::vector<InstanceData> FileUtils::readInstances(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Não foi possível abrir o arquivo: " + filename);
    }
    
    std::vector<InstanceData> instances;
    std::string line;
    
    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        auto parts = split(line, '|');
        if (parts.size() >= 4) {
            InstanceData instance;
            instance.id = trim(parts[0]);
            instance.formula = trim(parts[1]);
            instance.type = trim(parts[2]);
            instance.numVars = std::stoi(trim(parts[3]));
            instances.push_back(instance);
        }
    }
    
    file.close();
    return instances;
}

InstanceData FileUtils::readInstanceById(const std::string& filename, const std::string& instanceId) {
    auto instances = readInstances(filename);
    
    for (const auto& instance : instances) {
        if (instance.id == instanceId) {
            return instance;
        }
    }
    
    throw std::runtime_error("Instância não encontrada: " + instanceId);
}

void FileUtils::writeFormula(const std::string& filename, const std::string& formula, int numVariables) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Não foi possível criar o arquivo: " + filename);
    }
    
    file << formula << std::endl;
    if (numVariables > 0) {
        file << "# Variáveis: " << numVariables << std::endl;
    }
    file.close();
}

std::string FileUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::vector<bool>> FileUtils::generateTruthTable(int numVariables) {
    int numRows = std::pow(2, numVariables);
    std::vector<std::vector<bool>> truthTable;
    
    for (int i = 0; i < numRows; ++i) {
        std::vector<bool> row;
        for (int j = numVariables - 1; j >= 0; --j) {
            row.push_back((i >> j) & 1);
        }
        truthTable.push_back(row);
    }
    
    return truthTable;
}

std::vector<std::string> FileUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        result.push_back(token);
    }
    
    return result;
}