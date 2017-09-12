compiler:
	bison -d -v -g cminus.y
	flex cminus.l
	clang -o cminus *.c -ly -lfl -g
