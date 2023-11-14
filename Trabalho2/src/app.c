#include "app.h"

int getIPAddress(char *ipAddress, char *hostName) {
    struct hostent *h;
    if ((h = gethostbyname(hostName)) == NULL) {
        herror("gethostbyname");
        return -1;
    }
    strcpy(ipAddress, inet_ntoa(*((struct in_addr *)h->h_addr)));
    return 0;
}


int openAndConnectSocket(char* address, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    printf("Opening new socket...\n");

    /* server address handling */
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /* open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    printf("Connecting to new socket...\n");

    /* connect to the server */
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    return sockfd;
}


int parseArguments(struct FTPparameters* params, char* commandLineArg) {

    printf("Parsing command line arguments...\n");
    //verifying FTP protocol
    char* token = strtok(commandLineArg, ":");
    if((token == NULL) || (strcmp(token, "ftp") != 0)) {
        printf(" > Error in the protocol name (should be 'ftp')\n");
        return -1;
    }

    // parsing user name
    token = strtok(NULL, ":");
    if(token == NULL || (strlen(token) < 3) || (token[0] != '/') || (token[1] != '/')) {
        printf(" > Error parsing the user name\n");
        return -1;
    }
    strcpy(params->user, &token[2]);

    // parsing password
    token = strtok(NULL, "@");
    if(token == NULL || (strlen(token) == 0)) {
        printf(" > Error parsing the password\n");
        return -1;
    }
    strcpy(params->password, token);

    // parsing hostname
    token = strtok(NULL, "/");
    if(token == NULL || (strlen(token) == 0)) {
        printf(" > Error parsing the host name\n");
        return -1;
    }
    strcpy(params->host_name, token);

    // parsing file path
    token = strtok(NULL, "\0");
    if(token == NULL || (strlen(token) == 0)) {
        printf(" > Error parsing the host name\n");
        return -1;
    }
    strcpy(params->file_path, token);
    
    printf("Parsed command line arguments.\n");

    return 0;
}


int sendToControlSocket(struct FTP *ftp, char *command, char *argument) {

    printf("Sending command to control Socket: %s %s\n", command, argument);

    /* sends command's code */
    int bytes = write(ftp->control_socket_fd, command, strlen(command));
    if (bytes != strlen(command))
        return -1;
    /* sends a space to separate the command's code and argument */
    bytes = write(ftp->control_socket_fd, " ", 1);
    if (bytes != 1)
        return -1;
    /* sends command's argument */
    bytes = write(ftp->control_socket_fd, argument, strlen(argument));
    if (bytes != strlen(argument))
        return -1;
    /* sends end of line code to signal the end of the command */
    bytes = write(ftp->control_socket_fd, "\n", 1);
    if (bytes != 1)
        return -1;

    return 0;
}
