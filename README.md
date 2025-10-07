# 🧑‍💻 BSDSF23A015 - Operating Systems Assignment
Linux ls Command Implementation in C
📋 Project Overview

This project is part of the Operating Systems course assignment.
The goal is to progressively implement and extend the functionality of the Unix ls command using C, following a version-controlled development process with Git and GitHub.

🚀 Features Implemented
Version	Feature Description
v1.0.0	Basic listing of files in a directory
v1.1.0	Long listing format (-l option)
v1.2.0	Column display for better readability
v1.3.0	Alphabetical sorting of file names
v1.4.0	Colorized output using ANSI escape codes
v1.5.0	Executable file detection and color highlighting
v1.6.0	Recursive directory listing (-R option)
🧠 Concepts Practiced

File handling in C using <dirent.h> and <sys/stat.h>

Use of recursion for directory traversal

Bitwise operations for permission checks

ANSI escape sequences for colored output

Git branching, tagging, and merging workflow

🧪 How to Compile
make

This will:

Create bin/ and obj/ directories

Compile the source code (e.g., src/ls-v1.6.0.c)

Copy the executable to bin/ls

⚙️ How to Run
./bin/ls


Optional flags:

-l → Long listing

-x → Column display

-R → Recursive listing

Example:

./bin/ls -lR /tmp/test_ls

🧾 Report

All conceptual questions, explanations, and answers for each feature are documented in REPORT.md
.

🏁 Final Git Structure

Main Branch: main

Feature Branches:
feature-long-listing-v1.1.0, feature-column-display-v1.2.0, feature-color-output-v1.4.0, feature-recursive-listing-v1.6.0, etc.


👨‍🎓 Student Info

Name: Muhammad Osama Khan

Roll No: BSDSF23A015

Course: Operating Systems (BSDS)

Instructor: Prof Dr. Arif Butt
