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

    printf("IP Address: %s\n", address);
    printf("Port: %d\n", port);

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

    /* verifying FTP protocol */
    char *token = strtok(commandLineArg, ":");
    if ((token == NULL) || (strcmp(token, "ftp") != 0)) {
        printf("> Error in the protocol name (should be 'ftp')\n");
        return -1;
    }

    token = strtok(NULL, "\0");
    char string[MAX_LENGTH];
    strcpy(string, token);

    char aux[MAX_LENGTH];
    strcpy(aux, string);
    token = strtok(aux, ":");

    /* verifying user and password */
    if (token == NULL || (strlen(token) < 3) || (token[0] != '/') || (token[1] != '/')) {
        printf("> Error while parsing the user name\n");
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
        /* parsing user name */
        strcpy(params->user, &token[2]);
        /* parsing password */
        token = strtok(NULL, "@");
        if (token == NULL || (strlen(token) == 0)) {
            printf("> Error while parsing the password\n");
            return -1;
        }
        strcpy(params->password, token);

        token = strtok(NULL, "\0");
        strcpy(string, token);
    }

    /* parsing host name */
    token = strtok(string, "/");    
    if (token == NULL) {
        printf("> Error parsing the hostname\n");
        return -1;
    }
    strcpy(params->host_name, token);

    /* parsing file path and file name */ 
    token = strtok(NULL, "\0");
    if (token == NULL) {
        printf("> Error while parsing the file path\n");
        return -1;
    }
    char* last = strrchr(token, '/');
    if (last != NULL) { // path is set
        strncpy(params->file_path, token, last - token);
        strcpy(params->file_name, last + 1);
    }
    else {  // path is not set
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


int readReplyFromControlSocket(struct FTP *ftp, char *buffer) {

    printf("Reading reply from control Socket... \n");

    FILE *fp = fdopen(ftp->control_socket_fd, "r");
    do {
        /* reset the memory of the buffer in each iteration */
        memset(buffer, 0, MAX_LENGTH);                    // reads until:
        buffer = fgets(buffer, MAX_LENGTH, fp);          // - reaches end of reply  
        printf("> %s", buffer);                   // - reply code is not correct (fail safe)
    } while (buffer[3] != ' ' || !('1' <= buffer[0] && buffer[0] <= '5'));
    
    return 0;
}


int sendCommandHandleReply(struct FTP *ftp, char *command, char *argument, char *reply, int dowloadingFile) {

    if (sendCommandToControlSocket(ftp, command, argument) < 0) {
        printf("> Error while sending command  %s %s\n", command, argument);
        return -1;
    }
    
    int code;
    while (1) {
        readReplyFromControlSocket(ftp, reply);
        code = reply[0] - '0'; // first digit of reply code
        switch (code) {
            case 1:
                // expecting another reply
                if (dowloadingFile) return 1; // if downloading a file, reply will be 150
                else break;                   // else expect another reply
            case 2:
                // requested action successful
                return 2;
            case 3:
                // needs aditional information
                return 3;
            case 4:
                // try resending the command
                if (sendCommandToControlSocket(ftp, command, argument) < 0) {
                    printf("> Error while sending command  %s %s\n", command, argument);
                    return -1;
                }
                break;
            case 5:
                // Command was not accepted, close control socket & exit application
                printf("> Command was not accepted \n");
                close(ftp->control_socket_fd);
                exit(-1);
                break;
            default:
                break;
        }
    }
}


int login(struct FTP *ftp, char *user, char *password) {

    char* reply = (char *) malloc(MAX_LENGTH);
    
    printf("Sending User...\n\n");
    /* ret is the return value corresponding to the first digit of the reply code */
    int ret = sendCommandHandleReply(ftp, "user", user, reply, FALSE);
    if (ret != 3) {
        printf("> Error while sending User...\n\n");
        return -1;
    }

    printf("\nSending Password...\n\n");
    /* ret is the return value corresponding to the first digit of the reply code */
    ret = sendCommandHandleReply(ftp, "pass", password, reply, FALSE);
    if (ret != 2) {
        printf("> Error while sending Password...\n\n");
        return -1;
    }

    free(reply);

    return 0;
}


int changeWorkingDirectory(struct FTP* ftp, char* path) {

    char* reply = (char *) malloc(MAX_LENGTH);

    /* ret is the return value corresponding to the first digit of the reply code */
    int ret = sendCommandHandleReply(ftp, "cwd", path, reply, FALSE);
    if (ret != 2) {
        printf("> Error while sending command cwd\n\n");
        return -1;
    }

    free(reply);

	return 0;
}


int enablePassiveMode(struct FTP *ftp) {

    char reply[MAX_LENGTH];

    int ret = sendCommandHandleReply(ftp, "pasv", "", reply, FALSE);
    int ipPart1, ipPart2, ipPart3, ipPart4;
    int portPart1, portPart2;
    if (ret == 2) {
        // process ip and port informaiton
        if ((sscanf(reply, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
                    &ipPart1, &ipPart2, &ipPart3, &ipPart4, &portPart1, &portPart2)) < 0) {
            printf("> Error while calculating port.\n");
            return -1;
        }
    }
    else {
        printf("> Error while sending command pasv \n\n");
        return -1;
    }
    
    char* ipAddress = (char *) malloc(MAX_LENGTH);
    sprintf(ipAddress, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4);
    int port = portPart1 * 256 + portPart2;

    printf("\nConnecting to new data socket...\n");

    if ((ftp->data_socket_fd = openAndConnectSocket(ipAddress, port)) < 0) {
        printf("> Error while creating new data socket\n");
        return -1;
    }

    free(ipAddress);

    return 0;
}
