# Problemas Clássicos de Sistemas Operacionais

Este repositório contém implementações em **C** dos principais problemas clássicos de sincronização em sistemas operacionais, utilizando **PThreads** para manipulação de threads e controle de concorrência.

## Problemas Implementados

### 1. Jantar dos Filósofos
- Simulação do problema onde **N filósofos** compartilham **N garfos** e devem evitar um impasse (deadlock).
- Utiliza **mutex** e **variáveis de condição** para gerenciar o acesso aos recursos compartilhados.
- Garante que os filósofos possam comer e pensar sem entrar em estado de espera infinito.

### 2. Problema Leitor/Escritor
- Implementa a gestão de acesso concorrente a um recurso compartilhado onde **leitores podem acessar simultaneamente**, mas **escritores requerem acesso exclusivo**.
- Utiliza **mutex** e **variáveis de condição** para coordenar as threads leitoras e escritoras.
- Previne **inanição** (starvation) garantindo acesso justo aos escritores.

## Tecnologias Utilizadas
- **Linguagem:** C
- **Biblioteca:** PThreads (POSIX Threads)
- **Mecanismos de Sincronização:** Mutex, Semáforos e Variáveis de Condição

## Como Executar os Códigos

1. Clone o repositório:
   ```bash
   git clone https://github.com/CamiBregalda/Problemas-Classicos-de-Sistemas-Operacionais.git
   cd problemas-classicos-de-sistemas-operacionais
   ```
2. Compile e execute cada problema

## Autor
- **Camila Bregalda** - Desenvolvedora do projeto.

