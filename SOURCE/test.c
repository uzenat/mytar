#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv){

  if(argc != 2){ fprintf(stderr, "nombre d'argument invalide\n"); return 1; }

  pid_t pid;

  switch( pid= fork() ){
  case -1:
    perror("fork");
    return 1; 
  case 0:
    execlp("gzip", "gzip", argv[1], NULL);
    return 0;
  default:
    wait(NULL);
    break;
  }
  
  return 0;

}
