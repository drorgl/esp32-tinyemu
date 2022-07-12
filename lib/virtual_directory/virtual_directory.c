#include "virtual_directory.h"

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include <stdbool.h>

static char currdir[PATH_MAX + 1] = "";

static char *previous_strrchr(const char *str, char *from, char delimiter)
{
    while (from >= str)
    {
        if (from[0] == delimiter)
        {
            return from;
        }
        from--;
    }
    return NULL;
}

static void normalize_path(const char *path, char *normalized)
{
    strcpy(normalized, path);
    // printf("normalized1 %s\r\n", normalized);
    bool has_path_traversal = true;
    //remove all /./
    while (has_path_traversal)
    {
        char *dot = strstr(normalized, "/./");
        if (dot != NULL)
        {
            strcpy(dot, dot + 2);
        }
        else
        {
            has_path_traversal = false;
        }
        // printf("normalized2 %s\r\n", normalized);
    }

    // printf("normalized3 %s\r\n", normalized);
    has_path_traversal = true;
    while (has_path_traversal)
    {
        // printf("normalized3.1 %s\r\n", normalized);
        char *dotdot = strstr(normalized, "/../");
        if (dotdot != NULL)
        {
            // printf("found ..\r\n");
            if (dotdot == normalized)
            {
                // printf("already on the 1st /\r\n");
                //if its the first /, do nothing, just remove the ..
                strcpy(dotdot, dotdot + 3);
            }
            else
            {
                //find previous /
                char *previous_slash = previous_strrchr(normalized, dotdot - 1, '/');
                if (previous_slash != NULL)
                {
                    // printf("removing previous /\r\n");
                    //if its not, remove the previous /
                    strcpy(previous_slash, dotdot + 3);
                }
                else
                {
                    // printf("didn't find /\r\n");
                    has_path_traversal = false;
                }
            }
        }
        else
        {
            // printf("has no ..\r\n");
            has_path_traversal = false;
        }
        // printf("normalized4 %s\r\n", normalized);
    }
    // printf("normalized5 %s\r\n", normalized);
}

int vd_chdir(const char *path)
{
    if (!path)
    {
        errno = EFAULT;
        return -1;
    }

    if (!*path)
    {
        errno = ENOENT;
        return -1;
    }

    if (path[0] == '/' || path[0] == '\\' || path[1] == ':')
    {
        strncpy(currdir, path, PATH_MAX);
    }
    else if (currdir[strlen(currdir) - 1] != '/' && currdir[strlen(currdir) - 1] != '\\')
    {
        strncat(currdir, "/", sizeof(currdir)-1); //handle "\" ?
        strncat(currdir, path, sizeof(currdir)-1);
    }

    char npath[256] = {0};

    normalize_path(currdir, npath);

    strncpy(currdir, npath, PATH_MAX);

    return 0;
}

void vd_cwd(char *cwd_buffer, size_t cwd_buffer_length)
{
    strncpy(cwd_buffer, currdir, cwd_buffer_length);
}