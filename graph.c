#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "graph.h"

#define INITIAL_CAPACITY 10 // начальный размер массива вершин
#define HASH_CAPACITY 1024 // размер хэш-таблицы для имён
#define INF INT_MAX // это для заполнения изначальной матрицы расстояний

#define RED        "\x1b[1;31m"
#define CYAN       "\x1b[1;36m"
#define RESET      "\x1b[0m"

// --- Очередь для BFS ---
// Узел очереди — элемент связного списка
typedef struct QueueNode {
    int index;                // Индекс вершины в графе
    struct QueueNode* next;  // Указатель на следующий элемент в очереди
} QueueNode;

// Очередь (FIFO) — используется для обхода графа (например, BFS)
typedef struct {
    QueueNode* front;  // Первый элемент очереди (голова)
    QueueNode* rear;   // Последний элемент очереди (хвост)
} Queue;


// ----------- Хэш-таблица -------------- //
static unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

static NameHashTable* create_name_table() {
    NameHashTable* ht = malloc(sizeof(NameHashTable));
    if (!ht) return NULL;
    ht->capacity = HASH_CAPACITY;
    ht->table = calloc(ht->capacity, sizeof(NameHashNode*));
    return ht;
}

static void free_name_table(NameHashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < ht->capacity; i++) {
        NameHashNode* node = ht->table[i];
        while (node) {
            NameHashNode* tmp = node;
            node = node->next;
            free(tmp->name);
            free(tmp);
        }
    }
    free(ht->table);
    free(ht);
}

static int name_table_insert(NameHashTable* ht, const char* name, int index) {
    if (!ht || !name) return -1;
    unsigned long h = hash_string(name) % ht->capacity;
    NameHashNode* cur = ht->table[h];
    while (cur) {
        if (strcmp(cur->name, name) == 0)
            return -1;
        cur = cur->next;
    }
    NameHashNode* node = malloc(sizeof(NameHashNode));
    if (!node) return -1;
    node->name = strdup(name);
    node->index = index;
    node->next = ht->table[h];
    ht->table[h] = node;
    return 0;
}

static int name_table_find(NameHashTable* ht, const char* name) {
    if (!ht || !name) return -1;
    unsigned long h = hash_string(name) % ht->capacity;
    NameHashNode* node = ht->table[h];
    while (node) {
        if (strcmp(node->name, name) == 0)
            return node->index;
        node = node->next;
    }
    return -1;
}

// ----------- Граф -------------- //
Graph* create_graph() {
    Graph* g = malloc(sizeof(Graph));
    if (!g) return NULL;
    g->size = 0;
    g->capacity = INITIAL_CAPACITY;
    g->vertices = malloc(sizeof(Vertex) * g->capacity);
    if (!g->vertices) { free(g); return NULL; }
    g->name_index = create_name_table();
    if (!g->name_index) { free(g->vertices); free(g); return NULL; }
    return g;
}

void free_graph(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->size; i++) {
        free(g->vertices[i].person.name);
        Edge* e = g->vertices[i].edges;
        while (e) { Edge* tmp = e; e = e->next; free(tmp); }
    }
    free(g->vertices);
    free_name_table(g->name_index);
    free(g);
}

int find_person_index(Graph* g, const char* name) {
    if (!g) return -1;
    return name_table_find(g->name_index, name);
}

int add_person(Graph* g, Person p) {
    if (!g || !p.name) return -1;
    if (find_person_index(g, p.name) != -1) return -1;
    if (g->size >= g->capacity) {
        int new_cap = g->capacity * 2;
        Vertex* tmp = realloc(g->vertices, sizeof(Vertex) * new_cap);
        if (!tmp) return -1;
        g->vertices = tmp;
        g->capacity = new_cap;
    }
    Vertex* v = &g->vertices[g->size];
    v->person.name = strdup(p.name);
    v->person.gender = p.gender;
    v->person.birth_year = p.birth_year;
    v->person.death_year = p.death_year;
    v->edges = NULL;
    if (name_table_insert(g->name_index, v->person.name, g->size) != 0) {
        free(v->person.name);
        return -1;
    }
    g->size++;
    return g->size - 1;
}

int add_relation(Graph* g, const char* from, const char* to, RelationType relation) {
    if (!g||!from||!to) return -1;
    int fi = find_person_index(g, from);
    int ti = find_person_index(g, to);
    if (fi<0||ti<0) return -1;
    Edge* edge = malloc(sizeof(Edge));
    if (!edge) return -1;
    edge->to = ti;
    edge->relation = relation;
    edge->next = g->vertices[fi].edges;
    g->vertices[fi].edges = edge;
    return 0;
}

// --- Удаление ребра ---
int remove_relation(Graph* g, const char* from, const char* to, RelationType relation) {
    if (!g||!from||!to) return -1;
    int fi = find_person_index(g, from);
    int ti = find_person_index(g, to);
    if (fi<0||ti<0) return -1;
    Edge** pp = &g->vertices[fi].edges;
    while (*pp) {
        Edge* cur = *pp;
        if (cur->to==ti && cur->relation==relation) {
            *pp = cur->next;
            free(cur);
            return 0;
        }
        pp = &cur->next;
    }
    return -1;
}

// --- Удаление вершины ---
int remove_person(Graph* g, const char* name) {
    if (!g||!name) return -1;
    int idx = find_person_index(g, name);
    if (idx<0) return -1;
    // Удаляем все исходящие ребра
    Edge* e = g->vertices[idx].edges;
    while (e) { Edge* tmp = e; e = e->next; free(tmp); }
    // Удаляем все входящие ребра
    for (int i = 0; i < g->size; i++) {
        if (i==idx) continue;
        Edge** pp = &g->vertices[i].edges;
        while (*pp) {
            Edge* cur = *pp;
            if (cur->to == idx) {
                *pp = cur->next;
                free(cur);
            } else pp = &cur->next;
        }
    }
    // Освобождаем имя
    free(g->vertices[idx].person.name);
    // Сдвигаем массив вершин
    for (int i = idx; i < g->size-1; i++) {
        g->vertices[i] = g->vertices[i+1];
    }
    g->size--;
    // Перестраиваем хэш-таблицу
    free_name_table(g->name_index);
    g->name_index = create_name_table();
    for (int i = 0; i < g->size; i++) {
        name_table_insert(g->name_index, g->vertices[i].person.name, i);
    }
    return 0;
}

// --- Алгоритм Флойда-Уоршелла --- //
static int** floyd_warshall(Graph* g) { // двойной указатель потому что возвращаем матрицу расстояний - двумерный массив
    int n = g->size;
    int** dist = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        dist[i] = malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) {
            dist[i][j] = (i == j ? 0 : INF);
        }
    }
    // инициализация по рёбрам
    for (int i = 0; i < n; i++) {
        Edge* e = g->vertices[i].edges;
        while (e) {
            dist[i][e->to] = 1;
            e = e->next;
        }
    }
    // основной цикл
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < n; j++) {
                if (dist[k][j] == INF) continue;
                if (dist[i][k] + dist[k][j] < dist[i][j]) { // если нашли более короткий путь
                    // обновляем расстояние
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
    return dist;
}


// --- Распределение наследства --- //
void distribute_inheritance(Graph* g, const char* name, double amount) {
    int start = find_person_index(g, name);
    if (start == -1) {
        printf(RED "Человек '%s' не найден\n" RESET, name);
        return;
    }
    int n = g->size;

    // получаем матрицу кратчайших путей
    int** dist = floyd_warshall(g);

    // вычисляем веса и суммарный вес
    double total_weight = 0.0;
    double* weights = calloc(n, sizeof(double));
    for (int i = 0; i < n; i++) {
        if (i == start) continue;
        int d = dist[start][i];
        // потомки: путь должен быть конечным и >0, а человек жив
        if (d < INF && d > 0 && g->vertices[i].person.death_year < 0) {
            // вес = 1 / 2^(d-1)
            double w = 1.0 / pow(2.0, d - 1);
            weights[i] = w;
            total_weight += w;
        }
    }

    if (total_weight == 0.0) {
        printf("Нет живых потомков, которые могли бы распределить наследство\n");
    } else {
        double base = amount / total_weight;
        printf("Распределение наследства с '%s' (total: %.2f):\n", name, amount);
        for (int i = 0; i < n; i++) {
            if (weights[i] > 0) {
                double share = base * weights[i];
                printf(" - %s: %.2f\n", g->vertices[i].person.name, share);
            }
        }
    }

    // очистка
    for (int i = 0; i < n; i++) free(dist[i]);
    free(dist);
    free(weights);
}



// --- Остальные функции (BFS, Dijkstra и пр.) --- //

static void enqueue(Queue* q, int idx) {
    QueueNode* node = malloc(sizeof(QueueNode));
    node->index = idx;
    node->next = NULL;
    if (!q->rear) {
        q->front = q->rear = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
}



static int dequeue(Queue* q) {
    if (!q->front) return -1;
    QueueNode* node = q->front;
    int idx = node->index;
    q->front = node->next;
    if (!q->front) q->rear = NULL;
    free(node);
    return idx;
}



static int is_empty(Queue* q) {
    return q->front == NULL;
}


// --- Получение потомков (BFS по ребрам с relation == PARENT) --- //
void get_descendants(Graph* g, const char* name) {
    int start_idx = find_person_index(g, name);
    if (start_idx == -1) {
        printf(RED "Человек '%s' не найден.\n" RESET, name);
        return;
    }

    int* visited = calloc(g->size, sizeof(int));
    if (!visited) return;

    Queue q = {0};
    enqueue(&q, start_idx);
    visited[start_idx] = 1;

    printf("Потомки от '%s':\n", name);
    while (!is_empty(&q)) {
        int current = dequeue(&q);
        Edge* e = g->vertices[current].edges;
        while (e) {
            if (e->relation == PARENT) {
                int child_idx = e->to;
                if (!visited[child_idx]) {
                    visited[child_idx] = 1;
                    printf(" - %s\n", g->vertices[child_idx].person.name);
                    enqueue(&q, child_idx);
                }
            }
            e = e->next;
        }
    }
    free(visited);
}



int shortest_relation_path(Graph* g, const char* from, const char* to) {
    int start = find_person_index(g, from);
    int end = find_person_index(g, to);
    if (start == -1 || end == -1) return -1;

    int* dist = malloc(sizeof(int) * g->size);
    int* visited = calloc(g->size, sizeof(int));
    if (!dist || !visited) {
        free(dist);
        free(visited);
        return -1;
    }

    for (int i = 0; i < g->size; i++) dist[i] = INT_MAX;
    dist[start] = 0;

    for (;;) {
        int u = -1;
        int min_dist = INT_MAX;
        for (int i = 0; i < g->size; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }
        if (u == -1 || u == end)
            break;

        visited[u] = 1;
        Edge* e = g->vertices[u].edges;
        while (e) {
            int v = e->to;
            if (!visited[v] && dist[u] + 1 < dist[v]) {
                dist[v] = dist[u] + 1;
            }
            e = e->next;
        }
    }

    int result = (dist[end] == INT_MAX) ? -1 : dist[end];
    free(dist);
    free(visited);
    return result;
}



void print_graph(Graph* g) {
    if (!g) return;
    printf("Граф (кол-во вертексов: %d):\n", g->size);
    for (int i = 0; i < g->size; i++) {
        printf("  %s (%s, %d-%d)\n",
               g->vertices[i].person.name,
               g->vertices[i].person.gender == MALE ? "male" : "female",
               g->vertices[i].person.birth_year,
               g->vertices[i].person.death_year);
        Edge* e = g->vertices[i].edges;
        while (e) {
            if (e->relation == PARENT) {
                // Только родители (PARENT) выводятся циан
                printf("%s    -> %s%s\n",
                       CYAN,
                       g->vertices[e->to].person.name,
                       RESET);
            } else {
                // Остальные отношения без цвета
                printf("    -> %s\n",
                       g->vertices[e->to].person.name);
            }
            e = e->next;
        }
    }
}



// --- Экспорт в DOT ---
void export_dot(const Graph *g, const char *dot_path) {
    FILE *f = fopen(dot_path, "w");
    if (!f) return;
    fprintf(f, "digraph G {\n");
    for (int i = 0; i < g->size; i++) {
        fprintf(f, "  \"%s\";\n", g->vertices[i].person.name);
        for (Edge *e = g->vertices[i].edges; e; e = e->next) {
            // Экспортируем с меткой отношения
            fprintf(f,
                    "  \"%s\" -> \"%s\" [label=\"%s\"];\n",
                    g->vertices[i].person.name,
                    g->vertices[e->to].person.name,
                    e->relation == PARENT ? "parent" : "child");
        }
    }
    fprintf(f, "}\n");
    fclose(f);
}



// --- Вызов утилиты dot для рендеринга в PNG ---
void render_png(const char *dot_path, const char *png_path) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "dot -Tpng \"%s\" -o \"%s\"", dot_path, png_path);
    system(cmd);
}
