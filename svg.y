%{

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "figures.h"

int yyerror(char*);
int yylex();

HashMap* figures;

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
  Options* opts;
}

%token <real> REAL
%token <num> NUM
%token <str> COLOR NAME STRING
%token <dim> DIM
%token ALL AT CIRCLE COPY CREATE DELETE DESELECT DO DONE DUMP 
%token ELLIPSE FILL FONTSIZE FOREACH INVISIBLE LINE MOVE NOFILL
%token POLYGON RADIUS RENAME RECTANGLE ROTATE SELECT SET TEXT
%token THICKNESS VISIBLE WITH ZOOM EOL COORD_L COMMA COORD_R

%type <fig> FIGURE FIG_CIRCLE FIG_RECTANGLE FIG_LINE FIG_TEXT
%type <opts> OPTION

%start S
%%

S:
    %empty                 { }
  | S INPUT               { }
  ;

INPUT:
    error EOL {}
  | CREATE_CMD EOL {printf("[ACTION]: CREATE\n");}
  | DUMP_CMD EOL {printf("[ACTION]: DUMP\n");}
  | RENAME_CMD EOL {printf("[ACTION]: RENAME\n");}
  | DELETE_CMD EOL {printf("[ACTION]: DELETE\n");}
  | SET_CMD EOL {printf("[ACTION]: SET\n");}
  | SELECT_CMD EOL {printf("[ACTION]: SELECT\n");}
  | DESELECT_CMD EOL {printf("[ACTION]: DESELECT\n");}
  | MOVE_CMD EOL {printf("[ACTION]: MOVE\n");}
  | ZOOM_CMD EOL {printf("[ACTION]: ZOOM\n");}
  | ROTATE_CMD EOL {printf("[ACTION]: ROTATE\n");}
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
  SET NAME OPTION {applyOptions(figures, $2, $3);}
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

MOVE_CMD:
    MOVE COORD_L NUM COMMA NUM COORD_R {move(figures, $3, $5);}
  ;

ZOOM_CMD:
    ZOOM REAL {zoom(figures, $2);}
  ;

ROTATE_CMD:
    ROTATE NUM {rotate(figures, $2);}
  ;

OPTION:
  %empty {$$=get_default_options();}
  | THICKNESS NUM OPTION {$$=set_thickness($3, $2);}
  | FILL WITH COLOR OPTION {$$=set_fill_color($4, $3);}
  | COLOR OPTION {$$=set_color($2, $1);}
  | NOFILL OPTION {$$=set_fill_color($2, "none");}
  | INVISIBLE OPTION {$$=set_visible($2, 0);}
  | VISIBLE OPTION {$$=set_visible($2, 1);}
  | FONTSIZE NUM OPTION {$$=set_font_size($3, $2);}
  ;

FIGURE:
    FIG_CIRCLE {$$=$1;}
  | FIG_RECTANGLE {$$=$1;}
  | FIG_LINE {$$=$1;}
  | FIG_TEXT {$$=$1;}
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
%%

int yyerror(char *s) {
    fprintf(stderr, "[ERROR]: %s\n",s);

}

int main(void) {
    figures = createHashMap(1000);
    yyparse();
    freeHashMap(figures);
}

