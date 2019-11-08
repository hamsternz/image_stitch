
struct region {
   int image;
   int origin_x, origin_y;
   int limit_x,  limit_y;
};
struct spec {
   char *name;
   int n_regions;
   struct region *regions;
};

struct spec *read_spec(char *fname);
void release_spec(struct spec *s);
