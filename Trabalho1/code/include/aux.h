// Auxiliary file header.

#ifndef _AUX_H_
#define _AUX_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "macros.h"
#include "link_layer.h"

typedef enum{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP,
} STATE;

/**
 * Function to open the file descriptor through which to execute the serial port communications,
 * in the non-canonical mode, according to the serial port file transfer protocol
 * @param connectionParameters linklayer struct including info such as port
 * @param vtime Value to be assigned to the VTIME field of the new settings - time between bytes read
 * @param vmin Value to be assigned to the VMIN field of the new settings - minimum amount of bytes to read
 * @return File descriptor that was opened with the given port
 */
int openNonCanonical(LinkLayer connectionParameters, int vtime, int vmin);

/**
 * Function to open the file descriptor through which to execute the serial port communications,
 * in the non-canonical mode, according to the serial port file transfer protocol
 * @param fd File descriptor that was opened with the given port
 * @param oldtio termios used to store oldtio to restore the old port settings when closing the connection
 * @return Return -1 in an error occurs
 */
int closeNonCanonical(struct termios oldtio, int fd);


/**
 * Function to create the Block Check Character relative to the Address and Control fields
 * @param a Address Character of the frame
 * @param c Control Character of the frame
 * @return Expected value for the Block Check Character
 */
unsigned char createBCC(unsigned char a, unsigned char c);

/**
 * Function to create a supervision frame for the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param controlField Control field of the supervision frame
 * @param role Role for which to create the frame, marking the difference between the Transmitter and the Receiver
 * @return 0 if successful; negative if an error occurs
 */
int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role);

/**
 * Function to send a frame to the designated file descriptor
 * @param frame Start address of the frame to the sent
 * @param fd File descriptor to which to write the information
 * @param length Size of the frame to be sent (size of information to be written)
 * @return Number of bytes written if successful; negative if an error occurs
 */
int sendFrame(unsigned char* frame, int fd, int length);


/**
 * Function to create and handle a state machine as transmitter 
 * @param connectionParameters linklayer struct including info such as port
 * @param fd File descriptor from which to read the byte
 * @return 0 if successful; negative if an error occurs
 */
int stateMachineTx(LinkLayer connectionParameters, int fd);


/**
 * Function to create and handle a state machine as receiver
 * @param connectionParameters linklayer struct including info such as port
 * @param fd File descriptor from which to read the byte
 * @return 0 if successful; negative if an error occurs
 */
int stateMachineRx(LinkLayer connectionParameters, int fd);

#endif // _AUX_H_
