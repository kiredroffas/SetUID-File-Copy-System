/*  Erik Safford
    SetUID File Transfer(copy) System
    CS 426                            

    Allow a user to copy another user's files according to SetUID file permissions.
   
    If Alice wants to make her schedule available to Bob:
       - She places a copy of scopy in her public directory at /home/alice/public/scopy
       - Verifies that Bob will be able to access it
       - Sets the SetUID bit with 'chmod u+s scopy'
       - Create appropriate permission data so Bob can access her daily_schedule.txt

    When Bob wants to make a copy of Alice's file:
       - run the following command (./scopy protected_file my_copy): 
       >bob@wopr $ /home/alice/public/scopy /home/alice/daily_schedule.txt alices_schedule.txt
      
       (Assume that Bob can't read /home/alice/dailyschedule.txt directly)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILEPATH_SIZE 100

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Error: Invalid number of command line arguments\n");
        fprintf(stderr, "Proper format is: ./scopy protected_file my_copy\n");
        exit(1);
    }
    char protected_file[FILEPATH_SIZE];
    char copy_to_be_made[FILEPATH_SIZE];

    strcpy(protected_file, argv[1]);
    strcpy(copy_to_be_made, argv[2]);

    printf("protected file to be accessed: %s\n", protected_file);
    printf("copy of protected file to be made: %s\n", copy_to_be_made);

    return(0);
}