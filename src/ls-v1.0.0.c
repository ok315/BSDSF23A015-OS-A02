/*
 * Programming Assignment 02: ls-v1.4.0
 * Features:
 *  - v1.1.0: Long listing (-l)
 *  - v1.2.0: Multi-column (default)
 *  - v1.3.0: Horizontal listing (-x)
 *  - v1.4.0: Alphabetical sorting (default, -l, and -x) — locale-aware
 */

#define _XOPEN_SOURCE 700

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
#include <sys/ioctl.h>
#include <locale.h>    /* added for setlocale() and strcoll() */

int long_listing = 0;
int horizontal_listing = 0;

/* Function Declarations */
void do_ls(const char *dir);
void print_long_format(const char *path, const char *filename);
bool is_dir(const char *path);

/* Get file type character */
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

/* Format file permissions (e.g., rwxr-xr-x) */
void format_permissions(mode_t mode, char out[11]) {
    out[0] = file_type_char(mode);
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
    out[10] = '\0';
}

/* Comparison function for qsort() — locale-aware alphabetical sort using strcoll */
int compare_names(const void *a, const void *b) {
    const char * const *sa = a;
    const char * const *sb = b;
    return strcoll(*sa, *sb);
}

/* Get terminal width for column formatting */
static int get_terminal_width(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 80;
}

/* Horizontal printing for -x flag */
void print_horizontal(char **names, int count) {
    int term_width = get_terminal_width();
    int max_len = 0;

    for (int i = 0; i < count; ++i) {
        int len = strlen(names[i]);
        if (len > max_len) max_len = len;
    }

    int spacing = 2;
    int used = 0;

    for (int i = 0; i < count; ++i) {
        int len = strlen(names[i]);
        if (used + len > term_width - 1) {
            printf("\n");
            used = 0;
        }
        printf("%s", names[i]);
        used += len;
        if (i != count - 1 && used + spacing < term_width) {
            printf("  ");
            used += spacing;
        }
    }
    printf("\n");
}

/* Main directory listing function */
void do_ls(const char *dir) {
    DIR *dp = opendir(dir);
    if (!dp) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char **names = NULL;
    int count = 0, cap = 64;
    names = malloc(cap * sizeof(char *));
    if (!names) {
        perror("malloc");
        closedir(dp);
        return;
    }

    /* Read directory entries into array */
    while ((entry = readdir(dp))) {
        if (entry->d_name[0] == '.') continue; // skip hidden files
        if (count >= cap) {
            cap *= 2;
            char **tmp = realloc(names, cap * sizeof(char *));
            if (!tmp) {
                perror("realloc");
                closedir(dp);
                return;
            }
            names = tmp;
        }
        names[count++] = strdup(entry->d_name);
    }
    closedir(dp);

    /* If there are names, sort them before displaying */
    if (count > 0) {
        qsort(names, count, sizeof(char *), compare_names);
    }

    /* Display output depending on mode */
    if (long_listing) {
        for (int i = 0; i < count; ++i)
            print_long_format(dir, names[i]);
    } else if (horizontal_listing) {
        print_horizontal(names, count);
    } else {
        /* Default: down-then-across */
        int term_width = get_terminal_width();
        int max_len = 0;
        for (int i = 0; i < count; ++i)
            if ((int)strlen(names[i]) > max_len)
                max_len = strlen(names[i]);

        int spacing = 2;
        int col_width = max_len + spacing;
        if (col_width <= 0) col_width = max_len + 2;
        int cols = term_width / col_width;
        if (cols < 1) cols = 1;
        if (cols > count) cols = count;
        int rows = (count + cols - 1) / cols;

        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int idx = c * rows + r;
                if (idx < count)
                    printf("%-*s", col_width, names[idx]);
            }
            printf("\n");
        }
    }

    /* Free memory */
    for (int i = 0; i < count; ++i) free(names[i]);
    free(names);
}

/* Check if path is a directory */
bool is_dir(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) return false;
    return S_ISDIR(st.st_mode);
}

/* Print long listing (-l) details */
void print_long_format(const char *path, const char *filename) {
    struct stat st;
    char full[PATH_MAX];
    snprintf(full, sizeof(full), "%s/%s", path, filename);

    if (lstat(full, &st) == -1) {
        perror("lstat");
        return;
    }

    char perms[11];
    format_permissions(st.st_mode, perms);
    printf("%s %3lu ", perms, (unsigned long)st.st_nlink);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    printf("%-8s %-8s %8lld ", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?", (long long)st.st_size);

    char timebuf[64];
    struct tm *tm = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
    printf("%s %s\n", timebuf, filename);
}

/* Parse command line options and start ls */
int main(int argc, char *argv[]) {
    /* respect user's locale for sorting rules (LC_COLLATE) */
    setlocale(LC_COLLATE, "");

    int opt;
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        if (opt == 'l')
            long_listing = 1;
        else if (opt == 'x')
            horizontal_listing = 1;
        else {
            fprintf(stderr, "Usage: %s [-l|-x] [file...]\n", argv[0]);
            exit(1);
        }
    }

    int first_arg = optind;
    if (first_arg == argc)
        do_ls(".");
    else {
        for (int i = first_arg; i < argc; ++i) {
            if (is_dir(argv[i]))
                do_ls(argv[i]);
            else
                printf("%s\n", argv[i]);
        }
    }
    return 0;
}
