#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ncurses.h>
#include <malloc.h>

#include "read_spec.h"

struct image {
	   struct image *next;
	      char *name;
};
struct image *first_image;
int image_count = 0;

#define MAX_SPECS 100
struct spec *spec_list[MAX_SPECS];
int specs_used = 0;

#define PREVIEW_COLS (78)
#define PREVIEW_ROWS (24)

#define COL_1  1
#define COL_2  2
#define COL_3  3
#define COL_4  4

int generate(struct spec *s, int argc, char *argv[]);

char preview_chars[] = ".~-o|\\/";
void print_preview(struct spec *s, int highlight) {
   struct region *r;
   char preview[PREVIEW_COLS][PREVIEW_ROWS];
   int x,y;

   for(y = 0; y < PREVIEW_ROWS; y++) {
      for(x = 0; x < PREVIEW_COLS; x++) {
	 preview[x][y] = ' ';
      }
   }
   r = s->first_region;
   while(r != NULL) {
      int top,left,bottom,right;
      top    = r->origin_y*PREVIEW_ROWS/11812;
      left   = r->origin_x*PREVIEW_COLS/17718;
      bottom = (r->origin_y+r->limit_y)*PREVIEW_ROWS/11812;
      right  = (r->origin_x+r->limit_x)*PREVIEW_COLS/17718;
      for(y = top ; y < bottom && y < PREVIEW_ROWS; y++) {
         for(x = left ; x < right && x < PREVIEW_COLS; x++) {
	    if(r->image == highlight) {
               preview[x][y] = 'X';

	    } else {
	       preview[x][y] = preview_chars[r->image];
	    }
	 }
      }
      r = r->next;
   }
   clear();
   attron(COLOR_PAIR(COL_3));
   move(1,1);
   printw("Format : ");
   printw(s->name);
   for(y = 0; y < PREVIEW_ROWS; y++) {
      move(2+y,1);
      for(x = 0; x < PREVIEW_COLS; x++) {
	 if(preview[x][y] == 'X') {
            attron(COLOR_PAIR(COL_1));
	 } else {
            attron(COLOR_PAIR(COL_2));
	 }
         addch(preview[x][y]);
      }
   }
   
}

int read_all_specs(void) {
  DIR *d;
  struct dirent *de; 
  char fname[512];

  d = opendir("specs");
  if(d == NULL) {
     fprintf(stderr, "Unable to open spec dir\n");
     return 0;
  }

  de = readdir(d);
  while(de != NULL) {
    if(de->d_type == DT_REG && specs_used < MAX_SPECS) {
       int len = strlen(de->d_name);
       if(len > 5) {
	  if(strcmp(de->d_name+len-5, ".spec")==0) {
	     strcpy(fname,"specs/");
	     strcat(fname,de->d_name);
	     spec_list[specs_used] = read_spec(fname);
	     if(spec_list[specs_used] != NULL) {
		specs_used++;
	     } else {
	        printf("Unable to read %s\n",fname);
	     }
	  }
       }
    }
    de = readdir(d);
  }
  closedir(d);
  return 0;
}

void scan_all_files(void) {
  DIR *d;
  struct dirent *de; 
  d = opendir(".");
  if(d == NULL) {
     fprintf(stderr, "Unable to open spec dir\n");
     return;
  }

  de = readdir(d);
  while(de != NULL) {
    if(de->d_type == DT_REG && specs_used < MAX_SPECS) {
       int len = strlen(de->d_name);
       if(len > 4) {
	  if(strcmp(de->d_name+len-4, ".jpg")==0) {
	      struct image *i;
              // printf("Found image file '%s'\n",de->d_name);
	      i = malloc(sizeof(struct image));
	      if(i != NULL) {
		 i->name = malloc(strlen(de->d_name)+1);
		 if(i->name == NULL) {
	            free(i);
		 } else {
	            struct image *c;
	            strcpy(i->name, de->d_name);
                    /* Add to list */
	            i->next = NULL;
		    c = first_image;
		    if(c == NULL) {
		       first_image = i;
		    } else {
		       while(c->next != NULL) {
			 c = c->next;
		       }
		       c->next = i;
		    }
		    image_count++;
		 }
	      } 
	  }
       }
    }
    de = readdir(d);
  }
  closedir(d);
}

void print_images(int highlight, int images_needed) { 
   int i = 1;
     struct image *img = first_image;
     while(img != NULL) {
	char name[31];
        int j;
        for(j = 0; j < sizeof(name)-1 && img->name[j] != 0; j++)
	   name[j] = img->name[j];
        while(j<sizeof(name)-1) {
	   name[j] = ' ';
	   j++;
	}
	name[j] = 0;

        move(1+i,80);
        if(i == highlight) {
          attron(COLOR_PAIR(COL_1));
	} else if(i > images_needed) {
          attron(COLOR_PAIR(COL_3));
	} else {
          attron(COLOR_PAIR(COL_2));
	}
	printw(name);
        img = img->next;
	i++;
     }
}


int move_image_up(int cursor) {
  struct image *a, *b, *c, *d;
  // CHeck bounds (NOT ZERO BASED */
  if(cursor < 2 || cursor > image_count)
     return 0;
  /* Need to update the head cursor */
  if(cursor == 2) {
     a = first_image;
     b = first_image->next;
     a->next = b->next;
     b->next = a;
     first_image = b;
     return 1;
  }
  a = first_image;
  while(cursor > 3 && a->next != NULL) {
    a = a->next;
    cursor--;
  }

  if(a == NULL || a->next == NULL) {
    return 0;
  }

  b = a->next;
  c = b->next;
  d = c->next;
  a->next = c;
  c->next = b;
  b->next = d;
  return 1;
}

int move_image_down(int cursor) {
  return 0;
}

int ui(char **spec_name) { 
  int i=0;
  int highlight = 1;
  read_all_specs();
  scan_all_files();
  initscr();
  if (has_colors() == FALSE) {
    endwin();
    printf("Your terminal does not support color\n");
    return 1;
  }
  start_color();
  init_pair(COL_1, COLOR_BLACK, COLOR_RED);
  init_pair(COL_2, COLOR_WHITE, COLOR_BLUE);
  init_pair(COL_3, COLOR_WHITE, COLOR_BLACK);
  init_pair(COL_4, COLOR_RED,   COLOR_MAGENTA);

  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  while(1) {
     int c;
     int images_needed = 0;
     struct region *r = spec_list[i]->first_region;
     while(r != NULL) {
	if(images_needed < r->image)
	   images_needed = r->image;
	r = r->next;
     }
     print_preview(spec_list[i],highlight);
     print_images(highlight,images_needed);

     move(27,3);
     attron(COLOR_PAIR(COL_3));
     printw("left/right = select layout, up/down = select image, +/- = change image order, g = go, ESC to quit");
     refresh();
     c = getch();
     switch(c) {
	case KEY_UP:
	   if(highlight > 1) {
	     highlight--;
	   }
	   break;
	case KEY_DOWN:
	   if(highlight < image_count) {
	      highlight++;
	   }
	   break;
	case KEY_LEFT:
	   if(i == 0) 
	       i = specs_used-1;
	   else
	       i--;
	   break;
	case '_':
	case '-':
	   if(move_image_up(highlight)) {
	      highlight--;
	   }
	   break;
	case '+':
	case '=':
	   move_image_down(highlight);
	   break;
	case KEY_RIGHT:
	   if(i == specs_used-1) 
	       i = 0;
	   else
	       i++;
	   break;
	case 'G':
	case 'g':
           if(image_count < images_needed) {
              beep();
              break;
           }

	   clear();
	   refresh();
           endwin();
           char **files = malloc(images_needed*sizeof(char *));
	   if(files != NULL) {
	      int k;
	      struct image *img = first_image;

	      for(k = 0; k < images_needed && img != NULL; k++) {
		 files[k] = img->name;
                 img = img->next;
	      }

	      generate(spec_list[i],images_needed,files);
	      free(files);
	   }
	   return 1;
	case '\033':
	   clear();
	   refresh();
           endwin();
	   return 1;
        default:
	   break;
     }
  }
  return 1; 
}
