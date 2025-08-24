#include "../common/utils.h"
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <set>

class FormulaGenerator {
private:
    std::mt19937 gen;
    std::uniform_int_distribution<> varDist;
    std::uniform_int_distribution<> opDist;
    std::uniform_real_distribution<> probDist;
    std::uniform_int_distribution<> targetVarsDist;
    
    int minVariables;
    int maxVariables;
    int maxDepth;
    
public:
    FormulaGenerator(int seed, int minVars, int maxVars, int depth) 
        : gen(seed), minVariables(minVars), maxVariables(maxVars), maxDepth(depth),
          varDist(0, maxVars - 1), opDist(0, 3), probDist(0.0, 1.0),
          targetVarsDist(minVars, maxVars) {}
    
    std::string generateVariable() {
        char var = 'a' + varDist(gen);
        return std::string(1, var);
    }
    

    std::string generateFormulaWithTargetVars(int targetVars, int depth = 0) {
        if (depth >= maxDepth || probDist(gen) < 0.3) {
            char var = 'a' + (gen() % targetVars);
            return std::string(1, var);
        }
        
        int op = opDist(gen);
        
        switch (op) {
            case 0: // NOT
                return "¬" + generateFormulaWithTargetVars(targetVars, depth + 1);
            case 1: // AND
                return "(" + generateFormulaWithTargetVars(targetVars, depth + 1) + 
                       " ∧ " + generateFormulaWithTargetVars(targetVars, depth + 1) + ")";
            case 2: // OR
                return "(" + generateFormulaWithTargetVars(targetVars, depth + 1) + 
                       " ∨ " + generateFormulaWithTargetVars(targetVars, depth + 1) + ")";
            case 3: // IMPLICAÇÃO
                return "(" + generateFormulaWithTargetVars(targetVars, depth + 1) + 
                       " → " + generateFormulaWithTargetVars(targetVars, depth + 1) + ")";
            default:
                char var = 'a' + (gen() % targetVars);
                return std::string(1, var);
        }
    }
    
    // Força o uso de todas as variáveis no range desejado
    std::string generateFormulaWithAllVars(int targetVars) {
        std::string formula = generateFormulaWithTargetVars(targetVars);
        
        // Verifica quais variáveis estão sendo usadas
        std::set<char> usedVars;
        for (char c : formula) {
            if (c >= 'a' && c <= 'z') {
                usedVars.insert(c);
            }
        }
        
        // Se não temos todas as variáveis desejadas, força sua inclusão
        std::set<char> missingVars;
        for (int i = 0; i < targetVars; ++i) {
            char var = 'a' + i;
            if (usedVars.find(var) == usedVars.end()) {
                missingVars.insert(var);
            }
        }
        
        // Adiciona variáveis faltantes através de ORs (não afeta tautologias)
        for (char missingVar : missingVars) {
            formula = "(" + formula + " ∨ (" + std::string(1, missingVar) + " ∨ ¬" + std::string(1, missingVar) + "))";
        }
        
        return formula;
    }
    
    std::string generateFormula(int depth = 0) {
        int targetVars = targetVarsDist(gen);
        return generateFormulaWithAllVars(targetVars);
    }
    
    // Gera uma tautologia conhecida com número específico de variáveis
    std::string generateTautology() {
        int targetVars = targetVarsDist(gen);
        
        // Escolhe um tipo de tautologia base
        std::string baseTautology;
        char baseVar = 'a';
        
        switch (opDist(gen) % 3) {
            case 0:
                baseTautology = "(" + std::string(1, baseVar) + " ∨ ¬" + std::string(1, baseVar) + ")";
                break;
            case 1:
                baseTautology = "((" + std::string(1, baseVar) + " → " + std::string(1, baseVar) + "))";
                break;
            case 2:
                baseTautology = "¬(" + std::string(1, baseVar) + " ∧ ¬" + std::string(1, baseVar) + ")";
                break;
        }
        
        // Adiciona as outras variáveis como tautologias neutras
        for (int i = 1; i < targetVars; ++i) {
            char var = 'a' + i;
            baseTautology = "(" + baseTautology + " ∧ (" + std::string(1, var) + " ∨ ¬" + std::string(1, var) + "))";
        }
        
        return baseTautology;
    }
    
    // Gera uma contradição conhecida com número específico de variáveis
    std::string generateContradiction() {
        int targetVars = targetVarsDist(gen);
        
        // Base: contradição simples
        char baseVar = 'a';
        std::string baseContradiction = "(" + std::string(1, baseVar) + " ∧ ¬" + std::string(1, baseVar) + ")";
        
        // Adiciona as outras variáveis como tautologias neutras (não afeta a contradição)
        for (int i = 1; i < targetVars; ++i) {
            char var = 'a' + i;
            baseContradiction = "(" + baseContradiction + " ∧ (" + std::string(1, var) + " ∨ ¬" + std::string(1, var) + "))";
        }
        
        return baseContradiction;
    }
};

void printUsage() {
    std::cout << "Uso: ./generator <num_instancias> <min_variaveis> <max_variaveis> <profundidade_maxima> [seed]" << std::endl;
    std::cout << "  num_instancias: Número de instâncias a gerar" << std::endl;
    std::cout << "  min_variaveis: Número mínimo de variáveis diferentes (a-z)" << std::endl;
    std::cout << "  max_variaveis: Número máximo de variáveis diferentes (a-z)" << std::endl;
    std::cout << "  profundidade_maxima: Profundidade máxima da árvore de sintaxe" << std::endl;
    std::cout << "  seed: Semente para geração aleatória (opcional)" << std::endl;
    std::cout << std::endl;
    std::cout << "Distribuição de tipos:" << std::endl;
    std::cout << "  70% Tautologias" << std::endl;
    std::cout << "  20% Contradições" << std::endl;
    std::cout << "  10% Aleatórias" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        printUsage();
        return 1;
    }
    
    int numInstances = std::stoi(argv[1]);
    int minVars = std::stoi(argv[2]);
    int maxVars = std::stoi(argv[3]);
    int maxDepth = std::stoi(argv[4]);
    int seed = (argc > 5) ? std::stoi(argv[5]) : std::time(nullptr);
    
    if (maxVars > 26) {
        std::cout << "Erro: Máximo de 26 variáveis suportadas (a-z)" << std::endl;
        return 1;
    }
    
    if (minVars > maxVars) {
        std::cout << "Erro: Número mínimo de variáveis deve ser menor ou igual ao máximo" << std::endl;
        return 1;
    }
    
    FormulaGenerator generator(seed, minVars, maxVars, maxDepth);
    
    std::cout << "Gerando " << numInstances << " instâncias..." << std::endl;
    std::cout << "Parâmetros: min_vars=" << minVars << ", max_vars=" << maxVars 
              << ", max_depth=" << maxDepth << ", seed=" << seed << std::endl;
    std::cout << "Distribuição: 70% tautologias, 20% contradições, 10% aleatórias" << std::endl;
    
    // Nome do arquivo único com timestamp
    std::time_t now = std::time(nullptr);
    std::ostringstream filename;
    filename << "instances_" << now << ".txt";
    
    try {
        std::ofstream file(filename.str());
        if (!file.is_open()) {
            throw std::runtime_error("Não foi possível criar o arquivo: " + filename.str());
        }
        
        // Cabeçalho do arquivo
        file << "# Arquivo de Instâncias de Tautologias Booleanas" << std::endl;
        file << "# Gerado em: " << std::ctime(&now);
        file << "# Parâmetros: " << numInstances << " instâncias, " << minVars << "-" << maxVars 
             << " variáveis, profundidade " << maxDepth << ", seed " << seed << std::endl;
        file << "# Distribuição: 70% tautologias, 20% contradições, 10% aleatórias" << std::endl;
        file << "# Formato: INSTANCE_ID|FORMULA|TYPE|NUM_VARS" << std::endl;
        file << "#" << std::endl;
        
        //70% tautologias, 20% contradições, 10% aleatórias
        int tautologyCount = (int)(numInstances * 0.7);
        int contradictionCount = (int)(numInstances * 0.2);
        int randomCount = numInstances - tautologyCount - contradictionCount;
        
        std::cout << "Distribuição real: " << tautologyCount << " tautologias, " 
                  << contradictionCount << " contradições, " << randomCount << " aleatórias" << std::endl;
        
        for (int i = 0; i < numInstances; ++i) {
            std::string formula;
            std::string type;
            
            if (i < tautologyCount) {
                formula = generator.generateTautology();
                type = "TAUTOLOGY";
            } else if (i < tautologyCount + contradictionCount) {
                formula = generator.generateContradiction();
                type = "CONTRADICTION";
            } else {
                formula = generator.generateFormula();
                type = "RANDOM";
            }
            
            // Conta variáveis únicas na fórmula
            std::set<char> uniqueVars;
            for (char c : formula) {
                if (c >= 'a' && c <= 'z') {
                    uniqueVars.insert(c);
                }
            }
            
            // Formato: ID|FORMULA|TYPE|NUM_VARS
            file << std::setfill('0') << std::setw(3) << i << "|" 
                 << formula << "|" 
                 << type << "|" 
                 << uniqueVars.size() << std::endl;
            
            std::cout << "Instância " << std::setfill('0') << std::setw(3) << i 
                      << ": " << type << " - " << uniqueVars.size() << " vars - " << formula << std::endl;
        }
        
        file.close();
        std::cout << std::endl << "Geração concluída!" << std::endl;
        std::cout << "Arquivo criado: " << filename.str() << std::endl;
        std::cout << "Total de instâncias: " << numInstances << std::endl;
        std::cout << "Range de variáveis: " << minVars << " a " << maxVars << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro ao gerar arquivo: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
