%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

#include "figures.h"

int yyerror(char*);
int yylex();

HashMap* figures;
NameList* names;

%}

%code requires {
  #include "figures.h"
}

%union{
  long num;
  double real;
  char * str;
  Figure* fig;
  struct {
    long x;
    long y;
  } dim;
  Coord* point;
}

%token <real> REAL
%token <num> NUM
%token <str> COLOR NAME STRING
%token <dim> DIM
%token ALL AT CIRCLE COPY CREATE DELETE DESELECT DO DONE DUMP 
%token ELLIPSE FILL FONTSIZE FOREACH INVISIBLE LINE MOVE NOFILL
%token POLYGON RADIUS RENAME RECTANGLE ROTATE SELECT SET TEXT
%token THICKNESS VISIBLE WITH ZOOM EOL COORD_L COMMA COORD_R SEMICOLON

%type <fig> FIGURE FIG_CIRCLE FIG_RECTANGLE FIG_LINE FIG_TEXT FIG_POLYGON FIG_ELLIPSE
%type <point> COORD COORD_ALONE

%start S
%%

S:
    %empty                 { }
  | S INPUT               { }
  ;

INPUT:
      EOL {}
    | SEMICOLON {}
    | CMD EOL {}
    | CMD SEMICOLON {}
  ;

CMD:
    error {}
  | CREATE_CMD {printf("[ACTION]: CREATE\n");}
  | DUMP_CMD {printf("[ACTION]: DUMP\n");}
  | RENAME_CMD {printf("[ACTION]: RENAME\n");}
  | DELETE_CMD {printf("[ACTION]: DELETE\n");}
  | SET_CMD {printf("[ACTION]: SET\n");}
  | SELECT_CMD {printf("[ACTION]: SELECT\n");}
  | DESELECT_CMD {printf("[ACTION]: DESELECT\n");}
  | TRANSFORMATION_CMD {}
  | COPY_CMD {printf("[ACTION]: COPY\n");}
  ;

CREATE_CMD:
    CREATE FIGURE {add(figures, $2);}
  ;

DELETE_CMD:
    DELETE NAME {delete_fig(figures, $2);}
  ;

RENAME_CMD:
    RENAME NAME WITH NAME {rename_fig(figures, $2, $4);}
  ;

DUMP_CMD:
    DUMP {dump(figures);}
  | DUMP STRING {dump_to_file(figures, $2);}
  ;

SET_CMD:
  SET NAME_LIST OPTION {}
  ;

SELECT_CMD:
    SELECT SELECT_CMD_NAMES {}
  | SELECT ALL {set_all_selected(figures, 1);}
  ;

SELECT_CMD_NAMES:
    NAME {set_selected(figures, $1, 1);}
  | NAME COMMA SELECT_CMD_NAMES {{set_selected(figures, $1, 1);}}
  ;

DESELECT_CMD:
  DESELECT DESELECT_CMD_NAMES {}
  | DESELECT ALL {set_all_selected(figures, 0);}
  ;

DESELECT_CMD_NAMES:
    NAME {set_selected(figures, $1, 0);}
  | NAME COMMA SELECT_CMD_NAMES {set_selected(figures, $1, 0);}
  ;

TRANSFORMATION_CMD:
  MOVE_CMD {printf("[ACTION]: MOVE\n");}
  | ZOOM_CMD {printf("[ACTION]: ZOOM\n");}
  | ROTATE_CMD {printf("[ACTION]: ROTATE\n");}
  ;

MOVE_CMD:
    MOVE COORD_L NUM COMMA NUM COORD_R {move(figures, $3, $5);}
    | MOVE NAME_LIST COORD_L NUM COMMA NUM COORD_R {move_list(figures, names, $4, $6);}
  ;

ZOOM_CMD:
    ZOOM REAL {zoom(figures, $2);}
    | ZOOM NAME_LIST REAL {zoom_list(figures, names, $3);}
  ;

ROTATE_CMD:
    ROTATE NUM {rotate(figures, $2);}
    | ROTATE NAME_LIST NUM {rotate_list(figures, names, $3);}
  ;

COPY_CMD:
  COPY NAME NAME {copy(figures, $2, $3);}
  ;

OPTION:
  THICKNESS NUM  OPTION_OR_EMPTY {set_thickness(figures, names, $2);}
  | FILL WITH COLOR  OPTION_OR_EMPTY {set_fill_color(figures, names, $3);}
  | COLOR  OPTION_OR_EMPTY {set_color(figures, names, $1);}
  | NOFILL  OPTION_OR_EMPTY {set_fill_color(figures, names, "none");}
  | INVISIBLE  OPTION_OR_EMPTY {set_visible(figures, names, 0);}
  | VISIBLE  OPTION_OR_EMPTY {set_visible(figures, names, 1);}
  | FONTSIZE NUM  OPTION_OR_EMPTY {set_font_size(figures, names, $2);}
  ;

OPTION_OR_EMPTY:
  %empty {}
  | OPTION {}
  ;

FIGURE:
    FIG_CIRCLE {$$=$1;}
  | FIG_RECTANGLE {$$=$1;}
  | FIG_LINE {$$=$1;}
  | FIG_TEXT {$$=$1;}
  | FIG_POLYGON {$$=$1;}
  | FIG_ELLIPSE {$$=$1;}
  ;

FIG_CIRCLE:
    CIRCLE NAME AT COORD_L NUM COMMA NUM COORD_R RADIUS NUM {$$=createCircle($2, $5, $7, $10);}
  | CIRCLE NAME RADIUS NUM AT COORD_L NUM COMMA NUM COORD_R  {$$=createCircle($2, $7, $9, $4);}
  ;

FIG_RECTANGLE:
    RECTANGLE NAME AT COORD_L NUM COMMA NUM COORD_R DIM {$$=createRectangle($2, $5, $7, $9.x, $9.y);}
  | RECTANGLE NAME DIM AT COORD_L NUM COMMA NUM COORD_R {$$=createRectangle($2, $6, $8, $3.x, $3.y);}
  ;

FIG_LINE:
    LINE NAME COORD_L NUM COMMA NUM COORD_R COORD_L NUM COMMA NUM COORD_R {$$=createLine($2, $4, $6, $9, $11);}
  ;

FIG_TEXT:
    TEXT NAME AT COORD_L NUM COMMA NUM COORD_R STRING {$$=createText($2, $9, $5, $7);}
  | TEXT NAME STRING AT COORD_L NUM COMMA NUM COORD_R {$$=createText($2, $3, $6, $8);}
  ;

FIG_POLYGON:
    POLYGON NAME COORD {$$=createPolygon($2, $3);}
  ;

FIG_ELLIPSE:
    ELLIPSE NAME AT COORD_ALONE RADIUS DIM {$$=createEllipse($2, $4, $6.x, $6.y);}
  | ELLIPSE NAME AT RADIUS DIM AT COORD_ALONE {$$=createEllipse($2, $7, $5.x, $5.y);}
  ;

COORD:
    COORD_ALONE {$$=$1;}
  | COORD_L NUM COMMA NUM COORD_R COORD {$$=generate_coords($6, $2, $4);}
  ;

COORD_ALONE :
    COORD_L NUM COMMA NUM COORD_R {$$=generate_coords(NULL, $2, $4);}
  ;

NAME_LIST:
  NAME {names=appendName(NULL, $1);}
  | NAME COMMA NAME_LIST {names=appendName(names, $1);}
  ;

%%

int yyerror(char *s) {
    fprintf(stderr, "[ERROR]: %s\n",s);
    exit(0);
}

int main(int argc, char * argv[]) {
    if (argc == 2) {
        int fd = open(argv[1], O_RDONLY);
        dup2(fd, 0);
    }

    figures = createHashMap(1000);
    yyparse();
    freeHashMap(figures);
}