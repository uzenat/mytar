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


void pparse(char *ref, char **ref2){
  char path[256];
  strcpy(path, ref);
  int i,j=0, n = strlen(ref);
  ref2[j++]=ref;
  for(i=1;i<n;i++){
    if(ref[i-1]=='/') ref[i-1]='\0';
    if(ref[i-1]=='\0') ref2[j++]= ref+i;
  }
  ref2[j]=NULL;
}

/* test si ref1 contient ref2 */
int contient(char *ref1, char *ref2){
  int i=0,j;
  char ref11[256];
  char ref22[256];
  char *path1[256];
  char *path2[256];
  strcpy(ref11, ref1);
  strcpy(ref22, ref2);
  pparse(ref11, path1);
  pparse(ref22, path2);
  while( !(path1[i]==NULL || strcmp(path1[i],path2[0])==0)){i++;}
  for(j=0; path2[j]!=NULL; j++,i++){
    if(path1[i]==NULL) return 0;
    if(strcmp(path1[i], path2[j])!=0) return 0;
  }
  return 1;
}

/* test si ref1 contient ref2 */
int contients(char *ref1, char **ref2, int lenPath){
  int i = 0;
  for(i=0; i<lenPath; i++)
    if(contient(ref1, ref2[i])) return 1;
  return 0;
}


/* fonction qui supprime des élément de l'archive */

int delete(char *archive, char **path, int lenPath, int flagz){

  int j, input, output, lus;
  char archiveTMP[256];
  char buff[BUFF_SIZE];
  header hd;
  char *filename;
  struct flock verrou;
  
  verrou.l_whence = SEEK_SET;
  verrou.l_start = 0;
  verrou.l_len = 0;

  if(flagz){

    /* decompression de l'archive dans /tmp */
    decompression(archive);

    /* supression de l'extansion .gz dans la référence */
    archive[strlen(archive)-3]= '\0';

  } 
  
  if( (input=open(archive, O_RDONLY))==-1 ){
    fprintf(stderr, "ouverture de l'archive impossible\n");
    return -1;
  }

  verrou.l_type = F_RDLCK;
  while(fcntl(input, F_SETLKW, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }

  strcpy(archiveTMP, "/tmp/");
  strcat(archiveTMP, archive);

  if( (output=open(archiveTMP, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR))==-1 ){
    fprintf(stderr, "ouverture de l'archive impossible\n");
    return -1;
  }

  verrou.l_type = F_WRLCK;
  while(fcntl(output, F_SETLKW, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }
  
  
  /* lecture de l'archive */
  while( (lus=read(input, &hd, sizeof(header)))>0 ){

    filename= calloc(hd.path_length+1, sizeof(char));

    /* recupere les donnees d'entete */
    if(read(input, filename, hd.path_length)!= (ssize_t)(hd.path_length)){
      fprintf(stderr,"erreur\n");
      free(filename);
      return 1;
    }
    if(contients(filename, path, lenPath))
      lseek(input, hd.file_length, SEEK_CUR);
    else {
      write(output, &hd, sizeof(header));
      write(output, filename, hd.path_length);
      printf("--> %s\n", filename);
      
      /* ecriture de chaque caractere dans le fichier d'archive */
      for(j=0;j< (hd.file_length/BUFF_SIZE);j++){
	lus= read(input, buff, BUFF_SIZE);
	write(output, buff, lus);
      }
      lus= read(input, buff, hd.file_length%BUFF_SIZE);
      write(output,buff, lus);

    }

    free(filename);

  }
  
  verrou.l_type = F_UNLCK;
  /* fcntl(input, F_SETLKW, &verrou); */
  /* fcntl(output, F_SETLKW, &verrou); */
  close(input);
  close(output);

  /* ouverture du fichier créer en lecture */
  if( (input=open(archiveTMP, O_RDONLY))==-1 ){
    fprintf(stderr,"erreur ouverture en lecture\n");
    return -1;
  }

  verrou.l_type = F_RDLCK;
  while(fcntl(input, F_SETLKW, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }

  /* ouverture de l'archive en ecriture */
  if( (output=open(archive, O_TRUNC | O_WRONLY))==-1 ){
    fprintf(stderr, "erreur ouverture en ecriture\n");
    return -1;
  }

  verrou.l_type = F_WRLCK;
  while(fcntl(output, F_SETLKW, &verrou)==-1){
    printf("Un processus utilise déja ce fichier, veuillez attendre...\n");
    sleep(2);
  }

  while( (lus=read(input, buff, BUFF_SIZE))>0 )
    write(output, buff, lus);

  close(input);
  close(output);

  if(flagz) compression(archive);

  unlink(archiveTMP);
  
  return 0;
}
