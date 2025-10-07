/*
* Programming Assignment 02: ls-v1.1.0
* Feature Added: Long Listing Format (-l)
* Usage:
*       $ ./ls-v1.1.0
*       $ ./ls-v1.1.0 -l
*       $ ./ls-v1.1.0 /home /etc
*       $ ./ls-v1.1.0 -l /home /etc
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

extern int errno;

int long_listing = 0;  // global flag

void do_ls(const char *dir);
void print_long_format(const char *path, const char *filename);

int main(int argc, char const *argv[])
{
    // Check if -l option is passed
    int start_index = 1;
    if (argc > 1 && strcmp(argv[1], "-l") == 0)
    {
        long_listing = 1;
        start_index = 2;
    }

    // Default directory if none provided
    if (argc == 1 || (argc == 2 && long_listing))
    {
        do_ls(".");
    }
    else
    {
        for (int i = start_index; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i]);
            puts("");
        }
    }

    return 0;
}

void do_ls(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;

    while ((entry = readdir(dp)) != NULL)
    {
        // Skip hidden files
        if (entry->d_name[0] == '.')
            continue;

        if (long_listing)
            print_long_format(dir, entry->d_name);
        else
            printf("%s\n", entry->d_name);
    }

    if (errno != 0)
    {
        perror("readdir failed");
    }

    closedir(dp);
}

void print_long_format(const char *path, const char *filename)
{
    struct stat fileStat;
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    if (stat(fullpath, &fileStat) == -1)
    {
        perror("stat");
        return;
    }

    // File type and permissions
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");

    // Links
    printf(" %ld ", fileStat.st_nlink);

    // Owner and group names
    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);
    printf("%s %s ", pw ? pw->pw_name : "unknown", gr ? gr->gr_name : "unknown");

    // Size
    printf("%5ld ", fileStat.st_size);

    // Modification time
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&fileStat.st_mtime));
    printf("%s ", timebuf);

    // File name
    printf("%s\n", filename);
}
