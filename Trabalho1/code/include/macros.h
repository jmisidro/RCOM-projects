#ifndef _MACROS_H_
#define _MACROS_H_

// ---- macros for data link layer ----
#define MAX_SIZE_DATA   1024 // max size of a data packet
#define MAX_SIZE_PACK   (MAX_SIZE_DATA + 4) // max size of a data packet + 4 bytes for packet head
#define MAX_SIZE        (MAX_SIZE_PACK + 6) // max size of data in a frame + 4 bytes fot packet head, + 6 bytes for frame header and tail
#define MAX_SIZE_FRAME  (((MAX_SIZE_PACK + 1) * 2) + 5) // max size of a frame, with byte stuffing, is ((1029 * 2) + 5)
                             // 1029 -> all bytes that can suffer byte stuffing (and therefore be "duplicated"), which are the packet and the BCC2
                             // 5 -> the bytes that certainly won't suffer any byte stuffing (flags, bcc1, address and control bytes)

#define BUF_SIZE_SUP    5 // size of a supervision frame

#define BAUDRATE 9600 // 38400 is the normal value
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define N_TRIES 3
#define TIMEOUT 4

#define SUPERVISION 0
#define INFORMATION 1

#define FLAG     0x7E
#define END_SEND 0x03
#define END_REC  0x01
#define S_0      0x00
#define S_1      0x40
#define SET      0x03 // set up
#define DISC     0x0B // disconnect
#define UA       0x07 // unnumbered acknowledgment
#define RR_0     0X05 // Receive Ready / positive ACK for number 0
#define RR_1     0x85 // Receive Ready / positive ACK for number 1 --> R 0 0 0 0 1 0 1 -- R = N(r)
#define REJ_0    0x01 // Reject / positive ACK for number 0
#define REJ_1    0x81 // Reject / negative ACK for number 1 --> R 0 0 0 0 0 0 1 -- R = N(r)
#define VTIME_VALUE    0
#define VMIN_VALUE     0

#define BYTE_STUFFING_ESCAPE 0x5D
#define BYTE_STUFFING_FLAG 0x5E
#define ESCAPE_BYTE 0x7D

#define DATA_START    4


// ---- macros for application layer ----

#define CTRL_DATA       0x01
#define CTRL_START      0x02
#define CTRL_END        0x03

#define TYPE_FILESIZE   0x00
#define TYPE_FILENAME   0x01


#endif // _MACROS_H_