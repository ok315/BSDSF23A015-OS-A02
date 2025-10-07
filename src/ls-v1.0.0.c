/*
 * Programming Assignment 02: ls-v1.2.0
 * Features:
 *  - v1.1.0: Long listing (-l) (kept)
 *  - v1.2.0: Default multi-column display (down then across)
 *
 * Usage:
 *       $ ./ls
 *       $ ./ls -l
 *       $ ./ls /some/path
 *       $ ./ls -l /some/path
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
#include <sys/ioctl.h>   /* ioctl, winsize */
#include <sys/termios.h> /* optional */

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
        do_ls(".");
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

/* Simple comparator for alphabetical sort */
static int name_cmp(const void *a, const void *b) {
    const char * const *sa = a;
    const char * const *sb = b;
    return strcmp(*sa, *sb);
}

/* Get terminal width in columns; fallback to 80 on failure */
static int get_terminal_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0) return ws.ws_col;
    }
    return 80; /* fallback */
}

/* ---- Updated do_ls: gather filenames, compute columns, print down-then-across ---- */
void do_ls(const char *dir)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);

    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    /* If long listing is requested, keep original behavior: */
    if (long_listing) {
        errno = 0;
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;
            print_long_format(dir, entry->d_name);
        }
        if (errno != 0) {
            perror("readdir failed");
        }
        closedir(dp);
        return;
    }

    /* ---- Default behavior: gather filenames first ---- */
    int capacity = 64;
    int count = 0;
    int max_len = 0;
    char **names = malloc(capacity * sizeof(char *));
    if (!names) {
        perror("malloc");
        closedir(dp);
        return;
    }

    errno = 0;
    while ((entry = readdir(dp)) != NULL) {
        /* Skip hidden files (no -a yet) */
        if (entry->d_name[0] == '.')
            continue;

        if (count >= capacity) {
            capacity *= 2;
            char **tmp = realloc(names, capacity * sizeof(char *));
            if (!tmp) {
                perror("realloc");
                for (int i = 0; i < count; ++i) free(names[i]);
                free(names);
                closedir(dp);
                return;
            }
            names = tmp;
        }

        names[count] = strdup(entry->d_name);
        if (!names[count]) {
            perror("strdup");
            for (int i = 0; i < count; ++i) free(names[i]);
            free(names);
            closedir(dp);
            return;
        }

        int len = (int)strlen(names[count]);
        if (len > max_len) max_len = len;
        count++;
    }

    if (errno != 0) {
        perror("readdir failed");
    }
    closedir(dp);

    if (count == 0) {
        free(names);
        return;
    }

    /* Sort names alphabetically for consistent output */
    qsort(names, count, sizeof(char *), name_cmp);

    /* Determine terminal width and column layout */
    int term_width = get_terminal_width();
    const int spacing = 2;
    int col_width = max_len + spacing;
    if (col_width <= 0) col_width = max_len + 2;

    int cols = term_width / col_width;
    if (cols < 1) cols = 1;
    if (cols > count) cols = count;

    int rows = (count + cols - 1) / cols; /* ceil(count / cols) */

    /* Print in "down then across" order:
       iterate rows 0..rows-1, for each row print names[row + col*rows] for col=0..cols-1 if index < count
    */
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = c * rows + r;
            if (idx >= count) continue;

            /* For last column don't pad to avoid trailing spaces */
            if (c == cols - 1) {
                printf("%s", names[idx]);
            } else {
                /* left-justify within col_width */
                printf("%-*s", col_width, names[idx]);
            }
        }
        printf("\n");
    }

    /* Free memory */
    for (int i = 0; i < count; ++i) free(names[i]);
    free(names);
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
