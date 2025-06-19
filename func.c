#define _POSIX_C_SOURCE 199309L
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
#include "func.h"

/*

Image processing functions given to us by the teachers

*/
gdImagePtr  texture_image(gdImagePtr in_img, gdImagePtr texture_img){
	
	gdImagePtr out_img;

	int width,heigth;

	width = in_img->sx;
	heigth = in_img->sy;


	gdImageSetInterpolationMethod(texture_img, GD_BILINEAR_FIXED);
    gdImagePtr scalled_pattern = gdImageScale(texture_img, width, heigth);


	out_img =  gdImageClone (in_img);

	gdImageCopy(out_img, scalled_pattern, 0, 0, 0, 0, width, heigth);
	gdImageDestroy(scalled_pattern);
	return(out_img);		
} 
gdImagePtr  smooth_image(gdImagePtr in_img){
	
	gdImagePtr out_img;
	
	out_img =  gdImageClone (in_img);
	if (!out_img) {
		return NULL;
	}

	int ret = gdImageSmooth(out_img, 20);


	if (!out_img) {
		return NULL;
	}

	return(out_img);		
} 
gdImagePtr  contrast_image(gdImagePtr in_img){
	
	gdImagePtr out_img;
	
	out_img =  gdImageClone (in_img);
	if (!out_img) {
		return NULL;
	}

	int ret = gdImageContrast(out_img, -20);


	return(out_img);		
} 
gdImagePtr  sepia_image(gdImagePtr in_img){
	
	gdImagePtr out_img;
	
	out_img =  gdImageClone (in_img);
	if (!out_img) {
		return NULL;
	}

	int ret = gdImageColor(out_img, 120, 70, 0, 0);


	return(out_img);		
}
gdImagePtr read_png_file(char * file_name){

	FILE * fp;
	gdImagePtr read_img;

	fp = fopen(file_name, "rb");
   	if (!fp) {
        fprintf(stderr, "Can't read image %s\n", file_name);
        return NULL;
    }
    read_img = gdImageCreateFromPng(fp);
    fclose(fp);
  	if (read_img == NULL) {
    	return NULL;
    }

	return read_img;
}
int write_png_file(gdImagePtr write_img, char * file_name){
	FILE * fp;

	fp = fopen(file_name, "wb");
	if (fp == NULL) {
		return 0;
	}
	gdImagePng(write_img, fp);
	fclose(fp);

	return 1;
}
gdImagePtr read_jpeg_file(char * file_name){

	FILE * fp;
	gdImagePtr read_img;

	fp = fopen(file_name, "rb");
   	if (!fp) {
        fprintf(stderr, "Can't read image %s\n", file_name);
        return NULL;
    }
    read_img = gdImageCreateFromJpeg(fp);
    fclose(fp);
  	if (read_img == NULL) {
    	return NULL;
    }

	return read_img;
}
int write_jpeg_file(gdImagePtr write_img, char * file_name){
	FILE * fp;

	fp = fopen(file_name, "wb");
	if (fp == NULL) {
		return 0;
	}
	gdImageJpeg(write_img, fp, 70);
	fclose(fp);

	return 1;
}

/*

Function that computes the difference between 2 timespecs

*/
struct timespec diff_timespec(const struct timespec *time1, const struct timespec *time0) {
  assert(time1);
  assert(time0);
  struct timespec diff = {.tv_sec = time1->tv_sec - time0->tv_sec, //
      .tv_nsec = time1->tv_nsec - time0->tv_nsec};
  if (diff.tv_nsec < 0) {
    diff.tv_nsec += 1000000000; // nsec/sec
    diff.tv_sec--;
  }
  return diff;
}

/*

Thread function

*/
void *thread_function(void *arg){

  // Calculate time of each thread;
  struct timespec start_par_time ;
  struct timespec end_par_time ;
  clock_gettime(CLOCK_MONOTONIC, &start_par_time);

  ThreadData *data = (ThreadData *)arg;

  pthread_mutex_lock(&mutex);

  while(data->index < data->number_of_files){
    int filetoprocess = data->index;
    data->index++;
    pthread_mutex_unlock(&mutex);

    // For loop which will process the images
    char filepath[200];
    char output[250];

    snprintf(filepath, sizeof(filepath),"%s/%s",data->dir,data->strings_thread[filetoprocess]);
    snprintf(output, sizeof(output),"%s/old_photo_PAR_B/%s",data->dir,data->strings_thread[filetoprocess]);

    gdImagePtr img = read_jpeg_file(filepath);
    gdImagePtr in_texture_img =  read_png_file("./paper-texture.png");

	gdImagePtr newimg = contrast_image(img);
    gdImageDestroy(img);
	img = smooth_image(newimg);
    gdImageDestroy(newimg);
	newimg = texture_image(img , in_texture_img);
    gdImageDestroy(img);
	img = sepia_image(newimg);

    write_jpeg_file(img, output);

    // Free the memory 
    gdImageDestroy(img);
    gdImageDestroy(newimg);
    gdImageDestroy(in_texture_img); 

    pthread_mutex_lock(&mutex);
  }

  pthread_mutex_unlock(&mutex);

  clock_gettime(CLOCK_MONOTONIC, &end_par_time);
  // Return time
  struct timespec *par_time = (struct timespec *) malloc(sizeof(struct timespec));
  *par_time = diff_timespec(&end_par_time, &start_par_time);

  return (void *) par_time;
  
} 

/*

Function to create the subdirectory for the output files

*/
void create_directory(char *dirName){
  DIR *d;
  d = opendir(dirName);
  if(d == NULL){
    mkdir(dirName, 0777);
  }else{
    closedir(d);
  }
  return;
}

/*

Function to count .jpeg/.jpg files not present in the subdirectory

*/
int count_jpeg(DIR *D, const char *subdir) {
    struct dirent *entry;
    int count = 0;

    if (D == NULL) {
        fprintf(stderr, "Invalid directory stream.\n");
        return -1; // Return -1 for error
    }

    // Read entries in the directory
    while ((entry = readdir(D)) != NULL) {
        const char *filename = entry->d_name;
        const char *ext = strrchr(filename, '.'); // Get file extension

        if (ext != NULL) {
            // Check if the file has the correct extension
            if (strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".JPEG") == 0 ||
                strcmp(ext, ".jpg") == 0  || strcmp(ext, ".JPG") == 0) {

                // Check if the file exists in the subdirectory
                if (!file_exists_in_subdir(subdir, filename)) {
                    count++;
                }
            }
        }
    }

    return count;
}

/*

Helper function to check if a file exists in the subdirectory

*/
int file_exists_in_subdir(const char *subdir, const char *filename) {
    DIR *sub_dir = opendir(subdir);
    if (sub_dir == NULL) {
        fprintf(stderr, "Could not open subdirectory %s\n", subdir);
        return 0; // Assume the file does not exist if subdirectory cannot be opened
    }

    struct dirent *entry;
    while ((entry = readdir(sub_dir)) != NULL) {
        if (strcmp(entry->d_name, filename) == 0) {
            closedir(sub_dir);
            return 1; // File found
        }
    }

    closedir(sub_dir);
    return 0; // File not found
}

/*

Comparison function for sorting strings alphabetically

*/
int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/*

Function to fill strings array with valid filenames

*/
void fill_strings(DIR *D, char **strings, int numberofinputs, const char *subdir) {
    struct dirent *dir_entry;
    int count = 0;

    // Rewind the directory stream to the beginning
    rewinddir(D);

    while ((dir_entry = readdir(D)) != NULL && count < numberofinputs) {
        char *file_name = dir_entry->d_name;

        // Check if the file has a valid extension
        if (strstr(file_name, ".jpeg") || strstr(file_name, ".jpg") || strstr(file_name, ".JPEG") || strstr(file_name, ".JPG")) {
            // Check if the file already exists in the subdir
            if (!file_exists_in_subdir(subdir, file_name)) {
                // Copy the filename into the strings array
                strncpy(strings[count], file_name, 100);
                strings[count][99] = '\0'; // Ensure null-termination
                count++;
            }
        }
    }

    // Sort the strings array alphabetically
    qsort(strings, count, sizeof(char *), compare_strings);
}

/*

Comparison function for sorting by file size

*/
int compare_images_by_size(const void *a, const void *b) {
    const images *imgA = (const images *)a;
    const images *imgB = (const images *)b;
    if (imgA->size < imgB->size) return -1;
    if (imgA->size > imgB->size) return 1;
    return 0; // If sizes are equal
}

/*

Function to fill strings array with valid filenames by size

*/
void fill_strings_by_size(DIR *D, char **strings, int numberofinputs, const char *subdir) {
    struct dirent *dir_entry;
    images *image_data = malloc(numberofinputs * sizeof(images));
    if (image_data == NULL) {
        perror("Failed to allocate memory for image data");
        exit(1);
    }

    int count = 0;
    rewinddir(D); // Reset the directory stream
    while ((dir_entry = readdir(D)) != NULL && count < numberofinputs) {
        char *file_name = dir_entry->d_name;

        // Check if the file has a valid extension
        if (strstr(file_name, ".jpeg") || strstr(file_name, ".jpg") || strstr(file_name, ".JPEG") || strstr(file_name, ".JPG")) {
              // Check if the file already exists in the subdir
              if (!file_exists_in_subdir(subdir, file_name)) {

              struct stat file_stat;

              // Adjust subdir path by subtracting 16 characters
              char adjusted_subdir[200];
              strncpy(adjusted_subdir, subdir, strlen(subdir) - 16);
              adjusted_subdir[strlen(subdir) - 16] = '\0'; // Null-terminate the adjusted path

              // Generate the full file path
              char filepath[512];
              snprintf(filepath, sizeof(filepath), "%s/%s", adjusted_subdir, file_name);

              // Use `stat` to get file information
              if (stat(filepath, &file_stat) == -1) {
                  continue; // Skip this file
              }

              // Check if it's a regular file
              if (S_ISREG(file_stat.st_mode)) {
                  strncpy(image_data[count].image_name, file_name, 100);
                  image_data[count].image_name[99] = '\0'; // Ensure null-termination
                  image_data[count].size = file_stat.st_size;
                  count++;
              } else {
                  printf("Skipping non-regular file: %s\n", filepath); // Debugging output
              }
            }
        }
    }

    // Sort the image_data array by file size
    qsort(image_data, count, sizeof(images), compare_images_by_size);

    // Copy sorted filenames into the strings array
    for (int i = 0; i < count; i++) {
        strncpy(strings[i], image_data[i].image_name, 100);
        strings[i][99] = '\0'; // Ensure null-termination
    }

    // Free temporary storage
    free(image_data);
}
