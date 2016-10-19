CFLAGS= -std=c89 -Wall -Wextra
EXECUTABLE=mytar

$(EXECUTABLE):
	cd SOURCE && make mytar
	mv SOURCE/mytar mytar

clean:
	cd SOURCE && make clean
	rm -f *~

cleanall:
	cd SOURCE && make cleanall
	rm -f mytar
	rm -f *~
