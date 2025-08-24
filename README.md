# Tautology Checker

Verificador de tautologias booleanas desenvolvido para a disciplina de **Projeto e Análise de Algoritmos (PAA)**. Foi usado um algoritmo de força bruta para determinar se fórmulas lógicas proposicionais são tautologias.

## Características

- **Parser** para fórmulas com operadores Unicode (`¬`, `∧`, `∨`, `→`)
- **Avaliação iterativa** (não recursiva) para evitar stack overflow
- **Sistema de timeout** configurável para controle de tempo de execução
- **Gerador de instâncias** com distribuição controlada de tipos
- **Métricas de performance** tempo e número de 

## Estrutura do Projeto

```
├── common/
│   ├── ast.h/.cpp          # AST (Abstract Syntax Tree)
│   └── utils.h/.cpp        # Utilitários de I/O e manipulação
├── generator/
│   └── generator.cpp       # Gerador de instâncias de teste
├── solver/
│   ├── parser.h/.cpp       # Parser de fórmulas lógicas
│   ├── evaluator.h/.cpp    # Avaliador com métricas
│   └── main.cpp            # Programa principal do solver
├── Makefile                # Build para Linux/Unix
├── Makefile.windows        # Build para Windows
```

## Compilação

### Linux/Unix/macOS
```bash
make                   # Compila tudo
make clean             # Remove arquivos temporários
make help              # Mostra ajuda
```

### Windows
```bash
make -f Makefile.windows        # Compila tudo
make -f Makefile.windows help   # Mostra ajuda
```

## Uso

### Geração de Instâncias

```bash
# Instâncias simples (3-5 variáveis, 30 instâncias)
make generate-simple

# Instâncias médias (10-15 variáveis, 20 instâncias)  
make generate-medium

# Instâncias difíceis (20-26 variáveis, 10 instâncias)
make generate-hard
```

**Distribuição dos tipos gerados:**
- 70% Tautologias conhecidas
- 20% Contradições conhecidas  
- 10% Fórmulas aleatórias

### Verificação de Tautologias

```bash
# Testar todas as instâncias com timeout de 30s
make test

# Testar arquivo específico
./bin/solver instances_1724534567.txt -a -t 30 -s

# Testar instância específica por ID
./bin/solver instances_1724534567.txt -i 001 -v -d

```

### Opções do Solver

```
-t, --timeout SECONDS    Timeout em segundos (padrão: 30)
-i, --instance ID        Testa apenas instância específica
-a, --all                Testa todas as instâncias do arquivo
-v, --verbose            Informações detalhadas
-d, --debug              Mostra árvore de sintaxe (AST)
-s, --summary            Apenas resumo final
-h, --help               Ajuda
```

## Algoritmo

O verificador utiliza **força bruta**:

1. **Parse:** Converte fórmula em AST com precedência correta
2. **Coleta de variáveis:** Identifica variáveis únicas na fórmula
3. **Tabela verdade:** Gera todas as 2^n combinações possíveis
4. **Avaliação iterativa:** Testa cada combinação usando pilha explícita
5. **Resultado:** Tautologia se todas as avaliações forem verdadeiras

**Complexidade:** O(2^n × m) onde n = número de variáveis, m = tamanho da fórmula

## Exemplos de Uso Completo

### Fluxo Típico
```bash
# 1. Compilar
make

# 2. Gerar instâncias de teste
make generate-simple

# 3. Testar todas (com timeout)
make test

# 4. Analisar instância específica
./bin/solver instances_*.txt -i 005 -v -d
```

