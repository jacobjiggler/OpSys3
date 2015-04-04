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
  //mutex locks on totals or use join output
  //check for memory leaks
  //check for bad file permissions
  //confirm that totals are not overwriting each other and maybe use mutex locks on them
  //redo created subdir prints by passing in a created_subdir bool and creating the subdir inside of the function instead of before

  void *backup_file(void * arguments){
    struct dir_struct *dirs = arguments;
    char *file = dirs->current_dir;
    char *copy_file = dirs->copy_dir;
    printf("I am backing up this file -> %s \n", dirs->copy_dir);
    struct stat buf;
    if(stat(file, &buf) != 0) {
      printf("error with stating file %s \n", file);
    }
    int bytes = buf.st_size;
    char ch;
    FILE *source, *target;
    source = fopen(file, "r");
    if( source == NULL )
  {
     printf("error reading...\n");
     return 0;
  }
  target = fopen(copy_file, "w");

   if( target == NULL )
   {
      fclose(source);
      printf("error writing...\n");
      return 0;
   }

   while( ( ch = fgetc(source) ) != EOF )
      fputc(ch, target);


   fclose(source);
   fclose(target);
   printf("[thread %lu] Copied %d bytes from %s to %s \n", (unsigned long)pthread_self(), bytes, file, copy_file);


    //increment total bytes by bytes
    total_bytes += bytes;
    //increment total files by 1
    total_files += 1;

    return NULL;
  }

void *backup_folder(void * arguments){
  struct dir_struct *dirs = arguments;
  pthread_t threads[500];
  struct dir_struct* structs[200];
  int size = 0;
  int size2 = 0;
  struct dirent *dp;
  DIR *dfd;
  char *dir;
  char *copy_dir;
  dir = dirs->current_dir;
  copy_dir = dirs->copy_dir;
  //printf("name of dirs passed into function: %s %s \n", dir, copy_dir);
  if ((strcmp(&dir[strlen(dir)-2],"..") ==0) || (strcmp(&copy_dir[strlen(copy_dir)-2],"..") == 0)){
    printf("gotcha1 \n");
    return 0;
  }
  if ((dfd = opendir(dir)) == NULL)
  {
   fprintf(stderr, "Can't open %s\n", dir);
   return 0;
  }


  while ((dp = readdir(dfd)) != NULL)
{
    struct stat buf;
    struct stat copybuf;
    char tempdir[200];
    char tempcopydir[200];
    sprintf(tempdir , "%s/%s",dir,dp->d_name);
    //get location of new file by appending filename to copydir
    strcpy(tempcopydir, copy_dir);
    strcat( tempcopydir, &tempdir[strlen(dir)]);
    int len = strlen(tempdir);
    if (!strcmp(&tempdir[len-2],"..")){
      //printf("gotcha2 %s \n", &tempdir[len-2]);
      continue;
    }
    if( stat(tempdir,&buf ) == -1 )
    {
     printf("Unable to stat file: %s\n",tempdir) ;
     continue ;
    }
    //if object is directory
    if ( ( buf.st_mode & S_IFMT ) == S_IFDIR )
    {
      //ignore folders named these
      if (((len < 9) || strcmp(&tempdir[len-9],".mybackup")) && strcmp(&tempdir[len-1],".") && strcmp(&tempdir[len-2],"..")){

        //if folder already exists in .mybackup
        if( stat(tempcopydir,&copybuf ) == 0 )
        {
          struct dir_struct* tempstruct = (struct dir_struct *)malloc(sizeof(struct dir_struct));
          strcpy(tempstruct->current_dir,tempdir);
          strcpy(tempstruct->copy_dir,tempcopydir);
          structs[size2] = tempstruct;
          size2++;
          pthread_t thread;
          int rc = pthread_create( &thread, NULL, backup_folder, (void *)tempstruct);
          if ( rc != 0 )
          {
            // pthreads functions do NOT use errno or perror()
            fprintf( stderr, "pthread_create() failed (%d): %s",
                     rc, strerror( rc ) );
            return 0;
          }
          threads[size] = thread;
          size++;
          printf("[thread %lu] Backing up %s \n", (unsigned long)thread, &tempdir[strlen(dir)]);
        }
        //doesnt exist
        else{

          mkdir(tempcopydir,S_IRWXU);
          total_subdirectories++;
          struct dir_struct* tempstruct = (struct dir_struct *)malloc(sizeof(struct dir_struct));
          strcpy(tempstruct->current_dir,tempdir);
          strcpy(tempstruct->copy_dir,tempcopydir);
          structs[size2] = tempstruct;
          size2++;
          pthread_t thread;
          int rc = pthread_create( &thread, NULL, backup_folder, (void *)tempstruct);
          if ( rc != 0 )
          {
            // pthreads functions do NOT use errno or perror()
            fprintf( stderr, "pthread_create() failed (%d): %s",
                     rc, strerror( rc ) );
            return 0;
          }
          threads[size] = thread;
          size++;
          printf("[thread %lu] Backing up %s \n", (unsigned long)thread, &tempdir[strlen(dir)]);
          printf("[thread %lu] created %s \n", (unsigned long)thread, &tempdir[strlen(dir)]);


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
            const char *last_four = &tempdir[len-4];
            if(strcmp(".bak",last_four)){
              int already_exists = 0;
              strcat( tempcopydir, ".bak");
              if (stat(tempcopydir,&copybuf ) == 0){
                already_exists = 1;
                remove(tempcopydir);
              }
              struct dir_struct* tempstruct = (struct dir_struct *)malloc(sizeof(struct dir_struct));
              strcpy(tempstruct->current_dir,tempdir);
              strcpy(tempstruct->copy_dir,tempcopydir);
              structs[size2] = tempstruct;
              size2++;
              pthread_t thread;
              int rc = pthread_create( &thread, NULL, backup_file, (void *)tempstruct);
              if ( rc != 0 )
              {
                // pthreads functions do NOT use errno or perror()
                fprintf( stderr, "pthread_create() failed (%d): %s",
                         rc, strerror( rc ) );
                return 0;
              }
              threads[size] = thread;
              size++;
              if (already_exists){
                printf("[thread %lu] WARNING: %s exists (overwriting!) \n", (unsigned long)thread, &tempdir[strlen(dir)]);
              }
              printf("[thread %lu] Backing up %s ...\n", (unsigned long)thread, &tempdir[strlen(dir)]);


            }
          }
        }
      }
    }
}




    //join all the threads one at a time
    int itr = 0;
    while (itr < size){

      int rc2 = pthread_join( threads[itr], NULL );
      if ( rc2 != 0 )
        {
          //pthreads functions do NOT use errno or perror()
          fprintf( stderr, "pthread_join() failed (%d): %s", rc2, strerror( rc2 ) );

          //end the entire process? //
          exit( EXIT_FAILURE );
        }

      itr++;
    }
    //free up memory
    itr = 0;
    while (itr < size2){
      free(structs[itr]);
      itr++;
    }
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
  //struct dir_struct dirs;
  struct dir_struct* dirs = (struct dir_struct *)malloc(sizeof(struct dir_struct));
  getcwd(dirs->current_dir,1000);
  getcwd(dirs->copy_dir,1000);
  strcat(dirs->copy_dir, "/.mybackup");


  int rc = pthread_create( &thread, NULL, backup_folder, (void *)dirs);
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
    free(dirs);
    if (total_subdirectories != 1){
      printf("successfully backed up %d (%d) and %d subdirectories \n", total_files, total_bytes, total_subdirectories);
    }
    else {
      printf("successfully backed up %d (%d) and %d subdirectory \n", total_files, total_bytes, total_subdirectories);
    }
    //print "successfully backed up total files (total bytes) and totalsubdirectories subdirectory"

  return 1;
}
