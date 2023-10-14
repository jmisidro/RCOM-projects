// Auxiliary file header.

#ifndef _LL_AUX_H_
#define _LL_AUX_H_

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
#include "statemachine.h"
#include "link_layer.h"

/**
 * Handles the alarm signal
 * @param signal Signal that is received
 */
void alarmHandler(int signal);

/**
 * Function to install the alarm handler, using sigaction
 */
void alarmHandlerInstaller();

/**
 * Function to open the file descriptor through which to execute the serial port communications,
 * in the non-canonical mode, according to the serial port file transfer protocol
 * @param vtime Value to be assigned to the VTIME field of the new settings - time between bytes read
 * @param vmin Value to be assigned to the VMIN field of the new settings - minimum amount of bytes to read
 * @return File descriptor that was opened with the given port
 */
int openNonCanonical(int vtime, int vmin);

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
 * Function to create the Block Check Character relative to the Data Characters of the frame
 * @param frame Frame position where the Data starts
 * @param length Number of Data Characters to process
 * @return Expected value for the Block Check Character
 */
unsigned char createBCC_2(unsigned char* frame, int length);

/**
 * Function to create a supervision frame for the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param controlField Control field of the supervision frame
 * @param role Role for which to create the frame, marking the difference between the Transmitter and the Receiver
 * @return 0 if successful; negative if an error occurs
 */
int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role);


/**
 * Function to create an information frame for the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param controlField Control field of the supervision frame
 * @param infoField Start address of the information to be inserted into the information frame
 * @param infoFieldLength Number of data characters to be inserted into the information frame
 * @return Returns 0, as there is no place at which an error can occur
 */
int createInformationFrame(unsigned char* frame, unsigned char controlField, unsigned char* infoField, int infoFieldLength);

/**
 * Function to read a supervision frame, sent according to the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param fd File descriptor from which to read the frame
 * @param expectedBytes Array containing the possible expected control bytes of the frame
 * @param expectedBytesLength Number of possible expected control bytes of the frame
 * @param addressByte Address from which a frame is expected
 * @return Index of the expected byte found, in the expectedBytes array
 */
int readSupervisionFrame(unsigned char* frame, int fd, unsigned char* expectedBytes, int expectedBytesLength, unsigned char addressByte);

/**
 * Function to read an information frame, sent according to the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param fd File descriptor from which to read the frame
 * @param expectedBytes Array containing the possible expected control bytes of the frame
 * @param expectedBytesLength Number of possible expected control bytes of the frame
 * @param addressByte Address from which a frame is expected
 * @return Length of the data packet sent, including byte stuffing and BCC2
 */
int readInformationFrame(unsigned char* frame, int fd, unsigned char* expectedBytes, int expectedBytesLength, unsigned char addressByte);

/**
 * Function to send a frame to the designated file descriptor
 * @param frame Start address of the frame to the sent
 * @param fd File descriptor to which to write the information
 * @param length Size of the frame to be sent (size of information to be written)
 * @return Number of bytes written if successful; negative if an error occurs
 */
int sendFrame(unsigned char* frame, int fd, int length);

/**
 * Function to read a byte from the designated file descriptor
 * @param byte Address to which to store the byte
 * @param fd File descriptor from which to read the byte
 * @return Return value of the read() call if successful; negative if an error occurs
 */
int readByte(unsigned char* byte, int fd);

/**
 * Opens the connection for the receiver
 * @param fd File descriptor for the serial port
 * @return File descriptor; -1 in case of error
 */
int llOpenReceiver(int fd);

/**
 * Opens the connection for the transmitter
 * @param fd File descriptor for the serial port
 * @return File descriptor; -1 in case of error
 */
int llOpenTransmitter(int fd);

#endif // _LL_AUX_H_
