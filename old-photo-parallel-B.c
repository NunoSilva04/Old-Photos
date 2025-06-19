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


// Define mutex to prevent different threads from accessing ThreadData at the same time
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char *argv[]) {


  // Alloc memory for the timer mechanism
  struct timespec start_total_time;
  struct timespec start_no_par_time;
  struct timespec end_total_time;
  struct timespec end_no_par_time;


  // Start counting time
  clock_gettime(CLOCK_MONOTONIC, &start_total_time);
  clock_gettime(CLOCK_MONOTONIC, &start_no_par_time);


  // This variable will later assume 0 if we want to order by size, or 1 if we want to order by name
  bool order = 0;


  // Check if num of arguments is correct, if not abort execution
  if(argc != 4){
    printf("\nInvalid summon\n");

    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    return 0;
  }


  // Check if the number of threads if valid
  if(atoi(argv[2])<=0){
    printf("\nInvalid number of threads\n");

    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    return 0;
  }


  // Check if the last argument is valid, and set the boolean value
  if(strcmp(argv[3],"-size")==0){
    order = 0;
  } else if (strcmp(argv[3],"-name")==0){
    order = 1;
  } else {
    printf("\nInvalid last argument\n");

    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    return 0;
  }


  // Open the directory of the input files
  DIR *D;
  struct dirent *current_dir;
  struct stat current_dir_size;
  D = opendir(argv[1]);
  if(D == NULL){
    printf("\nCouldn't open directory\n");

    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    return 0;
  }


  // Create the subdirectory, if unexistent
  char subdirname[]= "/old_photo_PAR_B";
  char *subdir = (char*) malloc(strlen(argv[1]) + strlen(subdirname) + 2);
  if(subdir == NULL){
    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    return 1;
  }

  snprintf(subdir, strlen(argv[1]) + strlen(subdirname) + 2,"%s%s",argv[1],subdirname);
  create_directory(subdir);


  // Get the number of input files
  int numberofinputs = count_jpeg(D,subdir);
  if(numberofinputs<1){
    printf("\nNo images to process\n");

    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    free(subdir);
    return 0;
  }

  if(atoi(argv[2])>numberofinputs){
    printf("\nMore threads than images\n");

    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    free(subdir);
    return 0;
  }


  // Allocate memory for the array of strings
  char **strings = malloc(numberofinputs * sizeof(char *));
  if (strings == NULL) {
    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    free(subdir);
    return 1;
  }


  // Allocate memory for each individual string
  for (int i = 0; i < numberofinputs; i++) {
    strings[i] = malloc( 100 );
    if (strings[i] == NULL) {
      // Free memory before return and return
      pthread_mutex_destroy(&mutex);
      closedir(D);
      free(subdir);
      for (int ii = 0; ii < i; ii++) {
        free(strings[i]);
      }
      free(strings);
      return 1;
    }
  }


  // Fill the strings by alphabetical order
  if(order == 1){
    fill_strings(D,strings,numberofinputs,subdir);
  } else {
    fill_strings_by_size(D,strings,numberofinputs,subdir);
  }


  // Array to hold thread IDs
  pthread_t *threads;
  threads = malloc(numberofinputs * sizeof(pthread_t));
  if (threads == NULL) {
    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    free(subdir);
    for (int i = 0; i < numberofinputs; i++) {
      free(strings[i]);
    }
    free(strings);
    return 1;
  }


  // Allocate memory for thread data
  ThreadData *data = malloc(sizeof(ThreadData));
  if (data == NULL) {

    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    free(subdir);
    for (int i = 0; i < numberofinputs; i++) {
      free(strings[i]);
    }
    free(strings);
    free(threads);
    return 1;
  }

  data->strings_thread = strings;
  data->dir=argv[1];
  data->index=0;
  data->number_of_files=numberofinputs;


  // Calculate the non-paralelized time
  clock_gettime(CLOCK_MONOTONIC, &end_no_par_time);
  end_no_par_time = diff_timespec(&end_no_par_time, &start_no_par_time);


  // Create threads
  for (int i = 0; i < atoi(argv[2]); i++) {
    if (pthread_create(&threads[i], NULL, thread_function, data) != 0) {
      // Free memory before return and return
      pthread_mutex_destroy(&mutex);
      closedir(D);
      free(subdir);
      for (int i = 0; i < numberofinputs; i++) {
        free(strings[i]);
      }
      free(strings);
      free(threads);
      free(data);
      return 1;
    }
  }


  // Create the output file with the times of each thread
  char *filename = (char *) malloc( strlen(subdir) + 25 );
  if (filename == NULL) {
    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    free(subdir);
    for (int i = 0; i < numberofinputs; i++) {
      free(strings[i]);
    }
    free(strings);
    free(threads);
    free(data);
    return 1;
  }


  // Create the output file
  snprintf(filename, strlen(subdir) + 25,"%s/timing_B_%s%s.txt",subdir,argv[2],argv[3]);
  FILE *file = fopen(filename, "w+"); // "w+" opens for both writing and reading
  if (file == NULL) {
    // Free memory before return and return
    pthread_mutex_destroy(&mutex);
    closedir(D);
    free(subdir);
    for (int i = 0; i < numberofinputs; i++) {
      free(strings[i]);
    }
    free(strings);
    free(threads);
    free(data);
    free(filename);
    return 1;
  }


  // Keep reading stdin input 's' or 'S' until we have processed all of the files
  struct timeval timeout;
  char input[256];
  while (data->index != data->number_of_files) {
    // Set the timeout to 50ms
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;

    // Use select to wait for 50ms or until input is ready
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    if (ret == -1) {
      // Free memory before return and return
      pthread_mutex_destroy(&mutex);
      closedir(D);
      free(subdir);
      for (int i = 0; i < numberofinputs; i++) {
        free(strings[i]);
      }
      free(strings);
      free(threads);
      free(data);
      free(filename);
      free(file);
      return 1;

    } else if (FD_ISSET(STDIN_FILENO, &readfds)) {

      // Input is ready, read it
      if (fgets(input, sizeof(input), stdin) != NULL) {
        if(strcmp(input,"s\n")==0 || strcmp(input,"S\n")==0){
          printf("\nProgress : %d images processed & %d left\n",data->index,data->number_of_files-data->index);
        }
      }
    }
  }


  // Print the non paralelized time
  end_no_par_time.tv_nsec = end_no_par_time.tv_nsec / 10000;
  fprintf(file,"Non paralelized execution time : %ld,%ld seconds\n",end_no_par_time.tv_sec, end_no_par_time.tv_nsec);


  // Join threads
  for (int i = 0; i < atoi(argv[2]); i++) {
    struct timespec *execution_time;
    pthread_join(threads[i], (void **)&execution_time);
    execution_time->tv_nsec = execution_time->tv_nsec / 100000;
    fprintf(file,"Thread %d execution time        : %ld,%ld seconds\n",i+1,execution_time->tv_sec, execution_time->tv_nsec);
    free(execution_time);
  }


  // Calculate total execution time
  clock_gettime(CLOCK_MONOTONIC, &end_total_time);
  end_total_time = diff_timespec(&end_total_time, &start_total_time);


  // Print the total execution time
  end_total_time.tv_nsec = end_total_time.tv_nsec / 100000;
  fprintf(file,"Total execution time           : %ld,%ld seconds\n",end_total_time.tv_sec, end_total_time.tv_nsec);


  // Free the memory and return
  pthread_mutex_destroy(&mutex);
  closedir(D);
  free(subdir);
  for (int i = 0; i < numberofinputs; i++) {
    free(strings[i]);
  }
  free(strings);
  free(threads);
  free(data);
  free(filename);
  free(file);
  return 0;

}
