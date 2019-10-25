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
// Notes: 
// sudo login kired
// ./scopy /home/erik/Documents/SetUID-File-Copy-System/filecopy.haha filecopy.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#define FILEPATH_SIZE 100
#define FILELINE_SIZE 100

char *getlogin(void);

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
        printf("%s", line);
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
        
        int EUID = (int) geteuid();
        int RUID = (int) getuid();
        int RGID = (int) getgid();
        printf("your effective user id is %d\n", EUID);
        printf("your real user id is %d\n", RUID);
        printf("your real group id is: %d\n", RGID);

        //Ensure that effective UID of user is set to real UID of file (UID of owner)
        if(seteuid((int) geteuid()) != 0) {
            perror("seteuid() error");
            exit(1);
        }
        else {
            printf("your effective user id was changed to %d\n", (int) geteuid());
        }
        
        //Attempt to open the protected file that only owner has access to
        if( (fp = fopen(protected_file, "r")) == NULL) {
            perror("Error opening protected file: %s\n");
            fprintf(stderr, "%s could not be accessed\n", protected_file);
            fprintf(stderr, "before: EUID: %d RUID: %d\n", EUID, RUID);
            fprintf(stderr, "after: EUID: %d RUID: %d\n", (int) geteuid(), (int) getuid());
            exit(1);
        }
        printf("protected file %s opened successfully\n", protected_file);
        
        //Copy file into local directory as specified copy file name
        FILE *copy = NULL;
        char cpFilepath[256];
        if (getcwd(cpFilepath, sizeof(cpFilepath)) == NULL) {
            perror("getcwd() error, file could not be copied");
            exit(1);
        }
        else {
            printf("current working directory is: %s\n", cpFilepath);
        }
        //Create filepath for file copy (in the local directory)
        strcat(cpFilepath, "/");
        strcat(cpFilepath, copy_to_be_made);
        printf("copy filepath to be made: %s\n", cpFilepath);
        
        //Attempt to create the copy of the file and read contents into it
        copy = fopen(cpFilepath, "w");
        while(fgets(line, sizeof(line), fp) != NULL) { 
            printf("copying: %s", line);
            fprintf(copy, "%s", line);
        }
        fclose(fp);
        fclose(copy);

        //Set the effective UID of the user back to original real UID of user (not owner of file)
        if(seteuid(RUID) != 0) {
            perror("seteuid() error");
            exit(1);
        }
        else {
            printf("your effective user id was changed back to %d\n", RUID);
        }

        printf("file %s copied to %s successfully\n", protected_file, cpFilepath);
    }
    
    return(0);
}