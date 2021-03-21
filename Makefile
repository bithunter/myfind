CFLAGS += -Wall -Wextra -pedantic -Werror
#LIBS += -lncurses
CC = gcc

%.o: %.c
	$(CC) -c $^


myfind: myfind.o util.o dir.o glob.o defs.h
	$(CC) $(CFLAGS) $(LIBS) -o myfind myfind.o util.o dir.o glob.o defs.h

clean:
	rm -f myfind *.o

docs:
	@doxygen myfinddoxy

# git remote add origin https://github.com/bithunter/myfind.git
# git branch -M main
# git push -u origin main
# git pull

.PHONY: clean docs