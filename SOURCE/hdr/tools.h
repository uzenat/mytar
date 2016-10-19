

/***********************************/
/************* HEADER **************/
/***********************************/

struct header {
  size_t path_length; 
  off_t file_length;
  mode_t mode;
  time_t m_time;
  time_t a_time;
  char checksum[33];
};
typedef struct header header;

/* initialise un header */
void set_header(header *hd,size_t pl,off_t fl,mode_t m,time_t mt,time_t at,char chsm[32]);

/* recupere le header dans un fichier (on supposera que la tete de lecture
   sera placé au bonne endroit */
void get_header(header *hg, int desc);



/***********************************/
/************* TOOLS ***************/
/***********************************/


void getChecksum(const char *filename, char chsm[32]);


/* teste si une chaine dispose bien de l'extansion .mtr */
int isMTR(const char *s);
int isMTRGZ(const char *s);


/* initialise les flags, si la fonction renvoi 0 cela veut dire que les flags sont correct, sinon qu'il y a eu une mauvaise déclaration des flags */
int init_flag(char **argv, int argc, int *flagf, int *flagc, int *flagx, int *flaga, int *flagd, int *flagl, int *flagk, int *flagC, int *flagv, int *flagz, int *lenPath);

/* retourne les path1 ... pathn */
char **get_path(char **arg, int n);
int get_pathLen(char **arg, int n);

/* retourne la ref vers l'archive */
char *get_archive(char **arg, int n);

/* retourne la ref du repertoire dnné par le flag -C */
char *get_repC(char **arg, int n);

/* supprime le slash s'il existe */
void suppr_slash(char *s);


int cpyDirTmp(const char *filename);
int compression(char *filename);
int decompression(char *filename);
