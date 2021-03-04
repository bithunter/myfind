/**
 * @file
 * @brief File and directory explorer
 * @author Andreas Bauer, IC20B005
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "defs.h"

/**
 * @brief list a directory
 * 
 */
void do_dir(const char *dir_name) {
    DIR *directory = opendir(dir_name);

    struct dirent *entry;

    while ((entry = readdir(directory))) {
        if ((entry->d_type == 4) && ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))) {
            printf("Entering directory \"%s\":\n", entry->d_name);
            do_dir(entry->d_name);
        } else {
            struct stat file_stats;
            stat(entry->d_name, &file_stats);

            switch (file_stats.st_mode & __S_IFMT)
            {
            case __S_IFSOCK:
                putchar('s');
                break;
            case __S_IFLNK:
                putchar('l');
                break;
            case __S_IFBLK:
                putchar('b');
                break;
            case __S_IFDIR:
                putchar('d');
                break;
            case __S_IFCHR:
                putchar('c');
                break;
            case __S_IFIFO:
                putchar('p');
                break;
            case __S_IFREG:
                putchar('f');
                break;
            default:
            putchar('-');
                break;
            }
            printf(" %s - %ld of type %d\n", entry->d_name, file_stats.st_ino, entry->d_type);
        }
    }
    closedir(directory);
}