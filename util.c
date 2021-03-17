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
#include <fnmatch.h>
#include "defs.h"

int doesitmatch(struct myfind *task, char *name, int type){
	struct mypredicate *mypred = task->mypred;
	struct arguments *arg;

	if(!checkType(task, type)) return -1;
	arg = mypred->args;
	while(arg != NULL){
		if(!fnmatch(name, arg->argument, FNM_NOESCAPE | FNM_PERIOD)){
			return type;
		}
		arg = arg->next;
	}
	return 0;						// no match
}
/**
 *  @brief 
 * 
 */
int checkType(struct myfind *task, int type){
	struct mypredicate *mypred = task->mypred;

	while(mypred != NULL){
		if(type & mypred->predicate){
			return type;
		}
		mypred = mypred->next;
	}
	return 0;			// argument was not entered by user(-name, -user, ...)
}
/**
 * @fn int find_end_of_link_opt(int, char*[])
 * @brief find all the pre-options for symbolic links
 *
 * @param argc
 * @param argv
 * @return index of first filename or predicate
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
#if _DEBUG
			  printf("-H\n");
#endif
		  /* Meaning: dereference symbolic links on command line, but nowhere else. */
		  //set_follow_state(SYMLINK_DEREF_ARGSONLY);
		}
		  else if (0 == strcmp("-L", argv[i]))
		{
			  task->linkoption = 'L';
#if _DEBUG
			  printf("-L\n");
#endif
		  /* Meaning: dereference all symbolic links. */
		  //set_follow_state(SYMLINK_ALWAYS_DEREF);
		}else if (0 == strcmp("-P", argv[i]))
		{
			  task->linkoption = 'P';
#if _DEBUG
			  printf("-P\n");
#endif
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
 * @fn int test_expression(const char*)
 * @brief Test if an command line argument is a valid option
 *
 * @param arg Argument to be tested, if it's an option
 * @return
 */
int test_expression(const char *arg)
{
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
 * @fn int parse_arguments(int, char*[], int)
 * @brief identify index of first argument after filename and get all the following arguments
 *
 * @param argc
 * @param argv
 * @param end_of_link_opt	index of first possible filename
 * @return index of first argument after filename(s)
 */
int parse_arguments(struct myfind *task, int argc, char *argv[], int end_of_link_opt) {
	int i, y, end_of_filenames, found, arg_counter;
	struct mypredicate *mypred, *mypredinfo;
	struct arguments *myargs, *myargsinfo;

	struct options const myoptions[] =
	{
			{"-user", MYFIND_USER, 2},
			{"-name", MYFIND_NAME, 2},
			{"-type", MYFIND_TYPE, 1},
			{"-print", MYFIND_PRINT, 1},
			{"-ls", MYFIND_LS, 0},
			{"-maxdepth", MYFIND_MAXDEPTH, 1},
			{"--help", MYFIND_HELP, 0},
			{"END", 0, 0}
	};


	for (i = end_of_link_opt; i < argc && !test_expression(argv[i]); i++);		// find end of the given filenames
	end_of_filenames = i;														// mark the end of possible filenames (links, files, folder)
	while (i < argc )
	{
		if (!test_expression(argv[i]))											// is there still a filename where it shouldn't be? (-name & -user can have more parameters)
		{
		  printf("myfind: paths must precede expression: `%s'\n", argv[i]);		// is yes, show error message and quit
		  return 0;
		}
		else {
			y = 0; found = 0;
			if(strcmp(argv[i],"--help") == 0) {
				printHelp();
				return 0;
			}
			while(strcmp(myoptions[y].optname,"END") != 0)
			{
				if(strcmp(myoptions[y].optname,argv[i]) == 0) {						// compare expression with list of possible arguments
#if _DEBUG
					printf("Table[%d] -> %s",y, myoptions[y].optname);
#endif
					if(task->predicate & myoptions[y].opt_mode){
#if _DEBUG
						puts(" Duplikat!\n");
#endif
						return 0;													// option already set, no duplicate allowed
					}
					task->predicate = task->predicate | myoptions[y].opt_mode;							// set option-bit of a valid argument
					found = 1;																			// indicate, that we found a valid one
					if((myoptions[y].mode > 0) && ((i == (argc - 1)) || (argv[i+1][0] == '-'))) {		// mode > 0 indicates that an additional argument is required
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
					mypred->args = NULL;
					switch (myoptions[y].opt_mode){									// set type
					case MYFIND_USER:
						mypred->predicate = MYFIND_USER;
						break;
					case MYFIND_NAME:
						mypred->predicate = MYFIND_NAME;
						break;
					case MYFIND_TYPE:
						mypred->predicate = MYFIND_TYPE;
						break;
					case MYFIND_PRINT:
						mypred->predicate = MYFIND_PRINT;
						break;
					case MYFIND_LS:
						mypred->predicate = MYFIND_LS;
						break;
					case MYFIND_MAXDEPTH:
						mypred->predicate = MYFIND_MAXDEPTH;
						task->maxdepth = (atoi(argv[i+1]) < 0 ? 0 : atoi(argv[i+1]));
						break;
					default:
#if _DEBUG
						puts("\n");
#endif
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

						i++; arg_counter = 0;
						while(!test_expression(argv[i])){
							arg_counter++;
							if((myoptions[y].mode == 1) && (arg_counter > 1)){
								printf("myfind: too many arguments to `%s'\n",argv[i-2]);
								return 0;
							}
							myargs = malloc(sizeof(struct arguments));	// memory space for the argument
							if(!myargs){
								puts("myfind: out of memory\n");
								freeMemory(task);
								return 0;
							}
							myargs->argument = argv[i];				// save pointer to the argument
							myargs->next = NULL;
							if(mypred->args == NULL) {				// is this the first argument in the list?
								mypred->args = myargs;
							} else {
								myargsinfo->next = myargs;			// if arguments already exists, the new one is the successor of the last in the list
							}
#if _DEBUG
					printf(", %s",argv[i]);
#endif
							myargsinfo = myargs;
							myargsinfo->next = NULL;
							i++;
							if(i >= argc) break;
						}
#if _DEBUG
						puts("\n");
#endif
						break;
					} else {
						mypred->args = NULL;				// if mode == 0, no additional argument
						i++;
					}
#if _DEBUG
						puts("\n");
#endif
					break;
				}
				y++;
			}
		}
		if (!found) {															// nothing found? then it's an unknown one
			printf("myfind: unknown predicate `%s'\n",argv[i]);
			return 0;
		}
	}
	return end_of_filenames;
}
struct fileinfo *get_filestat(struct myfind *task, char *name){
	struct fileinfo *file_mem;
	struct stat details;

	if(lstat(name, &details) == -1){						// does file or path exist?
		printf("myfind: ‘%s’: No such file or directory\n",name);
		freeMemory(task);
		return (struct fileinfo *)-1;
	}

	file_mem = malloc(sizeof(struct fileinfo));
	if(!file_mem){
		freeMemory(task);
		return (struct fileinfo *)0;
	}
	memcpy(file_mem, &details, sizeof(struct stat));		// copy file-information
	return file_mem;
}

/**
 * @fn int get_filenames(struct myfind*, int, int, char*[], int, int)
 * @brief Makes a list of all given paths or filenames
 *
 * @param task
 * @param default_dir
 * @param argc
 * @param argv
 * @param end_of_link_opt
 * @param end_of_filenames
 * @return
 */
int get_filenames(struct myfind *task, char *start_dir, int argc, char *argv[],int end_of_link_opt, int end_of_filenames){
	int i = end_of_link_opt;
	struct fileinfo *file_mem, *fileinfo;
	//default_dir = default_dir;	// avoid unused

	if(i != end_of_filenames){										// is there at least one path or filename?
		while((i<end_of_filenames) & (i < argc)){					// get all names
			file_mem = get_filestat(task, argv[i]);
			if(file_mem == (struct fileinfo *)-1){					// does file or path exist?
				return -1;
			}
			if(file_mem == (struct fileinfo *)0){					// out of memory
				return 0;
			}
#if _DEBUG
			if(S_ISREG(file_mem->filestat.st_mode)) printf("%s ist eine reguläre Datei\n",argv[i]);
			if(S_ISCHR(file_mem->filestat.st_mode)) printf("%s ist eine Gerätedatei\n",argv[i]);
			if(S_ISDIR(file_mem->filestat.st_mode)) printf("%s ist ein Verzeichnis\n",argv[i]);
			if(S_ISLNK(file_mem->filestat.st_mode)) printf("%s ist ein Symbollink\n",argv[i]);
#endif
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
			fileinfo->name = argv[i];
			i++;
		}
	} else {
		// no userinput for a path or filename (before the -... options), take standard "." path
#if _DEBUG
		printf("Filename: '.'\n");
#endif
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
#if _DEBUG
			if(S_ISREG(file_mem->filestat.st_mode)) printf("%s ist eine reguläre Datei\n",".");
			if(S_ISCHR(file_mem->filestat.st_mode)) printf("%s ist eine Gerätedatei\n",".");
			if(S_ISDIR(file_mem->filestat.st_mode)) printf("%s ist ein Verzeichnis\n", file_mem->name);
			if(S_ISLNK(file_mem->filestat.st_mode)) printf("%s ist ein Symbollink\n",".");
#endif
	}
	return 1;
}
void freeMemory(struct myfind *task){
	struct fileinfo *fileinfo = task->fileinfo, *temp;
	struct mypredicate *mypredicate = task->mypred, *temp1;
	struct arguments *myarg, *temp2;
	while(fileinfo != NULL){
		temp = fileinfo->next;
		free(fileinfo);
		fileinfo = temp;
	}
	while(mypredicate != NULL){
		myarg = mypredicate->args;
		while(myarg != NULL){
			temp2 = myarg->next;
			free(myarg);
			myarg = temp2;
		}
		temp1 = mypredicate->next;
		free(mypredicate);
		mypredicate = temp1;
	}
}
void printHelp(){
	puts("\nUsage: .\\myfind [-H] [-L] [-P] [path...] [expression]\n"
			"default path is the current directory; default expression is -print\n"
			"expression may consist of: operators, options, tests, and actions:"
			"operators (decreasing precedence; -and is implicit where no others are given):\n"
			"( EXPR )   ! EXPR   -not EXPR   EXPR1 -a EXPR2   EXPR1 -and EXPR2\n"
			"EXPR1 -o EXPR2   EXPR1 -or EXPR2   EXPR1 , EXPR2\n"
			"positional options (always true): -daystart -follow -regextype\n"
			"\n"
			"normal options (always true, specified before other expressions):\n"
			"-depth --help -maxdepth LEVELS -mindepth LEVELS -mount -noleaf\n"
			"--version -xdev -ignore_readdir_race -noignore_readdir_race\n"
			"tests (N can be +N or -N or N): -amin N -anewer FILE -atime N -cmin N\n"
			"-cnewer FILE -ctime N -empty -false -fstype TYPE -gid N -group NAME\n"
			"-ilname PATTERN -iname PATTERN -inum N -iwholename PATTERN -iregex PATTERN\n"
			"-links N -lname PATTERN -mmin N -mtime N -name PATTERN -newer FILE\n"
			"-nouser -nogroup -path PATTERN -perm [-/]MODE -regex PATTERN\n"
			"-readable -writable -executable\n"
			"-wholename PATTERN -size N[bcwkMG] -true -type [bcdpflsD] -uid N\n"
			"-used N -user NAME -xtype [bcdpfls]      -context CONTEXT\n"
			"\n"
			"actions: -delete -print0 -printf FORMAT -fprintf FILE FORMAT -print\n"
			"-fprint0 FILE -fprint FILE -ls -fls FILE -prune -quit\n"
			"-exec COMMAND ; -exec COMMAND {} + -ok COMMAND ;\n"
			"-execdir COMMAND ; -execdir COMMAND {} + -okdir COMMAND ;\n"
			"\n"
			"You can report bugs in the \"myfind\" program via Email <ic20b005@technikum-wien.at>.");
}
