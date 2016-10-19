# MYTAR 

Compilation
==

make mytar


Execution
==

./mytar  [options]  -f archive.mtr[.gz] [path1] [path2] … [pathn] [-C rep]


Nettoyer
==

make clean 
make cleanall

Options
==

-f archive.mtr [path1] … [pathn]:
fournit le chemin vers l'archive et vers une liste de fichier [pathi] qui pourra resté vide.
Cette option est obligatoire.

-c:
option pour archiver les fichiers de la liste [pathi], celle ci ne doit pas être vide.
Cette option ne peut pas être utilisé avec: -x, -a, -d, -l

-x:
Si [pathi] est vide, alors extrait le totalité de l'archive, sinon extrait uniquement les [pathi] et reconstruit les arborescence si besoin est
Cette option ne peut pas être utilisé avec: -c, -a, -d, -l

-a:
Ajoute les [pathi] à l'archive, celle ci ne doit pas être vide.
Cette option ne peut pas être utilisé avec: -c, -x, -d, -l

-d:
Supprime les [pathi] de l'archive, celle ci ne doit pas être vide.
Cette option ne peut pas être utilisé avec: -c, -x, -a, -l

-l:
Liste le contenue de l'archive, en affichant les droit, la date de derniere modification, et la reference de larchive. Si [pathi] est vide, liste l'integralité de l'archive, sinon liste uniquement les [pathi]
Cette option ne peut pas être utilisé avec: -c, -x, -a, -d

-k:
Lors de l'extraction des fichiers, indique que les fichier existant ne doivent pas etre remplacé. Un message d'avertissement doit etre affiché, et l'extraction continue.
Cette option doit etre utilisé uniquement avec -x

-C rep:
rep devient la racine de l'arborescence archivé. Utilsé en mode création (archivage: -c) cela signifie que les chemins path1 ... pathn, ainsi que les chemins écrits dans l'archive, sont relatifs à rep ; en mode extraction, cela signifie que l'archive doit être restaurée dans rep. 
Cette option s'utilise uniquement avec -c et -x.

-z:
S'utilise avec l'archivage ou l'extraction. Elle permet de compressé l'ensemble des fichier apres leur archivage.