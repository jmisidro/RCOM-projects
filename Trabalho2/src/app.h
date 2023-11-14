#ifndef _AUX_H_
#define _AUX_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "macros.h"

/**
 * Struct that contains the necessary fields to parse the command line arguments passed
 */
struct FTPparameters{
    char user[MAX_LENGTH]; /* user string */
    char password[MAX_LENGTH]; /* password string */
    char host_name[MAX_LENGTH]; /* host name string */
    char file_path[MAX_LENGTH]; /* file path string */    
};

/**
 * Struct that contains the control and data file descriptors for the FTP protocol
 */
struct FTP{
    int control_socket_fd; /* File descriptor to control socket */
    int data_socket_fd; /* File descriptor to data socket */   
};

/**
 * Function that, having the host name, retrieves the IP address
 * 
 * @param idAddress Variable that is going to point to the IP Address
 * @param hostName The host's name
 * @return int 0 if success; -1 otherwise
 */
int getIPAddress(char *ipAddress, char *hostName);

/**
 * Function that opens a new TCP socket, and connects it to the address and port specified
 * 
 * @param address The IP address of the server
 * @param port The number of the port to be used
 * @return Socket descriptor if success; -1 otherwise
 */
int openAndConnectSocket(char* address, int port);

/**
 * Function that parses the command line arguments, retrieving a struct with all the individual fields
 * 
 * @param params Pointer to the structure that is going to have the individual fields
 * @param commandLineArg Argument from the command line, that is going to be parsed
 * @return 0 if sucess; -1 otherwise
 */
int parseArguments(struct FTPparameters* params, char* commandLineArg);

#endif // _AUX_H_