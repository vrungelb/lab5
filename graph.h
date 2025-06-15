#ifndef GRAPH_H
#define GRAPH_H

typedef enum { MALE, FEMALE } Gender;
typedef enum { PARENT, CHILD } RelationType;

typedef struct {
    char* name;
    Gender gender;
    int birth_year;
    int death_year;
} Person;

typedef struct Edge {
    int to;
    RelationType relation;
    struct Edge* next;
} Edge;

typedef struct {
    Person person;
    Edge* edges;
} Vertex;

typedef struct NameHashNode {
    char* name;
    int index;
    struct NameHashNode* next;
} NameHashNode;

typedef struct {
    NameHashNode** table;
    int capacity;
} NameHashTable;

typedef struct {
    Vertex* vertices;
    int size;
    int capacity;
    NameHashTable* name_index;
} Graph;

// Создание и уничтожение графа
Graph* create_graph();
void free_graph(Graph* g);

// Добавление человека и отношения
int add_person(Graph* g, Person p);
int add_relation(Graph* g, const char* from, const char* to, RelationType relation);

// Поиск по имени
int find_person_index(Graph* g, const char* name);

// Операции с графом
void get_descendants(Graph* g, const char* name);
int shortest_relation_path(Graph* g, const char* from, const char* to);
void distribute_inheritance(Graph* g, const char* name, double amount);

// Вспомогательные
void print_graph(Graph* g);

#endif // GRAPH_H
