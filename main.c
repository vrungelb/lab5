#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "graph.h"

#define RED        "\x1b[1;31m"
#define GREEN      "\x1b[1;32m"
#define YELLOW     "\x1b[1;33m"
#define CYAN       "\x1b[1;36m"
#define RESET      "\x1b[0m"

static void print_menu() {
    printf("\n" CYAN "=== Меню ===\n" RESET);
    printf("1) Добавить новую вершину (человек)\n");
    printf("2) Добавить новое ребро (родитель → ребёнок)\n");
    printf("3) Удалить вершину\n");
    printf("4) Удалить ребро\n");
    printf("5) Печатать граф (текст)\n");
    printf("6) Печатать граф (графически)\n");
    printf("7) Найти дистанцию отношений между двумя людьми\n");
    printf("8) Распределить наследство\n");
    printf("0) Выход\n");
    printf("Выберите опцию: ");
}

// Проверка и фильтрация валидных UTF-8 последовательностей
static void filter_valid_utf8(char* buf) {
    unsigned char* s = (unsigned char*)buf;
    size_t len = strlen(buf);
    size_t i = 0, j = 0;
    while (i < len) {
        unsigned char c = s[i];
        if (c < 0x80) {
            s[j++] = c;
            i++;
        } else if (c >= 0xC2 && c <= 0xDF) {
            if (i + 1 < len) {
                unsigned char c1 = s[i+1];
                if ((c1 & 0xC0) == 0x80) {
                    s[j++] = c;
                    s[j++] = c1;
                    i += 2;
                    continue;
                }
            }
            i++;
        } else if (c >= 0xE0 && c <= 0xEF) {
            if (i + 2 < len) {
                unsigned char c1 = s[i+1], c2 = s[i+2];
                if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80) {
                    s[j++] = c;
                    s[j++] = c1;
                    s[j++] = c2;
                    i += 3;
                    continue;
                }
            }
            i++;
        } else if (c >= 0xF0 && c <= 0xF4) {
            if (i + 3 < len) {
                unsigned char c1 = s[i+1], c2 = s[i+2], c3 = s[i+3];
                if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
                    s[j++] = c;
                    s[j++] = c1;
                    s[j++] = c2;
                    s[j++] = c3;
                    i += 4;
                    continue;
                }
            }
            i++;
        } else {
            i++;
        }
    }
    s[j] = '\0';
}

// Чтение строки из stdin с удалением ведущих/концевых пробельных символов и фильтрацией UTF-8
static void read_line(char* buf, int size) {
    if (!fgets(buf, size, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    char* start = buf;
    while (*start && isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start) - 1;
    while (end >= start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    if (start != buf) memmove(buf, start, strlen(start) + 1);
    filter_valid_utf8(buf);
}

int main() {
    setlocale(LC_ALL, "");
    Graph* g = create_graph();
    if (!g) {
        fprintf(stderr, RED "Ошибка создания графа" RESET);
        return 1;
    }

    int choice = -1;
    char buf[128];
    char name1[64], name2[64];
    Person tmp;

    while (1) {
        print_menu();
        if (!fgets(buf, sizeof(buf), stdin)) break;
        choice = atoi(buf);
        switch (choice) {
            case 1:
                tmp.name = malloc(64);
                if (!tmp.name) break;
                printf("Имя: ");
                read_line(tmp.name, 64);
                printf("Пол (M/F): ");
                read_line(buf, sizeof(buf));
                tmp.gender = (buf[0]=='M'||buf[0]=='m')? MALE : FEMALE;
                printf("Год рождения: ");
                read_line(buf, sizeof(buf));
                tmp.birth_year = atoi(buf);
                printf("Год смерти (-1 если жив): ");
                read_line(buf, sizeof(buf));
                tmp.death_year = atoi(buf);
                if (add_person(g, tmp) >= 0)
                    printf(GREEN "Человек '%s' добавлен" RESET, tmp.name);
                else
                    printf(RED "Не удалось добавить '%s'" RESET, tmp.name);
                free(tmp.name);
                break;

            case 2:
                printf("Имя родителя: ");
                read_line(name1, sizeof(name1));
                printf("Имя ребёнка: ");
                read_line(name2, sizeof(name2));
                if (add_relation(g, name1, name2, PARENT) == 0)
                    printf(GREEN "Связь '%s'→'%s' добавлена" RESET, name1, name2);
                else
                    printf(RED "Не удалось добавить связь" RESET);
                break;

            case 3:
                printf("Имя для удаления: ");
                read_line(name1, sizeof(name1));
                if (remove_person(g, name1) == 0)
                    printf(GREEN "Вершина '%s' удалена" RESET, name1);
                else
                    printf(RED "Не удалось удалить '%s'" RESET, name1);
                break;

            case 4:
                printf("Текущее состояние перед удалением:");
                print_graph(g);
                printf("Имя источника: ");
                read_line(name1, sizeof(name1));
                printf("Имя цели: ");
                read_line(name2, sizeof(name2));
                if (remove_relation(g, name1, name2, PARENT) == 0)
                    printf(GREEN "Ребро '%s'→'%s' удалено" RESET, name1, name2);
                else {
                    printf(RED "Не удалось удалить ребро" RESET);
                    int fi = find_person_index(g, name1);
                    int ti = find_person_index(g, name2);
                    printf("DEBUG: индексы: источник=%d, цель=%d", fi, ti);
                    if (fi>=0) {
                        printf("DEBUG: исходящие ребра из '%s':", name1);
                        Edge* e = g->vertices[fi].edges;
                        while (e) {
                            printf("  -> %s (relation=%s)",
                                   g->vertices[e->to].person.name,
                                   e->relation==PARENT? "PARENT":"CHILD");
                            e = e->next;
                        }
                    }
                }
                break;

            case 5:
                printf(CYAN "Текущий граф (текст):" RESET);
                print_graph(g);
                break;

            case 6: {
                char basename[64];
                printf("Введите имя файла (без расширения) для вывода графа: ");
                read_line(basename, sizeof(basename));
                if (basename[0] == '\0') strcpy(basename, "family_tree");
                char dot_path[128], png_path[128];
                snprintf(dot_path, sizeof(dot_path), "%s.dot", basename);
                snprintf(png_path, sizeof(png_path), "%s.png", basename);
                export_dot(g, dot_path);
                render_png(dot_path, png_path);
                printf(GREEN "Граф сохранён в файлы: %s и %s" RESET, dot_path, png_path);
                break;
            }

            case 7:
                printf("Имя первого человека: ");
                read_line(name1, sizeof(name1));
                printf("Имя второго человека: ");
                read_line(name2, sizeof(name2));
                {
                    int dist = shortest_relation_path(g, name1, name2);
                    if (dist >= 0)
                        printf(GREEN "Расстояние отношений между '%s' и '%s': %d" RESET, name1, name2, dist);
                    else
                        printf(YELLOW "Связь между '%s' и '%s' не найдена" RESET, name1, name2);
                }
                break;

            case 8:
                printf("Имя человека для распределения наследства: ");
                read_line(name1, sizeof(name1));
                printf("Сумма наследства: ");
                read_line(buf, sizeof(buf));
                {
                    double amount = atof(buf);
                    distribute_inheritance(g, name1, amount);
                }
                break;

            case 0:
                printf(YELLOW "Выход..." RESET);
                free_graph(g);
                return 0;

            default:
                printf(RED "Неверная опция" RESET);
        }
    }

    free_graph(g);
    return 0;
}
