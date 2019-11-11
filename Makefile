all: image_stitch

read_spec.o : read_spec.c read_spec.h
	gcc -c read_spec.c -Wall -pedantic -O4

image_stitch.o : image_stitch.c read_spec.h
	gcc -c image_stitch.c -Wall -pedantic -O4

image_stitch : image_stitch.o read_spec.o
	gcc -o image_stitch image_stitch.o read_spec.o -Wall -pedantic -O4 -ljpeg 
