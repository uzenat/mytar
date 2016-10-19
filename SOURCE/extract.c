#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include "hdr/tools.h"

#define BUFF_SIZE 1024

/* Methode pour extraire une archive */

int extract(char *archive, int flagv, int flagz, int flagk){

  /* declaration des variable */
  struct stat st;
  int input, output, lus, j;
  char buff[BUFF_SIZE];
  char filename[256];
  char archive2[256];
  header hd;
  char md5[33];
  struct flock verrou;

  verrou.l_type = F_RDLCK;
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  /* Si le flagz est activé */
  if(flagz){
    
    /* copie de l'archive compresser dans le repertoire /tmp */
    cpyDirTmp(archive);

    /* mise a joure de la variable archive 2 */
    strcpy(archive2,"/tmp/");
    strcat(archive2,archive);

    /* decompression de l'archive dans /tmp */
    decompression(archive2);

    /* supression de l'extansion .gz dans la référence */
    archive2[strlen(archive2)-3]= '\0';

  } 

  /* si le flag n'est pas activé, on garde la meme nom */
  else strcpy(archive2, archive);

  /* ouverture de l'archive en lecture */
  if( (input=open(archive2, O_RDONLY))==-1 ){
    fprintf(stderr,"erreur: %s: ouverture impossible\n", archive);
    return -1;
  }

  while(fcntl(input, F_SETLKW, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }

  /* lecture de l'archive */
  while( (lus=read(input, &hd, sizeof(header)))>0 ){

    
    /* recupere les donnees d'entete */
    if(read(input, filename, hd.path_length)!= (ssize_t)(hd.path_length)){
      fprintf(stderr,"erreur\n");
      return 1;
    }

    filename[hd.path_length]= '\0';

    printf("--> %s ", filename);
    
    /* si c'est un repertoire, on le creer puis on passe 
       a l'iteration suivante */
    if(S_ISDIR(hd.mode)){
      mkdir(filename, hd.mode);
      printf("\n");
      continue;
    }

    if(stat(filename, &st)==0){
      if(!flagk) {
	unlink(filename);
	/* creation du fichier */
	if( (output=open(filename, O_WRONLY | O_CREAT | O_TRUNC, hd.mode))==-1 ){
	  fprintf(stderr,"impossible de creer %s\n", filename);
	  lseek(input, hd.file_length, SEEK_CUR);
	  continue;
	}
      }
      else { 
	printf(" : le fichier existe deja"); 
	lseek(input, hd.file_length, SEEK_CUR);
	if(flagv) { 
	  getChecksum(filename, md5);
	  if(strcmp(md5, hd.checksum)==0) printf(" *emprunte md5 valide*\n");
	  else printf(" *emprunte md5 non valide*\n");
	} else printf("\n");
	continue;
      }
      
    }

    /* creation du fichier */
    else if( (output=open(filename, O_WRONLY | O_CREAT | O_TRUNC, hd.mode))==-1 ){
      fprintf(stderr,"impossible de creer %s\n", filename);
      lseek(input, hd.file_length, SEEK_CUR);
      continue;
    }

    

    /* parcourt le fichier et recupere les données pour les réecrire dans
       le nouveau fichier creer */
    for(j=0;j< (hd.file_length/BUFF_SIZE);j++){
      lus= read(input, buff, BUFF_SIZE);
      write(output, buff, lus);
    }
    lus= read(input, buff, hd.file_length%BUFF_SIZE);
    write(output,buff, lus);
    close(output);
    
    if(flagv) { 
      getChecksum(filename, md5);
      if(strcmp(md5, hd.checksum)==0) printf(" *emprunte md5 valide*\n");
      else printf(" *emprunte md5 non valide*\n");
    } else printf("\n");

  }

  close(input);

  return 0;
}

/* Extraction dans un repertoire donné par l'argument -C */

int extractC(char *archive, char *repC, int flagv, int flagz, int flagk){

  int input, output, lus, j;
  char buff[BUFF_SIZE];
  char filename[256];
  char archive2[256];
  char path[256];
  char md5[33];
  header hd;
  int flagRepC=0;
  struct flock verrou;

  verrou.l_type = F_RDLCK;
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  /* Si le flagz est activé */
  if(flagz){
    
    /* copie de l'archive compresser dans le repertoire /tmp */
    cpyDirTmp(archive);

    /* mise a joure de la variable archive 2 */
    strcpy(archive2,"/tmp/");
    strcat(archive2,archive);

    /* decompression de l'archive dans /tmp */
    decompression(archive2);

    /* supression de l'extansion .gz dans la référence */
    archive2[strlen(archive2)-3]= '\0';

  } 

  /* si le flag n'est pas activé, on garde la meme nom */
  else strcpy(archive2, archive);

  /* ouverture en lecture de l'archive */
  if( (input=open(archive, O_RDONLY))==-1 ){
    fprintf(stderr,"erreur: %s: ouverture impossible\n", archive);
    return -1;
  }

  while(fcntl(input, F_SETLKW, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }

  /* creation du repertoire donnée par -C */
  if(mkdir(repC, S_IRUSR | S_IWUSR | S_IXUSR)==-1){
   
    flagRepC=1;
    

  }
  
  strcpy(path, repC);

  /* lecture de l'archive */
  while( (lus=read(input, &hd, sizeof(header)))>0 ){
    
    /* lecture du champ path */
    if(read(input, filename, hd.path_length)!= (ssize_t)(hd.path_length)){
      fprintf(stderr,"erreur\n");     
      return 1;
    }

    /* marquage du nom du fichier dans filename */
    filename[hd.path_length]='\0';


    if(flagRepC) strcpy(path, filename);
    else {
      strcpy(path, repC);
      strcat(path, "/");
      strcat(path, filename);
    }

    printf("--> %s ", path);

    
    if(S_ISDIR(hd.mode)){
      mkdir(path, hd.mode | S_IWUSR);
      printf("\n");
      continue;
    }

   
    if( (output=open(path, O_WRONLY | O_CREAT | O_TRUNC, hd.mode))==-1 ){
      fprintf(stderr,"impossible de creer %s\n", path);
      lseek(input, hd.file_length, SEEK_CUR);
      continue;

    }
    for(j=0;j< (hd.file_length/BUFF_SIZE);j++){
      lus= read(input, buff, BUFF_SIZE);
      write(output, buff, lus);
    }
    lus= read(input, buff, hd.file_length%BUFF_SIZE);
    write(output,buff, lus);
    close(output);

    /* test de l'emprunte md5 si l'argument est renseigné */
    if(flagv) { 

      /* recupere le checksum */
      getChecksum(filename, md5);
      
      /* compare les deux chaines contenant l'emprunte */
      if(strcmp(md5, hd.checksum)==0) printf(" *emprunte md5 valide*\n");
      else printf(" *emprunte md5 non valide*\n");
      
    } else printf("\n");

  }

  close(input);

  return 0;
}
