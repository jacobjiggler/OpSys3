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


//RESTORE + RESTORE PRINTS
//also if you have time
  //mutex locks on totals
  //check for memory leaks
  //check for bad file permissions
  //confirm that totals are not overwriting each other and maybe use mutex locks on them
  //redo created subdir prints by passing in a created_subdir bool and creating the subdir inside of the function instead of before
void *backup_folder(void * arguments){
  struct dir_struct *dirs = arguments;
  printf("%s \n", dirs->copy_dir);
  pthread_t threads[1000];
  int size = 0;
  struct dirent *dp;
  DIR *dfd;
  char *dir;
  char *copy_dir;
  dir = dirs->current_dir;
  copy_dir = dirs->copy_dir;
  if ((dfd = opendir(dir)) == NULL)
  {
   fprintf(stderr, "Can't open %s\n", dir);
   return 0;
  }


  while ((dp = readdir(dfd)) != NULL)
{
    struct stat buf;
    struct stat copybuf;
    struct dir_struct tempstruct;
    sprintf(tempstruct.current_dir , "%s/%s",dir,dp->d_name);
    printf("%s \n", tempstruct.current_dir);
    //get location of new file by appending filename to copydir
    strcpy(tempstruct.copy_dir, copy_dir);
    strcat( tempstruct.copy_dir, &tempstruct.current_dir[strlen(dir)]);
    printf("%s \n", tempstruct.copy_dir);
    int len = strlen(tempstruct.current_dir);

    if( stat(tempstruct.current_dir,&buf ) == -1 )
    {
     printf("Unable to stat file: %s\n",tempstruct.current_dir) ;
     continue ;
    }
    //if object is directory
    if ( ( buf.st_mode & S_IFMT ) == S_IFDIR )
    {
      //ignore folders named these
      if (((len < 9) || strcmp(&tempstruct.current_dir[len-9],".mybackup")) && strcmp(&tempstruct.current_dir[len-1],".") && strcmp(&tempstruct.current_dir[len-2],"..")){

        //if folder already exists in .mybackup
        if( stat(tempstruct.copy_dir,&copybuf ) == 0 )
        {

          pthread_t thread;
          int rc = pthread_create( &thread, NULL, backup_folder, (void *)&tempstruct);
          if ( rc != 0 )
          {
            // pthreads functions do NOT use errno or perror()
            fprintf( stderr, "pthread_create() failed (%d): %s",
                     rc, strerror( rc ) );
            return EXIT_FAILURE;
          }
          threads[size] = thread;
          size++;
          printf("[thread %d] Backing up %s \n", (int)thread, &tempstruct.current_dir[strlen(dir)]);
        }
        //doesnt exist
        else{
          mkdir(tempstruct.copy_dir,S_IRWXU);
          total_subdirectories++;
          pthread_t thread;
          int rc = pthread_create( &thread, NULL, backup_folder, (void *)&tempstruct);
          if ( rc != 0 )
          {
            // pthreads functions do NOT use errno or perror()
            fprintf( stderr, "pthread_create() failed (%d): %s",
                     rc, strerror( rc ) );
            return EXIT_FAILURE;
          }
          threads[size] = thread;
          size++;
          printf("[thread %d] Backing up %s \n", (int)thread, &tempstruct.current_dir[strlen(dir)]);
          printf("[thread %d] created %s \n", (int)thread, &tempstruct.current_dir[strlen(dir)]);


        }



      }
    }
    //if object is normal file with permission
    else {
      if ( S_ISREG( buf.st_mode ) )
      {
        if ( buf.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) )
        {
          //make sure file you are backing up does not end in .bak
          if (len > 3){
            const char *last_four = &tempstruct.current_dir[len-4];
            if(strcmp(".bak",last_four)){


            }
          }
        }
      }
    }
  //lstat here
}
  //for all files
    //if directory
      //if not named .mybackup or . or ..
        //if folder already exists in .mybackup
          //call backup folder on that directory and keep thread number
          // with added folder name to both copy path and current_directory path
          //add thread to array
          //print "[thread threadnumber] Backing up foldername"
        //else
          //create folder in backup
          //increment total_subdirectories
          ////call backup folder on that directory and keep thread number
          // with added folder name to both copy path and current_directory path
          //add thread to array
          //print "[thread threadnumber] Backing up foldername"
          //print "[thread threadnumber] Created foldername"


    //else
      //if normal file and not ending in .bak
        // bool already_exists = false
        //if already exists(dont forget to check for .bak)
          //bool already_exists = true
          //delete old backup file
        //call backup_file and keep threadnumber
        //with added file name and .bak to both copy path and current directory
        // if already_exists
          //print "[thread threadnumber] WARNING: filename exists (overwriting!)"
        //print "[thread threadnumber] backing up filename ..."
        //add thread to array

  //for loop through threads array
    //join all the threads one at a time


  return NULL;
}
void *backup_file(void * arguments){
  struct dir_struct *dirs = arguments;
  printf("%s \n", dirs->copy_dir);
  //int bytes = lstat to get size of file
  // open new file with filename supplied
  //use read() and write() to copy file over with .bak
  //print "[thread threadnumber] copied x bytes from current dir to copy dir"
  //increment total bytes by bytes
  //increment total files by 1

  return NULL;
}
int main( int argc, char *argv[] ) {
  if ( argc > 2 ) /* argc should be 1 or 2 for correct execution */
    {
        /* We print argv[0] assuming it is the program name */
        printf( "Wrong number of arguments" );
        return 1;
    }

  pthread_t thread;
  //doesn't matter if .mybackup already exists. mkdir will return -1 if it doesnt
  mkdir(".mybackup",S_IRWXU);
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
    //print "successfully backed up total files (total bytes) and totalsubdirectories subdirectory"

  return 1;
}
