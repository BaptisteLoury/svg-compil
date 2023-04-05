#define MAX_KEY_LEN 100
#define MAX_VALUE_LEN 100

typedef struct fig Figure;

typedef struct opt Options;

typedef struct nd Node;

typedef struct hm HashMap;

typedef struct coord Coord;

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

Figure* copyFig(Figure * figure);

void applyOptions(HashMap* HashMap, char * name, Options* options);

Options* get_default_options();
Options* set_color(Options* opt, char * color);
Options* set_fill_color(Options* opt, char * color);
Options* set_visible(Options* opt, int visible);
Options* set_thickness(Options* opt, long thickness);
Options* set_font_size(Options* opt, long font_size);

void set_selected(HashMap* hashMap, char * name, int selected);
void set_all_selected(HashMap* hashMap, int selected);

void move(HashMap* hashMap, long x, long y);
void zoom(HashMap* hashMap, double factor);
void rotate(HashMap* hashMap, long angle);