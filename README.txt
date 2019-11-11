Erik Safford
SetUID File Transfer(copy) System
CS 426 

The overall structure of the project contains two main parts:
 -addConfig: Create/add to a .acl configuration file for a specified protected file
 -scopy: Executable put in a public directory so another user can use it to copy a protected file

To configure my scopy system:
    As Alice (owner of protected file):
        -In the directory of the protected file, create a .acl configuration file by running:
            './addConfig protectedFile.txt' to create a blank config file in that directory OR
            './addConfig protectedFile.txt username permission' to create/append specified user permission to the config file in that directory
            username must be alphanumeric characters, and permission must be r (read), w (write), or b (both).
            Both of the above commands will create 'protectedFile.txt.acl' with only owner access.
        -Place a copy of 'scopy' executable in Alice's public directory where Bob can access it,
         ensuring to run 'chmod u+s scopy' beforehand to set the seteuid bit on the executable so Bob can run it.
    
    As Bob (person trying to copy protected file):
        -From whereever in Bob's filesystem where he wants the protected file to be copied:
         '/home/Alice/Public/scopy filepathOfProtectedFile.txt fileCopy.txt'
         where Bob is accessing the scopy executable in Alice's public directory, copying protected file
         'filepathOfProtectedFile.txt' into 'filecopy.txt' in Bob's current directory.

         Another Example:
         '/home/erik/Public/scopy /home/erik/Documents/SetUID-File-Copy-System/file.txt filecopy.txt'
         This copies erik's protected file 'file.txt' to 'filecopy.txt' in the user's current directory.

To protect the security of the owner's (Alice) protected files, a .acl configuration file is created in the same
directory as the protected file, identifying who has access to the file, and what kind of access (read, write, both)
they have. Each line of the configuration file must begin with a username made of alphanumneric characters, then a single space,
and then a 'r' (read), 'w' (write), or 'b' (both) permission, followed by a single '\n'. Incorrectly formatted .acl configuration
files will not allow protected file copying to occur. Checks are also in place to ensure that .acl config files and protected 
files are not symbolic links and are only regular files. The .acl files and newly copied protected files are also chmod'd so
only the owner has read/write access to them.

No third party library installations should be necessary for this system, short of a C compiler. 