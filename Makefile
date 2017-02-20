all: brickbreaker

brickbreaker: brickbreaker.cpp glad.c
	g++ -o brickbreaker brickbreaker.cpp glad.c -lGL -lglfw -ldl

clean:
	rm brickbreaker
