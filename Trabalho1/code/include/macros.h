#ifndef _MACROS_H_
#define _MACROS_H_

// ---- macros for data link layer ----

#define BUF_SIZE_SUP    5 // size of a supervision frame

#define BAUDRATE 9600 //38400 is the normal value
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

#define N_TRIES 3
#define TIMEOUT 4

#define TRANSMITTER 0
#define RECEIVER    1

#define SUPERVISION 0
#define INFORMATION 1

#define NUM_RETR    3
#define TIMEOUT     3

#define FLAG     0x7E
#define END_SEND 0x03
#define END_REC  0x01
#define SET      0x03
#define UA       0x07
#define VTIME_VALUE    0
#define VMIN_VALUE     0

#define BYTE_STUFFING_ESCAPE 0x5D
#define BYTE_STUFFING_FLAG 0x5E
#define ESCAPE_BYTE 0x7D

#define DATA_START    4


// ---- macros for application layer ----


#endif // _MACROS_H_