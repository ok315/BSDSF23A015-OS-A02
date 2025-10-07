/*
 * Programming Assignment 02: ls-v1.1.0
 * Feature Added: Long Listing Format (-l)
 * Usage:
 *       $ ./ls-v1.1.0
 *       $ ./ls-v1.1.0 -l
 *       $ ./ls-v1.1.0 /home /etc
 *       $ ./ls-v1.1.0 -l /home /etc
 */

#define _XOPEN_SOURCE 700 /* for lstat, readlink, and templated functions */

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
#include <stdbool.h>
#include <limits.h>
#include <sys/types.h>

int long_listing = 0;  // global flag

void do_ls(const char *dir);
void print_long_format(const char *path, const char *filename);
bool is_dir(const char *path);

/* Convert mode to the first character (file type) */
char file_type_char(mode_t mode) {
    if (S_ISREG(mode)) return '-';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    if (S_ISCHR(mode)) return 'c';
    if (S_ISBLK(mode)) return 'b';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    return '?';
}

/* Format the rwx string including suid/sgid/sticky bits */
void format_permissions(mode_t mode, char out[11]) {
    out[0] = file_type_char(mode);

    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    /* user exec and suid handling */
    if (mode & S_ISUID) {
        out[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        out[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    /* group exec and sgid handling */
    if (mode & S_ISGID) {
        out[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        out[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    /* others exec and sticky handling */
    if (mode & S_ISVTX) {
        out[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        out[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    out[10] = '\0';
}

int main(int argc, char *argv[])
{
    int opt;
    /* parse options anywhere in argv */
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_listing = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [file...]\n", argv[0]);
                return 1;
        }
    }

    int first_arg = optind;

    /* If no paths provided, use current directory */
    if (first_arg >= argc) {
        if (long_listing) {
            /* if long listing for current directory */
            do_ls(".");
        } else {
            do_ls("."); /* existing simple listing uses do_ls */
        }
    } else {
        bool multiple = (argc - first_arg > 1);
        for (int i = first_arg; i < argc; ++i) {
            const char *path = argv[i];

            if (multiple) {
                printf("%s:\n", path);
            }

            /* If argument is a directory, list its contents; otherwise, print the file's long line or name */
            if (is_dir(path)) {
                do_ls(path);
            } else {
                /* Path is a file (or non-directory) — print one line */
                if (long_listing) {
                    /* We pass the directory part as '.' and filename as path for simplicity */
                    /* But better: split dirname and basename. We'll print directly using print_long_format with path as directory and filename as basename */
                    char *dup = strdup(path);
                    if (!dup) {
                        perror("strdup");
                        continue;
                    }
                    char *base = strrchr(dup, '/');
                    if (base) {
                        *base = '\0';
                        base++;
                        print_long_format(dup[0] ? dup : "/", base);
                    } else {
                        print_long_format(".", dup);
                    }
                    free(dup);
                } else {
                    printf("%s\n", path);
                }
            }

            if (multiple && i < argc - 1) puts("");
        }
    }

    return 0;
}

/* Helper: return true if path is directory */
bool is_dir(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) return false;
    return S_ISDIR(st.st_mode);
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
        /* Skip hidden files ('.*') for now — -a not implemented yet */
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
    struct stat st;
    char fullpath[PATH_MAX];
    if (strcmp(path, "/") == 0)
        snprintf(fullpath, sizeof(fullpath), "/%s", filename);
    else
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    if (lstat(fullpath, &st) == -1)
    {
        fprintf(stderr, "stat error on %s: %s\n", fullpath, strerror(errno));
        return;
    }

    /* Permissions and file type */
    char perms[11];
    format_permissions(st.st_mode, perms);
    printf("%s ", perms);

    /* Link count */
    printf("%3lu ", (unsigned long)st.st_nlink);

    /* Owner and group */
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    printf("%-8s %-8s ", pw ? pw->pw_name : "unknown", gr ? gr->gr_name : "unknown");

    /* Size */
    printf("%8lld ", (long long)st.st_size);

    /* Time: use abbreviated month, day, and HH:MM (simple) */
    char timebuf[64];
    struct tm *t = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", t);
    printf("%s ", timebuf);

    /* Name */
    printf("%s", filename);

    /* If symbolic link, append -> target */
    if (S_ISLNK(st.st_mode)) {
        char linktarget[PATH_MAX];
        ssize_t len = readlink(fullpath, linktarget, sizeof(linktarget) - 1);
        if (len != -1) {
            linktarget[len] = '\0';
            printf(" -> %s", linktarget);
        }
    }

    printf("\n");
}
