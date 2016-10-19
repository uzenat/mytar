#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "hdr/tools.h"



void set_header(header *hd, size_t pl, off_t fl, mode_t m, time_t mt, time_t at,char chsm[32]){ 
  hd->path_length=pl;
  hd->file_length=fl;
  hd->mode=m;   
  hd->m_time=mt;
  hd->a_time=at;
  int i;
  if(chsm!=NULL){
    for(i=0;i<32;i++)
      hd->checksum[i]=chsm[i];
    hd->checksum[32]='\0';
  }
}

void get_header(header *hd, int desc){

  if(read(desc, hd, sizeof(header))==-1)
    fprintf(stderr,"erreur: lecture du header impossible\n");

}


int isChrIn(const char *s, char c){
  int i, k=strlen(s);
  for(i=0;i<k; i++)
    if(s[i]==c) return 1;
  return 0;
}

void getChecksum(const char *filename, char chsm[33]){

  int tube[2]; 
  
  if( pipe(tube)==-1 )
    { perror("pipe"); return; }
  
  switch (fork()){
  case -1:
    perror("fork");
    return;
  case 0:
    dup2(tube[1],STDOUT_FILENO);
    execlp("md5sum", "md5sum", filename, NULL);
    return;
  default:
    wait(NULL);
    read(tube[0], chsm, 32);
    chsm[32]='\0';
    break;
  }

}


char *concat(const char *s1, const char *s2){
  char *s= calloc(strlen(s1)+strlen(s2)+1, sizeof(char));
  int i, j;
  int n1= strlen(s1), n2= strlen(s2);
  for(i= 0; i<n1; i++)
    s[i]= s1[i];
  for(j= 0; j<n2; j++)
    s[i++]= s2[j];
  return s;
}

char *copy(const char *s){
  if(s==NULL) return NULL;
  int i, k=strlen(s);
  char *cpy= calloc(k+1, sizeof(char));
  for(i=0;i<k+1; i++)
    cpy[i]=s[i];
  return cpy;
}

void copy2(const char *src, char *dest){
  if(src==NULL) dest=NULL;
  int i, k=strlen(src);
  for(i=0;i<k+1; i++)
    dest[i]=src[i];
  dest[k+2]='\0';
}

int isPrefix(const char *pre, const char *s){
  int i;
  int n1=strlen(pre);
  int n2= strlen(s);
  if(n1>n2) return 0;
  for(i=0;i<n1;i++)
    if(pre[i]!=s[i]) return 0;
  return 1;
}

int split(char *s, char **ss){
  
  int i, j=1;
  int n= strlen(s);
  
  ss[0]=s;
  for(i=1;i<n;i++){
    if(s[i]=='/') s[i]='\0';
    if(s[i-1]=='\0') ss[j++]=s+i;
  }

  return 0;

}

int isMTR(const char *s){
  int k= strlen(s);
  if(k<=4) return 0;
  if(s[k-1]!='r') return 0;
  if(s[k-2]!='t') return 0;
  if(s[k-3]!='m') return 0;
  if(s[k-4]!='.') return 0;
  return 1;
}

int isMTRGZ(const char *s){
  int k= strlen(s);
  if(k<=7) return 0;
  if(s[k-1]!='z') return 0;
  if(s[k-2]!='g') return 0;
  if(s[k-3]!='.') return 0;
  if(s[k-4]!='r') return 0;
  if(s[k-5]!='t') return 0;
  if(s[k-6]!='m') return 0;
  if(s[k-7]!='.') return 0;
  return 1;
}

void suppr_slash(char *s){
  if(s[strlen(s)-1]=='/') s[strlen(s)-1]='\0';
}

char *get_repC(char **arg, int n){
  char *repC;
  int i=0;
  while(i<n && (strcmp(arg[i],"-C")!=0))
    i++;
  if(i>=n-1) return NULL;
  repC= copy(arg[i+1]);
  return repC;
}

char *get_archive(char **arg, int n){
  char *archive;
  int i=0;
  while(i<n && !(arg[i][0]=='-' && (isChrIn(arg[i],'f'))) )
    i++;
  if(i>=n-1) return NULL;
  archive= copy(arg[i+1]);
  return archive;
}

int get_pathLen(char **arg, int n){
  
  int i=0, k, cpt=0;
  while(i<n && (strcmp(arg[i],"-f")!=0)) i++;
  if(i==n-1) return 0; 
  i += 2;
  if(i>=n) return 0;
  if(arg[i][0]=='-') return 0;
  for(k=i;k<n && arg[k][0]!='-'; k++)
    cpt++;
  return cpt;
  
}

char **get_path(char **arg, int n){
  char **path;
  int i=0, j=0, k, cpt=0;
  while( i<n && !(arg[i][0]=='-' && (isChrIn(arg[i],'f'))) )
    i++;
  if(i==n-1) return NULL;
  i+=2;
  if(i>=n) return NULL;
  if(arg[i][0]=='-') return NULL;
  for(k=i;k<n && arg[k][0]!='-'; k++)
    cpt++;
  path=calloc(cpt+1, sizeof(char *));
  for(k=i;k<n && arg[k][0]!='-';k++)
    path[j++]= copy(arg[k]);
  return path;
}

int cpyDirTmp(const char *filename){

  pid_t pid;

  switch( pid= fork() ){
  case -1:
    perror("fork");
    return 1; 
  case 0:
    execlp("cp", "cp", filename, "/tmp", NULL);
    return 0;
  default:
    wait(NULL);
    break;
  }
  
  return 0;

}


int compression(char *filename){

  pid_t pid;

  switch( pid= fork() ){
  case -1:
    perror("fork");
    return 1; 
  case 0:
    execlp("gzip", "gzip", filename, NULL);
    return 0;
  default:
    wait(NULL);
    break;
  }
  
  return 0;

}

int decompression(char *filename){

  pid_t pid;

  switch( pid= fork() ){
  case -1:
    perror("fork");
    return 1; 
  case 0:
    execlp("gunzip", "gunzip", filename, NULL);
    return 0;
  default:
    wait(NULL);
    break;
  }
  
  return 0;

}

int init_flag(char **argv, int argc, int *flagf, int *flagc, int *flagx, int *flaga, int *flagd, int *flagl, int *flagk, int *flagC, int *flagv, int *flagz, int *lenPath){
  
  int i, ii, iii, j, cpt=0;
  *flagf=0;*flagc=0;*flagx=0;*flaga=0;*flagl=0;*flagd=0;*flagk=0;*flagC=0;*flagv=0;*flagz=0;*lenPath=0;

  int flagextmtr=0;
  int flagextgz=0;
  
  for(i=0;i<argc;i++){
    if(argv[i][0]=='-'){

      /* test pour l'option -f */
      if(isChrIn(argv[i], 'f')){
	ii= i+1;

	/* test si il y a bien au moins 1 parametre apres -f */
	if(ii>=argc || argv[ii][0]=='-'){
	  fprintf(stderr,"erreur: aucun argument apres l'option -f\n");
	  return -1;
	}

	/* test si ce parametre est d'extansion .mtr */
	if(!( (flagextmtr=isMTR(argv[ii])) || (flagextgz=isMTRGZ(argv[ii])) ) )
	  {fprintf(stderr,"erreur: %s: n'est pas d'extansion .mtr ou .mtr.gz\n", argv[ii]);return -1;}

	/* determine le nombre des path */
	for(j=ii+1;j<argc && (argv[j][0]!='-');j++)
	  cpt++;
	*lenPath= cpt;

	/* si tout est bon, alors on peut initialiser flagf à 1 */
	*flagf=1;
	
      }

      /* test pour l'option -c */
      if(isChrIn(argv[i], 'c')){
	if(*flagx){ 
	  fprintf(stderr,"erreur: -c ne peut pas etre utilise avec -x\n");
	  return -1;
	}
	if(*flagl){ 
	  fprintf(stderr,"erreur: -c ne peut pas etre utilise avec -l\n");
	  return -1;
	}
	if(*flaga){ 
	  fprintf(stderr,"erreur: -c ne peut pas etre utilise avec -a\n");
	  return -1;
	}
	if(*flagd){ 
	  fprintf(stderr,"erreur: -c ne peut pas etre utilise avec -d\n");
	  return -1;
	}
	if(*flagk){ 
	  fprintf(stderr,"erreur: -c ne peut pas etre utilise avec -k\n");
	  return -1;
	}
	*flagc=1;
      }

      /* test pour l'option -x */
      if(isChrIn(argv[i], 'x')){
	if(*flagc){ 
	  fprintf(stderr,"erreur: -x ne peut pas etre utilise avec -c\n");
	  return -1;
	}
	if(*flagl){ 
	  fprintf(stderr,"erreur: -x ne peut pas etre utilise avec -l\n");
	  return -1;
	}
	if(*flaga){ 
	  fprintf(stderr,"erreur: -x ne peut pas etre utilise avec -a\n");
	  return -1;
	}
	if(*flagd){ 
	  fprintf(stderr,"erreur: -x ne peut pas etre utilise avec -d\n");
	  return -1;
	}
	*flagx=1;
      }

      /* test pour l'option -l */
      if(isChrIn(argv[i], 'l')){
	if(*flagx){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -x\n");
	  return -1;
	}
	if(*flagc){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -c\n");
	  return -1;
	}
	if(*flaga){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -a\n");
	  return -1;
	}
	if(*flagd){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -d\n");
	  return -1;
	}
	if(*flagk){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -k\n");
	  return -1;
	}
	if(*flagC){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -C\n");
	  return -1;
	}
	if(*flagv){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -v\n");
	  return -1;
	}
	if(*flagz){ 
	  fprintf(stderr,"erreur: -l ne peut pas etre utilise avec -z\n");
	  return -1;
	}
	*flagl=1;
      }
      
      /* test pour l'option -a */
      if(isChrIn(argv[i], 'a')){
	if(*flagx){ 
	  fprintf(stderr,"erreur: -a ne peut pas etre utilise avec -x\n");
	  return -1;
	}
	if(*flagl){ 
	  fprintf(stderr,"erreur: -a ne peut pas etre utilise avec -l\n");
	  return -1;
	}
	if(*flagc){ 
	  fprintf(stderr,"erreur: -a ne peut pas etre utilise avec -c\n");
	  return -1;
	}
	if(*flagd){ 
	  fprintf(stderr,"erreur: -a ne peut pas etre utilise avec -d\n");
	  return -1;
	}
	if(*flagk){ 
	  fprintf(stderr,"erreur: -a ne peut pas etre utilise avec -k\n");
	  return -1;
	}
	*flaga=1;
      }

      /* test pour l'option -d */
      if(isChrIn(argv[i], 'd')){
	if(*flagx){ 
	  fprintf(stderr,"erreur: -d ne peut pas etre utilise avec -x\n");
	  return -1;
	}
	if(*flagl){ 
	  fprintf(stderr,"erreur: -d ne peut pas etre utilise avec -l\n");
	  return -1;
	}
	if(*flagc){ 
	  fprintf(stderr,"erreur: -d ne peut pas etre utilise avec -c\n");
	  return -1;
	}
	if(*flaga){ 
	  fprintf(stderr,"erreur: -d ne peut pas etre utilise avec -a\n");
	  return -1;
	}
	if(*flagk){ 
	  fprintf(stderr,"erreur: -d ne peut pas etre utilise avec -k\n");
	  return -1;
	}
	if(*flagC){ 
	  fprintf(stderr,"erreur: -d ne peut pas etre utilise avec -C\n");
	  return -1;
	}
	if(*flagv){ 
	  fprintf(stderr,"erreur: -d ne peut pas etre utilise avec -v\n");
	  return -1;
	}
	*flagd=1;
      }

      /* test pour l'option -k */
      if(isChrIn(argv[i], 'k')){
	if(*flagl){ 
	  fprintf(stderr,"erreur: -k ne peut pas etre utilise avec -l\n");
	  return -1;
	}
	if(*flagc){ 
	  fprintf(stderr,"erreur: -k ne peut pas etre utilise avec -c\n");
	  return -1;
	}
	if(*flaga){ 
	  fprintf(stderr,"erreur: -k ne peut pas etre utilise avec -a\n");
	  return -1;
	}
	if(*flagd){ 
	  fprintf(stderr,"erreur: -k ne peut pas etre utilise avec -d\n");
	  return -1;
	}
	*flagk=1;
      }

      /* test pour l'option -C */
      if(strcmp(argv[i], "-C")==0){
	if(*flagl){ 
	  fprintf(stderr,"erreur: -C ne peut pas etre utilise avec -l\n");
	  return -1;
	}
	if(*flagd){ 
	  fprintf(stderr,"erreur: -C ne peut pas etre utilise avec -d\n");
	  return -1;
	}
	iii= i+1;
	if(iii>=argc || argv[iii][0]=='-'){
	  fprintf(stderr,"erreur: aucun argument apres l'option -C\n");
	  return -1;
	}
	*flagC=1;
      }
      
       /* test pour l'option -v */
      if(isChrIn(argv[i], 'v')){
	if(*flagl){ 
	  fprintf(stderr,"erreur: -v ne peut pas etre utilise avec -l\n");
	  return -1;
	}
	if(*flagd){ 
	  fprintf(stderr,"erreur: -v ne peut pas etre utilise avec -d\n");
	  return -1;
	}
	*flagv=1;
      }
       if(isChrIn(argv[i], 'z')){
	if(*flagl){ 
	  fprintf(stderr,"erreur: -z ne peut pas etre utilise avec -l\n");
	  return -1;
	}

	*flagz=1;
      }

    }

  }

    
  
  
  if(!*flagf) {
    fprintf(stderr,"erreur: l'option -f n'est pas presente\n");
    return -1;
  }
  

  if(*flagk && !flagx){
    fprintf(stderr, "erreur: l'option -k ne peut pas etre utilise sans -x\n");
    return -1;
  }

  if(*flagc && (*lenPath==0)){
    fprintf(stderr,"erreur: aucun fichier à archiver\n");
    return -1;
  }
  if(flagextgz && !*flagl){
    if(flagextgz && !*flagz) {
      fprintf(stderr,"erreur: l'option -z n'est pas spécifié\n");
      return -1;
    }
  }

  if(flagextmtr && *flagz){
    fprintf(stderr,"erreur: avec l'option -z vous devez specifié une archive d'extansion .mtr.gr\n");
    return -1;
  }

  if(flagextgz && *flagc) argv[ii][strlen(argv[ii])-3]= '\0'; 


  return 0;
}
