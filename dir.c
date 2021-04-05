/**
 * @file
 * @brief File and directory explorer
 * @author Andreas Bauer, IC20B005
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>
#include <dirent.h>
#include <glob.h>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <regex.h>
#include <sys/types.h>
#include "defs.h"

/**
 * int do_entry(struct myfind *task)
 * @brief call all directories, given by the filenames in the myfind-structure
 *
 * @param task	myfind-structure and argument information
 * @return		1 if operation is successful, 0 otherwise
 */
int do_entry(struct myfind *task){
	struct fileinfo *f_info = task->fileinfo;				// get first struct of entered file-/dirnames
	while(f_info != NULL){
		if(!do_dir(task, f_info->name, 0, 1)) return 0;		// display file- dir-info
		f_info = f_info->next;								// get next name
	}
	return 1;
}
/**
 * @fn int doesitmatch(struct myfind *task, struct dirent *dr, struct stat *attribut)
 * @brief check if file fits to the given specifications
 *
 * @param task		myfind-structure and argument information
 * @param dr		system-dirent struct of the file-information
 * @param attribut	stat-structure with the fileinformation from the stat() function
 * @return			1 if successfully, 0 if file does not meet the requirements
 */
int doesitmatch(struct myfind *task, struct dirent *dr, struct stat *attribut){
	struct mypredicate *mypred = task->mypred;		// pointer to the list of entered arguments ("-name, -type...")
	struct passwd *pw = getpwuid(attribut->st_uid);	// for the name of the owner of the file
	char *name = dr->d_name;						// filename to check
	char *username = task->user;
	char *arg[] ={"d","f","l"}, *tt;				// allowed filetypes
	int type = dr->d_type, match = 0, rv;
	regex_t exp;									//the compiled expression
	if(task->name) if(fnmatch(task->name, name, FNM_NOESCAPE | FNM_PERIOD)) return 0;	// if name doesn't match, don't show the info
	if(username){						
		rv = regcomp(&exp, "^[0-9]+$", REG_EXTENDED);	// only numbers allowed
		if (rv != 0) {
			puts("myfind: internal error (regex)");		// this error should not happen but just in case...
			return 0;
		}
		if(reg_match(&exp, username)){
			if(atoi(username) != attribut->st_uid){		// if user entered a valid number, compare the user-id
				regfree(&exp);
				return 0;
			}
		} else {
			if(fnmatch(username, pw->pw_name, FNM_NOESCAPE | FNM_PERIOD)){	// compare user-names
				regfree(&exp);
				return 0;	// check username
			}
		}
		regfree(&exp);
	}
	if(task->mtime){												// -mtime - defined?
		struct stat attr;
		int d_days, d_mode = ((task->mtime)[0] == '+' ? 1 : 0);		// check for mode if + or - prefix
		d_mode = ((task->mtime)[0] == '-' ? 2 : d_mode);			// +x -> mode = 1, -x -> mode = 2, x -> mode = 0 (x is the timerange for mtime)
		d_days = abs(atoi(task->mtime));							// number of days
		memcpy(&attr, attribut, sizeof(struct stat));				// local copy for the 'localtime' function
		struct tm *tt = localtime(&attr.st_mtime);
		double zt = difftime(time(NULL), mktime(tt))/86400;			// timedifference in days of the last-changed-time of file to present time
		switch(d_mode) {
			case 2:
				if(zt<=d_days) match = 1;		// for -mtime -x  -> match if file not older than: present time - x-days
				break;
			case 1:
				if(zt>=(d_days+1)) match = 1;	// for -mtime +x  -> match if file older than: present time - x+1-days
				break;
			case 0:
				if((zt>=d_days) && (zt<=(d_days+1))) match = 1;	// for -mtime x  -> match if file-make between: present time - x-days + (pt +x +1);
				break;
		}
		if(!match) return 0;					// file does nit match requirements
	}
	if(task->type){								// check in case of a -type argument
		char myType[strlen(task->type) + 1];	// memory for the list of types (like f,d,l...)
		strcpy(myType, task->type);
		tt = strtok(myType, ",");				// types are seperated by ','
		while(tt != NULL){
			switch (type)						// success if file-typ match with the types entered by the user
			{
			case DT_REG:
				if(strcmp(tt,arg[1]) == 0) return 1;
				break;
			case DT_DIR:
				if(strcmp(tt,arg[0]) == 0) return 1;
				break;
			case DT_LNK:
				if(strcmp(tt,arg[2]) == 0) return 1;
				break;
			default:
				return 0;
				break;
			}
			tt = strtok(NULL, ",");
		}
		return 0;								// no match, don't display fileinfo
	}

	return 1;						// passed all test successfully
}
/**
 * @fn int print_lstat(struct myfind *task, struct stat *attribut, char *fname)
 * @brief 
 *
 * @param task		myfind-structure and argument information
 * @param attribut	stat-structure with the fileinformation from the stat() function
 * @param fname		name of the file to display
 * @return			1 if successfull
 */
int print_lstat(struct myfind *task, struct stat *attribut, char *fname){
	const char *rwx = "rwxrwxrwx";
	struct stat attr;
	char l_rwx[11], linkbuf[PATH_MAX], makeTime[15];
	int i;
	struct tm *tt;
	struct passwd *pw;
	struct group *grp;

	int bits[]= {
	 S_IRUSR,S_IWUSR,S_IXUSR,		// user permissions
	 S_IRGRP,S_IWGRP,S_IXGRP,		// group permissions
	 S_IROTH,S_IWOTH,S_IXOTH 		// permissions for all others
	};
	l_rwx[0] = '-';
	l_rwx[10] = '\0';

	memcpy(&attr, attribut, sizeof(struct stat));

	if(task->predicate & MYFIND_LS){						// option "-ls" for output?
		pw = getpwuid(attribut->st_uid);
		grp = getgrgid(attribut->st_gid);
		if(S_ISDIR(attribut->st_mode))l_rwx[0] = 'd';		// add a 'd' if we deal with a directory

		// get permissions
		l_rwx[1]='\0';
		for(i=0; i<9; i++) {
			l_rwx[i+1]=(attribut->st_mode & bits[i]) ? rwx[i] : '-';	// set, if not 0
		}
		l_rwx[10]='\0';
		tt = localtime(&attr.st_mtime);
		strftime(makeTime, 15, "%b %d %H:%M" ,tt);			// time of the file last modification
	
		printf("%9lu%7lu%11s%4lu%9s%9s%9lu%13s %s", attribut->st_ino, (attribut->st_blocks)/2, l_rwx, attribut->st_nlink, pw->pw_name, grp->gr_name, attribut->st_size, makeTime, fname);
	} else {
		printf("%-40s ", fname);							// without '-ls' just display the filename
	}
	if( S_ISLNK(attribut->st_mode) ) {				// additional information for a symbol-link (link to original file)
		readlink(fname, linkbuf, PATH_MAX);
		linkbuf[attribut->st_size] = '\0';
		printf(" -> %s", linkbuf);
	}
	puts(" ");
	return 1;
}
/**
 * @fn int do_dir(struct myfind *task, char *dir_name, int depth, short flag)
 * @brief main prcedure to display a single file or a directory (incl. content), files have to match the requirements
 *
 * @param task		myfind-structure and argument information
 * @param dir_name	name of file or directory
 * @param depth		value of -maxdepth showes how deep we go into a directory
 * @param flag		indicates if the starting directory should be diplayed or not (avoids a directory to be displayed twice when func is called recursively)
 * @return			1 if successful, 0 if a error occured
 */
int do_dir(struct myfind *task, char *dir_name, int depth, short flag) {
	DIR *dir;
	char fname[PATH_MAX];
	int maxdepth = task->maxdepth, ls, tt, mstat;
	struct dirent *dirzeiger;
	struct stat attribut;
	struct mypredicate *pred = task->mypred;
	depth++;										// increase level in dir-hierarchy

	if(task->linkoption == 'L') mstat = stat(dir_name, &attribut); else mstat = lstat(dir_name, &attribut);
	if(mstat == -1) {
		printf("myfind: Error asking filestat for (%s)\n", fname);
		return 0;
	}
	if(S_ISREG(attribut.st_mode) || S_ISLNK(attribut.st_mode)){			// check, if filename belongs to a regular file, link or to a directory
		struct dirent dr;
		strcpy(dr.d_name, dir_name);
		dr.d_type = DT_REG;												// local dirent-struct with information for func 'doesitmatch'

		ls = (task->ls > 0 ? task->ls : 1);
		tt = doesitmatch(task, &dr, &attribut);
		if(tt == 1) {
			while(ls > 0){
				if(!print_lstat(task, &attribut, dir_name)) return 0;	// display fileinformation for a single file (or link)
				ls--;
			}
		}
	} else {										// open directory and display fileinformation (called recursiv up to defined depth by '-maxdepth')
		if((dir=opendir(dir_name)) == NULL) {
			printf("myfind: ‘%s’: Permission denied\n",dir_name);
		return 0;
		}
		// read the directory
		while((dirzeiger=readdir(dir)) != NULL) {
			if(flag){
				if((stat(dir_name, &attribut)) == -1) {
					printf("myfind: Error asking filestat for (%s)\n", fname);
					closedir(dir);
					return 0;
				}
				struct dirent dr;
				strcpy(dr.d_name, dir_name);
				dr.d_type = DT_DIR;												// local dirent-struct with information for func 'doesitmatch'
				tt = doesitmatch(task, &dr, &attribut);
				if(tt == 1) {
					if(!print_lstat(task, &attribut, dir_name)) return 0;		// output info for startdir at the first call is parameter matches
				}
				flag = 0;
			}
			if(strcmp("..", dirzeiger->d_name) != 0 && strcmp(".", dirzeiger->d_name) !=0 ) {	// don't display '.' and '..' content of a dir
				memset(&fname[0], 0, PATH_MAX);
				strcpy(&fname[0],dir_name);
				strcat(&fname[0], "/");
				strcat(&fname[0], dirzeiger->d_name);
				if(task->linkoption == 'L') mstat = stat(fname, &attribut); else mstat = lstat(fname, &attribut);	// '-L' gives the file-info instead of the link-info
				if(mstat == -1) {
					printf("myfind: Error asking filestat for (%s)\n", fname);
					closedir(dir);
					return 0;
				}
				ls = (task->ls > 0 ? task->ls : 1);				// get number of '-ls'
				tt = doesitmatch(task, dirzeiger, &attribut);
				if(tt == 1) {
					while(ls > 0){
						if(!print_lstat(task, &attribut, &fname[0])) { closedir(dir); return 0; }	// display fileinfo ls-times
						ls--;
					}
				}
				if(S_ISDIR(attribut.st_mode)) {					// is this a directory?
					if(depth < maxdepth || maxdepth == 0) {		// if still within -maxdepth ramge, call the function recursively again
						do_dir(task, &fname[0], depth, 0);
					}
				}
			}
		}
		closedir(dir);
	}
	return 1;
}
