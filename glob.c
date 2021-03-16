#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

/* Convert a wildcard pattern into a list of blank-separated
   filenames which match the wildcard.  */

char * glob_pattern(char *wildcard)
{
  char *gfilename;
  size_t cnt, length = 0;
  glob_t glob_results;
  char **p;

  glob(wildcard, GLOB_NOCHECK, 0, &glob_results);

  /* How much space do we need?  */
  for (p = glob_results.gl_pathv, cnt = glob_results.gl_pathc;
       cnt; p++, cnt--)
    length += strlen(*p) + 1;

  /* Allocate the space and generate the list.  */
  gfilename = calloc(length, 1);
  for (p = glob_results.gl_pathv, cnt = glob_results.gl_pathc;
       cnt; p++, cnt--)
    {
      strcat(gfilename, *p);
      if (cnt > 1)
        strcat(gfilename, " ");
    }

  globfree(&glob_results);
  printf("Filenames found: %s\n",(char *)gfilename);
  free(gfilename);
  return gfilename;
}