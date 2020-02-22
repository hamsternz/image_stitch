all: image_stitch overlay

overlay : overlay.o
	gcc -o overlay overlay.o read_spec.o -Wall -pedantic -O4 -ljpeg 

overlay.o : overlay.c
	gcc -c overlay.c -Wall -pedantic -O4

ui.o : ui.c ui.h
	gcc -c ui.c -Wall -pedantic -O4

read_spec.o : read_spec.c read_spec.h
	gcc -c read_spec.c -Wall -pedantic -O4

image_stitch.o : image_stitch.c read_spec.h
	gcc -c image_stitch.c -Wall -pedantic -O4

image_stitch : image_stitch.o read_spec.o ui.o
	gcc -o image_stitch image_stitch.o read_spec.o ui.o -Wall -pedantic -O4 -ljpeg  -lncurses
