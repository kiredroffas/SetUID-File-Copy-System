/*  Erik Safford
    SetUID File Transfer(copy) System
    CS 426                            

    Adds a configuration file into the directory of the file being shared,
    identifies user access to the file and ensures that the file being shared
    exists and is only accessible by the owner.  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#define FILEPATH_SIZE 100
#define FILELINE_SIZE 100

int main(int argc, char *argv[]) {
    if(argc != 2 && argc != 4) {
        fprintf(stderr, "Error: Too few/many command line arguments\n");
        fprintf(stderr, "Proper format is ./addConfig filepathOfFile username permission\n");
        exit(1);
    }

    char filepath[FILEPATH_SIZE];
    char aclfilepath[FILEPATH_SIZE];
        
    //Second command line arg is the filepath of the file to be copied
    //Add .acl to the filepath (path of new config file)
    strcpy(filepath, argv[1]);
    strcpy(aclfilepath, argv[1]);
    strcat(aclfilepath, ".acl");

    //If only a filepath is given, create blank config file with no permissions
    if(argc == 2) {
        FILE *fp = NULL;
        
        //Make sure protected file exists
        if( (fp = fopen(filepath, "r")) == NULL) { 
            perror("Error checking protected file");
            exit(1);
        }
        fclose(fp);
        printf("protected file exists\n");

        //chmod protected file so only owner has read/write access (just in case this wasn't done already)
        if(chmod(filepath, S_IRUSR | S_IWUSR) != 0) {
            perror("protected file chmod error");
            exit(1);
        }

        //Attempt to create config file or open it if it already exists
        if( (fp = fopen(aclfilepath, "a+")) == NULL) {
            perror("Error creating/opening config file");
            exit(1);
        }

        //chmod config file so only owner has read/write access (protect from 3rd parties)
        if(chmod(aclfilepath, S_IRUSR | S_IWUSR) != 0) {
            perror("config file chmod error");
            fclose(fp);
            exit(1);
        }

        //Print contents of the config file
        char line[FILELINE_SIZE];
        printf("\n%s now contains:\n", aclfilepath);
        while(fgets(line, sizeof(line), fp) != NULL) {
            printf("%s", line);
        }
        printf("\nBlank .acl file created, run with 4 command line arguments to add user permissions\n");
        printf("Proper format to add permissions is: ./addConfig filepathOfFileToBeCopied username permission\n");
        fclose(fp);
    }
    //Else if a filepath is given with included username and permission to add
    else if(argc == 4) {
        FILE *fp = NULL;

        //Third command line arg is the username of the user to give permissions to
        char *username = argv[2];
        //Fourth command line arg is the permission to give to the user of the file (r=read, w=write, b=both)
        char *permission = argv[3];

        //Make sure protected file exists
        if( (fp = fopen(filepath, "r")) == NULL) { 
            perror("Error checking protected file");
            exit(1);
        }
        fclose(fp);
        printf("protected file exists\n");

        //chmod protected file so only owner has read/write access (just in case this wasn't done already)
        if(chmod(filepath, S_IRUSR | S_IWUSR) != 0) {
            perror("protected file chmod error");
            exit(1);
        }

        //Check to make sure permission is read, write, or both
        if(strcmp(permission, "r") != 0 && strcmp(permission, "w") != 0 && strcmp(permission, "b") != 0) {
            fprintf(stderr, "Error: Invalid specified permission, must be r (read), w (write), or b (both)\n");
            exit(1);
        }
        printf("Attempting to add '%s' to the config file with '%s' permission\n", username, permission);

        //Attempt to create config file or open it if it already exists
        if( (fp = fopen(aclfilepath, "a+")) == NULL) {  //Open file with reading/appending at end of file (a+)
            perror("Error creating/opening config file");
            exit(1);
        }

        //chmod config file so only owner has read/write access (protect from 3rd parties)
        if(chmod(aclfilepath, S_IRUSR | S_IWUSR) != 0) {
            perror("config file chmod error");
            fclose(fp);
            exit(1);
        }

        //Add new username and permission to end of config file
        fprintf(fp, "%s %s\n", username, permission);
        
        //Print contents of the config file
        char line[FILELINE_SIZE];
        rewind(fp);  //Reset fp to beginning of the file
        printf("%s successfully created\n", aclfilepath);
        printf("\n%s now contains:\n", aclfilepath);
        while(fgets(line, sizeof(line), fp) != NULL) {  //Reads until \n or EOF
            printf("%s", line);
        }
        fclose(fp);
    }
}