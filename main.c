#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

int main() {
    setlocale(LC_ALL, "Russian");
    Graph* g = create_graph();

    // Добавляем людей
    add_person(g, (Person){"Ivan", MALE, 1940, 2000});
    add_person(g, (Person){"Maria", FEMALE, 1945, 2010});
    add_person(g, (Person){"Peter", MALE, 1965, -1});
    add_person(g, (Person){"Anna", FEMALE, 1970, -1});
    add_person(g, (Person){"Sergey", MALE, 1990, -1});
    add_person(g, (Person){"Olga", FEMALE, 1995, -1});

    // Добавляем отношения
    add_relation(g, "Ivan", "Peter", PARENT);
    add_relation(g, "Maria", "Peter", PARENT);
    add_relation(g, "Ivan", "Anna", PARENT);
    add_relation(g, "Maria", "Anna", PARENT);
    add_relation(g, "Peter", "Sergey", PARENT);
    add_relation(g, "Anna", "Olga", PARENT);

    // Вывод потомков Ивана
    printf("Descendants of Ivan:\n");
    get_descendants(g, "Ivan");

    // Поиск кратчайшего пути между Ivan и Olga
    int dist = shortest_relation_path(g, "Ivan", "Olga");
    if (dist >= 0) {
        printf("Shortest relationship distance between Ivan and Olga: %d\n", dist);
    } else {
        printf("No relation found between Ivan and Olga.\n");
    }

    // Распределяем наследство 100000 рублей от Ivan
    printf("Inheritance distribution from Ivan (100000 rubles):\n");
    distribute_inheritance(g, "Ivan", 100000.0);


    print_graph(g);
    free_graph(g);
    return 0;
}
