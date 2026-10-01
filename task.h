#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#define TASK_TITLE_MAX 128

typedef enum {
    TASK_PENDING = 0,
    TASK_DONE    = 1
} TaskStatus;

typedef struct {
    int        id;
    TaskStatus status;
    char       title[TASK_TITLE_MAX];
} Task;

/* Array dinâmico de tarefas, carregado na RAM. */
typedef struct {
    Task  *items;
    size_t count;
    size_t capacity;
    int    next_id;
} TaskList;

/* Ciclo de vida */
void tasklist_init(TaskList *list);
void tasklist_free(TaskList *list);

/* Persistência (formato texto: id|status|titulo, uma tarefa por linha).
 * Retornam 0 em sucesso e -1 em erro. Arquivo inexistente no load não é erro. */
int tasklist_load(TaskList *list, const char *path);
int tasklist_save(const TaskList *list, const char *path);

/* Operações. task_add retorna o novo ID (>0) ou -1 em erro.
 * Demais retornam 0 em sucesso, -1 se o ID não existe. */
int task_add(TaskList *list, const char *title);
int task_complete(TaskList *list, int id);
int task_remove(TaskList *list, int id);

/* Saída colorida (ANSI) no terminal. */
void task_print_all(const TaskList *list);

#endif /* TASK_H */
