#ifndef _MACROS_H_
#define _MACROS_H_

// MISC
#define FALSE 0
#define TRUE 1

// ---- macros for both layers ----

#define MAX_SIZE_DATA   1024 // max size of a data packet
#define MAX_SIZE_PACK   (MAX_SIZE_DATA + 4) // max size of a data packet + 4 bytes for packet head
#define MAX_SIZE        (MAX_SIZE_PACK + 6) // max size of data in a frame + 4 bytes fot packet head, + 6 bytes for frame header and tail
#define MAX_SIZE_FRAME  (((MAX_SIZE_PACK + 1) * 2) + 5) // max size of a frame, with byte stuffing, is ((1029 * 2) + 5)
                             // 1029 -> all bytes that can suffer byte stuffing (and therefore be "duplicated"), which are the packet and the BCC2
                             // 5 -> the bytes that certainly won't suffer any byte stuffing (flags, bcc1, address and control bytes)

#define BUF_SIZE_SUP    5 // size of a supervision frame

#define BAUDRATE 38400 // 38400 is the normal value
#define _POSIX_SOURCE 1 // POSIX compliant source

#define N_TRIES 3 // Number of tries before the alarm stops
#define TIMEOUT 4 // Timeout for the alarm

// ---- macros for data link layer ----

#define SUPERVISION 0 // Supervision frame
#define INFORMATION 1 // Information frame

#define FLAG     0x7E // Synchronisation: start or end of frame
#define ADD_SEND 0x03 // Address field in frames that are commands sent by the Sender or replies sent by the Receiver
#define ADD_REC  0x01 // Address field in frames that are commands sent by the Receiver or replies sent by the Sender
#define I_0      0x00 // Information frame number 0
#define I_1      0x40 // Information frame number 1
#define SET      0x03 // Set Up --> sent by the transmitter to initiate a connection
#define DISC     0x0B // Disconnect --> indicate the termination of a connection
#define UA       0x07 // Unnumbered Acknowledgment --> confirmation to the reception of a valid supervision frame
#define RR_0     0X05 // Receive Ready for number 0 --> indication sent by the Receiver that it is ready to receive an information frame number 0
#define RR_1     0x85 // Receive Ready for number 1 --> indication sent by the Receiver that it is ready to receive an information frame number 1
#define REJ_0    0x01 // Reject --> indication sent by the Receiver that it rejects an information frame number 0
#define REJ_1    0x81 // Reject --> indication sent by the Receiver that it rejects an information frame number 1
#define VTIME_VALUE    0
#define VMIN_VALUE     0

#define BYTE_STUFFING_ESCAPE 0x5D // If the octet 0x7D (ESCAPE_BYTE) occurs inside the frame, the octet is replaced by the sequence: 0x7D 0x5D (ESCAPE_BYTE BYTE_STUFFING_ESCAPE)
#define BYTE_STUFFING_FLAG 0x5E // If the octet 0x7E (FLAG) occurs inside the frame, the octet is replaced by the sequence: 0x7D 0x5E (ESCAPE_BYTE BYTE_STUFFING_FLAG)
#define ESCAPE_BYTE 0x7D // escape octet

#define DATA_START    4 // start of the data field in an I-frame


// ---- macros for application layer ----

#define CTRL_DATA       0x01 // value for DATA in control field (C) meaning its a data packet
#define CTRL_START      0x02 // value for START in control field (C) meaning its the START control packet
#define CTRL_END        0x03 // value for END in control field (C) meaning its the END control packet

#define TYPE_FILESIZE   0x00 // value for FILESIZE in the type field (T), in the control packet
#define TYPE_FILENAME   0x01 // value for FILENAME the type field (T), in the control packet


#endif // _MACROS_H_