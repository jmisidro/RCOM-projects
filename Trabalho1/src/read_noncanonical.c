// Read from serial port in non-canonical mode

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256

#define FLAG 0x7e
#define ADDRESS 0x03
#define SET 0x03
#define UA 0x07

typedef enum{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP,
} STATE;

int alarmEnabled = FALSE;
int alarmCount = 0;

// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}

// Inital State
STATE state = START;

int main(int argc, char *argv[])
{
    int fd;
    char readBuf[5], ua_buf[5];
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS5\n",
               argv[0],
               argv[0]);
        exit(1);
    }

    // Open serial port device for reading and writing and not as staterolling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(serialPortName, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; // staterol flag
    newtio.c_iflag = IGNPAR; // receiver config flag
    newtio.c_oflag = 0; // transmitter config flag

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 1;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    while(state != STOP) {
        
        if (read(fd,readBuf,1) <= 0)
            return -1; 
        switch(state) {
            case START:
				if (readBuf[0] == FLAG) {
					state = FLAG_RCV;
                    printf("SET: FLAG received\n");
                }
                else {
                    state = START;
                }
                break;

            case FLAG_RCV:
                if (readBuf[0] == ADDRESS) {
                    state = A_RCV;
                    printf("SET: A received\n");
                }
                else if (readBuf[0] == FLAG){
                    state = FLAG_RCV;
                }
                else {
                    state = START;
                }
                break;

            case A_RCV:
                if (readBuf[0] == SET) {
                    state = C_RCV;
                    printf("SET: C received\n");
                }
                else if (readBuf[0] == FLAG){
                    state = FLAG_RCV;
                }
                else {
                    state = START;
                }
                break;

            case C_RCV:
                if (readBuf[0] == (ADDRESS^SET)) {
                    state = BCC_OK;
                    printf("SET: BCC OK\n");
                }
                else if (readBuf[0] == FLAG){
                    state = FLAG_RCV;
                }
                else {
                    state = START;
                }
                break;

            case BCC_OK:
                if (readBuf[0] == FLAG) {
                    state = STOP;
                    printf("SET: FLAG #2 received\n");
                }
                else {
                    state = START;
                }
                break;

            default:
                state = START;
                break;
                 
        }
    }

    // Create UA Buffer to send
    ua_buf[0] = FLAG;
    ua_buf[1] = ADDRESS;
    ua_buf[2] = UA;
    ua_buf[3] = ua_buf[1]^ua_buf[2];
    ua_buf[4] = FLAG;
    write(fd,ua_buf,5);

    

    // The while() cycle should be changed in order to respect the specifications
    // of the protocol indicated in the Lab guide

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
