all: game

game:
	gcc -std=c99 -I ./include src/*.c -o main -Wall -Wno-missing-braces -L ./lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
