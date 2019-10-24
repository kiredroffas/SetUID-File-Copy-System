/*  Erik Safford
    SetUID File Transfer(copy) System
    CS 426                            

    Adds a configuration file into the directory of the file being shared,
    identifies user access to the file  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define FILEPATH_SIZE 100
#define FILELINE_SIZE 100

int main(int argc, char *argv[]) {
    //If only a filepath is given, create blank config file with no permissions
    if(argc == 2) {
        FILE *fp = NULL;
        char filepath[FILEPATH_SIZE];
        
        //Second command line arg is the filepath of the file to be copied
        //Add .acl to the filepath (path of new config file)
        strcpy(filepath, argv[1]);
        strcat(filepath, ".acl");
        
        //Attempt to create config file or open it if it already exists
        if( (fp = fopen(filepath, "a+")) == NULL) {
            perror("Error creating/opening config file");
            exit(1);
        }

        //Print contents of the config file
        char line[FILELINE_SIZE];
        printf("\n%s now contains:\n", filepath);
        while(fgets(line, sizeof(line), fp) != NULL) {
            printf(line);
        }
        printf("\nRun with 4 command line arguments to add user permissions\n");
        printf("Proper format is ./addConfig filepath_of_file_to_be_copied username permission\n");
        fclose(fp);
    }
    //Else if a filepath is given with included username and permission to add
    else if(argc == 4) {
        FILE *fp = NULL;
        char filepath[FILEPATH_SIZE];
        
        //Second command line arg is the filepath of the file to be copied
        //Add .acl to the filepath (path of new config file)
        strcpy(filepath, argv[1]);
        strcat(filepath, ".acl");
        //Third command line arg is the username of the user to give permissions to
        char *username = argv[2];
        //Fourth command line arg is the permission to give to the user of the file (r=read, w=write, b=both)
        char *permission = argv[3];
        //Check to make sure permission is read, write, or both
        if(strcmp(permission, "r") != 0 && strcmp(permission, "w") != 0 && strcmp(permission, "b") != 0) {
            fprintf(stderr, "Error: Invalid specified permission, must be r (read), w (write), or b (both)\n");
            exit(1);
        }
        printf("Attempting to add '%s' to the config file with '%s' permission\n", username, permission);

        //Attempt to create config file or open it if it already exists
        if( (fp = fopen(filepath, "a+")) == NULL) {  //Open file with reading/appending at end of file (a+)
            perror("Error creating/opening config file");
            exit(1);
        }

        //Add new username and permission to end of config file
        fprintf(fp, "%s %s\n", username, permission);
        
        //Print contents of the config file
        char line[FILELINE_SIZE];
        rewind(fp);  //Reset fp to beginning of the file
        printf("\n%s now contains:\n", filepath);
        while(fgets(line, sizeof(line), fp) != NULL) {  //Reads until \n or EOF
            printf(line);
        }
        fclose(fp);
    }
    //Else incorrect amount of command line arguments
    else {
        fprintf(stderr, "Error: Too few/many command line arguments\n");
        fprintf(stderr, "Proper format is ./addConfig filepath_of_file username permission\n");
        exit(1);
    }
}