/*  Erik Safford
    SetUID File Transfer(copy) System
    CS 426                       */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Error: Invalid number of command line arguments\n");
        fprintf(stderr, "Proper format is: ./scopy protected_file my_copy\n");
        exit(1);
    }
    char *protected_file = argv[1];
    char *copy_to_be_made = argv[2];
    printf("protected file to be accessed: %s\n", protected_file);
    printf("copy of protected file to be made: %s\n", copy_to_be_made);
    return(0);
}