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
#include "defs.h"

#define MYFIND_USER 1
#define MYFIND_NAME 2
#define MYFIND_TYPE 4
#define MYFIND_PRINT 8
#define MYFIND_LS 16
#define MYFIND_MAXDEPTH 32

static struct options const myoptions[] =
{
		{"-user", MYFIND_USER, 1},
		{"-name", MYFIND_NAME, 1},
		{"-type", MYFIND_TYPE, 1},
		{"-print", MYFIND_PRINT, 1},
		{"-ls", MYFIND_LS, 0},
		{"-maxdepth", MYFIND_MAXDEPTH, 1},
		{"END", 0, 0}
};
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
#if _DEBUG
			  task->linkoption = 'H';
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
	int i, y, end_of_filenames, found;
	struct mypredicate *mypred, *mypredinfo;


	for (i = end_of_link_opt; i < argc && !test_expression(argv[i]); i++);		// find end of the given filenames
	end_of_filenames = i;														// mark the end of possible filenames (links, files, folder)
	while (i < argc )
	{
		if (!test_expression(argv[i]))											// is there still a filename where it shouldn't be?
		{
		  printf("myfind: paths must precede expression: `%s'\n", argv[i]);		// is yes, show error message and quit
		  return 0;
		}
		y = 0; found = 0;
		while(strcmp(myoptions[y].optname,"END") != 0)
		{
			if(strcmp(myoptions[y].optname,argv[i]) == 0) {						// compare expression with list of possible arguments
#if _DEBUG
				printf("Table[%d] -> %s",y, myoptions[y].optname);
#endif
				if(task->predicate & myoptions[y].opt_mode){
					return 0;													// option already set, no duplicate allowed
				}
				task->predicate = task->predicate | myoptions[y].opt_mode;							// set option-bit of a valid argument
				found = 1;																			// indicate, that we found a valid one
				if((myoptions[y].mode == 1) && ((i == (argc - 1)) || (argv[i+1][0] == '-'))) {		// mode = 1 indicates that an additional argument is required
#if _DEBUG																							// is this the last user-input, or no following argument -> missing argument
					puts("\n");
#endif
					printf("myfind: missing argument to `%s'\n",argv[i]);
					return 0;
				}
				mypred = malloc(sizeof(struct mypredicate));					// memory space for the argument
				if(!mypred){
					puts("myfind: out of memory\n");
					freeMemory(task);
					return 0;
				}
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
				if(myoptions[y].mode == 1){						// mode = 1 requires an additional information (name, type, depth...)
					i++;
					mypred->argument = argv[i];				// save pointer to the argument
#if _DEBUG
				printf(", option: %s\n",argv[i]);
#endif
				} else {
					mypred->argument = NULL;				// if mode != 1, no additional argument
#if _DEBUG
				puts("\n");
#endif
				}
				mypredinfo = mypred;						// new struct is predecessor of next one
				mypredinfo->next = NULL;					// currently is this the last one in the list
				break;
			}
			y++;
		}
		if (!found) {															// nothing found? then it's an unknown one
			printf("myfind: unknown predicate `%s'\n",argv[i]);
			return 0;
		}
		i++;
	}
	return end_of_filenames;
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
int get_filenames(struct myfind *task, int default_dir, int argc, char *argv[],int end_of_link_opt, int end_of_filenames){
	int i = end_of_link_opt;
	struct stat details;
	struct fileinfo *file_mem, *fileinfo;
	default_dir = default_dir;	// avoid unused

	if(i != end_of_filenames){										// is there at least one path or filename?
		while((i<end_of_filenames) & (i < argc)){					// get all names
			if(lstat(argv[i],&details) == -1){						// does file or path exist?
				printf("myfind: file not found '%s'\n",argv[i]);
				freeMemory(task);
				return -1;
			}

			file_mem = malloc(sizeof(struct fileinfo));
			if(!file_mem){
				freeMemory(task);
				return 0;
			}
			memcpy(file_mem, &details, sizeof(struct stat));		// copy file-information
#if _DEBUG
		printf("st_mode = %d\n",details.st_mode);
		printf("mode S_IFREG = %d\n",S_IFREG);
		printf("mode S_IFCHR = %d\n",S_IFCHR);
		printf("mode S_IFDIR = %d\n",S_IFDIR);
		printf("mode S_IFLNK = %d\n\n\n",S_IFLNK);
		if(S_ISREG(file_mem->filestat.st_mode)) printf("%s ist eine reguläre Datei\n",argv[i]);
		if(S_ISCHR(file_mem->filestat.st_mode)) printf("%s ist eine Gerätedatei\n",argv[i]);
		if(S_ISDIR(file_mem->filestat.st_mode)) printf("%s ist ein Verzeichnis\n",argv[i]);
		if(S_ISLNK(file_mem->filestat.st_mode)) printf("%s ist ein Symbollink\n",argv[i]);
#endif
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
#if _DEBUG
		printf("Filename: '.'\n");
#endif
	}
	return 1;
}
void freeMemory(struct myfind *task){
	struct fileinfo *fileinfo = task->fileinfo, *temp;
	struct mypredicate *mypredicate = task->mypred, *temp1;

	while(fileinfo != NULL){
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