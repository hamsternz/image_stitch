
struct region {
   struct region *next;
   int image;
   int origin_x, origin_y;
   int limit_x,  limit_y;
};
struct spec {
   char *name;
   struct region *first_region;
};

struct spec *read_spec(char *fname);
void release_spec(struct spec *s);
