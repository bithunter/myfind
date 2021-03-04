/**
 * @file
 * @brief Linux File-Search
 * @author Andreas Bauer, IC20B005
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include "defs.h"


/**
 * @fn int main(int, char*[])
 * @brief Find procedure for files, folders and symbol links
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {

	int end_of_link_opt = 0; 		// First arg after any -H/-L etc.
	int end_of_filenames = 0;
	int temp, default_dir = 1;		// suppose we use standard dir,unless user gives a new path or file (set default_dir 0)
	char const *working_dir;		// here we start our search
	char const *start_dir = ".";

	DIR *dir;

	struct dirent *dirzeiger;
	static struct myfind tasktodo;
	tasktodo.fileinfo = NULL;


#if _DEBUG
	puts("***** DEBUG - MODE ON ***************************\n");
	printf("\nWorking directory: %s\n",getcwd(tasktodo.path, PATH_MAX));
	printf("Filename: %s\n\n",argv[0]);
#endif
	end_of_link_opt = find_end_of_link_opt(&tasktodo, argc, argv);												// get index of first possible filename
	if((end_of_filenames = parse_arguments(&tasktodo, argc, argv, end_of_link_opt)) == 0) return EXIT_FAILURE;	// get arg after list of filenames
	if(end_of_link_opt != end_of_filenames) default_dir = 0;													// is 0 when user entered at least one filename
#if _DEBUG
	printf("Position after filenames: %d\n",end_of_filenames);
	printf("end of leading options = %d\n\n", end_of_link_opt);
#endif
	if(default_dir) {
		working_dir = start_dir;
	} else {
		working_dir = argv[end_of_link_opt];
	}
	temp = get_filenames(&tasktodo, default_dir, argc, argv, end_of_link_opt, end_of_filenames);
	if(temp == 0) {
		puts("myfind: out of memory\n");
		return EXIT_FAILURE;
	} else if(temp == -1){
				return EXIT_FAILURE;
	}
#if _DEBUG
	puts("*************************************************\n");
#endif
	if((dir = opendir(working_dir)) != NULL) {
			while((dirzeiger=readdir(dir)) != NULL)
				printf("%s\n", dirzeiger->d_name);
	} else {
		printf("Fehler beim Öffnen von %s\n",working_dir);
		return EXIT_FAILURE;
	}
	if(closedir(dir) == -1) {
		printf("Fehler beim Schließen von %s\n",working_dir);
		return EXIT_FAILURE;
	}
	//do_dir(working_dir);

	printf("**************************************************\n");





	  const char *rwx = "rwxrwxrwx";
	  int bits[]= {
	     S_IRUSR,S_IWUSR,S_IXUSR,/*Zugriffsrechte User*/
	     S_IRGRP,S_IWGRP,S_IXGRP,/*Zugriffsrechte Gruppe*/
	     S_IROTH,S_IWOTH,S_IXOTH /*Zugriffsrechte der Rest*/
	  };
	  /* Arbeitsverzeichnis öffnen */
	  if((dir=opendir(working_dir)) == NULL) {
	    printf("Fehler bei opendir\n");
	    return (EXIT_FAILURE);
	  }
	  /* Das komplette Verzeichnis auslesen */
	  while((dirzeiger=readdir(dir)) != NULL) {
	    struct stat attribut;
	    int i;
	    char l_rwx[10];
	    l_rwx[9] = '\0';
	    if((lstat(dirzeiger->d_name, &attribut)) == -1) {
	      printf("Fehler bei stat\n");
	      //return (EXIT_FAILURE);
	    }
	    /* Dateiart erfragen */
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
	    else
	      printf("Unbekannte Datei          : ");
	    /* Dateinamen ausgeben */
	    printf("%-20s [",dirzeiger->d_name);
	    /* Einfache Zugriffsrechte erfragen */
	    l_rwx[0]='\0';
	    for(i=0; i<9; i++) { /*Wenn nicht 0, dann gesetzt*/
	      l_rwx[i]=(attribut.st_mode & bits[i]) ? rwx[i] : '-';
	    }
	    l_rwx[9]='\0';
	    printf("%s]\n",l_rwx);
	  }
	  closedir(dir);







	freeMemory(&tasktodo);
	return EXIT_SUCCESS;
}
