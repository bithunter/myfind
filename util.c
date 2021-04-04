/**
 * @file
 * @brief Linux File-Search
 * @author Andreas Bauer, IC20B005
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include "defs.h"


/**
 * @fn int find_end_of_link_opt(struct myfind *task, int argc, char *argv[])
 * @brief find all the pre-options for symbolic links
 *
 * @param task	myfind-structure and argument information
 * @param argc
 * @param argv	arguments from treminal
 * @return		index-number of first filename or predicate
 * @see main()
 */
int find_end_of_link_opt(struct myfind *task, int argc, char *argv[])
{
  int i, end_of_link_opt;

  for (i=1; (end_of_link_opt = i) < argc; ++i)
    {
		  if (0 == strcmp("-H", argv[i]))
		{
			task->linkoption = 'H';

		  /* Meaning: dereference symbolic links on command line, but nowhere else. */
		  //set_follow_state(SYMLINK_DEREF_ARGSONLY);
		}
		  else if (0 == strcmp("-L", argv[i]))
		{
			  task->linkoption = 'L';

		  /* Meaning: dereference all symbolic links. */
		  //set_follow_state(SYMLINK_ALWAYS_DEREF);
		}else if (0 == strcmp("-P", argv[i]))
		{
			  task->linkoption = 'P';

		  /* Meaning: dereference all symbolic links. */
		  //set_follow_state(SYMLINK_ALWAYS_DEREF);
		}else if (0 == strcmp("--", argv[i]))
		{
		  /* -- signifies the end of options. */
			end_of_link_opt = i+1;	/* Next time start with the next option */
		  break;
		}
		  else
		{
		  /* Hmm, must be one of
		   * (a) A path name
		   * (b) A predicate
		   */
			  end_of_link_opt = i; /* Next time start with this option */
		  break;
		}
    }
  return end_of_link_opt;
}
/**
 * @fn int test_expression(const char *arg, int type)
 * @brief Test if an command line argument is a valid option
 *
 * @param arg Argument to be tested, if it's an option
 * @return
 */
int test_expression(const char *arg, int type)
{
	switch (type)
	{
		case MYFIND_MTIME:		// '-mtime' allows a following '-'
		return 0;
	}
	switch (arg[0])
	{
	case '-':
		if (arg[1])	// "-foo" is an expression
	return 1;
		else
	return 0;		// Just "-" is a filename
		break;
	default:
		return 0;
	}
}
/**
 * @fn int parse_arguments(struct myfind *task, int argc, char *argv[], int end_of_link_opt)
 * @brief identify index of first argument after filename and get all the following arguments
 *
 * @param argc
 * @param argv
 * @param end_of_link_opt	index of first possible filename
 * @return index of first argument after filename(s)
 */
int parse_arguments(struct myfind *task, int argc, char *argv[], int end_of_link_opt) {
	int i, y, z, end_of_filenames, found, fnd, indx = 0;
	char *optn[] = {"-name", "-user", "-type", "-ls", "-print", "-mtime"};
	struct mypredicate *mypred, *mypredinfo;
	char *arg[] ={"d","f","l","END"}, *tt;

	struct options const myoptions[] =
	{
			{"-user", MYFIND_USER, 2},
			{"-name", MYFIND_NAME, 2},
			{"-type", MYFIND_TYPE, 2},
			{"-print", MYFIND_PRINT, 0},
			{"-ls", MYFIND_LS, 0},
			{"-maxdepth", MYFIND_MAXDEPTH, 2},		// argument needs one more parameter, no - allowed
			{"-mtime", MYFIND_MTIME, 1},			// additional info required, 1 means, a '-' is allowed, eg. -mtime -3
			{"--help", MYFIND_HELP, 0},
			{"END", 0, 0}
	};


	for (i = end_of_link_opt; i < argc && !test_expression(argv[i], 0); i++);	// find end of the given filenames
	end_of_filenames = i;														// mark the end of possible filenames (links, files, folder)
	while (i < argc )
	{
		if (!test_expression(argv[i], 0))										// is there still a filename where it shouldn't be?
		{
		  printf("myfind: paths must precede expression: `%s'\n", argv[i]);		// is yes, show error message and quit
		  if(mypred->predicate & MYFIND_NAME) printf("myfind: possible unquoted pattern after predicate `-name'?\n");	// -name argument without quotes?
		  return 0;
		}
		else {
			y = 0; found = 0;
			if(strcmp(argv[i],"--help") == 0) {
				printHelp();
				return 0;
			}
			if(strcmp(argv[i],"-version") == 0) {
				printVersion();
				return 0;
			}
			while(strcmp(myoptions[y].optname,"END") != 0)
			{
				if(strcmp(myoptions[y].optname,argv[i]) == 0) {						// compare expression with list of possible arguments
					if((task->predicate & myoptions[y].opt_mode) && !(myoptions[y].opt_mode & (MYFIND_LS|MYFIND_MAXDEPTH))){
						return 0;													// option already set, no duplicate allowed
					}
					task->predicate = task->predicate | myoptions[y].opt_mode;							// set option-bit of a valid argument
					found = 1;																			// indicate, that we found a valid one
					if((myoptions[y].mode > 0) && ((i == (argc - 1)) || ((argv[i+1][0] == '-') && (myoptions[y].mode > 1)))) {		// mode > 0 indicates that an additional argument is required
																									// is this the last user-input, or no following argument -> missing argument
						printf("myfind: missing argument to `%s'\n",argv[i]);
						return 0;
					}
					mypred = malloc(sizeof(struct mypredicate));					// memory space for the argument
					if(!mypred){
						puts("myfind: out of memory\n");
						freeMemory(task);
						return 0;
					}
					mypred->argument = NULL;
					switch (myoptions[y].opt_mode){									// get type of argument
					case MYFIND_USER:
						mypred->predicate = MYFIND_USER;
						indx = 1;
						break;
					case MYFIND_NAME:
						mypred->predicate = MYFIND_NAME;
						indx = 0;
						break;
					case MYFIND_TYPE:
						mypred->predicate = MYFIND_TYPE;
						indx = 2;
						break;
					case MYFIND_PRINT:
						mypred->predicate = MYFIND_PRINT;
						indx = 4;
						break;
					case MYFIND_LS:
						mypred->predicate = MYFIND_LS;
						task->ls++;
						indx = 3;
						break;
					case MYFIND_MAXDEPTH:
						mypred->predicate = MYFIND_MAXDEPTH;
						break;
					case MYFIND_MTIME:
						mypred->predicate = MYFIND_MTIME;
						indx = 5;
						break;
					default:
						printf("myfind: unknown predicate `%s'\n",argv[i]);
						return 0;
					}
					if(task->mypred == NULL) {						// is this the first argument in the list?
						task->mypred = mypred;
					} else {
						mypredinfo->next = mypred;					// if arguments already exists, the new one is the successor of the last in the list
					}
					mypredinfo = mypred;							// new struct is predecessor of next one
					mypredinfo->next = NULL;						// currently is this the last one in the list
					if(myoptions[y].mode > 0){						// mode > 0 requires additional information (type, depth...)

						i++;
						if(!test_expression(argv[i], mypredinfo->predicate) ){
							mypredinfo->argument = strdup(argv[i]);				// save pointer to the argument
							if(mypredinfo->predicate == MYFIND_TYPE) { 
								char myType[strlen(argv[i]) + 1];
								if(task->type != NULL) break;					// type already set
								task->type = argv[i];
								// check the syntax
								strcpy(myType, argv[i]);
								tt = strtok(myType, ",");
								while(tt != NULL){
									z = 0; fnd = 0;
									if(strlen(tt) !=1){
										puts("myfind: Must separate multiple arguments to -type using: ','");
										return -1;
									}
									while(arg[z] != "END"){
										if(!strcmp(tt,arg[z])) { fnd = 1; break; }		// is it a valid file-type?
										z++;
									}
									if(!fnd){
										printf("myfind: Unknown argument to -type: %s\n",tt);
										return 0;
									}
									tt = strtok(NULL, ",");
								}
							}

							switch(mypredinfo->predicate){
								case MYFIND_USER:
									if(task->user != NULL) break;
									task->user = strdup(argv[i]);
									break;
								case MYFIND_TYPE:
									break;
								case MYFIND_NAME:
									if(task->name != NULL) break;
									task->name = strdup(argv[i]);
									break;
								case MYFIND_MAXDEPTH:					// additional 'maxdepth' will overwrite each other and cause a warning
									task->maxdepth = atoi(argv[i]);		// give warning, if '-maxdepth' isn't on first position
									if(task->user || task->type || task->name || task->ls) {
										printf("find: warning: you have specified the -maxdepth option after a non-option argument %s, but options are not positional (-maxdepth affects tests specified before it as well as those specified after it). Please specify options before other arguments.\n",optn[indx]);
									}
									break;
								case MYFIND_MTIME:
									if(task->mtime != NULL) break;
									task->mtime = strdup(argv[i]);
									break;
								default:
									return 0;		// unknown type
									break;
							}

						} else {
							printf("myfind: missing argument to `%s'\n",argv[i-1]);
							return 0;
						}
					} else {
						mypredinfo->argument = NULL;				// if mode == 0, no additional argument
					}
					i++;
					break;
				}
				y++;
			}
		}
		if (!found) {												// nothing found? then it's an unknown one
			printf("myfind: unknown predicate `%s'\n",argv[i]);
			return 0;
		}
	}
	return end_of_filenames;
}
/**
 * @fn struct fileinfo *get_filestat(struct myfind *task, char *name)
 * @brief 
 *
 * @param task	myfind-structure and argument information
 * @param name	name of the file to test
 * @return pointer to the new struct if successfully, 0 otherwise
 */
struct fileinfo *get_filestat(struct myfind *task, char *name){
	struct fileinfo *file_mem;
	struct stat details;

	if(lstat(name, &details) == -1){						// does file or path exist?
		printf("myfind: ‘%s’: No such file or directory\n",name);
		freeMemory(task);
		return (struct fileinfo *)-1;
	}

	file_mem = malloc(sizeof(struct fileinfo));				// get ram for the fileinformation-struct
	if(!file_mem){
		freeMemory(task);
		return (struct fileinfo *)0;
	}
	memcpy(file_mem, &details, sizeof(struct stat));		// copy file-information
	return file_mem;
}
/**
 * @fn int get_filenames(struct myfind *task, char *start_dir, int argc, char *argv[], int end_of_link_opt, int end_of_filenames)
 * @brief Makes a list of all given paths or filenames
 *
 * @param task	myfind-structure and argument information
 * @param start_dir is '.' at the first call, if not otherwise defined
 * @param argc
 * @param argv
 * @param end_of_link_opt	index of first possible filename
 * @param end_of_filenames	end of filenames, index of first possible argument (starts with '-....')
 * @return 0 = error, 1 = success, -1 file does not exist
 */
int get_filenames(struct myfind *task, char *start_dir, int argc, char *argv[], int end_of_link_opt, int end_of_filenames){
	int i = end_of_link_opt;
	struct fileinfo *file_mem, *fileinfo;

	if(i != end_of_filenames){										// is there at least one path or filename?
		while((i<end_of_filenames) & (i < argc)){					// get all names
			file_mem = get_filestat(task, argv[i]);
			if(file_mem == (struct fileinfo *)-1){					// does file or path exist?
				return -1;
			}
			if(file_mem == (struct fileinfo *)0){					// out of memory
				return 0;
			}
			if(!S_ISDIR(file_mem->filestat.st_mode)) {
				task->predicate = task->predicate | MYFIND_ISFILE;	// indicates that user entered a filename instead of a path before "-...." arguments
			}
			if(task->fileinfo == NULL) {
				task->fileinfo = file_mem;			// first entry in the list?
			} else {
				fileinfo->next = file_mem;			// otherwise it's the successor of the last one
			}
			fileinfo = file_mem;					// set actual pointer to the new element
			fileinfo->next = NULL;
			fileinfo->name = strdup(argv[i]);
			i++;
		}
	} else {
		// no userinput for a path or filename (before the -... options), take standard "." path
		file_mem = get_filestat(task, ".");
		if(file_mem == (struct fileinfo *)-1){		// does file or path exist?
			return -1;
		}
		if(file_mem == (struct fileinfo *)0){		// out of memory
			return 0;
		}
		task->fileinfo = file_mem;			// has to be the first entry in the list
		file_mem->next = NULL;
		file_mem->name = start_dir;
	}
	return 1;
}
/**
 * @fn void freeMemory(struct myfind *task)
 * @brief free all the reserved memory
 *
 * @param task	myfind-structure and argument information
 * @return none
 */
void freeMemory(struct myfind *task){
	struct fileinfo *fileinfo = task->fileinfo, *temp;			// get pointer to all structure with reserved memory
	struct mypredicate *mypredicate = task->mypred, *temp1;
	struct arguments *myarg, *temp2;
	while(fileinfo != NULL){									// free the memory
		temp = fileinfo->next;
		free(fileinfo);
		fileinfo = temp;
	}
	while(mypredicate != NULL){
		temp1 = mypredicate->next;
		free(mypredicate);
		mypredicate = temp1;
	}
}
/**
 * @fn void printHelp()
 * @brief Print help-text after the '--help' argument
 *
 * @return none
 */
void printHelp(){
	puts("Usage: .\\myfind [-H] [-L] [-P] [path...] [expression]\n"
			"default path is the current directory; default expression is -print\n"
			"expression may consist of: operators and options:"
			"operators (decreasing precedence; -and is implicit where no others are given):\n"
			"( EXPR )   ! EXPR   -not EXPR   EXPR1 -a EXPR2   EXPR1 -and EXPR2\n"
			"EXPR1 -o EXPR2   EXPR1 -or EXPR2   EXPR1 , EXPR2\n"
			"\n"
			"normal options (always true, specified before other expressions):\n"
			"---help -maxdepth -mtime N -name PATTERN -user NAME -type TYP\n"
			"\n"
			"You can report bugs in the \"myfind\" program via Email <ic20b005@technikum-wien.at>.");
}
/**
 * @fn void printVersion()
 * @brief print software credentials 
 *
 * @return none
 */
void printVersion(){
	puts("myfind (SS2021) 1.0.0\n"
			"Copyright (C) 2021 Technikum Wien - IC2020\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n"
			"Written by Andreas Bauer.");
}
