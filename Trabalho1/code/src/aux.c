// Write to serial port in non-canonical mode
#include "aux.h"

int alarmEnabled = FALSE;
int alarmCount = 0;

// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}


int openNonCanonical(LinkLayer connectionParameters, struct termios oldtio, int vtime, int vmin) {
    // Program usage: Uses either COM1 or COM2
    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Polling read

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

    return fd;
}

int closeNonCanonical(int fd, struct termios oldtio) {

    // Wait until all bytes have been written to the serial port
    sleep(1);

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);
}



unsigned char createBCC(unsigned char a, unsigned char c) {
    return a ^ c;
}

int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role) {

    frame[0] = FLAG;

    if(role == TRANSMITTER) {
        if(controlField == SET) {
            frame[1] = END_SEND;
        }
        else if(controlField == UA ) {
            frame[1] = END_REC;
        }
        else return -1;
    }
    else if(role == RECEIVER) {
        if(controlField == SET ) {
            frame[1] = END_REC;
        }
        else if(controlField == UA ) {
            frame[1] = END_SEND;
        }
        else return -1;
    }
    else return -1;

    frame[2] = controlField;

    frame[3] = createBCC(frame[1], frame[2]);

    frame[4] = FLAG;

    return 0;
}


int sendFrame(unsigned char* frame, int fd, int length) {
    if( (write(fd, frame, length)) <= 0){
        return -1;
    }

    return 1;
}

int readByte(unsigned char* byte, int fd) {

    if(read(fd, byte, sizeof(unsigned char)) <= 0)
        return -1;

    return 0;
}

int stateMachineTx(int fd) {
    // Initial state
    STATE state = START;

    // Set alarm function handler
    (void)signal(SIGALRM, alarmHandler);

    // Create SET Buffer to send
    char set_buf[5], ua_buf[5];
    createSupervisionFrame(set_buf, END_SEND, TRANSMITTER);

    while(state != STOP && alarmCount < 4) {

        if (alarmEnabled == FALSE)
        {
            if( (write(fd, set_buf, 5)) <= 0)
                return -1;
            alarm(3); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
        }

        readByte(fd, ua_buf);

        switch(state) {
            case START:
				if (ua_buf[0] == FLAG) {
					state = FLAG_RCV;
                    printf("UA: FLAG received\n");
                }
                else {
                    state = START;
                }
                break;

            case FLAG_RCV:
                if (ua_buf[0] == END_REC) {
                    state = A_RCV;
                    printf("UA: A received\n");
                }
                else if (ua_buf[0] == FLAG){
                    state = FLAG_RCV;
                    printf("UA: FLAG received repeated\n");
                }
                else {
                    state = START;
                }
                break;

            case A_RCV:
                if (ua_buf[0] == UA) {
                    state = C_RCV;
                    printf("UA: C received\n");
                }
                else if (ua_buf[0] == FLAG){
                    state = FLAG_RCV;
                }
                else {
                    state = START;
                }
                break;

            case C_RCV:
                if (ua_buf[0] == createBCC(END_REC,UA)) {
                    state = BCC_OK;
                    printf("UA: BCC OK\n");
                }
                else if (ua_buf[0] == FLAG){
                    state = FLAG_RCV;
                }
                else {
                    state = START;
                }
                break;

            case BCC_OK:
                if (ua_buf[0] == FLAG) {
                    state = STOP;
                    printf("UA: FLAG #2 received\n");
                    alarm(0);
                    alarmEnabled = TRUE;
                    printf("alarm stopped\n");
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
}

int stateMachineRx(int fd) {
    return -1; 
}
