CC=gcc
WARNING=-Wall -Wextra
LIBES=-lm -lX11
OPTION=-O2 -fopenmp
MACROS=


MAIN_CFILES = ALife.c

CFILES = $(MAIN_CFILES)


MAIN_OFILES = ALife.o

OFILES = $(MAIN_OFILES)

OUTNAME = ALife




ImageSegmentation: $(OFILES)
	$(CC) $(WARNING) $(LIBES) $(OPTION) -o $(OUTNAME) $^


ALife.o: ALife.c
	$(CC) $(WARNING) $(OPTION) $(MACROS) -c $^



debug: $(CFILES)
	$(CC) $(WARNING) $(LIBES) -g -O2 $(MACROS) -o $(OUTNAME) $^

debugmp: $(CFILES)
	$(CC) $(WARNING) $(LIBES) $(OPTION) $(MACROS) -g -O2 -o $(OUTNAME) $^

clean:
	rm -f $(OFILES)
	find -name "*.gch" -exec rm {} +

