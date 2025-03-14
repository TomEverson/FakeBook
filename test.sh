#!/bin/bash

# Compile main.c and all .c files in the utils folder
gcc main.c  $(find utils -name "*.c") $(find routes -name "*.c") -o server 

# Run the File
exec ./server 
