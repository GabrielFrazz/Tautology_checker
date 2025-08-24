CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Diretórios
COMMON_DIR = common
GENERATOR_DIR = generator
SOLVER_DIR = solver
BUILD_DIR = build
BIN_DIR = bin

# Arquivos fonte
COMMON_SOURCES = $(COMMON_DIR)/ast.cpp $(COMMON_DIR)/utils.cpp
GENERATOR_SOURCES = $(GENERATOR_DIR)/generator.cpp
SOLVER_SOURCES = $(SOLVER_DIR)/parser.cpp $(SOLVER_DIR)/evaluator.cpp $(SOLVER_DIR)/main.cpp

# Arquivos objeto
COMMON_OBJECTS = $(patsubst $(COMMON_DIR)/%.cpp,$(BUILD_DIR)/common/%.o,$(COMMON_SOURCES))
GENERATOR_OBJECTS = $(patsubst $(GENERATOR_DIR)/%.cpp,$(BUILD_DIR)/generator/%.o,$(GENERATOR_SOURCES))
SOLVER_OBJECTS = $(patsubst $(SOLVER_DIR)/%.cpp,$(BUILD_DIR)/solver/%.o,$(SOLVER_SOURCES))

# Executáveis
GENERATOR_BIN = $(BIN_DIR)/generator
SOLVER_BIN = $(BIN_DIR)/solver

# Targets principais
.PHONY: all clean generate-simple generate-medium generate-hard test help

all: $(GENERATOR_BIN) $(SOLVER_BIN)

# Compilação
$(GENERATOR_BIN): $(COMMON_OBJECTS) $(GENERATOR_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SOLVER_BIN): $(COMMON_OBJECTS) $(SOLVER_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilação dos objetos
$(BUILD_DIR)/common/%.o: $(COMMON_DIR)/%.cpp | $(BUILD_DIR)/common
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/generator/%.o: $(GENERATOR_DIR)/%.cpp | $(BUILD_DIR)/generator
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/solver/%.o: $(SOLVER_DIR)/%.cpp | $(BUILD_DIR)/solver
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Criação de diretórios
$(BUILD_DIR)/common $(BUILD_DIR)/generator $(BUILD_DIR)/solver:
	@mkdir -p $@

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Limpeza
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@rm -f instances_*.txt 2>/dev/null || true
	@echo "Arquivos removidos"

# Geração de instâncias
generate-simple: $(GENERATOR_BIN)
	@echo "Gerando instâncias simples (3-5 variáveis)..."
	$(GENERATOR_BIN) 30 3 5 4

generate-medium: $(GENERATOR_BIN)
	@echo "Gerando instâncias médias (10-15 variáveis)..."
	$(GENERATOR_BIN) 20 10 15 6

generate-hard: $(GENERATOR_BIN)
	@echo "Gerando instâncias difíceis (20-26 variáveis)..."
	$(GENERATOR_BIN) 10 20 26 8

# Teste das instâncias
test: $(SOLVER_BIN)
	@echo "Testando instâncias..."
	@for f in instances_*.txt; do \
		if [ -f "$$f" ]; then \
			echo "Testando arquivo: $$f"; \
			$(SOLVER_BIN) "$$f" -a -t 30 -s; \
			echo ""; \
		fi; \
	done
	@if ! ls instances_*.txt >/dev/null 2>&1; then \
		echo "Nenhuma instância encontrada! Execute 'make generate-simple' primeiro."; \
	fi

# Ajuda
help:
	@echo "Targets disponíveis:"
	@echo "  all              - Compila gerador e solver"
	@echo "  generate-simple  - Gera 30 instâncias simples (3-5 variáveis)"
	@echo "  generate-medium  - Gera 20 instâncias médias (10-15 variáveis)"
	@echo "  generate-hard    - Gera 10 instâncias difíceis (20-26 variáveis)"
	@echo "  test             - Testa todas as instâncias com timeout de 30s"
	@echo "  clean            - Remove arquivos de build e instâncias"
	@echo "  help             - Mostra esta mensagem"
	@echo ""
	@echo "Distribuição de tipos:"
	@echo "  70% Tautologias, 20% Contradições, 10% Aleatórias"
	@echo ""
	@echo "Fluxo típico:"
	@echo "  make                    # Compila tudo"
	@echo "  make generate-simple    # Gera instâncias simples"
	@echo "  make test               # Testa as instâncias"
