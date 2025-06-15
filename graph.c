#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "graph.h"

#define INITIAL_CAPACITY 10
#define HASH_CAPACITY 1024

// ----------- Хэш-таблица --------------

static unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
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
            return -1; // Already exists
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

// ----------- Граф --------------

Graph* create_graph() {
    Graph* g = malloc(sizeof(Graph));
    if (!g) return NULL;
    g->size = 0;
    g->capacity = INITIAL_CAPACITY;
    g->vertices = malloc(sizeof(Vertex) * g->capacity);
    if (!g->vertices) {
        free(g);
        return NULL;
    }
    g->name_index = create_name_table();
    if (!g->name_index) {
        free(g->vertices);
        free(g);
        return NULL;
    }
    return g;
}

void free_graph(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->size; i++) {
        free(g->vertices[i].person.name);
        Edge* e = g->vertices[i].edges;
        while (e) {
            Edge* tmp = e;
            e = e->next;
            free(tmp);
        }
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
    if (find_person_index(g, p.name) != -1) return -1; // Already exists

    if (g->size >= g->capacity) {
        int new_capacity = g->capacity * 2;
        Vertex* tmp = realloc(g->vertices, sizeof(Vertex) * new_capacity);
        if (!tmp) return -1;
        g->vertices = tmp;
        g->capacity = new_capacity;
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
    if (!g || !from || !to) return -1;
    int from_idx = find_person_index(g, from);
    int to_idx = find_person_index(g, to);
    if (from_idx == -1 || to_idx == -1) return -1;

    Edge* edge = malloc(sizeof(Edge));
    if (!edge) return -1;
    edge->to = to_idx;
    edge->relation = relation;
    edge->next = g->vertices[from_idx].edges;
    g->vertices[from_idx].edges = edge;

    return 0;
}

// --- Очередь для BFS ---

typedef struct QueueNode {
    int index;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
} Queue;

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

// --- Получение потомков (BFS по ребрам с relation == PARENT) ---

void get_descendants(Graph* g, const char* name) {
    int start_idx = find_person_index(g, name);
    if (start_idx == -1) {
        printf("Person '%s' not found.\n", name);
        return;
    }

    int* visited = calloc(g->size, sizeof(int));
    if (!visited) return;

    Queue q = {0};
    enqueue(&q, start_idx);
    visited[start_idx] = 1;

    printf("Descendants of '%s':\n", name);
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

// --- Поиск кратчайшего пути (число рёбер) — алгоритм Дейкстры по неотрицательным ребрам длины 1 ---
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

// --- Функция распределения наследства (примитивная, демонстрация) ---

// Здесь нужно реализовать алгоритмы Флойда-Воршалла и вычисление степени родства.
// Для краткости приведём простую заглушку:
void distribute_inheritance(Graph* g, const char* name, double amount) {
    printf("Inheritance distribution function not implemented yet.\n");
}


// --- Вспомогательная функция для отладки ---
void print_graph(Graph* g) {
    if (!g) return;
    printf("Graph (vertices: %d):\n", g->size);
    for (int i = 0; i < g->size; i++) {
        printf("  %s (%s, %d-%d)\n", g->vertices[i].person.name,
               g->vertices[i].person.gender == MALE ? "male" : "female",
               g->vertices[i].person.birth_year,
               g->vertices[i].person.death_year);
        Edge* e = g->vertices[i].edges;
        while (e) {
            printf("    -> %s (%s)\n", g->vertices[e->to].person.name,
                   e->relation == PARENT ? "PARENT" : "CHILD");
            e = e->next;
        }
    }
}
