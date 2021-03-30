/**
 * @file
 * @def DEFS_H_
 * @brief necessary definitions to make the hole thing work
 *
 */
//#pragma once
#ifndef	_DEFS_H
#define	_DEFS_H	1
#define _DEBUG 1
#include <sys/stat.h>
#include <limits.h>

#define MYFIND_USER 1
#define MYFIND_NAME 1 << 1
#define MYFIND_TYPE 1 << 2
#define MYFIND_PRINT 1 << 3
#define MYFIND_LS 1 << 4
#define MYFIND_MAXDEPTH 1 << 5
#define MYFIND_HELP 1 << 6
#define MYFIND_ISFILE 1 << 7		// already filename-input before -name ?

/**
 * @struct myfind
 * @brief holds the result of the parser, link-options, all filenames and the valid predicates for filename-actions
 *
 */
struct myfind {
	char linkoption;					// may be either L, H or P; last one overrides the others
	int predicate;						// options (Bit 0 = user, 1 = name, 2 = type, 3 = print, 4 = ls)
	struct fileinfo *fileinfo;			// names of files, directory or link
	struct mypredicate *mypred;			// arguments to describe the file and search-mode (after path)
	int maxdepth;						// how deep do we search the directory-tree?
	short ls;
	char *name;
	char *type;
	char *user;
	char path[PATH_MAX];				// path to working directory
};
/**
 * @struct options
 * @brief List of the known predicates
 *
 */
struct options {
	char *optname;	/*!< name of a valid option */
	int opt_mode;	/*!< indicator of option */
	int mode;		// 1 = additional argument must follow
};

/**
 * @struct filename
 * @brief detail of a filename (file, directory or link and file-details)
 *
 */
struct fileinfo {
	struct stat filestat;	// file-information (copy of stat - structure
	struct fileinfo *next;
	char *name;				// filename
};
/**
 *  @struct mypredicate
 *  @brief 
 * 
 */
struct mypredicate {
	int predicate;				// type of predicate user = 1, name = 2, type = 4, print = 8, ls = 16
	struct mypredicate *next;
	char *argument;				// argument without quotes
};


int find_end_of_link_opt(struct myfind *, int , char **);
int test_expression(const char *);
int parse_arguments(struct myfind *, int, char **, int);
int get_filenames(struct myfind *, char *, int, char **, int, int);
void freeMemory(struct myfind *);
int do_dir(struct myfind *, char *, int, short);
int do_entry(struct myfind *);
char *glob_pattern(char *);
void printHelp();
int doesitmatch(struct myfind *, char *, int);
int print_lstat(struct myfind *, struct stat *, char *);

#endif /* DEFS_H_ */
