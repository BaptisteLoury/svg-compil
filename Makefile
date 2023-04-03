BIN= svg

all: $(BIN).y $(BIN).l
	bison -d -g -v $(BIN).y
	dot -Tpng $(BIN).dot -o $(BIN).png
	flex -dTv $(BIN).l
	$(CC) -c lex.yy.c -o lex.yy.o
	$(CC) -c figures.c -o figures.o
	$(CC) -c $(BIN).tab.c -o $(BIN).tab.o
	$(CC) -o $(BIN).bin lex.yy.o $(BIN).tab.o figures.o -lm

clean:
	rm -fv figures.o $(BIN).bin $(BIN).tab.h $(BIN).tab.c lex.yy.c lex.yy.o $(BIN).tab.o lex.backup $(BIN).dot $(BIN).png $(BIN).output *~ 