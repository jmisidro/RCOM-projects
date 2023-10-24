// Link layer header.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include <termios.h>
#include "macros.h"
#include "ll_aux.h"

typedef enum
{
    LlTx, /* Transmitter */
    LlRx, /* Receiver */
} LinkLayerRole;

typedef struct
{
    char serialPort[50]; /* Device /dev/ttySx, x = 0, 1 */
    LinkLayerRole role; /*  Role played in transfer: Transmitter ou Receiver */
    int baudRate; /* Rate at which information is transferred in the channel*/
    unsigned int nRetransmissions;/* Number of retries in case of error */
    unsigned int timeout; /* Value of the timer in seconds */
    unsigned char frame[MAX_SIZE_FRAME]; /* Frame */
    unsigned int frame_length; /* Current frame size */
    unsigned int sequenceNumber; /* Frame sequence number: 0, 1 */

} LinkLayer;

// global variables
LinkLayer ll;
struct termios oldtio;

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000



/**
 * Open a connection using the "port" parameters defined in struct LinkLayer (global variable).
 * @return File descriptor; negative value otherwise
 */
int llopen();

/**
 * Opens the connection for the receiver
 * @param fd File descriptor for the serial port
 * @return File descriptor; negative value otherwise
 */
int llOpenReceiver(int fd);

/**
 * Opens the connection for the transmitter
 * @param fd File descriptor for the serial port
 * @return File descriptor; negative value otherwise
 */
int llOpenTransmitter(int fd);

/**
 * Transfer data stored in packet.
 * @param fd File descriptor for the serial port
 * @param packet Packet (data) to be wrriten
 * @param length Size of the packet
 * @return Number of chars written; negative value otherwise
 */
int llwrite(int fd, unsigned char *packet, int length);

/**
 * Receive data in packet.
 * @param fd File descriptor for the serial port
 * @param packet Packet to store the data read in
 * @return Number of chars read; negative value otherwise
 */
int llread(int fd, unsigned char *packet);

/**
 * Close previously opened connection.
 * @param fd File descriptor for the serial port
 * @return Positive value if it was sucessful; negative value otherwise
 */
int llclose(int fd);

/**
 * Closes the connection for the receiver
 * @param fd File descriptor for the serial port
 * @return Positive value if it was sucessful; negative value otherwise
 */
int llCloseReceiver(int fd);

/**
 * Closes the connection for the transmitter
 * @param fd File descriptor for the serial port
 * @return Positive value if it was sucessful; negative value otherwise
 */
int llCloseTransmitter(int fd);

#endif // _LINK_LAYER_H_
