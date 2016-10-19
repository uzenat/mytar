#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "hdr/tools.h"
#include "hdr/archive.h"

#define BUFF_SIZE 1024

mode_t getumask(void)
{
    mode_t mask = umask(0);
    umask(mask);
    return mask;
}


/* Archivage d'un fichier regulier */

int archiveREG(int archive, char * path, int flagv){

  /* declaration des variable */
  struct stat st;
  int input, lus;
  char buff[BUFF_SIZE];
  header hd;
  char chsm[32];

  /* initialisation du stat pour path, et ouverture de path en lecture */
  if(stat(path, &st)==-1 || (input=open(path,O_RDONLY))==-1){
    fprintf(stderr,"erreur: %s\n", path);
    return -1;
  }

  if(flagv) {
    getChecksum(path, chsm);

    /* recupere les informations d'entete */
    set_header(&hd, strlen(path), st.st_size, st.st_mode, st.st_mtime, st.st_atime, chsm);
  }
  else set_header(&hd, strlen(path), st.st_size, st.st_mode, st.st_mtime, st.st_atime, NULL);

  /* ecriture de l'entete */
  write(archive, &hd, sizeof(header));

  /* ecriture du nom du fichier */
  write(archive, path, hd.path_length);

  /* ecriture de chaque caractere dans le fichier d'archive */
  while( (lus=read(input,buff, BUFF_SIZE))>0 )
    write(archive, buff, lus);

  /* fermeture du fichier en lecture */
  close(input);

  return 0;
}


/* Archivage d'un fichier regulier lorsque -C est presente */

int archiveREGC(int archive, char *path, char *repC, int flagv){

  /* declaration des variables */
  struct stat st;
  int input, lus;
  char buff[BUFF_SIZE];
  header hd;
  char path2[256];
  char chsm[32];
 
  /* test l'ouverture de path */
  if(stat(path, &st)==-1 || (input=open(path,O_RDONLY))==-1){
    fprintf(stderr,"erreur: %s\n", path);
    return -1;
  }

  if(flagv) {
    getChecksum(path, chsm);

    /* recupere les informations d'entete */
    set_header(&hd, strlen(path)+strlen(repC)+1, st.st_size, st.st_mode, st.st_mtime, st.st_atime, chsm);
  }
  else set_header(&hd, strlen(path)+strlen(repC)+1, st.st_size, st.st_mode, st.st_mtime, st.st_atime, NULL);

  strcpy(path2, repC);
  strcat(path2, "/");
  strcat(path2, path);
  
  /* ecrit l'entete suivie du nom du fichier*/
  write(archive, &hd, sizeof(header));
  write(archive, path2, hd.path_length);

  /* ecrit le contenu du fichier */
  while( (lus=read(input,buff, BUFF_SIZE))>0 )
    write(archive, buff, lus);

  /* ferme le fichier */
  close(input);
  


  return 0;
}

/* Archive un repertoire */

int archiveDIR(int archive, char *path, int flagv){

  /* declaration des variables */
  struct stat st;
  struct dirent *sd;
  DIR *dir;
  int input;
  char path2[256];
  header hd;

  /* test si l'ouverture du fichier path se passe bien */
  if(stat(path, &st)==-1 || (input=open(path,O_RDONLY))==-1){
    fprintf(stderr, "erreur: %s\n", path);
    return -1;
  }


  /* initialisation du header */
  set_header(&hd, strlen(path), 0, st.st_mode, st.st_mtime, st.st_atime, NULL);

  /* ecriture du header et du nom du fichier */
  write(archive, &hd, sizeof(header));
  write(archive, path, hd.path_length);

  /* test de l'ouverture du repertoire */
  if( (dir=opendir(path))==NULL ){
    fprintf(stderr,"ouverture de %s impossible\n", path);
    return -1;
  }

  /* parcours les fichier du repertoire */
  while( (sd=readdir(dir))!=NULL ){

    /* ignore les fichier commencant par . */
    if(sd->d_name[0]=='.') continue;

    /* mise a jour du nom du fichier */
    strcpy(path2, path);
    strcat(path2, "/");
    strcat(path2, sd->d_name);

    /* recupere les donne stat du fichier */
    if(stat(path2, &st)==-1){
      fprintf(stderr,"erreur stat %s\n", path2);
      continue;
    }

    /* si cest un repertoire alors appel recursif sur ce repertoire */
    if(S_ISDIR(st.st_mode)) archiveDIR(archive, path2, flagv);

    /* sinon on archive le fichier regulier */
    else archiveREG(archive, path2, flagv);

    
  }
  
  return 0;

}

/* Archivage d'un repertoire avec l'option -C */

int archiveDIRC(int archive, char *path, char *repC, int flagv){

  /* declaration des variables */
  struct stat st;
  struct dirent *sd;
  DIR *dir;
  int input;
  char path3[256];
  header hd;

  /* test si l'ouverture à bien fonctionné */
  if(stat(path, &st)==-1 || (input=open(path,O_RDONLY))==-1){
    fprintf(stderr, "erreur: %s\n", path);
    return -1;
  }

  strcpy(path3, repC);
  strcat(path3, "/");
  strcat(path3, path);

  /* initialise le header */
  set_header(&hd, strlen(path3), 0, st.st_mode, st.st_mtime, st.st_atime, NULL);

  /* ecrit l'entete + le nom de du repertoire */
  write(archive, &hd, sizeof(header));
  write(archive, path3, hd.path_length);
  
  /* ouvre le repertoire */
  if( (dir=opendir(path))==NULL ){
    fprintf(stderr,"ouverture de %s impossible\n", path);
    return -1;
  }

  /* parcours les sous fichier du repertoire */
  while( (sd=readdir(dir))!=NULL ){


    if(sd->d_name[0]=='.') continue;
    
    char path2[256];
    strcpy(path2, path);
    strcat(path2, "/");
    strcat(path2, sd->d_name);
    
    
    if(stat(path2, &st)==-1){
      fprintf(stderr,"erreur stat %s\n", path2);
      continue;
    }


    /* si c'est un repertoire */
    if(S_ISDIR(st.st_mode)) archiveDIRC(archive, path2, repC, flagv);
    
    /* sinon */
    else archiveREGC(archive, path2, repC, flagv);
    
 
    
  }
  
  return 0;

}

/* Fonction principale de l'archivage */

int archive(char *archive, char **path, int lenPath, int flagv){

  /* il doit forcement avoir des fichiers path */
  if(path==NULL) return -1;

  int output, i;
  struct stat st;
  struct flock verrou;

  verrou.l_type = F_WRLCK;
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  /* ouverture de l'archive */
  if( (output=open(archive, O_WRONLY | O_CREAT | O_TRUNC, 0660))==-1 ){
    fprintf(stderr,"erreur: impossible de creer %s\n", archive);
    return -1;
  }

  while(fcntl(output, F_SETLK, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(1);
  }

  /* boucle pour parcourir tous les fichiers path pour les traité */
  for(i=0;i<lenPath;i++){

    /* recupere la structure stat */
    if(stat(path[i], &st)==-1){
      fprintf(stderr,"erreur stat %s\n", path[i]);
      continue;
    }

    /* si c'est un repertoire */
    if(S_ISDIR(st.st_mode)) {
      suppr_slash(path[i]);
      archiveDIR(output, path[i], flagv);
    }

    /* sinon */
    else archiveREG(output, path[i], flagv);

  }

  printf("archivage reussi\n");
  close(output);

  return 0;
}


/* Fonction principale de l'archivage avec l'option -C */

int archiveC(char *archive, char **path, int lenPath, char *repC, int flagv){
  if(path==NULL || repC==NULL) return -1;

  int output, i, mdir;
  struct stat st;
  header hd;
  struct flock verrou;

  verrou.l_type = F_WRLCK;
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  /* ouverture du fichier d'archive */
  if( (output=open(archive, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR|S_IWUSR))==-1 ){
    fprintf(stderr,"erreur: impossible de creer %s\n", archive);
    return -1;
  }

  while(fcntl(output, F_SETLK, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }


  /* creation du repertoire en question */
  mdir= mkdir(repC, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);

  /* recupere les donnee stat dun repertoire */
  stat(repC, &st);

  /* si le repertoire n'existait pas, alors on le supprime */
  if(mdir!=-1) rmdir(repC);
   
  /* initialise le header */
  set_header(&hd, strlen(repC), 0, st.st_mode, st.st_mtime, st.st_atime, NULL);

  /* ecrit le header + le nom du fichier */
  write(output, &hd, sizeof(header));
  write(output, repC, hd.path_length);

  /* parcours les fichier de path */
  for(i=0;i<lenPath;i++){

    if(stat(path[i], &st)==-1){
      fprintf(stderr,"erreur stat %s\n", path[i]);
      continue;
    }

    suppr_slash(repC);
    if(S_ISDIR(st.st_mode)){ 
      suppr_slash(path[i]);
      archiveDIRC(output, path[i], repC, flagv);
    }
    else archiveREGC(output, path[i], repC, flagv);

  }

  printf("archivage reussi\n");
  close(output);

  return 0;
}

/* Methode pour ajouter une liste de fichier à une archive deja existante */

int addArchive(char *archive, char **path, int lenPath, int flagv, int flagz){

  if(path==NULL) return -1;

  int output, i;
  struct stat st;
  struct flock verrou;

  verrou.l_type = F_WRLCK;
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  if(flagz){

    /* decompression de l'archive dans /tmp */
    decompression(archive);

    /* supression de l'extansion .gz dans la référence */
    archive[strlen(archive)-3]= '\0';

  } 

  /* Ouverture de l'archive en mode ajout -> O_APPEND */
  if( (output=open(archive, O_WRONLY | O_APPEND))==-1 ){
    fprintf(stderr,"erreur: %s n'existe pas\n", archive);
    return -1;
  }

  while(fcntl(output, F_SETLK, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }

  for(i=0;i<lenPath;i++){
    if(stat(path[i], &st)==-1){
      fprintf(stderr,"erreur stat %s\n", path[i]);
      continue;
    }

    if(S_ISDIR(st.st_mode)){ 
      suppr_slash(path[i]);
      archiveDIR(output, path[i], flagv);
    }
    else archiveREG(output, path[i], flagv);

  }

  if(flagz) compression(archive);

  printf("ajout reussi\n");
  close(output);

  return 0;
}

/* Ajoute des fichiers a une archive deja existante avec l'option -C */

int addArchiveC(char *archive, char **path, int lenPath, char *repC, int flagv, int flagz){

  if(path==NULL || repC==NULL) return -1;

  int output, i, mdir;
  struct stat st;
  header hd;
  struct flock verrou;

  verrou.l_type = F_WRLCK;
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  if(flagz){

    /* decompression de l'archive dans /tmp */
    decompression(archive);

    /* supression de l'extansion .gz dans la référence */
    archive[strlen(archive)-3]= '\0';

  } 



  if( (output=open(archive, O_WRONLY | O_APPEND))==-1 ){
    fprintf(stderr,"erreur: impossible de creer %s\n", archive);
    return -1;
  }

  while(fcntl(output, F_SETLK, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }

  mdir= mkdir(repC, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IXOTH);

  stat(repC, &st);
  
  if(mdir!=-1) rmdir(repC);
  
  set_header(&hd, strlen(repC), 0, st.st_mode, st.st_mtime, st.st_atime, NULL);


  write(output, &hd, sizeof(header));
  write(output, repC, hd.path_length);


  for(i=0;i<lenPath;i++){
    if(stat(path[i], &st)==-1){
      fprintf(stderr,"erreur stat %s\n", path[i]);
      continue;
    }
    
    suppr_slash(repC);
    if(S_ISDIR(st.st_mode)){ 
      suppr_slash(path[i]);
      archiveDIRC(output, path[i], repC, flagv);
    }
    else archiveREGC(output, path[i], repC, flagv);

  }
  
  if(flagz) compression(archive);

  printf("ajout reussi\n");
  close(output);
  
  return 0;
}
