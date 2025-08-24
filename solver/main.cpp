#include "../common/ast.h"
#include "../common/utils.h"
#include "parser.h"
#include "evaluator.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

// Global variables for timeout handling
static jmp_buf timeout_jump;
static bool timeout_occurred = false;
static int timeout_seconds = 30; // Default timeout

void timeout_handler(int sig) {
    timeout_occurred = true;
    longjmp(timeout_jump, 1);
}

class TimeoutEvaluator : public Evaluator {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    int timeout_ms;
    
public:
    TimeoutEvaluator(int timeout_milliseconds = 30000) : timeout_ms(timeout_milliseconds) {}
    
    bool isTautologyWithTimeout(std::shared_ptr<ASTNode> root, const std::vector<std::string>& variables) {
        resetMetrics();
        start_time = std::chrono::high_resolution_clock::now();
        

        signal(SIGALRM, timeout_handler);
        timeout_occurred = false;
        
        alarm(timeout_seconds);
        
        if (setjmp(timeout_jump) != 0) {
            alarm(0);
            throw std::runtime_error("TIMEOUT: Formula evaluation exceeded " + std::to_string(timeout_seconds) + " seconds");
        }
        
        try {
            auto truthTable = FileUtils::generateTruthTable(variables.size());
            
            for (const auto& row : truthTable) {
                auto current_time = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
                
                if (elapsed.count() > timeout_ms) {
                    alarm(0); 
                    throw std::runtime_error("TIMEOUT: Formula evaluation exceeded " + std::to_string(timeout_ms) + " milliseconds");
                }
                
                std::unordered_map<std::string, bool> currentAssignment;
                for (size_t i = 0; i < variables.size(); ++i) {
                    currentAssignment[variables[i]] = row[i];
                }
                
                setAssignment(currentAssignment);
                
                if (!evaluate(root)) {
                    alarm(0);
                    return false;
                }
            }
            
            alarm(0);
            return true;
            
        } catch (...) {
            alarm(0); 
            throw;
        }
    }
};

class PerformanceMonitor {
private:
    std::chrono::high_resolution_clock::time_point startTime;
    long long lastExecutionTime;
    
public:
    void start() {
        startTime = std::chrono::high_resolution_clock::now();
    }
    
    void stop(const std::string& filename, bool isTautology, const Evaluator::Metrics& metrics, bool timedOut = false) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        lastExecutionTime = duration.count();
        
        std::cout << "=== RESULTADO ===" << std::endl;
        std::cout << "Instância: " << filename << std::endl;
        
        if (timedOut) {
            std::cout << "Status: TIMEOUT - Não foi possível determinar em tempo hábil" << std::endl;
        } else {
            std::cout << "É tautologia: " << (isTautology ? "SIM" : "NÃO") << std::endl;
        }
        
        std::cout << "Tempo de execução: " << duration.count() << " μs" << std::endl;
        std::cout << "Avaliações realizadas: " << metrics.evaluations << std::endl;
        std::cout << "Operações na pilha: " << metrics.stackOperations << std::endl;
        std::cout << "Tamanho máximo da pilha: " << metrics.maxStackSize << std::endl;
        std::cout << "=================" << std::endl << std::endl;
    }
    
    long long getLastExecutionTime() const {
        return lastExecutionTime;
    }
};

void printUsage() {
    std::cout << "Uso: ./solver_timeout <arquivo> [opcoes]" << std::endl;
    std::cout << "  arquivo: Caminho para arquivo de fórmula ou instâncias" << std::endl;
    std::cout << "Opções:" << std::endl;
    std::cout << "  -t, --timeout SECONDS: Define timeout em segundos (padrão: 30)" << std::endl;
    std::cout << "  -i, --instance ID: Testa apenas a instância com ID específico" << std::endl;
    std::cout << "  -a, --all: Testa todas as instâncias do arquivo" << std::endl;
    std::cout << "  -v, --verbose: Mostra informações detalhadas" << std::endl;
    std::cout << "  -d, --debug: Mostra a árvore de sintaxe" << std::endl;
    std::cout << "  -s, --summary: Mostra apenas resumo" << std::endl;
    std::cout << "  -h, --help: Mostra esta mensagem" << std::endl;
    std::cout << std::endl;
    std::cout << "Exemplos:" << std::endl;
    std::cout << "  ./solver_timeout formula.txt -t 60 -v    # Timeout de 60 segundos" << std::endl;
    std::cout << "  ./solver_timeout instances.txt -a -t 10  # Timeout de 10 segundos para todas" << std::endl;
}

void testFormulaWithTimeout(const std::string& formula, const std::string& id, const std::string& type, 
                           bool verbose, bool debug, PerformanceMonitor& monitor, int timeoutSeconds) {
    
    if (verbose) {
        std::cout << "\n=== TESTANDO FÓRMULA " << id << " ===" << std::endl;
        std::cout << "Fórmula: " << formula << std::endl;
        std::cout << "Timeout configurado: " << timeoutSeconds << " segundos" << std::endl;
        if (!type.empty()) {
            std::cout << "Tipo esperado: " << type << std::endl;
        }
    }
    
    try {
        // Parse da fórmula
        Parser parser(formula);
        auto ast = parser.parse();
        
        if (debug) {
            std::cout << "\n=== ÁRVORE DE SINTAXE ===" << std::endl;
            ast->print();
            std::cout << "========================\n" << std::endl;
        }
        
        // Coleta variáveis
        std::vector<std::string> variables;
        ast->collectVariables(variables);
        std::sort(variables.begin(), variables.end());
        
        if (verbose) {
            std::cout << "Variáveis encontradas (" << variables.size() << "): ";
            for (size_t i = 0; i < variables.size(); ++i) {
                std::cout << variables[i];
                if (i < variables.size() - 1) std::cout << ", ";
            }
            std::cout << std::endl;
            
            long long totalCombinations = 1LL << variables.size();
            std::cout << "Total de combinações a testar: " << totalCombinations << std::endl;
            
            if (variables.size() > 20) {
                std::cout << "AVISO: Fórmula com muitas variáveis (" << variables.size() 
                          << "). Pode exceder o timeout!" << std::endl;
            }
        }
        
        // Verifica se é tautologia com timeout
        monitor.start();
        timeout_seconds = timeoutSeconds; // Set global timeout
        TimeoutEvaluator evaluator(timeoutSeconds * 1000);
        
        bool isTautology = evaluator.isTautologyWithTimeout(ast, variables);
        monitor.stop(id, isTautology, evaluator.metrics, false);
        
        if (!type.empty()) {
            bool expectedTautology = (type == "TAUTOLOGY");
            if (isTautology == expectedTautology || isTautology == (type == "RANDOM")) {
                std::cout << "✓ Resultado consistente com tipo esperado!" << std::endl;
            } else {
                std::cout << "AVISO: Resultado inconsistente! Esperado: " 
                          << (expectedTautology ? "tautologia" : "não-tautologia") 
                          << ", obtido: " << (isTautology ? "tautologia" : "não-tautologia") << std::endl;
            }
        }
        
    } catch (const std::runtime_error& e) {
        std::string error_msg = e.what();
        if (error_msg.find("TIMEOUT") != std::string::npos) {
            TimeoutEvaluator dummy_evaluator;
            monitor.stop(id, false, dummy_evaluator.metrics, true);
            std::cout << "⏰ " << error_msg << std::endl;
        } else {
            throw; 
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    std::string filename = argv[1];
    std::string instanceId;
    int timeoutSeconds = 30;
    bool verbose = false;
    bool debug = false;
    bool testAll = false;
    bool summaryOnly = false;
    

    for (int i = 2; i < argc; ++i) {
        if ((strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) && i + 1 < argc) {
            timeoutSeconds = std::stoi(argv[++i]);
        } else if ((strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--instance") == 0) && i + 1 < argc) {
            instanceId = argv[++i];
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            testAll = true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug = true;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--summary") == 0) {
            summaryOnly = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage();
            return 0;
        }
    }
    
    if (timeoutSeconds <= 0) {
        std::cerr << "Erro: Timeout deve ser um valor positivo" << std::endl;
        return 1;
    }
    
    try {
        PerformanceMonitor monitor;
        
        // Detect file type
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Não foi possível abrir o arquivo: " + filename);
        }
        
        std::string firstLine;
        std::getline(file, firstLine);
        file.close();
        
        bool isInstancesFile = (firstLine.find("# Arquivo de Instâncias") != std::string::npos);
        
        if (isInstancesFile) {
            if (!instanceId.empty()) {
                // Test specific instance
                auto instance = FileUtils::readInstanceById(filename, instanceId);
                testFormulaWithTimeout(instance.formula, instance.id, instance.type, 
                                     verbose, debug, monitor, timeoutSeconds);
                
            } else if (testAll) {
                // Test all instances
                auto instances = FileUtils::readInstances(filename);
                
                std::cout << "Testando " << instances.size() << " instâncias com timeout de " 
                          << timeoutSeconds << " segundos cada..." << std::endl;
                
                int successCount = 0;
                int timeoutCount = 0;
                int tautologyCount = 0;
                
                for (const auto& instance : instances) {
                    try {
                        testFormulaWithTimeout(instance.formula, instance.id, instance.type, 
                                             verbose && !summaryOnly, debug, monitor, timeoutSeconds);
                        successCount++;
                        
                        Parser parser(instance.formula);
                        auto ast = parser.parse();
                        std::vector<std::string> variables;
                        ast->collectVariables(variables);
                        
                        if (variables.size() <= 15) { 
                            TimeoutEvaluator evaluator(timeoutSeconds * 1000);
                            if (evaluator.isTautologyWithTimeout(ast, variables)) {
                                tautologyCount++;
                            }
                        }
                        
                    } catch (const std::runtime_error& e) {
                        if (std::string(e.what()).find("TIMEOUT") != std::string::npos) {
                            timeoutCount++;
                            if (!summaryOnly) {
                                std::cout << "Instância " << instance.id << " excedeu timeout" << std::endl;
                            }
                        } else {
                            throw;
                        }
                    }
                }
                
                // Final summary
                std::cout << "\n=== RESUMO FINAL ===" << std::endl;
                std::cout << "Total de instâncias: " << instances.size() << std::endl;
                std::cout << "Processadas com sucesso: " << successCount << std::endl;
                std::cout << "Timeouts: " << timeoutCount << std::endl;
                std::cout << "Tautologias encontradas: " << tautologyCount << std::endl;
                std::cout << "==================" << std::endl;
                
            } else {
                // List available instances
                auto instances = FileUtils::readInstances(filename);
                std::cout << "Arquivo contém " << instances.size() << " instâncias." << std::endl;
                std::cout << "Timeout configurado: " << timeoutSeconds << " segundos" << std::endl;
                std::cout << "Use -a para testar todas ou -i ID para testar uma específica." << std::endl;
            }
            
        } else {
            // Single formula file
            std::string formula = FileUtils::readFormula(filename);
            testFormulaWithTimeout(formula, filename, "", verbose, debug, monitor, timeoutSeconds);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
