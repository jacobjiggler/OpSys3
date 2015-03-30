//Jacob Martin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

int total_files = 0;
int total_bytes = 0;
int total_subdirectories = 0;
struct dir_struct {
  char current_dir[1000];
  char copy_dir[1000];
};

void *backup_folder(void * arguments){
  struct dir_struct *dirs = arguments;
  printf("%s \n", dirs->copy_dir);

  //code here
  return NULL;
}
int main( int argc, char *argv[] ) {
  if ( argc > 2 ) /* argc should be 1 or 2 for correct execution */
    {
        /* We print argv[0] assuming it is the program name */
        printf( "Wrong number of arguments" );
        return 1;
    }
  // pass in directories without copy

  pthread_t thread;
  mkdir(".mybackup",1777);
  struct dir_struct dirs;
  getcwd(dirs.current_dir,1000);
  getcwd(dirs.copy_dir,1000);
  strcat(dirs.copy_dir, "/.mybackup");
  int rc = pthread_create( &thread, NULL, backup_folder, (void *)&dirs);
  if ( rc != 0 )
  {
    /* pthreads functions do NOT use errno or perror() */
    fprintf( stderr, "pthread_create() failed (%d): %s",
             rc, strerror( rc ) );
    return EXIT_FAILURE;
  }
  int rc2 = pthread_join( thread, NULL );
  if ( rc2 != 0 )
    {
      /* pthreads functions do NOT use errno or perror() */
      fprintf( stderr, "pthread_join() failed (%d): %s",
               rc, strerror( rc2 ) );

      /* end the entire process? */
      exit( EXIT_FAILURE );
    }


  return 1;
}
