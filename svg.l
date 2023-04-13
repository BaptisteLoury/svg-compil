%{
  #include "svg.tab.h"

  #include <stdlib.h>
%}

digit     [0-9]
hexaDigit [0-9a-zA-Z]
pos_num {digit}+
num   -?{digit}+
real {num}\.{pos_num}?
name [a-zA-Z][a-zA-Z0-9\-_]*
string \"(\\(.|\n)|[^\\"\n])*\"
hexa_x_3 {hexaDigit}{hexaDigit}{hexaDigit}
color #{hexa_x_3}{hexa_x_3}?
dim {pos_num}x{pos_num}

%option noyywrap
%%

<<EOF>> {exit(0);}

{string} {
  yylval.str = strdup(yytext);
  return STRING;
}

{real} {
      yylval.real = strtod(yytext, NULL);
      return REAL;
}

{num} {
      yylval.num = strtoul(yytext, NULL, 10);
      return NUM;
}

{color} {
  yylval.str = strdup(yytext);
  return COLOR;
}

[aA][lL][lL]   {return ALL;}
[aA][tT] {return AT;}
[cC][iI][rR][cC][lL][eE] {return CIRCLE;}
[cC][oO][pP][yY] {return COPY;}
[cC][rR][eE][aA][tT][eE] {return CREATE;}
[dD][eE][lL][eE][tT][eE] {return DELETE;}
[dD][eE][sS][eE][lL][eE][cC][tT] {return DESELECT;}
[dD][uU][mM][pP] {return DUMP;}
[eE][lL][lL][iI][pP][sS][eE] {return ELLIPSE;}
[eE][oO][lL] {return EOL;}
[fF][iI][lL][lL] {return FILL;}
[fF][oO][nN][tT][sS][iI][zZ][eE] {return FONTSIZE;}
[iI][nN][vV][iI][sS][iI][bB][lL][eE] {return INVISIBLE;}
[lL][iI][nN][eE] {return LINE;}
[mM][oO][vV][eE] {return MOVE;}
[nN][oO][fF][iI][lL][lL] {return NOFILL;}
[pP][oO][lL][yY][gG][oO][nN] {return POLYGON;}
[rR][aA][dD][iI][uU][sS] {return RADIUS;}
[rR][eE][nN][aA][mM][eE] {return RENAME;}
[rR][eE][cC][tT][aA][nN][gG][lL][eE] {return RECTANGLE;}
[rR][oO][tT][aA][tT][eE] {return ROTATE;}
[sS][eE][lL][eE][cC][tT] {return SELECT;}
[sS][eE][tT] {return SET;}
[tT][eE][xX][tT] {return TEXT;}
[tT][hH][iI][cC][kK][nN][eE][sS][sS] {return THICKNESS;}
[vV][iI][sS][iI][bB][lL][eE] {return VISIBLE;}
[wW][iI][tT][hH] {return WITH;}
[zZ][oO][oO][mM] {return ZOOM;}

"\n" {return EOL;}
"(" {return COORD_L;}
")" {return COORD_R;}
"," {return COMMA;}
";" {return SEMICOLON;}
{dim} {
  yylval.dim.x = strtoul(strtok(yytext, "x"), NULL, 10);
  yylval.dim.y = strtoul(strtok(NULL, "x"), NULL, 10);
  return DIM;
}

{name} {
  yylval.str = strdup(yytext);
  return NAME;
}

.     { /* On ignore */ }

