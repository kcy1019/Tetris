all	: tetris.o
	gcc tetris.c -o tetris -O3 -lncurses
	echo "" > rank.txt
	chmod u+rw rank.txt

clean:
	rm tetris *.o rank.txt
