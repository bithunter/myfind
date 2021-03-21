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
#include <pwd.h>
#include <grp.h>
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

int print_lstat(struct myfind *task, struct stat *attribut, char *fname){
const char *rwx = "rwxrwxrwx";
char l_rwx[11], linkbuf[PATH_MAX];
int i;
struct passwd *pw;
struct group *grp;

	int bits[]= {
	 S_IRUSR,S_IWUSR,S_IXUSR,// Zugriffsrechte User
	 S_IRGRP,S_IWGRP,S_IXGRP,// Zugriffsrechte Gruppe
	 S_IROTH,S_IWOTH,S_IXOTH // Zugriffsrechte der Rest
	};
	l_rwx[0] = '-';
	l_rwx[10] = '\0';
	if((lstat(fname, attribut)) == -1) {
		printf("Fehler bei stat (%s)\n", fname);
		return 0;
	}
	if(task->predicate & MYFIND_LS){						// option "-ls" for output?
		pw = getpwuid(attribut->st_uid);
		grp = getgrgid(attribut->st_gid);
		if(S_ISDIR(attribut->st_mode))l_rwx[0] = 'd';

		// Einfache Zugriffsrechte erfragen
		l_rwx[1]='\0';
		for(i=0; i<9; i++) { // Wenn nicht 0, dann gesetzt
			l_rwx[i+1]=(attribut->st_mode & bits[i]) ? rwx[i] : '-';
		}
		l_rwx[10]='\0';
		printf("%9lu%7lu%11s%4lu %10s %10s %-40s", attribut->st_ino, attribut->st_blocks/2, l_rwx, attribut->st_nlink, pw->pw_name, grp->gr_name, fname);
	} else {
		printf("%-40s ", fname);
	}
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

	// open directory
	if((dir=opendir(dir_name)) == NULL) {
		printf("myfind: ‘%s’: Permission denied\n",dir_name);
	return 0;
	}
	// Das komplette Verzeichnis auslesen
	while((dirzeiger=readdir(dir)) != NULL) {
		if(strcmp("..", dirzeiger->d_name) && strcmp(".", dirzeiger->d_name) &&(doesitmatch(task, dirzeiger->d_name, MYFIND_NAME))) {
			if(flag){
				if(!print_lstat(task, &attribut, dir_name)) return 0;			// output info for startdir at the first call
				flag = 0;
			}
			memset(&fname[0], 0, PATH_MAX);
			strcpy(&fname[0],dir_name);
			strcat(&fname[0], "/");
			strcat(&fname[0], dirzeiger->d_name);

			if(!print_lstat(task, &attribut, &fname[0])) return 0;

			if(S_ISDIR(attribut.st_mode)) {
				if(depth < maxdepth || maxdepth == 0) {
					do_dir(task, &fname[0], maxdepth, depth, 0);
				}
			}
		}
	}
	closedir(dir);
	return 1;
}
