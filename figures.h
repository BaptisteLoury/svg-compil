#define MAX_KEY_LEN 100
#define MAX_VALUE_LEN 100

typedef enum {
    e_CIRCLE,
    e_RECTANGLE,
    e_LINE,
    e_TEXT,
    e_POLYGON,
    e_ELLIPSE
} fig_type;

typedef struct {
    char * string;
    long long1;
    long long2;
    int int1;
    double double1;
} Transfer;

typedef struct fig Figure;
typedef struct opt Options;
typedef struct nd Node;
typedef struct hm HashMap;
typedef struct coord Coord;
typedef struct nameL NameList;

HashMap* createHashMap(int size);
int add(HashMap* hashMap, Figure* figure);
void freeHashMap(HashMap* hashMap);
int name_exists(HashMap* hashMap, char * name);

void iterate_over_map(HashMap* hashMap, void (*func)(Figure*, Transfer), Transfer args, int all);
void iterate_over_list(HashMap* hashMap, NameList* list, void (*func)(Figure*, Transfer), Transfer args);

Figure* create_figure(fig_type type);
void set_name(Figure* figure, char * name);
void set_coords(Figure* figure, Coord* coords);
void set_radius(Figure* figure, long radius);
void set_text(Figure* figure, char * text);
void set_dim(Figure*  figure, long x, long y);

char * get_name(Figure* figure);

Coord* generate_coords(Coord* coords, long x, long y);

int delete_fig(HashMap* hashMap, char * name);
int rename_fig(HashMap* hashMap, char * old, char * new);

void dump(HashMap* hashMap);
void dump_to_file(HashMap* hashMap, char * file);

int copy(HashMap* hashMap, char * toCopy, char * copy);

void set_color(Figure* figure, Transfer transfer);
void set_fill_color(Figure* figure, Transfer transfer);
void set_visible(Figure* figure, Transfer transfer);
void set_thickness(Figure* figure, Transfer transfer);
void set_font_size(Figure* figure, Transfer transfer);
void set_selected(Figure* figure, Transfer transfer);

void rotate_fig(Figure* figure, Transfer args);
void homothetie(Figure* figure, Transfer args);
void move_fig(Figure* figure, Transfer args);

NameList* appendName(NameList* list, char * name);