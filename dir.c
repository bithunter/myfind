/**
 * @file
 * @brief File and directory explorer
 * @author Andreas Bauer, IC20B005
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <glob.h>
#include <limits.h>
#include <unistd.h>
#include "defs.h"

/**
 * @brief
 *
 */
int do_entry(struct myfind *task){
	struct fileinfo *f_info = task->fileinfo;
	return do_dir(task, f_info->name, task->maxdepth, 0);
}
/**
 * @brief list a directory
 * 
 */
int do_dir(struct myfind *task, char *dir_name, int maxdepth, int depth) {
	DIR *dir;
	char fname[PATH_MAX], linkbuf[PATH_MAX];
	struct dirent *dirzeiger;
	const char *rwx = "rwxrwxrwx";
	struct stat attribut;
	int i;
	char l_rwx[10];

	depth++;						// increase position in dir hierarchy
	int bits[]= {
	 S_IRUSR,S_IWUSR,S_IXUSR,// Zugriffsrechte User
	 S_IRGRP,S_IWGRP,S_IXGRP,// Zugriffsrechte Gruppe
	 S_IROTH,S_IWOTH,S_IXOTH // Zugriffsrechte der Rest
	};
	// Arbeitsverzeichnis öffnen
	if((dir=opendir(dir_name)) == NULL) {
	printf("Fehler bei opendir\n");
	return 0;
	}
	// Das komplette Verzeichnis auslesen
	while((dirzeiger=readdir(dir)) != NULL) {
		if(strcmp("..", dirzeiger->d_name) && strcmp(".", dirzeiger->d_name) &&(doesitmatch(task, dirzeiger->d_name, MYFIND_NAME))) {
			l_rwx[9] = '\0';
			memset(&fname[0], 0, PATH_MAX);
			strcpy(&fname[0],dir_name);
			strcat(&fname[0], "/");
			strcat(&fname[0], dirzeiger->d_name);

			if((lstat(&fname[0], &attribut)) == -1) {
			  printf("Fehler bei stat (%s)\n", &fname[0]);
			  //return (EXIT_FAILURE);
			}
			// Dateiart erfragen
			if( S_ISREG(attribut.st_mode) )
			  printf("Reguläre Datei           : ");
			else if( S_ISDIR(attribut.st_mode) )
			  printf("Verzeichnis              : ");
			else if( S_ISCHR(attribut.st_mode) )
			  printf("zeichenorient. Gerätedatei : ");
			else if( S_ISBLK(attribut.st_mode) )
			  printf("blockorient. Gerätedatei : ");
			else if( S_ISFIFO(attribut.st_mode) )
			  printf("FIFO oder named Pipe     : ");
			else if( S_ISLNK(attribut.st_mode) )
			  printf("Symbollink               : ");
			else
			  printf("Unbekannte Datei         : ");
			// Dateinamen ausgeben
			printf("%-40s [", &fname[0]);
			// Einfache Zugriffsrechte erfragen
			l_rwx[0]='\0';
			for(i=0; i<9; i++) { // Wenn nicht 0, dann gesetzt
			  l_rwx[i]=(attribut.st_mode & bits[i]) ? rwx[i] : '-';
			}
			l_rwx[9]='\0';
			printf("%s]",l_rwx);
			if( S_ISLNK(attribut.st_mode) ) {
				readlink(fname, linkbuf, PATH_MAX);
				linkbuf[attribut.st_size] = '\0';
				printf(" %s", linkbuf);
			}
			puts(" ");
			if(S_ISDIR(attribut.st_mode)) {
				if((depth <= maxdepth) | !maxdepth) {
					do_dir(task, &fname[0], maxdepth, depth);
				}
			}
		}
	}
	closedir(dir);
	return 1;
}
