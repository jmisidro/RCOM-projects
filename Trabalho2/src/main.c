#include <stdio.h> 
#include "macros.h"
#include "app.h"

int main(int argc, char *argv[]) {
    if(argc != 2) { 
        printf("Usage: %s %s \n", argv[0], "ftp://{user}:{password}@{host}/{url-path}");
        return -1;
    }
       
    printf("\n--------- Verifying FTP parameters ---------\n\n");

    // parse command line arguments
    struct FTPparameters params;
    if(parseArguments(&params, argv[1]) != 0) {
        return -1;
    }

    printf("User: %s\n", params.user);
    printf("Password: %s\n", params.password);
    printf("Host name: %s\n", params.host_name);
    printf("File path: %s\n", params.file_path);

    printf("\n--------- FTP parameters verified ----------\n\n");


    struct FTP ftp;
    char command[MAX_LENGTH];           // buffer to send commands
    char response[MAX_LENGTH];    // buffer to read the response from commands

    // get IP Address
    char ipAddress[MAX_LENGTH];
    if (getIPAddress(ipAddress, params.host_name) < 0) {
        return -1;
    }

    printf("\n----- Connecting to new control Socket -----\n\n");

    printf("IP Address: %s\n", ipAddress);
    printf("Port: %d\n", FTP_PORT_NUMBER);

    // create and connect socket to server
    if ((ftp.control_socket_fd = openAndConnectSocket(ipAddress, FTP_PORT_NUMBER)) < 0) {
        printf("Error opening new socket\n");
        return -1;
    }
    printf("\n----- Connected to new control Socket ------\n\n");

    if (sendToControlSocket(&ftp, "user", params.user) < 0) {
        return -1;
    }

    return 0;
}
