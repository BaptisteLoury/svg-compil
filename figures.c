#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "figures.h"

#define DEFAULT_THICKNESS -1
#define DEFAULT_VISIBILITY 1
#define DEFAULT_FONT_SIZE 16
#define DEFAULT_COLOR "#000"
#define DEFAULT_ANGLE 0

typedef enum {
    CIRCLE,
    RECTANGLE,
    LINE,
    TEXT,
    POLYGON,
    ELLIPSE
} fig_type;

struct fig {
    fig_type type;
    char name[MAX_KEY_LEN];
    Coord* coords;
    long radius;
    char text[MAX_KEY_LEN];
    Options* opts;
    int selected;
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
    Node* next;
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

struct nameL {
    char name[MAX_KEY_LEN];
    NameList* next;
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

void freeFig(Figure* figure) {
    if (figure != NULL) {
        free(figure->opts);
        Coord * current = figure->coords;
        while (current != NULL) {
            Coord* temp = current;
            current = current->next;
            free(temp);
        }
        free(figure);
    }
}

void freeHashMap(HashMap* hashMap) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            Node* next = node->next;
            if(node->value != NULL) {
                freeFig(node->value);
            }
            free(node);
            node = next;
        }
    }
    free(hashMap->buckets);
    free(hashMap);
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

Coord get_centroid(Figure* figure) {
    long x = 0;
    long y = 0;
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
    Coord centroid;
    centroid.x = x;
    centroid.y = y;
    return centroid;
}

Figure* createCircle(char * name, long x, long y, long radius) {
    Figure* circle = createFigure(name, CIRCLE, generate_coords(NULL, x, y));
    circle->radius = radius;
    return circle;
}

Figure* createPolygon(char * name, Coord* points) {
    Figure* polygon = createFigure(name, POLYGON, points);
    return polygon;
}

Figure* createRectangle(char * name, long x, long y, long dimension_x, long dimension_y) {
    Coord* points = generate_coords(NULL, x + dimension_x, y);
    points = generate_coords(points, x + dimension_x, y + dimension_y);
    points = generate_coords(points, x, y + dimension_y);
    points = generate_coords(points, x, y);
    return createPolygon(name, points);
}

Figure* createLine(char * name, long x1, long y1, long x2, long y2) {
    Figure* line = createFigure(name, LINE, generate_coords(generate_coords(NULL, x1, y1), x2, y2));
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
    return textF;
}

Figure* createEllipse(char * name, Coord* points, long rx, long ry) {
    points->next = generate_coords(NULL, rx, ry);
    Figure* textF = createFigure(name, ELLIPSE, points);
    return textF;
}

void print_options(FILE* stream, Figure* figure) {
    fprintf(stream, " stroke=\"%s\" fill=\"%s\"", figure->opts->color, figure->opts->fill_color);
    if (figure->opts->thickness != DEFAULT_THICKNESS) {
        fprintf(stream, " stroke-width=\"%ld\"", figure->opts->thickness);
    }
    if (!figure->opts->visible) {
        fprintf(stream, " visibility=\"hidden\"");
    }
    if(figure->opts->rotation_angle != DEFAULT_ANGLE) {
        Coord centroid = get_centroid(figure);
        fprintf(stream, " transform=\"rotate(%ld, %ld, %ld)\"", figure->opts->rotation_angle, centroid.x, centroid.y);
    }
}

void print_circle(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<circle cx=\"%ld\" cy=\"%ld\" r=\"%ld\"", figure->coords->x, figure->coords->y, figure->radius);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_polygon(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<polygon points=\"");
    Coord* point = figure->coords;
    while (point != NULL) {
        fprintf(stream, "%ld,%ld ", point->x, point->y);
        point = point->next;
    }
    fprintf(stream, "\"");
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_line(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<line x1=\"%ld\" y1=\"%ld\" x2=\"%ld\" y2=\"%ld\"", figure->coords->x, figure->coords->y, figure->coords->next->x, figure->coords->next->y);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_ellipse(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<ellipse cx=\"%ld\" cy=\"%ld\" rx=\"%ld\" ry=\"%ld\"", figure->coords->x, figure->coords->y, figure->coords->next->x, figure->coords->next->y);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_text(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<text x=\"%ld\" y=\"%ld\"", figure->coords->x, figure->coords->y);
    print_options(stream, figure);
    fprintf(stdout, "%ld\n", figure->opts->font_size);
    fprintf(stream, " font-size=\"%ld\"", figure->opts->font_size);
    fprintf(stream, " >");
    fprintf(stream, "%s", figure->text);
    fprintf(stream, "</text>\n");
}

char * toSvg(FILE* stream, Figure* figure) {
    switch (figure->type) {
        case CIRCLE:
            print_circle(stream, figure);
            break;
        case LINE:
            print_line(stream, figure);
            break;
        case TEXT:
            print_text(stream, figure);
            break;
        case POLYGON:
            print_polygon(stream, figure);
            break;
        case ELLIPSE:
            print_ellipse(stream, figure);
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
        freeFig(figure);
    }
}

void rename_fig(HashMap* hashMap, char * old, char * new) {
    Figure* figure = get(hashMap, old);
    if(figure != NULL) {
        strcpy(figure->name, new);
        deleteKey(hashMap, old);
        put(hashMap, new, figure);
    }
}

Coord* copyCoord(Coord* orig) {
    Coord* new = NULL;
    if  (orig != NULL) {
        new = (Coord*) malloc(sizeof(Coord));
        memcpy(new, orig, sizeof(Coord));
        new->next = copyCoord(orig->next);
    }
    return new;
}

Figure* copyFig(Figure * figure) {
    Figure* copy = (Figure*) malloc(sizeof(Figure));
    memcpy(copy, figure, sizeof(Figure));

    copy->opts = (Options*) malloc(sizeof(Options));
    memcpy(copy->opts, figure->opts, sizeof(Options));

    copy->coords = copyCoord(figure->coords);

    return copy;
}

void copy(HashMap* hashMap, char * toCopy, char * copy) {
    Figure * orig = get(hashMap, toCopy);
    if (orig != NULL && !exists(hashMap, copy)) {
        Figure * new = copyFig(orig);
        strcpy(new->name, copy);
        put(hashMap, copy, new);
    }
}

void applyOptions(HashMap* HashMap, char * name, Options* options) {
    Figure* figure = get(HashMap, name);
    if (figure != NULL) {
        free(figure->opts);
        figure->opts = options;
    }
}

void apply_options_list(HashMap* HashMap, NameList* list, Options* options) {
    while (list != NULL) {
        Options* for_current = (Options*) malloc(sizeof(Options));
        memcpy(for_current, options, sizeof(Options));
        applyOptions(HashMap, list->name, for_current);
        list = list->next;
    }
    free(options);
}

void set_color(HashMap* hashMap, NameList* list, char * color) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        if (figure != NULL) {
            strcpy(figure->opts->color, color);
        }
        list = list->next;
    }
}

void set_fill_color(HashMap* hashMap, NameList* list, char * color) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        if (figure != NULL) {
            strcpy(figure->opts->fill_color, color);
        }
        list = list->next;
    }
}

void set_visible(HashMap* hashMap, NameList* list, int visible) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        if (figure != NULL) {
            figure->opts->visible = visible;
        }
        list = list->next;
    }
}

void set_thickness(HashMap* hashMap, NameList* list, long thickness) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        if (figure != NULL) {
            figure->opts->thickness = thickness;
        }
        list = list->next;
    }
}

void set_font_size(HashMap* hashMap, NameList* list, long font_size) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        if (figure != NULL) {
            figure->opts->font_size = font_size;
        }
        list = list->next;
    }
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

NameList* appendName(NameList* list, char * name) {
    NameList* newList = (NameList*) malloc(sizeof(NameList));
    strcpy(newList->name, name);
    newList->next = list;
    return newList;
}

void move_fig(Figure* figure, long x, long y) {
    Coord* coord = figure->coords;
    while (coord != NULL) {
        coord->x += x;
        coord->y += y;
        coord = coord->next;
    }
}

void move(HashMap* hashMap, long x, long y) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL && node->value->selected) {
                move_fig(node->value, x, y);
            }
            node = node->next;
        }
    }
}

void move_list(HashMap* hashMap, NameList* list, long x, long y) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        move_fig(figure, x, y);
        list = list->next;
    }
}

void homothetie(Figure* figure, double factor) {
    switch(figure->type) {
        case CIRCLE:
            figure->radius *= factor;
            break;
        case TEXT:
            figure->opts->font_size *= factor;
            break;
        default:;
            Coord centroid = get_centroid(figure);
            Coord* point = figure->coords;
            while (point != NULL) {
                long x_from_centroid = centroid.x - point->x;
                long y_from_centroid = centroid.y - point->y;
                point->x = centroid.x + (factor * x_from_centroid);
                point->y = centroid.y + (factor * y_from_centroid);
                point = point->next;
            }
            break;
    }
}

void zoom(HashMap* hashMap, double factor) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL && node->value->selected) {
                homothetie(node->value, factor);
            }
            node = node->next;
        }
    }
}

void zoom_list(HashMap* hashMap, NameList* list, double factor) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        homothetie(figure, factor);
        list = list->next;
    }
}

void rotate_fig(Figure* figure, long angle) {
    figure->opts->rotation_angle += angle;
    figure->opts->rotation_angle %= 360;
}

void rotate(HashMap* hashMap, long angle) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL && node->value->selected) {
                rotate_fig(node->value, angle);
            }
            node = node->next;
        }
    }
}

void rotate_list(HashMap* hashMap, NameList* list, long angle) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        rotate_fig(figure, angle);
        list = list->next;
    }
}