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
    char file_name[MAX_LENGTH]; /* file name string */    
};

/**
 * Struct that contains the control and data file descriptors for the FTP
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
 * @return 0 if success; -1 otherwise
 */
int getIPAddress(char *ipAddress, char *hostName);

/**
 * Function that opens a new TCP socket, and connects it to the address and port specified
 * 
 * @param address The IP address of the server
 * @param port The number of the port to be used
 * @return Socket descriptor if success; -1 otherwise
 */
int openAndConnectSocket(char *address, int port);

/**
 * Function that parses the command line arguments, retrieving a struct with all the individual fields
 * 
 * @param params Pointer to the structure that is going to have the individual fields
 * @param commandLineArg Argument from the command line, that is going to be parsed
 * @return 0 if sucess; -1 otherwise
 */
int parseArguments(struct FTPparameters *params, char *commandLineArg);

/**
 * Function that sends a command through the control socket
 * 
 * @param ftp Struct containing the socket descriptors
 * @param command Command to be sent
 * @param argument Argument of the command to be sent
 * @return 0 if success; -1 otherwise
 */
int sendCommandToControlSocket(struct FTP *ftp, char *command, char *argument);

/**
 * Function that reads a reply from a control socket, according to FTP
 * 
 * @param ftp Struct containing the socket descriptors
 * @param buffer Buffer containing the reply code and message received from the server
 * @return 0 if success; -1 otherwise
 */
int readReplyFromControlSocket(struct FTP *ftp, char *buffer);

/**
 * Function that sends a command to the control socket and handles the reply received
 * 
 * @param ftp Struct containing the socket descriptors
 * @param command Command to be sent
 * @param argument Argument of the command to be sent
 * @param reply Buffer that is going to store the reply code and message received from the server
 * @param dowloadingFile Indicates if the file is about to be transfered from the data socket
 * @return Positive if success (the first digit of the reply code); -1 otherwise
 */
int sendCommandHandleReply(struct FTP *ftp, char *command, char *argument, char *reply, int dowloadingFile);


/**
 * Function that sends the login information to the socket for authentication
 * 
 * @param ftp Struct containing the socket descriptors
 * @param user User to be sent to the socket
 * @param password Password to be sent to the socket
 * @return 0 if success; -1 otherwise
 */
int login(struct FTP *ftp, char *user, char *password);

/**
 * Function to change the working directory of the FTP server, using the CWD command
 * 
 * @param ftp Struct containing the socket descriptors
 * @param path Path of the new working directory to be changed to
 * @return 0 if successful; -1 otherwise
 */
int changeWorkingDirectory(struct FTP* ftp, char* path);

/**
 * Function that instructs the FTP server to enter a Passive session rather than Active  
 * and then obtains a server IP address and Port for the data socket used in the transfer of a file
 * 
 * @param ftp Struct containing the socket descriptors
 * @return 0 if success; -1 otherwise
 */
int enablePassiveMode(struct FTP *ftp);

/**
 * Function that sends the RETR command to the control socket, 
 * so the file can be transfered in the data socket
 * Then proceeds to download the file sent from the server
 * through the data socket, and saves it in a local file
 * 
 * @param ftp Struct containing the socket descriptors
 * @param fileName The name of the file to be transfered
 * @return 0 if successful; -1 otherwise
 */
int retrieveFile(struct FTP *ftp, char *fileName);

#endif // _AUX_H_