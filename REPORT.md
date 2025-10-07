## Feature 2: Long Listing Format (-l) - Report Questions

### 1. Difference between `stat()` and `lstat()` system calls:
The crucial difference between `stat()` and `lstat()` lies in how they handle symbolic links:
- `stat()` returns information about the file the symbolic link points to (the target).
- `lstat()` returns information about the symbolic link itself.

In the context of the `ls` command, it is more appropriate to use `lstat()` when you want to display information about the symbolic link rather than its target, especially to distinguish links from regular files or directories in the long listing format.

---

### 2. Extracting file type and permission bits from `st_mode`:

The `st_mode` field in the `struct stat` contains both the file type and permission bits encoded as bitfields.

- To extract the **file type**, you can use bitwise AND (`&`) with the `S_IFMT` mask, then compare it with file type macros like:
  - `S_IFDIR` (directory)
  - `S_IFREG` (regular file)
  - `S_IFLNK` (symbolic link)
  - and others...

Example:

if ((fileStat.st_mode & S_IFMT) == S_IFDIR) {
    // It's a directory
}


## Feature 3: Column Display

### 1. Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop through the list of filenames insufficient for this task?

The "down then across" printing logic arranges filenames in columns where items are printed top to bottom in each column before moving to the next column horizontally. To achieve this layout, the filenames must be accessed in a specific order that corresponds to rows rather than just a simple linear list.

A simple single loop through the filenames prints them sequentially, which results in a row-wise layout (left to right, top to bottom). This does not produce the desired columnar arrangement, where each column's items are vertically aligned.

To print "down then across," the program calculates the number of rows based on the total number of files and columns. Then it prints filenames by iterating row-wise, accessing elements at indices:
[row], [row + num_rows], [row + 2 * num_rows], ..., thus printing vertically down each column before moving horizontally across columns.

----

### 2. What is the purpose of the ioctl system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size?

The ioctl system call with the TIOCGWINSZ request retrieves the current terminal window size (number of columns and rows). This allows the program to dynamically determine the terminal's width at runtime.

By knowing the terminal width, the program can calculate how many columns of filenames can fit on the screen properly, adjusting the output to the user's terminal size. This makes the display adaptive and maximizes the use of available space.

If the program only used a fixed-width fallback (like 80 columns), it would not adapt to different terminal sizes. This could cause issues such as:

Wasted space when running in wider terminals, making the output less efficient.

Wrapping or truncated output in narrower terminals, causing readability problems.

Dynamic detection ensures a better user experience across various terminal environments.
