#ifndef FUNC_H
#define FUNC_H
#include <dirent.h> 
#include <stdio.h> 
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <gd.h>
#include <sys/select.h>

extern pthread_mutex_t mutex;

typedef struct images{
  char image_name[100];
  long int size;
}images;

typedef struct ThreadData{
  char **strings_thread;
  char *dir;
  int index;
  int number_of_files;
}ThreadData;

gdImagePtr  texture_image(gdImagePtr , gdImagePtr );
gdImagePtr  smooth_image(gdImagePtr );
gdImagePtr  contrast_image(gdImagePtr );
gdImagePtr  sepia_image(gdImagePtr );
gdImagePtr read_png_file(char * );
int write_png_file(gdImagePtr , char * );
gdImagePtr read_jpeg_file(char * );
int write_jpeg_file(gdImagePtr , char * );
struct timespec diff_timespec(const struct timespec *, const struct timespec *);
void *thread_function(void *);
void create_directory(char *);
int count_jpeg(DIR *, const char *);
int file_exists_in_subdir(const char *, const char *);
int compare_strings(const void *, const void *);
void fill_strings(DIR *, char **, int , const char *);
int compare_images_by_size(const void *, const void *);
void fill_strings_by_size(DIR *, char **, int , const char *);

#endif