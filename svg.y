%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


int yyerror(char*);
int yylex();

int line = 1;

%}

%code requires {
  #include "figures.h"

  HashMap* figures;
  NameList* names;
  Figure* current;
  Transfer transfer;
}

%union{
  long num;
  double real;
  char * str;
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
%token ALL AT CIRCLE COPY CREATE DELETE DESELECT DUMP 
%token ELLIPSE FILL FONTSIZE INVISIBLE LINE MOVE NOFILL
%token POLYGON RADIUS RENAME RECTANGLE ROTATE SELECT SET TEXT
%token THICKNESS VISIBLE WITH ZOOM EOL COORD_L COMMA COORD_R SEMICOLON
%type <point> COORD_AT_LEAST_3 COORD_1 COORD_2

%start S
%%

S:
    %empty                 { }
  | S INPUT               { }
  ;

INPUT:
      EOL {line++;}
    | SEMICOLON {}
    | CMD EOL {line++;}
    | CMD SEMICOLON {}
  ;

CMD:
    error {}
  | CREATE_CMD {}
  | DUMP_CMD {}
  | RENAME_CMD {}
  | DELETE_CMD {}
  | SET_CMD {}
  | SELECT_CMD {}
  | DESELECT_CMD {}
  | TRANSFORMATION_CMD {}
  | COPY_CMD {}
  ;

CREATE_CMD:
    CREATE CREATE_FIGURE {if(!add(figures, current)) {yyerror("this name is already taken");} current = NULL;}
  ;

DELETE_CMD:
    DELETE NAME {if(!delete_fig(figures, $2)) {yyerror("this name is not set");} }
  ;

RENAME_CMD:
    RENAME NAME WITH NAME {if(!rename_fig(figures, $2, $4)) {yyerror("source name is not set or target name is already taken");}}
  ;

DUMP_CMD:
    DUMP {dump(figures);}
  | DUMP STRING {dump_to_file(figures, $2);}
  ;

SET_CMD:
  SET NAME_LIST OPTION_LIST {}
  | SET ALL OPTION_ALL {}
  ;

SELECT_CMD:
    SELECT NAME_LIST {transfer.int1 = 1;iterate_over_list(figures, names, set_selected, transfer);}
  | SELECT ALL {transfer.int1 = 1;iterate_over_map(figures, set_selected, transfer, 1);}
  ;

DESELECT_CMD:
  DESELECT NAME_LIST {transfer.int1 = 0;iterate_over_list(figures, names, set_selected, transfer);}
  | DESELECT ALL {transfer.int1 = 0;iterate_over_map(figures, set_selected, transfer, 1);}
  ;

TRANSFORMATION_CMD:
  MOVE_CMD {}
  | ZOOM_CMD {}
  | ROTATE_CMD {}
  ;

MOVE_CMD:
    MOVE COORD_L NUM COMMA NUM COORD_R {transfer.long1=$3;transfer.long2=$5;iterate_over_map(figures, move_fig, transfer, 0);}
    | MOVE ALL COORD_L NUM COMMA NUM COORD_R {transfer.long1=$4;transfer.long2=$6;iterate_over_map(figures, move_fig, transfer, 1);}
    | MOVE NAME_LIST COORD_L NUM COMMA NUM COORD_R {transfer.long1=$4;transfer.long2=$6;iterate_over_list(figures, names, move_fig, transfer);}
  ;

ZOOM_CMD:
    ZOOM REAL {transfer.double1 = $2; iterate_over_map(figures, homothetie, transfer, 0);}
    | ZOOM ALL REAL {transfer.double1 = $3;iterate_over_map(figures, homothetie, transfer, 1);}
    | ZOOM NAME_LIST REAL {transfer.double1 = $3;iterate_over_list(figures, names, homothetie, transfer);}
  ;

ROTATE_CMD:
    ROTATE NUM {transfer.long1 = $2; iterate_over_map(figures, rotate_fig, transfer, 0);}
    | ROTATE ALL NUM {transfer.long1 = $3;iterate_over_map(figures, rotate_fig, transfer, 1);}
    | ROTATE NAME_LIST NUM {transfer.long1 = $3;iterate_over_list(figures, names, rotate_fig, transfer);}
  ;

COPY_CMD:
  COPY NAME NAME {if(!copy(figures, $2, $3)) {yyerror("source name is not set or target name is already taken");}}
  ;

OPTION_LIST:
  THICKNESS NUM OPTION_OR_EMPTY_LIST {transfer.long1 = $2;iterate_over_list(figures, names, set_thickness, transfer);}
  | FILL WITH COLOR OPTION_OR_EMPTY_LIST {transfer.string = $3;iterate_over_list(figures, names, set_fill_color, transfer);}
  | COLOR OPTION_OR_EMPTY_LIST {transfer.string = $1;iterate_over_list(figures, names, set_color, transfer);}
  | NOFILL OPTION_OR_EMPTY_LIST {transfer.string = "none";iterate_over_list(figures, names, set_fill_color, transfer);}
  | INVISIBLE OPTION_OR_EMPTY_LIST {transfer.int1 = 0;iterate_over_list(figures, names, set_visible, transfer);}
  | VISIBLE OPTION_OR_EMPTY_LIST {transfer.int1 = 1;iterate_over_list(figures, names, set_visible, transfer);}
  | FONTSIZE NUM OPTION_OR_EMPTY_LIST {transfer.long1 = $2;iterate_over_list(figures, names, set_font_size, transfer);}
  ;

OPTION_OR_EMPTY_LIST:
  %empty {}
  | OPTION_LIST {}
  ;

OPTION_ALL:
  THICKNESS NUM OPTION_OR_EMPTY_ALL {transfer.long1 = $2;iterate_over_map(figures, set_thickness, transfer, 1);}
  | FILL WITH COLOR OPTION_OR_EMPTY_ALL {transfer.string = $3;iterate_over_map(figures, set_fill_color, transfer, 1);}
  | COLOR OPTION_OR_EMPTY_ALL {transfer.string = $1;iterate_over_map(figures, set_color, transfer, 1);}
  | NOFILL OPTION_OR_EMPTY_ALL {transfer.string = "none";iterate_over_map(figures, set_fill_color, transfer, 1);}
  | INVISIBLE OPTION_OR_EMPTY_ALL {transfer.int1 = 0;iterate_over_map(figures, set_visible, transfer, 1);}
  | VISIBLE OPTION_OR_EMPTY_ALL {transfer.int1 = 1;iterate_over_map(figures, set_visible, transfer, 1);}
  | FONTSIZE NUM OPTION_OR_EMPTY_ALL {transfer.long1 = $2;iterate_over_map(figures, set_font_size, transfer, 1);}
  ;

OPTION_OR_EMPTY_ALL:
  %empty {}
  | OPTION_ALL {}
  ;

SET_OPTION:
  THICKNESS NUM SET_OPTION_OR_EMPTY {transfer.long1 = $2;set_thickness(current, transfer);}
  | FILL WITH COLOR SET_OPTION_OR_EMPTY {transfer.string = $3;set_fill_color(current, transfer);}
  | COLOR SET_OPTION_OR_EMPTY {transfer.string = $1;set_color(current, transfer);}
  | NOFILL SET_OPTION_OR_EMPTY {transfer.string = "none";set_fill_color(current, transfer);}
  | INVISIBLE SET_OPTION_OR_EMPTY {transfer.int1 = 0;set_visible(current, transfer);}
  | VISIBLE SET_OPTION_OR_EMPTY {transfer.int1 = 1;set_visible(current, transfer);}
  | FONTSIZE NUM SET_OPTION_OR_EMPTY {transfer.long1 = $2;set_font_size(current, transfer);}
  ;

SET_OPTION_OR_EMPTY:
  %empty {}
  | SET_OPTION {}
  ;

CREATE_FIGURE:
    CREATE_CIRCLE {}
  | CREATE_RECTANGLE {}
  | CREATE_LINE {}
  | CREATE_TEXT {}
  | CREATE_POLYGON {}
  | CREATE_ELLIPSE {}
  ;

CREATE_CIRCLE:
    FIG_CIRCLE SET_OPTION_OR_EMPTY SET_NAME CREATE_CIRCLE_NODE_1 {}
  | FIG_CIRCLE SET_OPTION_OR_EMPTY SET_1_COORD CREATE_CIRCLE_NODE_2 {}
  | FIG_CIRCLE SET_OPTION_OR_EMPTY SET_RADIUS CREATE_CIRCLE_NODE_3 {}
  ;
CREATE_CIRCLE_NODE_1:
    SET_OPTION_OR_EMPTY SET_1_COORD CREATE_CIRCLE_NODE_6 {}
  | SET_OPTION_OR_EMPTY SET_RADIUS CREATE_CIRCLE_NODE_4 {}
  ;
CREATE_CIRCLE_NODE_2:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_CIRCLE_NODE_6 {}
  | SET_OPTION_OR_EMPTY SET_RADIUS CREATE_CIRCLE_NODE_5 {}
  ;
CREATE_CIRCLE_NODE_3:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_CIRCLE_NODE_4 {}
  | SET_OPTION_OR_EMPTY SET_1_COORD CREATE_CIRCLE_NODE_5 {}
  ;
CREATE_CIRCLE_NODE_4:
    SET_OPTION_OR_EMPTY SET_1_COORD SET_OPTION_OR_EMPTY{}
  ;
CREATE_CIRCLE_NODE_5:
    SET_OPTION_OR_EMPTY SET_NAME SET_OPTION_OR_EMPTY {}
  ;
CREATE_CIRCLE_NODE_6:
    SET_OPTION_OR_EMPTY SET_RADIUS SET_OPTION_OR_EMPTY {}
  ;

FIG_CIRCLE:
  CIRCLE {current=create_figure(e_CIRCLE);}
  ;

CREATE_RECTANGLE:
    FIG_RECTANGLE SET_OPTION_OR_EMPTY SET_NAME CREATE_RECTANGLE_NODE_1 {}
  | FIG_RECTANGLE SET_OPTION_OR_EMPTY SET_1_COORD CREATE_RECTANGLE_NODE_2 {}
  | FIG_RECTANGLE SET_OPTION_OR_EMPTY SET_DIM CREATE_RECTANGLE_NODE_3 {}
  ;
CREATE_RECTANGLE_NODE_1:
    SET_OPTION_OR_EMPTY SET_1_COORD CREATE_RECTANGLE_NODE_6 {}
  | SET_OPTION_OR_EMPTY SET_DIM CREATE_RECTANGLE_NODE_4 {}
  ;
CREATE_RECTANGLE_NODE_2:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_RECTANGLE_NODE_6 {}
  | SET_OPTION_OR_EMPTY SET_DIM CREATE_RECTANGLE_NODE_5 {}
  ;
CREATE_RECTANGLE_NODE_3:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_RECTANGLE_NODE_4 {}
  | SET_OPTION_OR_EMPTY SET_1_COORD CREATE_RECTANGLE_NODE_5 {}
  ;
CREATE_RECTANGLE_NODE_4:
    SET_OPTION_OR_EMPTY SET_1_COORD SET_OPTION_OR_EMPTY{}
  ;
CREATE_RECTANGLE_NODE_5:
    SET_OPTION_OR_EMPTY SET_NAME SET_OPTION_OR_EMPTY {}
  ;
CREATE_RECTANGLE_NODE_6:
    SET_OPTION_OR_EMPTY SET_DIM SET_OPTION_OR_EMPTY {}
  ;
FIG_RECTANGLE:
  RECTANGLE {current=create_figure(e_RECTANGLE);}
  ;

CREATE_LINE:
    FIG_LINE SET_OPTION_OR_EMPTY SET_NAME CREATE_LINE_NODE_1 {}
  | FIG_LINE SET_OPTION_OR_EMPTY SET_2_COORD CREATE_LINE_NODE_2 {}
  ;
CREATE_LINE_NODE_1:
  SET_OPTION_OR_EMPTY SET_2_COORD SET_OPTION_OR_EMPTY {}
  ;
CREATE_LINE_NODE_2:
  SET_OPTION_OR_EMPTY SET_NAME SET_OPTION_OR_EMPTY {}
  ;

FIG_LINE:
  LINE {current=create_figure(e_LINE);}
  ;

CREATE_TEXT:
    FIG_TEXT SET_OPTION_OR_EMPTY SET_NAME CREATE_TEXT_NODE_1 {}
  | FIG_TEXT SET_OPTION_OR_EMPTY SET_1_COORD CREATE_TEXT_NODE_2 {}
  | FIG_TEXT SET_OPTION_OR_EMPTY SET_TEXT CREATE_TEXT_NODE_3 {}
  ;
CREATE_TEXT_NODE_1:
    SET_OPTION_OR_EMPTY SET_1_COORD CREATE_TEXT_NODE_6 {}
  | SET_OPTION_OR_EMPTY SET_TEXT CREATE_TEXT_NODE_4 {}
  ;
CREATE_TEXT_NODE_2:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_TEXT_NODE_6 {}
  | SET_OPTION_OR_EMPTY SET_TEXT CREATE_TEXT_NODE_5 {}
  ;
CREATE_TEXT_NODE_3:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_TEXT_NODE_4 {}
  | SET_OPTION_OR_EMPTY SET_1_COORD CREATE_TEXT_NODE_5 {}
  ;
CREATE_TEXT_NODE_4:
    SET_OPTION_OR_EMPTY SET_1_COORD SET_OPTION_OR_EMPTY{}
  ;
CREATE_TEXT_NODE_5:
    SET_OPTION_OR_EMPTY SET_NAME SET_OPTION_OR_EMPTY {}
  ;
CREATE_TEXT_NODE_6:
    SET_OPTION_OR_EMPTY SET_TEXT SET_OPTION_OR_EMPTY {}
  ;

FIG_TEXT:
  TEXT {current=create_figure(e_TEXT);}
  ;

CREATE_POLYGON:
    FIG_POLYGON SET_OPTION_OR_EMPTY SET_NAME CREATE_POLYGON_NODE_1 {}
  | FIG_POLYGON SET_OPTION_OR_EMPTY SET_AT_LEAST_3_COORD CREATE_POLYGON_NODE_2 {}
  ;
CREATE_POLYGON_NODE_1:
  SET_OPTION_OR_EMPTY SET_AT_LEAST_3_COORD SET_OPTION_OR_EMPTY {}
  ;
CREATE_POLYGON_NODE_2:
  SET_OPTION_OR_EMPTY SET_NAME SET_OPTION_OR_EMPTY {}
  ;
FIG_POLYGON:
  POLYGON {current=create_figure(e_POLYGON);}
  ;

CREATE_ELLIPSE:
    FIG_ELLIPSE SET_OPTION_OR_EMPTY SET_NAME CREATE_ELLIPSE_NODE_1 {}
  | FIG_ELLIPSE SET_OPTION_OR_EMPTY SET_1_COORD CREATE_ELLIPSE_NODE_2 {}
  | FIG_ELLIPSE SET_OPTION_OR_EMPTY RADIUS SET_DIM CREATE_ELLIPSE_NODE_3 {}
  ;
CREATE_ELLIPSE_NODE_1:
    SET_OPTION_OR_EMPTY SET_1_COORD CREATE_ELLIPSE_NODE_6 {}
  | SET_OPTION_OR_EMPTY RADIUS SET_DIM CREATE_ELLIPSE_NODE_4 {}
  ;
CREATE_ELLIPSE_NODE_2:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_ELLIPSE_NODE_6 {}
  | SET_OPTION_OR_EMPTY RADIUS SET_DIM CREATE_ELLIPSE_NODE_5 {}
  ;
CREATE_ELLIPSE_NODE_3:
    SET_OPTION_OR_EMPTY SET_NAME CREATE_ELLIPSE_NODE_4 {}
  | SET_OPTION_OR_EMPTY SET_1_COORD CREATE_ELLIPSE_NODE_5 {}
  ;
CREATE_ELLIPSE_NODE_4:
    SET_OPTION_OR_EMPTY SET_1_COORD SET_OPTION_OR_EMPTY{}
  ;
CREATE_ELLIPSE_NODE_5:
    SET_OPTION_OR_EMPTY SET_NAME SET_OPTION_OR_EMPTY {}
  ;
CREATE_ELLIPSE_NODE_6:
    SET_OPTION_OR_EMPTY RADIUS SET_DIM SET_OPTION_OR_EMPTY {}
  ;
FIG_ELLIPSE:
  ELLIPSE {current=create_figure(e_ELLIPSE);}
  ;

COORD_AT_LEAST_3:
    COORD_L NUM COMMA NUM COORD_R COORD_2 {$$=generate_coords($6, $2, $4);}
  | COORD_L NUM COMMA NUM COORD_R COORD_AT_LEAST_3 {$$=generate_coords($6, $2, $4);}
  ;

COORD_2:
  COORD_L NUM COMMA NUM COORD_R COORD_1 {$$=generate_coords($6, $2, $4);}
  ;

COORD_1:
    COORD_L NUM COMMA NUM COORD_R {$$=generate_coords(NULL, $2, $4);}
  ;

NAME_LIST:
    NAME {if (!name_exists(figures, $1)) {yyerror("name is not set");} else {names=appendName(NULL, $1);} }
  | NAME COMMA NAME_LIST {names=appendName(names, $1);}
  ;

SET_NAME:
    NAME {set_name(current, $1);}
  ;
SET_AT_LEAST_3_COORD:
    COORD_AT_LEAST_3 {set_coords(current, $1);}
  ;
SET_2_COORD:
    COORD_2 {set_coords(current, $1);}
  ;
SET_1_COORD:
    AT COORD_1 {set_coords(current, $2);}
  ;
SET_RADIUS:
    RADIUS NUM {set_radius(current, $2);}
  ;
SET_TEXT:
    STRING {set_text(current, $1);}
  ;
SET_DIM:
    DIM {set_dim(current, $1.x, $1.y);}
  ;

%%

int yyerror(char *s) {
    fprintf(stderr, "[ERROR] l.%d: %s\n",line,s);
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