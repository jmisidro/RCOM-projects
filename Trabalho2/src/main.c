#include <stdio.h> 
#include "macros.h"
#include "app.h"

int main(int argc, char *argv[]) {
    if(argc != 2) { 
        printf("Usage: %s %s", argv[0], "ftp://{user}:{password}@{host}/{url-path}");
        return -1;
    }
    // parse command line arguments
    struct FTPparameters params;
    if(parseArguments(&params, argv[1]) != 0) {
        return -1;
    }

    printf("User: %s\n", params.user);
    printf("Password: %s\n", params.password);
    printf("Host name: %s\n", params.host_name);
    printf("File path: %s\n", params.file_path);

    return 0;
}
