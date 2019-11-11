#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include "read_spec.h"

static struct spec *work_in_progress_spec;

static char buffer[1024];
/*****************************************************/
int read_line(FILE *f) {
   int i = 0;
   int c;

   c = getc(f);
   while(c == '\r' || c == '\n' || c == ' ' || c == '\t')
      c = getc(f);
   if(c == EOF)
     return 0;
   while(c != '\r' && c != '\n' && c != EOF) {
     if(i < sizeof(buffer)-1) {
       buffer[i] = c;
       i++;
     }
     c = getc(f);
   }
   buffer[i] = '\0';
   return 1;
}
/*****************************************************/
int create_work_in_progress(void) {
   work_in_progress_spec = malloc(sizeof(struct spec));
   if(work_in_progress_spec == NULL)
      return 0;
   memset(work_in_progress_spec, 0, sizeof(struct spec));
   return 1;
}
/*****************************************************/
void release_spec(struct spec *s) {
   if(s == NULL) {
      return;
   }

   while(s->first_region != NULL) {
      struct region *r;
      r = s->first_region;
      s->first_region = r->next;
      free(r);
   }

   if(s->name != NULL) {
      free(s->name);
      s->name = NULL;
   }

   free(s);
}

/*****************************************************/
void release_work_in_progress(void) {
   release_spec(work_in_progress_spec);
   work_in_progress_spec = NULL;
}
/*****************************************************/
int process_line(void) {
   struct region *r, *c;
   int i = 0;

   /* Eat whitespace */
   while(buffer[i] == ' ' || buffer[i] == '\t')
      i++;

   /* Ignore comments */
   if(buffer[i] == '#')
      return 1;

   /* If we don't have a text line, then we take it */
   if(work_in_progress_spec->name == NULL) {
      work_in_progress_spec->name = malloc(strlen(buffer+i)+1);
      if(work_in_progress_spec->name == NULL) {
	 return 0;
      }
      strcpy(work_in_progress_spec->name, buffer+i);
      fprintf(stderr,"Reading in spec '%s'\n",buffer);
      return 1;
   } 

   /*******************
    * Format should be 
    * - image number      Integer
    * - x origin (in mm)  Integer
    * - y origin (in mm)  Integer
    * - width (in mm)     Integer
    * - height (in mm)    Integer
    ****************/
   int image_id;
   int x,y,w,h;

   image_id = 0;
   x = y = w = h = 0.0;

   /* Image ID */
   if(buffer[i] < '0' || buffer [i] > '9') {
      fprintf(stderr, "Expecting digit\n");
      return 0;
   }
   while(buffer[i] >= '0' && buffer [i] <= '9') {
     image_id = image_id * 10 + buffer[i] - '0';
     i++;
   }

   /****************************************************/
   /* whitespace comma whitespace */
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;
   if(buffer[i] != ',') { fprintf(stderr,"Missing comma\n"); return 0; }
   i++;
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;

   /* x */
   if(buffer[i] < '0' || buffer [i] > '9') {
      fprintf(stderr, "Expecting digit\n");
      return 0;
   }
   while(buffer[i] >= '0' && buffer [i] <= '9') {
     x = x * 10 + buffer[i] - '0';
     i++;
   }

   /****************************************************/
   /* whitespace comma whitespace */
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;
   if(buffer[i] != ',') { fprintf(stderr,"Missing comma\n"); return 0; }
   i++;
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;

   /* y */
   if(buffer[i] < '0' || buffer [i] > '9') {
      fprintf(stderr, "Expecting digit\n");
      return 0;
   }
   while(buffer[i] >= '0' && buffer [i] <= '9') {
     y = y * 10 + buffer[i] - '0';
     i++;
   }

   /****************************************************/
   /* whitespace comma whitespace */
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;
   if(buffer[i] != ',') { fprintf(stderr,"Missing comma\n"); return 0; }
   i++;
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;

   /* w */
   if(buffer[i] < '1' || buffer [i] > '9') {
      fprintf(stderr, "Expecting digit\n");
      return 0;
   }
   while(buffer[i] >= '0' && buffer [i] <= '9') {
     w = w * 10 + buffer[i] - '0';
     i++;
   }

   /****************************************************/
   /* whitespace comma whitespace */
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;
   if(buffer[i] != ',') { fprintf(stderr,"Missing comma\n"); return 0; }
   i++;
   while(buffer[i] == ' ' || buffer[i] == '\t') i++;

   /* h */
   if(buffer[i] < '1' || buffer [i] > '9') {
      fprintf(stderr, "Expecting digit\n");
      return 0;
   }
   while(buffer[i] >= '0' && buffer [i] <= '9') {
     h = h * 10 + buffer[i] - '0';
     i++;
   }

   r = malloc(sizeof(struct region));
   if(r == NULL) {
      fprintf(stderr,"Unable to malloc");
      return 0;
   }
   r->image = image_id;
   r->origin_x     = (int)((x+0)*(300.0/25.4));
   r->origin_y     = (int)((y+0)*(300.0/25.4));
   r->limit_x      = (int)((x+w)*(300.0/25.4))-r->origin_x;
   r->limit_y      = (int)((y+h)*(300.0/25.4))-r->origin_y;

   printf("%i %i %i %i %i\n", r->image, r->origin_x, r->origin_y, r->limit_x, r->limit_y);

   r->next = NULL;
   if(work_in_progress_spec->first_region == NULL) {
      work_in_progress_spec->first_region = r;
   } else {
      c = work_in_progress_spec->first_region;
      while(c->next != NULL) {
	c = c->next;
      }
      c->next = r;
   }

   return 1;
}
/******************************************************/
struct spec *read_spec(char *fname) {
   FILE *f;
   struct spec *result;

   f = fopen(fname,"r");
   if(f == NULL) {
     return NULL;
   }
   create_work_in_progress();
   while(read_line(f)) {
      if(!process_line())
      {
	fprintf(stderr,"Invalid line '%s'\n",buffer);
	release_work_in_progress();
	fclose(f);
	return NULL;
      }
   }
   fclose(f);
   result = work_in_progress_spec;
   work_in_progress_spec = NULL;
   return result;
}
