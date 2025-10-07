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
```c
if ((fileStat.st_mode & S_IFMT) == S_IFDIR) {
    // It's a directory
}
