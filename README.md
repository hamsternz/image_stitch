# image_stitch

For digital fabric printing, this hack to combines
- six 50x50cm jpegs (@300 DPI) into a 150x100cm panel 
- two 75x100cm jpegs (@300 DPI) into a 150x100 panel 
- two 50x50cm jpegs and a 100cm x 100cm image (@300 DPI) into a 150x100 panel 

Build "image_stitch" and then use:

    image_stitch image1.jpg image2.jpg image3.jpg image4.jpg image5.jpg image6.jpg

For best results:
- the 50cm x 50cm images should be 5906x5906.
- the 75cm x 100cm images should be 8859x11812
- the 100cm x 100cm images should be 11812x11812

It will write an output file "out.jpg" that will be 150cm x 100cm (@300DPI).

A small black outline will be put on each panel to aid with cutting.
