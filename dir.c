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
	while(f_info != NULL){
		do_dir(task, f_info->name, task->maxdepth, 0, 1);
		f_info = f_info->next;
	}
	return 1;
}

int print_lstat(struct stat *attribut, char *fname){
const char *rwx = "rwxrwxrwx";
char l_rwx[10], linkbuf[PATH_MAX];
int i;

	int bits[]= {
	 S_IRUSR,S_IWUSR,S_IXUSR,// Zugriffsrechte User
	 S_IRGRP,S_IWGRP,S_IXGRP,// Zugriffsrechte Gruppe
	 S_IROTH,S_IWOTH,S_IXOTH // Zugriffsrechte der Rest
	};
	l_rwx[9] = '\0';
	if((lstat(fname, attribut)) == -1) {
		printf("Fehler bei stat (%s)\n", fname);
		return 0;
	}
	// Dateiart erfragen
	if( S_ISREG(attribut->st_mode) )
		printf("Reguläre Datei           : ");
	else if( S_ISDIR(attribut->st_mode) )
		printf("Verzeichnis              : ");
	else if( S_ISCHR(attribut->st_mode) )
		printf("zeichenorient. Gerätedatei : ");
	else if( S_ISBLK(attribut->st_mode) )
		printf("blockorient. Gerätedatei : ");
	else if( S_ISFIFO(attribut->st_mode) )
		printf("FIFO oder named Pipe     : ");
	else if( S_ISLNK(attribut->st_mode) )
		printf("Symbollink               : ");
	else
		printf("Unbekannte Datei         : ");
	// Dateinamen ausgeben
	printf("%-40s [", fname);
	// Einfache Zugriffsrechte erfragen
	l_rwx[0]='\0';
	for(i=0; i<9; i++) { // Wenn nicht 0, dann gesetzt
		l_rwx[i]=(attribut->st_mode & bits[i]) ? rwx[i] : '-';
	}
	l_rwx[9]='\0';
	printf("%s]",l_rwx);
	if( S_ISLNK(attribut->st_mode) ) {
		readlink(fname, linkbuf, PATH_MAX);
		linkbuf[attribut->st_size] = '\0';
		printf(" %s", linkbuf);
	}
	puts(" ");
	return 1;
}
/**
 * @brief list a directory
 * 
 */
int do_dir(struct myfind *task, char *dir_name, int maxdepth, int depth, short flag) {
	DIR *dir;
	char fname[PATH_MAX];
	struct dirent *dirzeiger;
	struct stat attribut;

	depth++;						// increase position in dir hierarchy
	if(flag){
		if(!print_lstat(&attribut, dir_name)) return 0;
	}

	// Arbeitsverzeichnis öffnen
	if((dir=opendir(dir_name)) == NULL) {
		printf("Fehler bei opendir\n");
	return 0;
	}
	// Das komplette Verzeichnis auslesen
	while((dirzeiger=readdir(dir)) != NULL) {
		if(strcmp("..", dirzeiger->d_name) && strcmp(".", dirzeiger->d_name) &&(doesitmatch(task, dirzeiger->d_name, MYFIND_NAME))) {
			memset(&fname[0], 0, PATH_MAX);
			strcpy(&fname[0],dir_name);
			strcat(&fname[0], "/");
			strcat(&fname[0], dirzeiger->d_name);

			if(!print_lstat(&attribut, &fname[0])) return 0;

			/*if((lstat(&fname[0], &attribut)) == -1) {
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
			puts(" ");*/
			if(S_ISDIR(attribut.st_mode)) {
				if((depth <= maxdepth) | !maxdepth) {
					do_dir(task, &fname[0], maxdepth, depth, 0);
				}
			}
		}
	}
	closedir(dir);
	return 1;
}
