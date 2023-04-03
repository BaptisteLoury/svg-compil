#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "figures.h"

#define DEFAULT_THICKNESS -1
#define DEFAULT_VISIBILITY 1
#define DEFAULT_FONT_SIZE -1
#define DEFAULT_COLOR "#000"
#define DEFAULT_ANGLE 0

typedef enum {
    CIRCLE,
    RECTANGLE,
    LINE,
    TEXT
} fig_type;

struct fig {
    fig_type type;
    char name[MAX_KEY_LEN];
    Coord* coords;
    long radius;
    long dimension_x;
    long dimension_y;
    char text[MAX_KEY_LEN];
    Options* opts;
    int selected;
    Coord* centroid;
};

struct opt {
    char color[8];
    char fill_color[8];
    long thickness;
    int visible;
    long font_size;
    long rotation_angle;
};

struct nd {
    char key[MAX_KEY_LEN];
    Figure* value;
    struct Node* next;
};

struct hm {
    Node** buckets;
    int size;
};

struct coord {
    long x;
    long y;
    Coord* next;
};

int hash(char* key, int size) {
    int sum = 0;
    for (int i = 0; i < strlen(key); i++) {
        sum += key[i];
    }
    return sum % size;
}

HashMap* createHashMap(int size) {
    HashMap* hashMap = (HashMap*) malloc(sizeof(HashMap));
    hashMap->buckets = (Node**) calloc(size, sizeof(Node*));
    hashMap->size = size;
    return hashMap;
}

void put(HashMap* hashMap, char* key, Figure* value) {
    int index = hash(key, hashMap->size);
    Node* node = hashMap->buckets[index];
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            node->value = value;
            return;
        }
        node = node->next;
    }
    node = (Node*) malloc(sizeof(Node));
    strcpy(node->key, key);
    node->value = value;
    node->next = hashMap->buckets[index];
    hashMap->buckets[index] = node;
}

void add(HashMap* hashMap, Figure* figure) {
    put(hashMap, figure->name, figure);
}

Figure* get(HashMap* hashMap, char* key) {
    int index = hash(key, hashMap->size);
    Node* node = hashMap->buckets[index];
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

int exists(HashMap* hashMap, char* key) {
    return hashMap != NULL && get(hashMap, key) != NULL;
}

void deleteKey(HashMap* hashMap, char* key) {
    int index = hash(key, hashMap->size);
    Node* node = hashMap->buckets[index];
    Node* prev = NULL;
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            if (prev == NULL) {
                hashMap->buckets[index] = node->next;
            } else {
                prev->next = node->next;
            }
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

void freeHashMap(HashMap* hashMap) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            Node* next = node->next;
            if(node->value != NULL) {
                free(node->value);
            }
            free(node);
            node = next;
        }
    }
    free(hashMap->buckets);
    free(hashMap);
}

Figure* createFigure(char * name, fig_type type, Coord* coords) {
    Figure* figure = (Figure*) malloc(sizeof(Figure));
    figure->type = type;
    strcpy(figure->name, name);
    figure->coords = coords;
    figure->opts = get_default_options();
    return figure;
}

Coord* generate_coords(Coord* coords, long x, long y) {
    Coord* generated = (Coord*) malloc(sizeof(Coord));
    generated->x = x;
    generated->y = y;
    generated->next = coords;
    return generated;
}

void set_centroid(Figure* figure) {
    long x = 0;
    long y = 0;
    switch (figure->type)
    {
    case RECTANGLE:
        x = figure->coords->x + figure->dimension_x/2;
        y = figure->coords->y + figure->dimension_y/2;
        break;
    default:;
        int i = 0;
        Coord* coord = figure->coords;
        while(coord != NULL) {
            x += coord->x;
            y += coord->y;
            i++;
            coord = coord->next;
        }
        x /= i;
        y /= i;
        break;
    }
    fprintf(stdout, "\n%ld;%ld\n", x, y);
    figure->centroid = generate_coords(NULL, x, y);
}

Figure* createCircle(char * name, long x, long y, long radius) {
    Figure* circle = createFigure(name, CIRCLE, generate_coords(NULL, x, y));
    circle->radius = radius;
    set_centroid(circle);
    return circle;
}

Figure* createRectangle(char * name, long x, long y, long dimension_x, long dimension_y) {
    Figure* rectangle = createFigure(name, RECTANGLE, generate_coords(NULL, x, y));
    rectangle->dimension_x = dimension_x;
    rectangle->dimension_y = dimension_y;
    set_centroid(rectangle);
    return rectangle;
}

Figure* createLine(char * name, long x1, long y1, long x2, long y2) {
    Figure* line = createFigure(name, LINE, generate_coords(generate_coords(NULL, x1, y1), x2, y2));
    set_centroid(line);
    return line;
}

char * removeDblQuote(char * string) {
    int len = strlen(string);
    char* new_str = (char*) malloc(len-1);
    for (int i = 1; i < len-1; i++) {
        new_str[i-1] = string[i];
    }
    new_str[len-2] = '\0';
    return new_str;
}

Figure* createText(char * name, char * text, long x, long y) {
    Figure* textF = createFigure(name, TEXT, generate_coords(NULL, x, y));
    strcpy(textF->text, removeDblQuote(text));
    set_centroid(textF);
    return textF;
}

void print_options(FILE* stream, Figure* figure) {
    fprintf(stream, " stroke=\"%s\"", figure->opts->color);
    fprintf(stream, " fill=\"%s\"", figure->opts->fill_color);
    if (figure->opts->thickness != DEFAULT_THICKNESS) {
        fprintf(stream, " stroke-width=\"%ld\"", figure->opts->thickness);
    }
    if (!figure->opts->visible) {
        fprintf(stream, " visibility=\"hidden\"");
    }
    if(figure->opts->rotation_angle != DEFAULT_ANGLE) {
        fprintf(stream, " transform=\"rotate(%ld, %ld, %ld)\"", figure->opts->rotation_angle, figure->centroid->x, figure->centroid->y);
    }
}

void print_circle(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<circle cx=\"%ld\" cy=\"%ld\" r=\"%ld\"", figure->coords->x, figure->coords->y, figure->radius);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_rectangle(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<rect x=\"%ld\" y=\"%ld\" width=\"%ld\" height=\"%ld\"", figure->coords->x, figure->coords->y, figure->dimension_x, figure->dimension_y);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_line(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<line x1=\"%ld\" y1=\"%ld\" x2=\"%ld\" y2=\"%ld\"", figure->coords->x, figure->coords->y, figure->coords->next->x, figure->coords->next->y);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_text(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<text x=\"%ld\" y=\"%ld\"", figure->coords->x, figure->coords->y);
    print_options(stream, figure);
    fprintf(stdout, "%ld\n", figure->opts->font_size);
    if (figure->opts->font_size != DEFAULT_FONT_SIZE) {
        fprintf(stream, " font-size=\"%ld\"", figure->opts->font_size);
    }
    fprintf(stream, " >");
    fprintf(stream, "%s", figure->text);
    fprintf(stream, "</text>\n");
}

char * toSvg(FILE* stream, Figure* figure) {
    switch (figure->type) {
        case CIRCLE:
            print_circle(stream, figure);
            break;
        case RECTANGLE:
            print_rectangle(stream, figure);
            break;
        case LINE:
            print_line(stream, figure);
            break;
        case TEXT:
            print_text(stream, figure);
            break;
        default:
            fprintf(stream, "undefined\n");
    }
}

void dump_to_desc(HashMap* hashMap, FILE* stream) {
    fprintf(stream,
        "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"800\" height=\"600\" viewBox=\"0 0 800 600\">\n");
    fprintf(stream,
        "\t<rect x=\"0\" y=\"0\" width=\"800\" height=\"600\" fill=\"none\" stroke=\"black\" />\n");
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL) {
                toSvg(stream, node->value);
            }
            node = node->next;
        }
    }
    fprintf(stream, "</svg>");
}

void dump(HashMap* hashMap) {
    dump_to_desc(hashMap, stdout);
}

void dump_to_file(HashMap* hashMap, char * file) {
    char * fileName = removeDblQuote(file);

    FILE* f = fopen(fileName, "w+");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file %s\n", fileName);
    } else {
        dump_to_desc(hashMap, f);
        fclose(f);
        fprintf(stdout, "Successfully dumped to %s\n", fileName);
    }
    free(fileName);
}

void delete_fig(HashMap* hashMap, char * name) {
    Figure* figure = get(hashMap, name);
    if(figure != NULL) {
        deleteKey(hashMap, name);
        free(figure);
    }
}

void rename_fig(HashMap* hashMap, char * old, char * new) {
    Figure* figure = get(hashMap, old);
    if(figure != NULL) {
        Figure* newFigure = copyFig(figure);
        strcpy(newFigure->name, new);
        delete_fig(hashMap, old);
        put(hashMap, new, newFigure);
    }
}

Figure* copyFig(Figure * figure) {
    Figure* copy = (Figure*) malloc(sizeof(Figure));
    copy->type = figure->type;
    strcpy(copy->name, figure->name);
    copy->coords = figure->coords;
    copy->radius = figure->radius;
    copy->dimension_x = figure->dimension_x;
    copy->dimension_y = figure->dimension_y;
    strcpy(copy->text, figure->text);
    copy->opts = figure->opts;
    return copy;
}

void applyOptions(HashMap* HashMap, char * name, Options* options) {
    Figure* figure = get(HashMap, name);
    if (figure != NULL) {
        free(figure->opts);
        figure->opts = options;
    }
}

Options* get_default_options() {
    Options* opt = (Options*) malloc(sizeof(Options));
    strcpy(opt->color, DEFAULT_COLOR);
    strcpy(opt->fill_color, DEFAULT_COLOR);
    opt->thickness = DEFAULT_THICKNESS;
    opt->visible = DEFAULT_VISIBILITY;
    opt->font_size = DEFAULT_FONT_SIZE;
    return opt;
}
Options* set_color(Options* opt, char * color) {
    strcpy(opt->color, color);
    return opt;
}

Options* set_fill_color(Options* opt, char * color) {
    strcpy(opt->fill_color, color);
    return opt;
}

Options* set_visible(Options* opt, int visible) {
    opt->visible = visible;
    return opt;
}

Options* set_thickness(Options* opt, long thickness) {
    opt->thickness = thickness;
    return opt;
}

Options* set_font_size(Options* opt, long font_size) {
    opt->font_size = font_size;
    return opt;
}

void set_selected(HashMap* hashMap, char * name, int selected) {
    Figure* figure = get(hashMap, name);
    if(figure != NULL) {
        figure->selected = selected;
    }
}

void set_all_selected(HashMap* hashMap, int selected) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL) {
                node->value->selected = selected;
            }
            node = node->next;
        }
    }
}

void move(HashMap* hashMap, long x, long y) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL && node->value->selected) {
                Coord* coord = node->value->coords;
                while (coord != NULL) {
                    coord->x += x;
                    coord->y += y;
                    coord = coord->next;
                }
            }
            node = node->next;
        }
    }
}

void zoom(HashMap* hashMap, double factor) {
    
}

void rotate(HashMap* hashMap, long angle) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL && node->value->selected) {
                node->value->opts->rotation_angle += angle;
                node->value->opts->rotation_angle %= 360;
            }
            node = node->next;
        }
    }
}