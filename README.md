# 📝 TaskCLI

Gerenciador de tarefas de linha de comando escrito em **C11**, com persistência em arquivo, alocação dinâmica de memória e saída colorida no terminal.

![Linguagem](https://img.shields.io/badge/linguagem-C11-blue)
![Build](https://img.shields.io/badge/build-make-green)
![Licença](https://img.shields.io/badge/licença-MIT-lightgrey)

---

## 📖 Sobre o projeto

O **TaskCLI** permite criar, listar, concluir e excluir tarefas diretamente pelo terminal. Todas as tarefas são salvas em um arquivo de texto, então nada se perde ao fechar o programa.

É um projeto pequeno e propositalmente enxuto, pensado para **praticar os fundamentos de C** em um cenário realista, com código organizado em módulos, tratamento de erros e boas práticas de memória.

### Para que serve?

- **Aprendizado:** consolida `struct`, ponteiros, alocação dinâmica (`malloc`/`realloc`/`free`), manipulação de arquivos (`FILE *`) e modularização.
- **Portfólio:** demonstra domínio de C além do básico: código modular, `Makefile`, flags rigorosas de compilação e verificação com sanitizers.
- **Uso real:** pode ser usado no dia a dia como uma lista de afazeres rápida, sem abrir nenhum aplicativo.

### Onde pode ser usado?

- No terminal do dia a dia (Linux, macOS, WSL).
- Em scripts de automação (ex.: `./taskcli add "Fazer backup"` dentro de um shell script ou cron).
- Como base didática para aulas e cursos de programação em C.

---

## ✨ Funcionalidades

| Comando        | Descrição                                   |
| -------------- | ------------------------------------------- |
| `add "titulo"` | Cria uma nova tarefa                        |
| `list`         | Lista todas as tarefas, com status e resumo |
| `done <id>`    | Marca uma tarefa como concluída             |
| `rm <id>`      | Exclui uma tarefa                           |
| `help`         | Mostra a ajuda                              |

Destaques técnicos:

- 💾 **Persistência** em arquivo de texto legível (`id|status|titulo`).
- 🧠 **Array dinâmico** na RAM (`realloc` com crescimento por dobro).
- 🔒 **Gravação segura:** escreve em arquivo temporário e usa `rename`, evitando corromper o arquivo original em caso de falha.
- 🎨 **Cores ANSI**, desativadas automaticamente quando a saída não é um terminal ou quando `NO_COLOR` está definido.
- 🛡️ **Validação de entrada:** IDs validados com `strtol`; caracteres que quebrariam o formato do arquivo são sanitizados.
- 🔢 **IDs estáveis:** o ID de uma tarefa excluída nunca é reaproveitado.

---

## 🚀 Como começar

### Pré-requisitos

- Compilador C com suporte a C11 (`gcc` ou `clang`)
- `make`

No Ubuntu/Debian:

```bash
sudo apt install build-essential
```

### Compilar

```bash
git clone https://github.com/Lazarin123/TaskCLI.git
cd taskcli
make
```

O executável `taskcli` será gerado na raiz do projeto.

### Usar

```bash
./taskcli add "Estudar ponteiros"
./taskcli add "Montar portfólio"
./taskcli list
./taskcli done 1
./taskcli rm 2
```

Exemplo de saída do `list`:

```text
 ID  Status  Tarefa
--------------------------------------
  1  [x]     Estudar ponteiros
  3  [ ]     Montar portfólio
--------------------------------------
1 de 2 concluída(s)
```

(No terminal, tarefas concluídas aparecem em verde e pendentes em amarelo.)

### Instalar globalmente (opcional)

```bash
sudo cp taskcli /usr/local/bin/
taskcli list
```

---

## ⚙️ Configuração

| Variável de ambiente | Descrição                      | Padrão                        |
| -------------------- | ------------------------------ | ----------------------------- |
| `TASKCLI_FILE`       | Caminho do arquivo de dados    | `tasks.txt` (diretório atual) |
| `NO_COLOR`           | Se definida, desativa as cores | não definida                  |

Exemplo, usando uma lista separada por projeto:

```bash
TASKCLI_FILE=~/projetos/site/tasks.txt ./taskcli list
```

---

## 🗂️ Estrutura do projeto

```text
taskcli/
├── main.c      # Entrada do programa: interpreta comandos e orquestra o fluxo
├── task.h      # Interface pública: structs (Task, TaskList) e protótipos
├── task.c      # Implementação: lista dinâmica, persistência, operações e exibição
├── Makefile    # Build automatizado com flags rigorosas
├── .gitignore
├── LICENSE
└── README.md
```

### Modelagem de dados

```c
typedef struct {
    int        id;
    TaskStatus status;                 /* TASK_PENDING ou TASK_DONE */
    char       title[TASK_TITLE_MAX];
} Task;

typedef struct {
    Task  *items;     /* array dinâmico */
    size_t count;
    size_t capacity;
    int    next_id;
} TaskList;
```

### Formato do arquivo de dados

Uma tarefa por linha, campos separados por `|`:

```text
1|1|Estudar ponteiros
3|0|Montar portfólio
```

O segundo campo é o status (`0` = pendente, `1` = concluída).

### Fluxo de execução

1. Carrega o arquivo para um array dinâmico na memória.
2. Executa o comando pedido sobre o array.
3. Se algo mudou, regrava o arquivo de forma segura (temporário + `rename`).
4. Libera a memória e encerra.

---

## 🛠️ Desenvolvimento

### Targets do Makefile

| Comando      | O que faz                                                        |
| ------------ | ---------------------------------------------------------------- |
| `make`       | Compila com `-Wall -Wextra -std=c11 -pedantic -O2`               |
| `make debug` | Compila com símbolos de depuração e **AddressSanitizer + UBSan** |
| `make run`   | Compila e executa `list`                                         |
| `make clean` | Remove binários e arquivos temporários                           |

### Verificando vazamentos de memória

```bash
make debug
./taskcli add "teste"
./taskcli list
```

Se houver vazamento ou acesso inválido, o sanitizer exibirá um relatório detalhado.

### Compatibilidade

Desenvolvido para Linux e macOS. O código contém proteções para Windows (`_WIN32`), mas o `rename` do Windows não sobrescreve arquivos existentes; para uso nativo no Windows, recomenda-se WSL.

---

## 🎓 Conceitos praticados

- Estruturas (`struct`) e enumerações (`enum`)
- Manipulação de arquivos (`fopen`, `fgets`, `sscanf`, `fprintf`, `fclose`, `rename`)
- Ponteiros e alocação dinâmica (`realloc`, `memmove`, `free`)
- Modularização em múltiplos arquivos (`.h` / `.c`) e _include guards_
- Tratamento de erros e códigos de retorno
- Códigos de escape ANSI
- Automação de build com `Makefile`

---

## 📄 Licença

Distribuído sob a licença MIT. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.
