#!/bin/bash

# Compile main.c and all .c files in the utils folder
gcc main.c  $(find utils -name "*.c") $(find routes -name "*.c")  $(find db -name "*.c") -o server -lsqlite3

# Run the File
exec ./server 
