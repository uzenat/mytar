SOURCES=tools.c archive.c extract.c list.c delete.c main.c
OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-Wall -pedantic -g -std=c99 -D _POSIX_C_SOURCE=200112L
EXECUTABLE=mytar

$(EXECUTABLE): $(OBJECTS)
	gcc -o $(EXECUTABLE) $(OBJECTS)

%.o : %.c
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f *~

cleanall:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
	rm -f *~
