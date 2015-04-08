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
pthread_mutex_t locksubdirstats;
pthread_mutex_t lockfilestats;
struct dir_struct {
  char current_dir[1000];
  char copy_dir[1000];
};

//also if you have time
  //fix read()/write() to be more efficient
  //check for memory leaks
  //redo created subdir prints by passing in a created_subdir bool and creating the subdir inside of the function instead of before



    void *backup_file(void * arguments){
      struct dir_struct *dirs = arguments;
      char *file = dirs->current_dir;
      char *copy_file = dirs->copy_dir;
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

     pthread_mutex_lock(&lockfilestats);
      //increment total bytes by bytes
      total_bytes += bytes;
      //increment total files by 1
      total_files += 1;
      pthread_mutex_unlock(&lockfilestats);
      return NULL;
    }


//TODO RESTORE
//TO MAKE SURE YOU DONT BACKUP FOR NO REASON CHECK FILE MODIFIED DATE WITH STAT DATE.




  void *restore_folder(void * arguments){
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
            int rc = pthread_create( &thread, NULL, restore_folder, (void *)tempstruct);
            if ( rc != 0 )
            {
              // pthreads functions do NOT use errno or perror()
              fprintf( stderr, "pthread_create() failed (%d): %s",
                       rc, strerror( rc ) );
              return 0;
            }
            threads[size] = thread;
            size++;
            printf("[thread %lu] Restoring directory %s \n", (unsigned long)thread, &tempdir[strlen(dir)]);
          }
          //doesnt exist
          else{

            mkdir(tempcopydir,S_IRWXU);
            pthread_mutex_lock(&locksubdirstats);
            total_subdirectories++;
            pthread_mutex_unlock(&locksubdirstats);
            struct dir_struct* tempstruct = (struct dir_struct *)malloc(sizeof(struct dir_struct));
            strcpy(tempstruct->current_dir,tempdir);
            strcpy(tempstruct->copy_dir,tempcopydir);
            structs[size2] = tempstruct;
            size2++;
            pthread_t thread;
            int rc = pthread_create( &thread, NULL, restore_folder, (void *)tempstruct);
            if ( rc != 0 )
            {
              // pthreads functions do NOT use errno or perror()
              fprintf( stderr, "pthread_create() failed (%d): %s",
                       rc, strerror( rc ) );
              return 0;
            }
            threads[size] = thread;
            size++;
            printf("[thread %lu] Restoring directory %s \n", (unsigned long)thread, &tempdir[strlen(dir)]);
            printf("[thread %lu] created directory %s \n", (unsigned long)thread, &tempdir[strlen(dir)]);


          }



        }
      }
      //if object is normal file with permission
      else {
        if ( S_ISREG( buf.st_mode ) )
        {

          if ( buf.st_mode & ( S_IRUSR | S_IRGRP | S_IROTH ) )
          {

            //make sure file you are backing up does not end in .bak or is the executable or source code
            if (len > 3){
              const char *last_four = &tempdir[len-4];
              if(!strcmp(".bak",last_four) && ((len < 12) || strcmp(&tempdir[len-12],"mybackup.bak")) && ((len < 12) || strcmp(&tempdir[len-14],"mybackup.c.bak"))){
                tempcopydir[len-4] = '\0';
                printf("%s \n", tempcopydir);
                if (stat(tempcopydir,&copybuf ) == 0){
                  printf("imagine I removed the file here \n");
                  //remove(tempcopydir);
                }
                struct dir_struct* tempstruct = (struct dir_struct *)malloc(sizeof(struct dir_struct));
                strcpy(tempstruct->current_dir,tempdir);
                strcpy(tempstruct->copy_dir,tempcopydir);
                structs[size2] = tempstruct;
                size2++;
                pthread_t thread;
                printf("[thread %lu] Restoring %s ...\n", (unsigned long)thread, &tempdir[strlen(dir) + 1]);
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


              }
              else {
                printf("I didn't restore anything \n tempdir %s \n tempcopydir %s", tempdir, tempcopydir);
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
          pthread_mutex_lock(&locksubdirstats);
          total_subdirectories++;
          pthread_mutex_unlock(&locksubdirstats);
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

        if ( buf.st_mode & ( S_IRUSR | S_IRGRP | S_IROTH ) )
        {

          //make sure file you are backing up does not end in .bak
          //its almost always > 3 because len also includes the directories before it
          if (len > 3){
            const char *last_four = &tempdir[len-4];
            if(strcmp(".bak",last_four)){
              int already_exists = 0;
              strcat( tempcopydir, ".bak");
              if (stat(tempcopydir,&copybuf ) == 0){
                if (copybuf.st_ctime > buf.st_ctime){
                  printf("[thread %lu] Backup copy of %s is already up-to-date\n", (unsigned long)pthread_self(), &tempdir[strlen(dir) + 1]);
                  continue;
                }
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
              printf("[thread %lu] Backing up %s ...\n", (unsigned long)thread, &tempdir[strlen(dir)+1]);
              if (already_exists){
                printf("[thread %lu] WARNING: %s exists (overwriting!) \n", (unsigned long)thread, &tempcopydir[strlen(dir)+1]);
              }

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
  int restore = 0;
  if (argc > 1 && strcmp(argv[1],"-r")){
    restore = 1;
  }
  pthread_t thread;
  if (pthread_mutex_init(&lockfilestats, NULL) != 0)
{
    printf("\n mutex init failed\n");
    return 1;
}
  if (pthread_mutex_init(&locksubdirstats, NULL) != 0)
  {
    printf("\n mutex init failed\n");
    return 1;
  }
  //doesn't matter if .mybackup already exists. mkdir will return -1 if it doesnt
  mkdir(".mybackup",S_IRWXU);
  //struct dir_struct dirs;
  struct dir_struct* dirs = (struct dir_struct *)malloc(sizeof(struct dir_struct));
  int rc = 1;
  if (restore == 0){
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
}
else {
  struct stat buf;
  getcwd(dirs->copy_dir,1000);
  getcwd(dirs->current_dir,1000);

  strcat(dirs->current_dir, "/.mybackup");
  //check to make sure .mybackup exists
  if(stat(dirs->current_dir,&buf ) == 0 ){
    rc = pthread_create( &thread, NULL, restore_folder, (void *)dirs);
    if ( rc != 0 )
    {
      /* pthreads functions do NOT use errno or perror() */
      fprintf( stderr, "pthread_create() failed (%d): %s",
               rc, strerror( rc ) );
      return EXIT_FAILURE;
    }
  }
  else {
    printf("error .mybackup doesn't exist. Why are you restoring if you have'nt backed up yet? \n");
    return 1;
  }
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
    pthread_mutex_destroy(&lockfilestats);
    pthread_mutex_destroy(&locksubdirstats);
    if (restore == 0){
      if (total_subdirectories != 1){
        printf("successfully backed up %d files (%d bytes) and %d subdirectories \n", total_files, total_bytes, total_subdirectories);
      }
      else {
        printf("successfully backed up %d files(%d bytes) and %d subdirectory \n", total_files, total_bytes, total_subdirectories);
      }
    }
    else {
      if (total_subdirectories != 1){
        printf("successfully restored %d files (%d bytes) and %d subdirectories \n", total_files, total_bytes, total_subdirectories);
      }
      else {
        printf("successfully restored %d files(%d bytes) and %d subdirectory \n", total_files, total_bytes, total_subdirectories);
      }
    }
    //print "successfully backed up total files (total bytes) and totalsubdirectories subdirectory"

  return 1;
}
