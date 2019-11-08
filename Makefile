all: read_spec image_stitch

read_spec : read_spec.c read_spec.h
	gcc -o read_spec read_spec.c -Wall -pedantic -O4 -ljpeg 

image_stitch : image_stitch.c
	gcc -o image_stitch image_stitch.c -Wall -pedantic -O4 -ljpeg 
