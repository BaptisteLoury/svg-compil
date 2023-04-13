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


struct opt {
    char color[8];
    char fill_color[8];
    long thickness;
    int visible;
    long font_size;
    long rotation_angle;
};

struct fig {
    fig_type type;
    char name[MAX_KEY_LEN];
    Coord* coords;
    long long1;
    long long2;
    char text[MAX_KEY_LEN];
    Options opts;
    int selected;
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

int add(HashMap* hashMap, Figure* value) {
    char * key = value->name;
    int index = hash(key, hashMap->size);
    Node* node = hashMap->buckets[index];
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return 0;
        }
        node = node->next;
    }
    node = (Node*) malloc(sizeof(Node));
    strcpy(node->key, key);
    node->value = value;
    node->next = hashMap->buckets[index];
    hashMap->buckets[index] = node;
    return 1;
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

void free_coords(Coord* coords) {
    while (coords != NULL) {
        Coord* temp = coords;
        coords = coords->next;
        free(temp);
    }
}

void freeFig(Figure* figure) {
    if (figure != NULL) {
        free_coords(figure->coords);
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


void iterate_over_map(HashMap* hashMap, void (*func)(Figure*, Transfer), Transfer args, int all) {
    for (int i = 0; i < hashMap->size; i++) {
        Node* node = hashMap->buckets[i];
        while (node != NULL) {
            if(node->value != NULL && (all || node->value->selected)) {
                func(node->value, args);
            }
            node = node->next;
        }
    }
}
void iterate_over_list(HashMap* hashMap, NameList* list, void (*func)(Figure*, Transfer), Transfer args) {
    while (list != NULL) {
        Figure* figure = get(hashMap, list->name);
        if (figure != NULL) {
            func(figure, args);
        }
        list = list->next;
    }
}

Options get_default_options() {
    Options opt;
    strcpy(opt.color, DEFAULT_COLOR);
    strcpy(opt.fill_color, DEFAULT_COLOR);
    opt.thickness = DEFAULT_THICKNESS;
    opt.visible = DEFAULT_VISIBILITY;
    opt.font_size = DEFAULT_FONT_SIZE;
    return opt;
}

Figure* create_figure(fig_type type) {
    Figure* figure = (Figure*) malloc(sizeof(Figure));
    figure->type = type;
    figure->opts = get_default_options();
    return figure;
}

void set_name(Figure* figure, char * name) {
    strcpy(figure->name, name);
}
void set_coords(Figure* figure, Coord* coords) {
    free_coords(figure->coords);
    figure->coords = coords;
}
void set_radius(Figure* figure, long radius) {
    figure->long1 = radius;
}
void set_dim(Figure*  figure, long x, long y) {
    figure->long1 = x;
    figure->long2 = y;
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
void set_text(Figure* figure, char * text) {
    strcpy(figure->text, removeDblQuote(text));
}

char * get_name(Figure* figure) {
    return figure->name;
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
    Coord centroid;
    centroid.x = x/i;
    centroid.y = y/i;
    return centroid;
}

void print_options(FILE* stream, Figure* figure) {
    fprintf(stream, " stroke=\"%s\" fill=\"%s\"", figure->opts.color, figure->opts.fill_color);
    if (figure->opts.thickness != DEFAULT_THICKNESS) {
        fprintf(stream, " stroke-width=\"%ld\"", figure->opts.thickness);
    }
    if (!figure->opts.visible) {
        fprintf(stream, " visibility=\"hidden\"");
    }
    if(figure->opts.rotation_angle != DEFAULT_ANGLE) {
        Coord centroid = get_centroid(figure);
        fprintf(stream, " transform=\"rotate(%ld, %ld, %ld)\"", figure->opts.rotation_angle, centroid.x, centroid.y);
    }
}

void print_circle(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<circle cx=\"%ld\" cy=\"%ld\" r=\"%ld\"", figure->coords->x, figure->coords->y, figure->long1);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_rectangle(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<rect x=\"%ld\" y=\"%ld\" width=\"%ld\" height=\"%ld\"", figure->coords->x - (figure->long1/2), figure->coords->y - (figure->long2/2), figure->long1, figure->long2);
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
    fprintf(stream, "\t<ellipse cx=\"%ld\" cy=\"%ld\" rx=\"%ld\" ry=\"%ld\"", figure->coords->x, figure->coords->y, figure->long1, figure->long2);
    print_options(stream, figure);
    fprintf(stream, " />\n");
}

void print_text(FILE* stream, Figure* figure) {
    fprintf(stream, "\t<text x=\"%ld\" y=\"%ld\"", figure->coords->x, figure->coords->y);
    print_options(stream, figure);
    fprintf(stream, " font-size=\"%ld\"", figure->opts.font_size);
    fprintf(stream, " >");
    fprintf(stream, "%s", figure->text);
    fprintf(stream, "</text>\n");
}

char * toSvg(FILE* stream, Figure* figure) {
    switch (figure->type) {
        case e_CIRCLE:
            print_circle(stream, figure);
            break;
        case e_LINE:
            print_line(stream, figure);
            break;
        case e_TEXT:
            print_text(stream, figure);
            break;
        case e_POLYGON:
            print_polygon(stream, figure);
            break;
        case e_RECTANGLE:
            print_rectangle(stream, figure);
            break;
        case e_ELLIPSE:
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
    }
    free(fileName);
}

int delete_fig(HashMap* hashMap, char * name) {
    Figure* figure = get(hashMap, name);
    if(figure != NULL) {
        deleteKey(hashMap, name);
        freeFig(figure);
        return 1;
    }
    return 0;
}

int rename_fig(HashMap* hashMap, char * old, char * new) {
    Figure* figure = get(hashMap, old);
    if(figure != NULL) {
        strcpy(figure->name, new);
        deleteKey(hashMap, old);
        add(hashMap, figure);
        return 1;
    }
    return 0;
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

    copy->coords = copyCoord(figure->coords);

    return copy;
}

int copy(HashMap* hashMap, char * toCopy, char * copy) {
    Figure * orig = get(hashMap, toCopy);
    if (orig != NULL && get(hashMap, copy) == NULL) {
        Figure * new = copyFig(orig);
        strcpy(new->name, copy);
        add(hashMap, new);
        return 1;
    }
    return 0;
}
void set_color(Figure* figure, Transfer transfer) {
    strcpy(figure->opts.color, transfer.string);
}
void set_fill_color(Figure* figure, Transfer transfer) {
    strcpy(figure->opts.fill_color, transfer.string);
}
void set_visible(Figure* figure, Transfer transfer) {
    figure->opts.visible = transfer.int1;
}
void set_thickness(Figure* figure, Transfer transfer) {
    figure->opts.thickness = transfer.long1;
}
void set_font_size(Figure* figure, Transfer transfer) {
    figure->opts.font_size = transfer.long1;
}

void set_selected(Figure* figure, Transfer transfer) {
    figure->selected = transfer.int1;
}

NameList* appendName(NameList* list, char * name) {
    NameList* newList = (NameList*) malloc(sizeof(NameList));
    strcpy(newList->name, name);
    newList->next = list;
    return newList;
}

void move_fig(Figure* figure, Transfer args) {
    Coord* coord = figure->coords;
    while (coord != NULL) {
        coord->x += args.long1;
        coord->y += args.long2;
        coord = coord->next;
    }
}

void homothetie(Figure* figure, Transfer args) {
    switch(figure->type) {
        case e_CIRCLE:
            figure->long1 *= args.double1;
            break;
        case e_TEXT:
            figure->opts.font_size *= args.double1;
            break;
        case e_RECTANGLE:
        case e_ELLIPSE:
            figure->long1 *= args.double1;
            figure->long2 *= args.double1;
            break;
        default:;
            Coord centroid = get_centroid(figure);
            Coord* point = figure->coords;
            while (point != NULL) {
                long x_from_centroid = centroid.x - point->x;
                long y_from_centroid = centroid.y - point->y;
                point->x = centroid.x + (args.double1 * x_from_centroid);
                point->y = centroid.y + (args.double1 * y_from_centroid);
                point = point->next;
            }
            break;
    }
}

void rotate_fig(Figure* figure, Transfer args) {
    figure->opts.rotation_angle += args.long1;
    figure->opts.rotation_angle %= 360;
}

int name_exists(HashMap* hashMap, char * name) {
    Figure* figure = get(hashMap, name);
    return figure != NULL;
}