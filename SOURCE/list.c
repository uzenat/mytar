#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "hdr/tools.h"

/** Option -l du programe : liste les fichiers d'une archive */


/* initialise dans smode les droits du fichier exactement comme ls -l */
void string_mode(mode_t mode, char smode[11]){
  smode[0]= (S_ISDIR(mode))?'d':'-';
  smode[0]= (S_ISLNK(mode))?'-':smode[0];
  smode[1]= (mode & S_IRUSR)?'r':'-';
  smode[2]= (mode & S_IWUSR)?'w':'-';
  smode[3]= (mode & S_IXUSR)?'x':'-';
  smode[4]= (mode & S_IRGRP)?'r':'-';
  smode[5]= (mode & S_IWGRP)?'w':'-';
  smode[6]= (mode & S_IXGRP)?'x':'-';
  smode[7]= (mode & S_IROTH)?'r':'-';
  smode[8]= (mode & S_IWOTH)?'w':'-';
  smode[9]= (mode & S_IXOTH)?'x':'-';
  smode[10]= '\0';
}

/* fonction principale, elle prend en argument une reference d'une archive */
int list(char *archive){

  int cpt=0;
  struct flock verrou;
  int input, lus;
  char filename[256];
  char archive2[256];
  char smode[11];
  header hd;
  int flag=0;

  if(isMTRGZ(archive)){
    cpyDirTmp(archive);
    strcpy(archive2,"/tmp/");
    strcat(archive2,archive);
    decompression(archive2);
    archive2[strlen(archive2)-3]= '\0';
    flag=1;
  }
  else strcpy(archive2, archive);


  if( (input=open(archive2, O_RDONLY))==-1 ){perror("open");return -1;}

  verrou.l_type = F_RDLCK;
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  while(fcntl(input, F_SETLKW, &verrou)==-1){
    printf("Un processus utitilise deja %s, patienter...\n", archive);
    sleep(2);
    cpt++;
    if(cpt>10) {perror("fork"); return -1;}
  }

  /* boucle qui lit le contenu de l'archive */
  while( (lus=read(input, &hd, sizeof(header)))>0 ){

    
    /* lit le nom du fichier */
    if(read(input, filename, hd.path_length)!= (ssize_t)(hd.path_length))
      {perror("read");lseek(input, hd.path_length, SEEK_CUR);printf("%s\n", filename);continue;}


    /* recupere la chaine de caractere des droit du fichier */
    string_mode(hd.mode, smode);

    /* ecrit sur la sortie standard les droit */
    write(STDOUT_FILENO, smode, strlen(smode));

    /* recupere la date */
    char *date= ctime(&hd.m_time);
    date[strlen(date)-1]= 0;

    /* affiche la date sur la sortie standard */
    write(STDOUT_FILENO," ",1);
    write(STDOUT_FILENO,date, strlen(date));
    write(STDOUT_FILENO," ",1);
    
    /* affiche le nom du fichier sur la sortie standad */
    filename[hd.path_length]='\0';
    write(STDOUT_FILENO,filename, strlen(filename));
    write(STDOUT_FILENO,"\n", 1);

    lseek(input, hd.file_length, SEEK_CUR);
  }
  
  verrou.l_type = F_UNLCK;
  fcntl(input, F_SETLKW, &verrou);

  close(input);
  if(flag) unlink(archive2);

  
    
  return 0;
}


