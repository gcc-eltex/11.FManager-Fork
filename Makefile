# Исходники
SRC=$(wildcard src/*.c)
#Обьектные файлы
OBJ=$(patsubst src/%.c,%.o,$(SRC))

all: libs
	gcc file_manager.c -o file_manager -L./lib/ -lfmanager -lncursesw -lm
libs: $(OBJ)
	ar -cr ./lib/libfmanager.a $(OBJ)
	rm -rf *.o
$(OBJ):
	gcc $(SRC) -c
clear:
	rm -rf *.o
run:
	./file_manager
