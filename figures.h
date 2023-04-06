#define MAX_KEY_LEN 100
#define MAX_VALUE_LEN 100

typedef struct fig Figure;

typedef struct opt Options;

typedef struct nd Node;

typedef struct hm HashMap;

typedef struct coord Coord;

typedef struct nameL NameList;

HashMap* createHashMap(int size);

void add(HashMap* hashMap, Figure* figure);

void freeHashMap(HashMap* hashMap);

Figure* createCircle(char * name, long x, long y, long radius);

Figure* createRectangle(char * name, long x, long y, long dimension_x, long dimension_y);

Figure* createPolygon(char * name, Coord* points);

Figure* createEllipse(char * name, Coord* points, long rx, long ry);

Figure* createLine(char * name, long x1, long y1, long x2, long y2);

Figure* createText(char * name, char * text, long x, long y);

Coord* generate_coords(Coord* coords, long x, long y);

void delete_fig(HashMap* hashMap, char * name);

void rename_fig(HashMap* hashMap, char * old, char * new);

void dump(HashMap* hashMap);

void dump_to_file(HashMap* hashMap, char * file);

void copy(HashMap* hashMap, char * toCopy, char * copy);

void apply_options_list(HashMap* HashMap, NameList* list, Options* options);

void set_color(HashMap* hashMap, NameList* list, char * color);
void set_fill_color(HashMap* hashMap, NameList* list, char * color);
void set_visible(HashMap* hashMap, NameList* list, int visible);
void set_thickness(HashMap* hashMap, NameList* list, long thickness);
void set_font_size(HashMap* hashMap, NameList* list, long font_size);

void set_selected(HashMap* hashMap, char * name, int selected);
void set_all_selected(HashMap* hashMap, int selected);

void move(HashMap* hashMap, long x, long y);
void zoom(HashMap* hashMap, double factor);
void rotate(HashMap* hashMap, long angle);

void move_list(HashMap* hashMap, NameList* list, long x, long y);
void zoom_list(HashMap* hashMap, NameList* list, double factor);
void rotate_list(HashMap* hashMap, NameList* list, long angle);

NameList* appendName(NameList* list, char * name);