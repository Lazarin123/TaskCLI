#define _POSIX_C_SOURCE 200809L /* isatty / fileno */

#include "task.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define INITIAL_CAPACITY 8
#define LINE_BUF_SIZE    512
#define PATH_BUF_SIZE    512

/* ---------- Cores ANSI ---------- */

#define ANSI_RESET  "\x1b[0m"
#define ANSI_BOLD   "\x1b[1m"
#define ANSI_DIM    "\x1b[2m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_CYAN   "\x1b[36m"

/* Só colore se for um terminal e NO_COLOR não estiver definido. */
static int use_color(void)
{
    static int cached = -1;
    if (cached < 0) {
        cached = (getenv("NO_COLOR") == NULL);
#ifndef _WIN32
        if (cached && !isatty(fileno(stdout)))
            cached = 0;
#endif
    }
    return cached;
}

static const char *col(const char *code)
{
    return use_color() ? code : "";
}

/* ---------- Helpers internos ---------- */

static int ensure_capacity(TaskList *list)
{
    if (list->count < list->capacity)
        return 0;

    size_t new_cap = list->capacity ? list->capacity * 2 : INITIAL_CAPACITY;
    Task *tmp = realloc(list->items, new_cap * sizeof *tmp);
    if (tmp == NULL)
        return -1; /* list->items continua válido */

    list->items = tmp;
    list->capacity = new_cap;
    return 0;
}

static long find_index(const TaskList *list, int id)
{
    for (size_t i = 0; i < list->count; i++) {
        if (list->items[i].id == id)
            return (long)i;
    }
    return -1;
}

/* ---------- Ciclo de vida ---------- */

void tasklist_init(TaskList *list)
{
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    list->next_id = 1;
}

void tasklist_free(TaskList *list)
{
    free(list->items);
    tasklist_init(list);
}

/* ---------- Persistência ---------- */

int tasklist_load(TaskList *list, const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
        return (errno == ENOENT) ? 0 : -1;

    char line[LINE_BUF_SIZE];
    while (fgets(line, sizeof line, f) != NULL) {
        int id, status;
        char title[TASK_TITLE_MAX];

        /* 127 = TASK_TITLE_MAX - 1 */
        if (sscanf(line, "%d|%d|%127[^\n]", &id, &status, title) != 3)
            continue; /* ignora linhas corrompidas */
        if (status != TASK_PENDING && status != TASK_DONE)
            continue;

        if (ensure_capacity(list) != 0) {
            fclose(f);
            return -1;
        }

        Task *t = &list->items[list->count++];
        t->id = id;
        t->status = (TaskStatus)status;
        snprintf(t->title, sizeof t->title, "%s", title);

        if (id >= list->next_id)
            list->next_id = id + 1;
    }

    fclose(f);
    return 0;
}

/* Escreve em arquivo temporário e renomeia: se algo falhar no meio,
 * o arquivo original não é corrompido. */
int tasklist_save(const TaskList *list, const char *path)
{
    char tmp_path[PATH_BUF_SIZE];
    if (snprintf(tmp_path, sizeof tmp_path, "%s.tmp", path) >= (int)sizeof tmp_path)
        return -1;

    FILE *f = fopen(tmp_path, "w");
    if (f == NULL)
        return -1;

    for (size_t i = 0; i < list->count; i++) {
        const Task *t = &list->items[i];
        if (fprintf(f, "%d|%d|%s\n", t->id, (int)t->status, t->title) < 0) {
            fclose(f);
            remove(tmp_path);
            return -1;
        }
    }

    if (fclose(f) != 0) {
        remove(tmp_path);
        return -1;
    }
    if (rename(tmp_path, path) != 0) {
        remove(tmp_path);
        return -1;
    }
    return 0;
}

/* ---------- Operações ---------- */

int task_add(TaskList *list, const char *title)
{
    if (title == NULL || title[0] == '\0')
        return -1;
    if (ensure_capacity(list) != 0)
        return -1;

    Task *t = &list->items[list->count];
    t->id = list->next_id;
    t->status = TASK_PENDING;
    snprintf(t->title, sizeof t->title, "%s", title);

    /* '|' e quebras de linha quebrariam o formato do arquivo. */
    for (char *p = t->title; *p; p++) {
        if (*p == '|' || *p == '\n' || *p == '\r')
            *p = ' ';
    }

    list->count++;
    return list->next_id++;
}

int task_complete(TaskList *list, int id)
{
    long idx = find_index(list, id);
    if (idx < 0)
        return -1;
    list->items[idx].status = TASK_DONE;
    return 0;
}

int task_remove(TaskList *list, int id)
{
    long idx = find_index(list, id);
    if (idx < 0)
        return -1;

    size_t i = (size_t)idx;
    memmove(&list->items[i], &list->items[i + 1],
            (list->count - i - 1) * sizeof list->items[0]);
    list->count--;
    return 0;
}

/* ---------- Exibição ---------- */

void task_print_all(const TaskList *list)
{
    if (list->count == 0) {
        printf("%sNenhuma tarefa cadastrada.%s\n", col(ANSI_DIM), col(ANSI_RESET));
        return;
    }

    printf("%s%s ID  Status  Tarefa%s\n", col(ANSI_BOLD), col(ANSI_CYAN), col(ANSI_RESET));
    printf("%s--------------------------------------%s\n", col(ANSI_DIM), col(ANSI_RESET));

    size_t done = 0;
    for (size_t i = 0; i < list->count; i++) {
        const Task *t = &list->items[i];
        if (t->status == TASK_DONE) {
            done++;
            printf("%s%3d  [x]     %s%s\n",
                   col(ANSI_GREEN), t->id, t->title, col(ANSI_RESET));
        } else {
            printf("%s%3d  [ ]     %s%s\n",
                   col(ANSI_YELLOW), t->id, t->title, col(ANSI_RESET));
        }
    }

    printf("%s--------------------------------------%s\n", col(ANSI_DIM), col(ANSI_RESET));
    printf("%s%zu de %zu concluída(s)%s\n",
           col(ANSI_DIM), done, list->count, col(ANSI_RESET));
}
