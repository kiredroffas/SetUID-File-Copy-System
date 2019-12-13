/*  Erik Safford
    SetUID File Transfer(copy) System
    CS 426                            

    The purpose of this program is to allow a user to copy another user's files according to SetUID file permissions.
   
    If Alice wants to make her schedule available to Bob:
       - She places a copy of scopy in her public directory at /home/alice/public/scopy
       - Verifies that Bob will be able to access it via .acl configuration file
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
// /home/erik/Public/scopy /home/erik/Documents/SetUID-File-Copy-System/filecopy.txt filecopy.txt
// test .acl symlink: /home/erik/Public/scopy /home/erik/Documents/SetUID-File-Copy-System/junklink filecopy.txt
// test fileToBeCopy symlink: /home/erik/Public/scopy /home/erik/Documents/SetUID-File-Copy-System/junkk junkktest

#define DEBUG 0  //0 = no print statements, 1 = all print statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#define FILEPATH_SIZE 200
#define FILELINE_SIZE 100

char *getlogin(void);
size_t strlcat(char *dest, const char *src, size_t size);
size_t strlcpy(char *dest, const char *src, size_t size); 

// Version of strlcat() so don't have to worry about linking library
#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t size) {
    size_t srclen;   // Length of source string
    size_t dstlen;   // Length of destination string 

    // Figure out how much room is left...
    dstlen = strlen(dst);
    size -= dstlen + 1;

    // If no room, return immediately
    if (!size) {
        return (dstlen); 
    }

    // Figure out how much room is needed
    srclen = strlen(src);

    // Copy the appropriate amount...
    if (srclen > size)
        srclen = size;
        memcpy(dst + dstlen, src, srclen);
        dst[dstlen + srclen] = '\0';
        return (dstlen + srclen);
}
#endif

// Version of strlcpy() so don't have to worry about linking library
#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size) {
    size_t srclen;   // Length of source string

    // Figure out how much room is needed
    size --;
    srclen = strlen(src);

    // Copy the appropriate amount
    if (srclen > size) {
        srclen = size;
    }
    memcpy(dst, src, srclen);
    dst[srclen] = '\0';
    return (srclen);
}
#endif

//Check a specified filepath for regular file/symbolic link
int checkReg(char *filepath) {
    struct stat buf;
    int x;

    //Get information about the file with lstat()
    if ((x = lstat(filepath, &buf)) != 0) {
        perror("lstat() error, file could not be checked for symlink");
        exit(1);
    }

    //Check lstat() information to see if file is regular readable file or symlink
    if (S_ISLNK(buf.st_mode)) {
        fprintf(stderr, "Error: file is a symbolic link\n");
        exit(1);
    }
    else if (S_ISREG(buf.st_mode)) {
        return (1); //return true if file is regular, exit otherwise
    }
    else {
        fprintf(stderr, "Error: file is not a regular file\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    //Store effective (Alice) and real (Bob) UID's for later seteuid() use
    int EUID = (int)geteuid();
    int RUID = (int)getuid();
    if(DEBUG) printf("your effective user id is %d\n", EUID);
    if(DEBUG) printf("your real user id is %d\n", RUID);

    //Set the effective UID of user to real UID of user (Bob), initially set to Alice
    if (seteuid(RUID) != 0) {
        perror("seteuid() error");
        exit(1);
    }
    else {
        if(DEBUG) printf("your effective user id was changed to %d\n", RUID);
    }

    //Check for valid command line arguments
    if (argc < 3) {
        fprintf(stderr, "Error: Invalid number of command line arguments\n");
        fprintf(stderr, "Proper format is: ./scopy protected_file my_copy\n");
        exit(1);
    }

    //Get file to access and filename copy to be made from command line
    char protectedFile[FILEPATH_SIZE];
    char copyToBeMade[FILEPATH_SIZE];
    if(strlcpy(protectedFile, argv[1], sizeof(protectedFile)) >= sizeof(protectedFile)) {
        fprintf(stderr, "strlcpy error");
        exit(1);
    }
    printf("protected file to be accessed: %s\n", protectedFile);
    if(strlcpy(copyToBeMade, argv[2], sizeof(copyToBeMade)) >= sizeof(copyToBeMade)) {
        fprintf(stderr, "strlcpy error");
        exit(1);
    }
    printf("copy of protected file to be made: %s\n", copyToBeMade);

    //Get username of the user to check permission for in local .acl config file
    char *username;
    if ((username = getlogin()) == NULL) {
        perror("error getting username of user");
        exit(1);
    }
    printf("user attempting to copy file is: %s\n", username);

    //Create .acl filepath of file to be copied's config file
    FILE *fp = NULL;
    char configFile[FILELINE_SIZE];
    if(strlcpy(configFile, protectedFile, sizeof(configFile)) >= sizeof(configFile)) {
        fprintf(stderr, "strlcpy error");
        exit(1);
    }
    if(strlcat(configFile, ".acl", sizeof(configFile)) >= sizeof(configFile)) {
        fprintf(stderr, "strlcat error");
        exit(1);
    }

    //Since Alice is owner of .acl config file, must seteuid() back to Alice to lstat()/fopen()
    if (seteuid(EUID) != 0) {
        perror("seteuid() error");
        exit(1);
    }
    else {
        if(DEBUG) printf("your effective user id was changed to %d\n", EUID);
    }

    //Ensure that the filepath of the .acl configFile is not a symbolic link
    int isReg = 0;
    printf("checking if %s is a regular file...\n", configFile);
    if ((isReg = checkReg(configFile)) != 1) {
        fprintf(stderr, "checkReg(configFile) error\n");
        exit(1);
    }
    printf("%s is regular file, attempting to open for reading\n", configFile);

    //Attempt to open .acl configuration file to check for active user read permissions
    if ((fp = fopen(configFile, "r")) == NULL) {
        perror("Error opening .acl configuration file");
        fprintf(stderr, "%s could not be accessed\n", configFile);
        exit(1);
    }
    printf("%s opened successfully\n", configFile);

    //.acl file checked/opened, change effective UID back to Bob
    if (seteuid(RUID) != 0) {
        perror("seteuid() error");
        exit(1);
    }
    else {
        if(DEBUG) printf("your effective user id was changed to %d\n", RUID);
    }

    //Ensure that the opened .acl config file contains no invalid entries
    char line[FILELINE_SIZE];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if(DEBUG) printf("checking: %s", line);

        //find the index of the space and count of spaces/newline in the line to determine how long username is
        int spaceIndex = -1;
        int spaceCount = 0;
        for (int i = 0; i < strlen(line); i++) {
            if (isspace(line[i]) != 0) { //If the char in the line is a space or \n
                if (spaceIndex == -1) {
                    spaceIndex = i; //Only want to save the first occuring single space
                }
                spaceCount++; //space count will keep track of space and \n
            }
        }
        if(DEBUG) printf("space index: %d, space count: %d\n", spaceIndex, spaceCount);
        if (spaceCount != 2) {  //isspace() should return true twice for the single space and the \n
            fprintf(stderr, ".acl config file line '%s' contains invalid entries (too many/few spaces)\n", line);
            exit(1);
        }

        //check to make sure username (before the single space) contains only alphanumeric characters
        for (int i = 0; i < spaceIndex; i++) {
            if(DEBUG) printf("i = %d\n", i);
            if (isalnum(line[i]) == 0) { //If a char in the line is not a alphanumeric character, error
                fprintf(stderr, ".acl config file line '%s' contains invalid entries (non-alphanumeric username)\n", line);
                exit(1);
            }
        }

        //check to make sure that after the single space is a valid r, w, or b followed by a \n
        if(DEBUG) printf("strlen %d\n", (int)strlen(line));
        if (line[strlen(line) - 2] != 'r' && line[strlen(line) - 2] != 'w' && line[strlen(line) - 2] != 'b') {
            fprintf(stderr, ".acl config file line '%s' contains invalid entries (invalid permission/spacing)\n", line);
            exit(1);
        }
        if (line[strlen(line) - 1] != '\n') {
            fprintf(stderr, ".acl config file line '%s' contains invalid entries (missing newline)\n", line);
            exit(1);
        }

        //check if there is garbage between the single space and the read permission/newline
        //Ex: user w\n     -> strlen = 7, spaceIndex = 4    -> difference of 3         -> valid
        //Ex: user d%s2w\n -> strlen = 11, spaceIndex = 4   -> difference other then 3 -> invalid
        if (strlen(line) - spaceIndex != 3) {
            fprintf(stderr, ".acl config file line '%s' contains invalid entries (garbage before permission/invalid spacing)\n", line);
            exit(1);
        }
    }

    //Search .acl file for username's file permission (r, w, b)
    char read[FILELINE_SIZE];
    char write[FILELINE_SIZE];
    char both[FILELINE_SIZE];
    if(strlcpy(read, username, sizeof(read)) >= sizeof(read)) {
        fprintf(stderr, "strlcpy error");
        exit(1);
    }
    if(strlcat(read, " r\n", sizeof(read)) >= sizeof(read)) {
        fprintf(stderr, "strlcat error");
        exit(1);
    }
    if(strlcpy(write, username, sizeof(write)) >= sizeof(write)) {
        fprintf(stderr, "strlcpy error");
        exit(1);
    }
    if(strlcat(write, " w\n", sizeof(write)) >= sizeof(write)) {
        fprintf(stderr, "strlcat error");
        exit(1);
    }
    if(strlcpy(both, username, sizeof(both)) >= sizeof(both)) {
        fprintf(stderr, "strlcpy error");
        exit(1);
    }
    if(strlcat(both, " b\n", sizeof(both)) >= sizeof(both)) {
        fprintf(stderr, "strlcat error");
        exit(1);
    }
    printf("Looking for:\n %s or %s in .acl config file\n\n", read, both);

    int access = 0; //0 = false (default), 1 = true
    rewind(fp);     //rewind file pointer to beginning of file
    if(DEBUG) printf(".acl config file contains: \n");
    while (fgets(line, sizeof(line), fp) != NULL) {
        //Read/compare each line from .acl file to verify if user has access to file
        if(DEBUG) printf("%s", line);
        if (strcmp(line, read) == 0 || strcmp(line, both) == 0) {
            printf("Matching config username found: %s", line);
            int permissionIndex = strlen(username) + 1;
            printf("User '%s' has permission '%c'\n", username, line[permissionIndex]);
            access = 1;
            break;
        }
    }

    //If the user's username was not found to have valid read/both permission in the .acl config file
    if (access == 0) {
        fprintf(stderr, "%s does not have valid permissions to copy this file\n", username);
        fclose(fp);
        exit(1);
    }
    //Else the user has read/both permission to the file, commence copying of the file
    else {
        printf("File copying good to go!\n");
        fclose(fp); //Close fp to .acl config file

        //Ensure that effective UID of user is set to real UID of file owner (Alice)
        if (seteuid(EUID) != 0) {
            perror("seteuid() error");
            exit(1);
        }
        else {
            if(DEBUG) printf("your effective user id was changed to %d\n", EUID);
        }

        //Ensure that the filepath of the file to be copied is not a symbolic link
        isReg = 0;
        printf("checking if %s is a regular file...\n", protectedFile);
        if ((isReg = checkReg(protectedFile)) != 1) {
            fprintf(stderr, "checkReg(configFile) error\n");
            exit(1);
        }
        printf("%s is regular file, attempting to open for reading\n", protectedFile);

        //Attempt to open the protected file that only owner has access to
        if ((fp = fopen(protectedFile, "r")) == NULL) {
            perror("Error opening protected file: %s\n");
            fprintf(stderr, "%s could not be accessed\n", protectedFile);
            fprintf(stderr, "before: EUID: %d RUID: %d\n", EUID, RUID);
            fprintf(stderr, "after: EUID: %d RUID: %d\n", (int)geteuid(), (int)getuid());
            exit(1);
        }
        printf("protected file %s opened successfully\n", protectedFile);

        //protected file checked/opened, set the effective UID of the user back to original real UID of user (Bob)
        if (seteuid(RUID) != 0) {
            perror("seteuid() error");
            exit(1);
        }
        else {
            if(DEBUG) printf("your effective user id was changed to %d\n", RUID);
        }

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
        if(strlcat(cpFilepath, "/", sizeof(cpFilepath)) >= sizeof(cpFilepath)) {
            fprintf(stderr, "strlcat error");
            exit(1);
        }
        if(strlcat(cpFilepath, copyToBeMade, sizeof(cpFilepath)) >= sizeof(cpFilepath)) {
            fprintf(stderr, "strlcat error");
            exit(1);
        }
        printf("copy filepath to be made: %s\n", cpFilepath);

        //Attempt to create the copy of the file and read contents into it
        if ((copy = fopen(cpFilepath, "w")) == NULL) {
            perror("copy filepath fopen() error");
            exit(1);
        }
        while (fgets(line, sizeof(line), fp) != NULL) {
            if(DEBUG) printf("copying: %s", line);
            fprintf(copy, "%s", line);
        }
        fclose(fp);

        //chmod newly copied file so only owner (Bob) has read/write access (protect from 3rd parties)
        if (chmod(cpFilepath, S_IRUSR | S_IWUSR) != 0) {
            perror("copied file chmod error");
            fclose(fp);
            exit(1);
        }
        printf("file chmod 700 successfull\n");
        fclose(copy);

        printf("file %s copied to %s successfully\n", protectedFile, cpFilepath);
    }

    return (0);
}
