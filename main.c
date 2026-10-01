#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"

#define DEFAULT_DB_FILE "tasks.txt"

static void print_usage(const char *prog)
{
    printf("Uso: %s <comando> [argumentos]\n\n"
           "Comandos:\n"
           "  add \"titulo\"   Cria uma nova tarefa\n"
           "  list           Lista todas as tarefas\n"
           "  done <id>      Marca a tarefa como concluída\n"
           "  rm <id>        Exclui a tarefa\n"
           "  help           Mostra esta ajuda\n\n"
           "Variável de ambiente:\n"
           "  TASKCLI_FILE   Caminho do arquivo de dados (padrão: %s)\n",
           prog, DEFAULT_DB_FILE);
}

/* Converte string em ID positivo. Retorna 0 em sucesso, -1 em erro. */
static int parse_id(const char *s, int *out)
{
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v <= 0 || v > 2147483647L)
        return -1;
    *out = (int)v;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage(argv[0]);
        return (argc < 2) ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    const char *db = getenv("TASKCLI_FILE");
    if (db == NULL || db[0] == '\0')
        db = DEFAULT_DB_FILE;

    TaskList list;
    tasklist_init(&list);

    if (tasklist_load(&list, db) != 0) {
        fprintf(stderr, "Erro: não foi possível ler '%s'.\n", db);
        tasklist_free(&list);
        return EXIT_FAILURE;
    }

    const char *cmd = argv[1];
    int rc = EXIT_SUCCESS;
    int dirty = 0; /* só regrava o arquivo se algo mudou */

    if (strcmp(cmd, "add") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Erro: informe o título. Ex.: %s add \"Estudar C\"\n", argv[0]);
            rc = EXIT_FAILURE;
        } else {
            int id = task_add(&list, argv[2]);
            if (id < 0) {
                fprintf(stderr, "Erro: não foi possível criar a tarefa.\n");
                rc = EXIT_FAILURE;
            } else {
                printf("Tarefa #%d criada.\n", id);
                dirty = 1;
            }
        }
    } else if (strcmp(cmd, "list") == 0) {
        task_print_all(&list);
    } else if (strcmp(cmd, "done") == 0 || strcmp(cmd, "rm") == 0) {
        int id;
        if (argc < 3 || parse_id(argv[2], &id) != 0) {
            fprintf(stderr, "Erro: informe um ID válido. Ex.: %s %s 1\n", argv[0], cmd);
            rc = EXIT_FAILURE;
        } else {
            int is_done = (strcmp(cmd, "done") == 0);
            int res = is_done ? task_complete(&list, id) : task_remove(&list, id);
            if (res != 0) {
                fprintf(stderr, "Erro: tarefa #%d não encontrada.\n", id);
                rc = EXIT_FAILURE;
            } else {
                printf("Tarefa #%d %s.\n", id, is_done ? "concluída" : "excluída");
                dirty = 1;
            }
        }
    } else {
        fprintf(stderr, "Comando desconhecido: '%s'\n\n", cmd);
        print_usage(argv[0]);
        rc = EXIT_FAILURE;
    }

    if (dirty && tasklist_save(&list, db) != 0) {
        fprintf(stderr, "Erro: não foi possível salvar '%s'.\n", db);
        rc = EXIT_FAILURE;
    }

    tasklist_free(&list);
    return rc;
}
