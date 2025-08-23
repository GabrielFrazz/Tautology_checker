**Estrutura básica:**
```
src/
├── common/
│   ├── ast.h/.cpp          # AST Node apenas
│   └── utils.h/.cpp        # File I/O básico
├── generator/
│   └── main.cpp            # Gerador super simples
└── solver/
    ├── parser.h/.cpp       # Parser simples
    ├── evaluator.h/.cpp    # Evaluator com recursão simples
    └── main.cpp            # Main solver
```