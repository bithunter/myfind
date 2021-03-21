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
	char *start_dir = ".";

	//DIR *dir;

	//struct dirent *dirzeiger;
	static struct myfind tasktodo = {
			' ',
			0,
			NULL,
			NULL,
			0,				// 0 = search hole directory
			NULL, NULL, NULL,
			{ 0 }
	};

	end_of_link_opt = find_end_of_link_opt(&tasktodo, argc, argv);												// get index of first possible filename
	if((end_of_filenames = parse_arguments(&tasktodo, argc, argv, end_of_link_opt)) == 0) return EXIT_FAILURE;	// get arg after list of filenames
	if(end_of_link_opt != end_of_filenames) default_dir = 0;													// is 0 when user entered at least one filename

	if(default_dir) {					// default_dir != 0 -> start in actual position
		working_dir = start_dir;
	} else {
		working_dir = argv[end_of_link_opt];
	}

	//glob_pattern("test-fi*");

	temp = get_filenames(&tasktodo, start_dir, argc, argv, end_of_link_opt, end_of_filenames);
	if(temp == 0) {
		puts("myfind: out of memory\n");
		return EXIT_FAILURE;
	} else if(temp == -1){
				return EXIT_FAILURE;
	}

	if((tasktodo.predicate & (MYFIND_ISFILE | MYFIND_NAME)) == (MYFIND_ISFILE | MYFIND_NAME)){
		puts("myfind: When first argument is a filename, no \"-data\" is allowed!\n");
		return 0;																					// filename already set, no double filename (in -name) allowed
	}

	if(!do_entry(&tasktodo)) puts("Error building tree!");

	freeMemory(&tasktodo);
	return EXIT_SUCCESS;
}
