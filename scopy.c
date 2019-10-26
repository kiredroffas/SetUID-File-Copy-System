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
// test .acl symlink: ./scopy /home/erik/Documents/SetUID-File-Copy-System/junklink filecopy.txt
// test fileToBeCopy symlink: ./scopy /home/erik/Documents/SetUID-File-Copy-System/junkk junkktest

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#define FILEPATH_SIZE 100
#define FILELINE_SIZE 100

char *getlogin(void);

//Check a specified filepath for regular file/sumbolic link
int checkReg(char *filepath) {
    struct stat buf;
    int x;
    
    //Get information about the file with lstat()
    if((x = lstat(filepath, &buf)) != 0) {
        perror("lstat() error, file could not be checked for symlink");
        exit(1);
    }

    //Check lstat() information to see if file is regular readable file or symlink
    if(S_ISLNK(buf.st_mode)) {
        fprintf(stderr, "Error: file is a symbolic link\n");
        exit(1);
    }
    else if(S_ISREG(buf.st_mode)) {
        return(1); //return true if file is regular, exit otherwise
    }
    else {
        fprintf(stderr, "Error: file is not a regular file\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    //Check for valid command line arguments
    if(argc < 3) {
        fprintf(stderr, "Error: Invalid number of command line arguments\n");
        fprintf(stderr, "Proper format is: ./scopy protected_file my_copy\n");
        exit(1);
    }

    //Get file to access and filename copy to be made from command line
    char protectedFile[FILEPATH_SIZE];
    char copyToBeMade[FILEPATH_SIZE];
    strcpy(protectedFile, argv[1]);
    printf("protected file to be accessed: %s\n", protectedFile);
    strcpy(copyToBeMade, argv[2]);
    printf("copy of protected file to be made: %s\n", copyToBeMade);

    //Get username of the user to check permission for in local .acl config file
    char *username;
    if( (username = getlogin()) == NULL) {
        perror("error getting username of user");
        exit(1);
    } 
    printf("user attempting to copy file is: %s\n", username);

    //Create filepath of file to be copied's .acl config file
    FILE *fp = NULL;
    char configFile[FILELINE_SIZE];
    strcpy(configFile, protectedFile);
    strcat(configFile, ".acl");

    //Ensure that the filepath of the .acl configFile is not a symbolic link
    int isReg = 0;
    printf("checking if %s is a regular file...\n", configFile);
    if( (isReg = checkReg(configFile)) != 1) {
        fprintf(stderr,"checkReg(configFile) error\n");
        exit(1);
    }
    printf("%s is regular file, attempting to open for reading\n", configFile);

    //Attempt to open .acl configuration file to check for active user read permissions
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
    printf("Looking for:\n %s or %s in .acl config file\n\n", read, both);
    
    //Read/compare each line from .acl file to verify if user has access to file
    int access = 0;  //0 = false (default), 1 = true
    printf(".acl config file contains: \n");
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
        
        //Ensure that the filepath of the file to be copied is not a symbolic link
        isReg = 0;
        printf("checking if %s is a regular file...\n", protectedFile);
        if( (isReg = checkReg(protectedFile)) != 1) {
            fprintf(stderr,"checkReg(configFile) error\n");
            exit(1);
        }
        printf("%s is regular file, attempting to open for reading\n", protectedFile);

        //Attempt to open the protected file that only owner has access to
        if( (fp = fopen(protectedFile, "r")) == NULL) {
            perror("Error opening protected file: %s\n");
            fprintf(stderr, "%s could not be accessed\n", protectedFile);
            fprintf(stderr, "before: EUID: %d RUID: %d\n", EUID, RUID);
            fprintf(stderr, "after: EUID: %d RUID: %d\n", (int) geteuid(), (int) getuid());
            exit(1);
        }
        printf("protected file %s opened successfully\n", protectedFile);
        
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
        strcat(cpFilepath, copyToBeMade);
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

        printf("file %s copied to %s successfully\n", protectedFile, cpFilepath);
    }
    
    return(0);
}