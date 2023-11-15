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

    // verifying FTP protocol
    char *token = strtok(commandLineArg, ":");
    if ((token == NULL) || (strcmp(token, "ftp") != 0)) {
        printf("-> Error in the protocol name (should be 'ftp')\n");
        return -1;
    }

    token = strtok(NULL, "\0");
    char string[MAX_LENGTH];
    strcpy(string, token);

    char aux[MAX_LENGTH];
    strcpy(aux, string);
    token = strtok(aux, ":");

    if (token == NULL || (strlen(token) < 3) || (token[0] != '/') || (token[1] != '/')) {
        printf("-> Error parsing the user name\n");
        return -1;
    }
    else if (strcmp(token, string) == 0) { 
        // if user and password are not set, set them to anonymous
        strcpy(params->user, "anonymous");
        strcpy(params->password, "");

        char aux2[MAX_LENGTH];
        strcpy(aux2, &string[2]);
        strcpy(string, aux2);
    }
    else {
        // parsing user name
        strcpy(params->user, &token[2]);
        // parsing password
        token = strtok(NULL, "@");
        if (token == NULL || (strlen(token) == 0)) {
            printf("-> Error parsing the password\n");
            return -1;
        }
        strcpy(params->password, token);

        token = strtok(NULL, "\0");
        strcpy(string, token);
    }

    // parsing host name
    token = strtok(string, "/");    
    if (token == NULL) {
        printf("-> Error parsing the hostname\n");
        return -1;
    }
    strcpy(params->host_name, token);

    // parsing file path and file name
    token = strtok(NULL, "\0");
    if (token == NULL) {
        printf("-> Error parsing the file path\n");
        return -1;
    }
    char* last = strrchr(token, '/');
    if (last != NULL) {
        strncpy(params->file_path, token, last - token);
        strcpy(params->file_name, last + 1);
    }
    else {
        strcpy(params->file_path, "");
        strcpy(params->file_name, token);
    }

    return 0;
}


int sendCommandToControlSocket(struct FTP *ftp, char *command, char *argument) {

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


int readReplyFromControlSocket(struct FTP *ftp, char *buffer, size_t size) {

    printf("Reading reply from control Socket... \n");

    FILE *fp = fdopen(ftp->control_socket_fd, "r");
    do {
        /* reset the memory of the buffer in each iteration */
        memset(buffer, 0, size);                    // reads until:
        buffer = fgets(buffer, size, fp);          // - reaches end of reply  
        printf("> %s", buffer);                   // - reply code is not correct (fail safe)
    } while (buffer[3] != ' ' || !('1' <= buffer[0] && buffer[0] <= '5'));
    
    return 0;
}
