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

##Feature 4: Multi-Column Display (-x)
### 1. Comparison of Implementation Complexity

The “down-then-across” (vertical) printing logic is more complex than the “across” (horizontal) printing logic.

In the vertical layout (default mode), the program needs to:

Compute both the number of rows and columns.

Arrange filenames in a column-major order (down each column first, then across).

Handle uneven divisions when files don’t fit perfectly into the grid.

Perform index calculations such as:

idx = c * rows + r;


These steps require more pre-calculation and indexing logic.

In contrast, the horizontal layout (using -x) prints files left to right, wrapping lines when reaching terminal width.
It only needs the number of columns and uses straightforward, sequential printing logic.

✅ Conclusion:
The vertical (down-then-across) method requires more pre-calculation because it involves computing both rows and columns, while the horizontal (across) layout only depends on column width.

### 2. Display Mode Management Strategy

The program uses flag variables and conditional logic to manage the three display modes (-l, -x, and default).

During command-line argument parsing:

int long_listing = 0;
int horizontal_mode = 0;

while ((opt = getopt(argc, argv, "lx")) != -1) {
    switch (opt) {
        case 'l': long_listing = 1; break;
        case 'x': horizontal_mode = 1; break;
    }
}


After parsing:

If -l is active → program calls print_long_format() for detailed listings.

If -x is active → program calls print_horizontal() for left-to-right display.

Otherwise → program calls the default vertical printing (print_vertical() or similar logic).

This structure allows the program to decide dynamically which function to execute based on the active display flag, keeping the code clean and modular.

## Feature 5: Alphabetical Sorting

### 1: Why is it necessary to read all directory entries into memory before you can sort them?

When you perform alphabetical sorting using qsort() (or any sorting algorithm), you need all items (in this case, filenames) available in memory because sorting works by comparing and rearranging elements.
If you tried to sort while reading directly from the directory stream, you wouldn’t have access to all entries at once — you only get one entry per call to readdir(). So, to sort:

You first read all entries (e.g., filenames) using readdir() and store them in an array.

Then you apply qsort() on that array.

Finally, you print them in sorted order.

⚠️ Potential Drawbacks:

Memory Usage:
If a directory contains millions of files, all filenames must be stored in memory. This can consume a huge amount of RAM.

Performance Overhead:
Sorting a massive number of entries takes significant time (O(n log n) complexity).

Scalability Issue:
On systems with limited memory, the program could crash or slow down due to lack of available space.

I/O Delay:
Reading millions of entries first means you delay showing any output until everything is loaded and sorted.

🧠 Example Summary (for your report):

It is necessary to read all directory entries into memory before sorting because sorting requires access to the complete list of items to compare and arrange them.
The drawback is that for directories with millions of files, this approach can consume large amounts of memory, reduce performance, and may not scale efficiently on systems with limited resources.

### 2. Explain the purpose and signature of the comparison function required by qsort(). How does it work, and why must it take const void * arguments?

✅ Explanation:

The qsort() function in C is a generic sorting function.
It doesn’t know what type of data you’re sorting (strings, integers, structs, etc.), so it requires a comparison function to decide the order of elements.

🧾 Function Signature:
int compare(const void *a, const void *b);

💡 How it Works:

The compare function is called repeatedly by qsort() to determine the order of elements.

Inside it, you cast the void pointers to the actual data type you’re sorting.

It should return:

< 0 if the first element should come before the second

0 if they are equal

> 0 if the first element should come after the second

🧩 Example:

For sorting an array of strings (filenames):

int compare(const void *a, const void *b) {
    const char *strA = *(const char **)a;
    const char *strB = *(const char **)b;
    return strcmp(strA, strB);
}

🧠 Why const void *:

void * allows the comparison function to be generic, so qsort() can sort any type of data.

const means the comparison function cannot modify the actual data being compared — it only reads them to decide order.

✅ Example Summary (for your report):

The comparison function tells qsort() how to order two elements.
Its signature int compare(const void *a, const void *b) allows generic sorting of any data type.
The parameters are const void * because they represent pointers to constant data of unknown type, ensuring the comparison function does not alter the data.
The function returns a negative value if a < b, zero if equal, and positive if a > b.

## Feature 06: Colorized Output

### 1. How do ANSI escape codes work to produce color in a standard Linux terminal?

ANSI escape codes are special sequences of characters that control the formatting, color, and other output options on text terminals.
They start with the escape character \033 (or \x1B), followed by a [ and then a series of parameters ending with a letter that specifies the action.

For example, to set text color, the format is:

\033[<code>m


\033[ – starts the escape sequence

<code> – specifies the color or formatting

m – ends the sequence (for text attributes)

Example: To print text in green, you use the code 32 for foreground green.

printf("\033[32mThis text is green!\033[0m\n");


\033[32m → starts green text

\033[0m → resets to normal (removes color effect)

Some common color codes:

Color	Code
Black	30
Red	31
Green	32
Yellow	33
Blue	34
Magenta	35
Cyan	36
White	37

### 2. Which bits in st_mode determine if a file is executable by owner, group, or others?

The st_mode field (from struct stat) contains permission bits that describe file type and access rights.

To determine if a file is executable, you check these bits:

Executable by	Bit Macro	Octal Value	Meaning
Owner	S_IXUSR	0100	Execute permission for file owner
Group	S_IXGRP	0010	Execute permission for group members
Others	S_IXOTH	0001	Execute permission for everyone else

Example in C:

struct stat fileStat;
stat(filename, &fileStat);

if (fileStat.st_mode & S_IXUSR)
    printf("Executable by owner\n");
if (fileStat.st_mode & S_IXGRP)
    printf("Executable by group\n");
if (fileStat.st_mode & S_IXOTH)
    printf("Executable by others\n");

