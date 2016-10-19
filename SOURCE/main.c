#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "hdr/tools.h"
#include "hdr/archive.h"
#include "hdr/extract.h"
#include "hdr/list.h"
#include "hdr/delete.h"


/* PROGRAMME MAIN */

int main(int argc, char **argv){


  /* test si le nombre d'argument est strictement inf. à 3: erreur */
  if(argc < 3) {
    fprintf(stderr,"usage: %s: option archive.mtr [path1] ... [pathn]\n", argv[0]);
    return 1;
  }

  /* declaration des variables */
  int flagf, flagc, flagx, flaga, flagl, flagd, flagk, flagC, flagv, flagz, lenPath;
  char *arch;
  char **path=NULL;
  char *repC=NULL;

  /* test si les options sont valide */
  if(init_flag(argv, argc, &flagf, &flagc, &flagx, &flaga, &flagd, &flagl, &flagk, &flagC, &flagv, &flagz, &lenPath)==-1) return 1;

  /* recupere les l'archive, la liste path et le repertoire de -C */
  arch= get_archive(argv, argc);
  if(lenPath>0) path= get_path(argv, argc);
  if(flagC) repC= get_repC(argv, argc);



  /* Si l'option -c est presente */
  if(flagc){

    if(flagC) archiveC(arch, path, lenPath, repC, flagv); 
    else archive(arch, path, lenPath, flagv);
    if(flagz) {printf("compression %s\n", arch); compression(arch); }

  }

  else if(flagx){

    if(flagC){
      
      if(lenPath!=0) printf("extraction de %i fichier de %s dans %s\n", lenPath, arch, repC);
      else extractC(arch,repC, flagv, flagz, flagk);
    }
    else {
      if(lenPath!=0) ;
      else extract(arch, flagv, flagz, flagk);
    }

  }

  else if(flagl){

    list(arch);

  }

  else if(flaga){

    if(flagC) addArchiveC(arch, path, lenPath, repC, flagv, flagz);
    else {
      addArchive(arch, path, lenPath, flagv, flagz);
    }
  }

  else if(flagd){
    
    delete(arch, path, lenPath, flagz);

  }
  else if(flagz){
    printf("compression\n");
  }

  else fprintf(stderr,"usage: %s: option archive.mtr [path1] ... [pathn]\n", argv[0]);

  free(arch);
  if(path!=NULL) free(path);
  if(repC!=NULL) free(repC);

  return 0;

}
