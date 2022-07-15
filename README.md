# Blobby

Created bloddy, a file archiver from scratch using bit shift and C. made use of a sophisticated recursive algorithm to traverse through every file and subdirectory in a given directory.

Given the -l command line argument blobby.c should for each file in the specified blob print:
- The file/directory permissions in octal
- The file/directory size in bytes
- The file/directory pathname
```bash
> ./blobby -l examples/text_file.blob
100644    56 hello.txt
# List the details of each item in the blob called 4_files.blob, which is in the examples directory

> ./blobby -l examples/4_files.blob
100644   256 256.bin
100644    56 hello.txt
100444   166 last_goodbye.txt
100464   148 these_days.txt
```

Given the -x command line argument blobby.c should:
 - Extract the files in the specified blob.

```bash
Use your program to extract the contents of hello_world.blob.
> ./blobby -x examples/hello_world.blob
Extracting: hello.c
Extracting: hello.cpp
Extracting: hello.d
Extracting: hello.go
Extracting: hello.hs
Extracting: hello.java
Extracting: hello.js
Extracting: hello.pl
Extracting: hello.py
Extracting: hello.rs
Extracting: hello.s
Extracting: hello.sh
Extracting: hello.sql
```

Given the -c command line argument blobby.c should:
 - Create a blob containing the specified files.

```bash
> ./blobby -c selamat.blob hello.txt hola.txt hi.txt
Adding: hello.txt
Adding: hola.txt
Adding: hi.txt
# List the contents of selamat.blob.
```
