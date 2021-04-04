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
	char *start_dir = ".";


	static struct myfind tasktodo = { };

	end_of_link_opt = find_end_of_link_opt(&tasktodo, argc, argv);												// get index of first possible filename
	if((end_of_filenames = parse_arguments(&tasktodo, argc, argv, end_of_link_opt)) == 0) return EXIT_FAILURE;	// get arg after list of filenames
	if(end_of_link_opt != end_of_filenames) default_dir = 0;													// is 0 when user entered at least one filename

	temp = get_filenames(&tasktodo, start_dir, argc, argv, end_of_link_opt, end_of_filenames);		// get the list of all entered names to search (dir and files)
	if(temp == 0) {
		puts("myfind: out of memory\n");
		return EXIT_FAILURE;
	} else if(temp == -1){
				return EXIT_FAILURE;
	}

	if(!do_entry(&tasktodo)) puts("Error building tree!");			// display all entered dir or files

	freeMemory(&tasktodo);					// free all memory we used
	return EXIT_SUCCESS;					// end successfully
}
