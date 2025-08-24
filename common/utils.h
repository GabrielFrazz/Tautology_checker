#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

struct InstanceData {
    std::string id;
    std::string formula;
    std::string type;
    int numVars;
};

class FileUtils {
public:

    static std::string readFormula(const std::string& filename);

    static std::vector<InstanceData> readInstances(const std::string& filename);
    
    static InstanceData readInstanceById(const std::string& filename, const std::string& instanceId);
    
    static void writeFormula(const std::string& filename, const std::string& formula, int numVariables = 0);

    static std::string trim(const std::string& str);
    
    static std::vector<std::vector<bool>> generateTruthTable(int numVariables);
    
    static std::vector<std::string> split(const std::string& str, char delimiter);
};

#endif