EXE = mydiz
CC = gcc

mydiz: *.c 
	$(CC) -o mydiz *.c

.PHONY : clean
clean:
	rm $(EXE);rm *.di
