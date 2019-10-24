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
       - run the following command: ./scopy protected_file my_copy
       Example:
       >bob@wopr $ /home/alice/public/scopy /home/alice/daily_schedule.txt alices_schedule.txt
      
       (Assume that Bob can't read /home/alice/dailyschedule.txt directly)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#define FILEPATH_SIZE 100
#define FILELINE_SIZE 100

char *getlogin(void);
int seteuid(uid_t uid);

int main(int argc, char *argv[]) {
    //Check for valid command line arguments
    if(argc < 3) {
        fprintf(stderr, "Error: Invalid number of command line arguments\n");
        fprintf(stderr, "Proper format is: ./scopy protected_file my_copy\n");
        exit(1);
    }

    //Get file to access and filename copy to be made from command line
    char protected_file[FILEPATH_SIZE];
    char copy_to_be_made[FILEPATH_SIZE];
    strcpy(protected_file, argv[1]);
    printf("protected file to be accessed: %s\n", protected_file);
    strcpy(copy_to_be_made, argv[2]);
    printf("copy of protected file to be made: %s\n", copy_to_be_made);

    //Get username of the user to check permission for in local file access control list
    char *username;
    if( (username = getlogin()) == NULL) {
        perror("error getting username of user");
        exit(1);
    } 
    printf("user attempting to copy file is: %s\n", username);

    //Open .acl configuration file to check for active user read permissions
    FILE *fp = NULL;
    char configFile[FILELINE_SIZE];
    strcpy(configFile, protected_file);
    strcat(configFile, ".acl");

    if( (fp = fopen(configFile, "r")) == NULL) {
        perror("Error opening .acl configuration file");
        fprintf(stderr, "%s could not be accessed\n", configFile);
        exit(1);
    }
    printf("%s opened successfully\n", configFile);

    //Search .acl file for username's file permission (r, w, b)
    char line[FILELINE_SIZE];
    char read[FILELINE_SIZE]; char write[FILELINE_SIZE]; char both[FILELINE_SIZE];
    strcpy(read, username);
    strcat(read, " r\n");
    strcpy(write, username);
    strcat(write, " w\n");
    strcpy(both, username);
    strcat(both, " b\n");
    printf("Looking for:\n %s %s or %s in .acl config file\n\n", read, write, both);

    printf(".acl config file contains: \n");
    int access = 0;  //0 = false, 1 = true
    //Read/compare each line from .acl file
    while(fgets(line, sizeof(line), fp) != NULL) { 
        printf(line);
        if(strcmp(line, read) == 0 || strcmp(line, both) == 0) {
            printf("Matching config username found: %s", line);
            int permissionIndex = strlen(username) + 1;
            printf("User '%s' has permission '%c'\n", username, line[permissionIndex]);
            access = 1;
            break;
        }
    }

    //If the user's username was not found to have valid read/both permission in the .acl config file
    if(access == 0) {
        fprintf(stderr, "%s does not have valid permissions to copy this file\n", username);
        fclose(fp);
        exit(1);
    }
    //Else the user has read/both permission to the file, commence copying of the file
    else {
        printf("File copying good to go!\n");
        fclose(fp);
        
        seteuid(1000);
        // printf("Real user id = %d, Effective User id = %d\n",getuid(),geteuid());
    }

    //fclose(fp);
    
    return(0);
}