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
   return 0;
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
/************************************************************/
int main(int argc, char *argv[]) {
  struct spec *s;
  if(argc == 1) {
     fprintf(stderr,"No spec offered\n");
     return 0;
  }

  s = read_spec(argv[1]);
  if(s == NULL) {
     fprintf(stderr,"Failed to read spec\n");
     return 3;
  } else {
     fprintf(stderr,"Loaded spec\n");
     release_spec(s);
  }
  return 0;
}
