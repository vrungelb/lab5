#ifndef GRAPH_H
#define GRAPH_H

// Пол человека
typedef enum {
    MALE,   // Мужской
    FEMALE  // Женский
} Gender;

// Тип родственной связи между двумя людьми в графе
typedef enum {
    PARENT, // from -> to: "from" является родителем "to"
    CHILD   // from -> to: "from" является ребёнком "to"
} RelationType;

// Структура, представляющая одного человека (вершину графа)
typedef struct {
    char* name;         // Имя человека (уникальное в пределах графа)
    Gender gender;      // Пол
    int birth_year;     // Год рождения
    int death_year;     // Год смерти; если < 0, человек считается живым
} Person;

// Структура ребра (связи) между вершинами графа
typedef struct Edge {
    int to;                     // Индекс вершины, на которую указывает ребро
    RelationType relation;     // Тип отношения (PARENT или CHILD)
    struct Edge* next;         // Следующее ребро в списке (связный список)
} Edge;

// Структура вершины графа: содержит данные человека и список его связей
typedef struct {
    Person person;     // Сам человек
    Edge* edges;       // Список исходящих рёбер (отношений с другими людьми)
} Vertex;

// Узел хэш-таблицы: связывает имя с индексом вершины
typedef struct NameHashNode {
    char* name;                  // Имя человека (ключ)
    int index;                   // Индекс в массиве вершин
    struct NameHashNode* next;  // Следующий элемент в цепочке (при коллизии)
} NameHashNode;

// Хэш-таблица для быстрого поиска индекса по имени
typedef struct {
    NameHashNode** table;  // Массив указателей на цепочки
    int capacity;          // Размер таблицы (количество ячеек)
} NameHashTable;

// Основная структура графа
typedef struct {
    Vertex* vertices;           // Динамический массив всех вершин графа
    int size;                   // Количество добавленных вершин
    int capacity;               // Текущая вместимость массива vertices
    NameHashTable* name_index;  // Хэш-таблица для быстрого поиска по имени
} Graph;

// --- ФУНКЦИИ РАБОТЫ С ГРАФОМ ---

/**
 * Создаёт и инициализирует новый пустой граф.
 * @return Указатель на созданный граф, либо NULL при ошибке.
 */
Graph* create_graph();

/**
 * Освобождает всю память, связанную с графом (включая вершины, рёбра и таблицу имён).
 * @param g Указатель на граф, который нужно удалить.
 */
void free_graph(Graph* g);


// --- ДОБАВЛЕНИЕ ДАННЫХ ---

/**
 * Добавляет нового человека (вершину) в граф.
 * Имя должно быть уникальным. Если имя уже существует, функция возвращает -1.
 * @param g Указатель на граф.
 * @param p Структура Person с данными.
 * @return Индекс новой вершины в массиве или -1 при ошибке.
 */
int add_person(Graph* g, Person p);

/**
 * Добавляет направленную связь (ребро) между двумя людьми.
 * Например: from -> to как PARENT означает "from — родитель to".
 * @param g Указатель на граф.
 * @param from Имя начального человека (источник ребра).
 * @param to Имя конечного человека (приёмник ребра).
 * @param relation Тип связи (PARENT или CHILD).
 * @return 0 при успехе, -1 при ошибке.
 */
int add_relation(Graph* g, const char* from, const char* to, RelationType relation);


// --- ПОИСК ---

/**
 * Ищет индекс человека по имени.
 * @param g Указатель на граф.
 * @param name Имя человека.
 * @return Индекс в массиве вершин или -1, если человек не найден.
 */
int find_person_index(Graph* g, const char* name);


// --- ОБРАБОТКА СВЯЗЕЙ ---

/**
 * Выводит всех потомков заданного человека, обходя связи типа PARENT (BFS).
 * @param g Указатель на граф.
 * @param name Имя начального человека.
 */
void get_descendants(Graph* g, const char* name);

/**
 * Находит кратчайший путь (по количеству связей) от одного человека до другого.
 * Использует алгоритм Дейкстры с единичными весами рёбер.
 * @param g Указатель на граф.
 * @param from Имя начальной вершины.
 * @param to Имя конечной вершины.
 * @return Длина пути (число шагов) или -1, если путь не найден.
 */
int shortest_relation_path(Graph* g, const char* from, const char* to);

/**
 * Распределяет указанную сумму наследства среди всех живых потомков.
 * Потомки получают доли в зависимости от степени родства:
 *   1/1 для детей, 1/2 для внуков, 1/4 для правнуков и т.д.
 * @param g Указатель на граф.
 * @param name Имя умершего наследодателя.
 * @param amount Общая сумма наследства.
 */
void distribute_inheritance(Graph* g, const char* name, double amount);


// --- УТИЛИТЫ ---

/**
 * Печатает весь граф в текстовом виде.
 * Показывает людей и их связи.
 * @param g Указатель на граф.
 */
void print_graph(Graph* g);


// --- ЭКСПОРТ ГРАФА ---

/**
 * Экспортирует граф в файл формата DOT (Graphviz).
 * @param g Указатель на граф.
 * @param dot_path Путь к выходному .dot-файлу.
 */
void export_dot(const Graph *g, const char *dot_path);

/**
 * Вызывает утилиту dot (Graphviz) для генерации PNG-изображения из .dot-файла.
 * @param dot_path Путь к входному .dot-файлу.
 * @param png_path Путь к выходному PNG-файлу.
 */
void render_png(const char *dot_path, const char *png_path);


// --- УДАЛЕНИЕ ---

/**
 * Удаляет связь между двумя людьми.
 * Удаляется только указанное направление и тип связи.
 * @param g Указатель на граф.
 * @param from Имя источника ребра.
 * @param to Имя цели.
 * @param relation Тип связи.
 * @return 0 при успехе, -1 если связь не найдена.
 */
int remove_relation(Graph* g, const char* from, const char* to, RelationType relation);

/**
 * Удаляет человека из графа вместе со всеми его связями.
 * Автоматически удаляет все рёбра, в которых он участвовал.
 * @param g Указатель на граф.
 * @param name Имя человека для удаления.
 * @return 0 при успехе, -1 если человек не найден.
 */
int remove_person(Graph* g, const char* name);

#endif // GRAPH_H
