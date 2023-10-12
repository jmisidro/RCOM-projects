// Write to serial port in non-canonical mode
#include "aux.h"

// global variables
int finish, num_retr, resendFrame;

/**
 * Handles the alarm signal
 * @param signal Signal that is received
 */
void alarmHandler(int signal) {

  if(num_retr < ll.nRetransmissions){
    resendFrame = TRUE;
    printf("Alarm: Timeout, Sending frame again... (Number of retries: %d)\n", num_retr);
    alarm(ll.timeout);
    num_retr++;

  }
  else{
    printf("Alarm: Number of retries exceeded\n");
    finish = TRUE;
  }
}

void alarmHandlerInstaller() {
    // Set alarm function handler
    (void)signal(SIGALRM, alarmHandler);
}


int openNonCanonical(LinkLayer connectionParameters, int vtime, int vmin) {
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
    newtio.c_cc[VTIME] = vtime;
    newtio.c_cc[VMIN] = vmin;

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

int closeNonCanonical(struct termios oldtio, int fd) {

    // Wait until all bytes have been written to the serial port
    sleep(1);

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}



unsigned char createBCC(unsigned char a, unsigned char c) {
    return a ^ c;
}

unsigned char createBCC_2(unsigned char* frame, int length) {

  unsigned char bcc2 = frame[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ frame[i];
  }

  return bcc2;
}

int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role) {

    frame[0] = FLAG;

    if(role == LlTx) {
        if(controlField == SET) {
            frame[1] = END_SEND;
        }
        else if(controlField == UA ) {
            frame[1] = END_REC;
        }
        else return -1;
    }
    else if(role == LlRx) {
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

int createInformationFrame(unsigned char* frame, unsigned char controlField, unsigned char* infoField, int infoFieldLength) {

  frame[0] = FLAG;

  frame[1] = END_SEND;

  frame[2] = controlField;

  frame[3] = createBCC(frame[1], frame[2]);

  for(int i = 0; i < infoFieldLength; i++) {
    frame[i + 4] = infoField[i];
  }

  frame[infoFieldLength + 4] = createBCC_2(infoField, infoFieldLength);

  frame[infoFieldLength + 5] = FLAG;

  return 0;
}


int readSupervisionFrame(unsigned char* frame, int fd, unsigned char* expectedBytes, int expectedBytesLength, unsigned char addressByte) {

    state_machine *st = create_state_machine(expectedBytes, expectedBytesLength, addressByte);

    unsigned char byte;

    while(st->state != STOP && !finish && !resendFrame) {
        if(readByte(&byte, fd) == 0)
            event_handler(st, byte, frame, SUPERVISION);
    }

    int ret = st->foundIndex;

    destroy_st(st);

    if(finish || resendFrame)
        return -1;

    return ret;

}


int readInformationFrame(unsigned char* frame, int fd, unsigned char* expectedBytes, int expectedBytesLength, unsigned char addressByte) {

    state_machine *st = create_state_machine(expectedBytes, expectedBytesLength, addressByte);
    unsigned char byte;

    while(st->state != STOP) {
        if(readByte(&byte, fd) == 0)
            event_handler(st, byte, frame, INFORMATION);
    }

    // dataLength = length of the data packet sent from the application on the transmitter side
    //              (includes data packet + bcc2, with stuffing)
    int ret = st->dataLength;

    destroy_st(st);

    return ret;
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

int stateMachineTx(LinkLayer connectionParameters, int fd) {
    // Initial state
    state state = START;

    alarmHandlerInstaller();

    // Create SET Buffer to send
    char set_buf[5], ua_buf[5];
    createSupervisionFrame(set_buf, SET, LlTx);

    while(state != STOP && !finish) {

        if (resendFrame == FALSE)
        {
            // Resend SET Frame
            if( (sendFrame(set_buf, fd, sizeof(set_buf))) <= 0)
                return -1;
            alarm(connectionParameters.timeout); // Set alarm to be triggered in # seconds
            resendFrame = TRUE;
        }

        read(fd, ua_buf, 1);  

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
                if (ua_buf[0] == END_SEND) {
                    state = A_RCV;
                    printf("UA: A received\n");
                }
                else if (ua_buf[0] == FLAG){
                    state = FLAG_RCV;
                    //printf("UA: FLAG received repeated\n");
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
                if (ua_buf[0] == createBCC(END_SEND,UA)) {
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
                    resendFrame = TRUE;
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

    return 0;
}

int stateMachineRx(LinkLayer connectionParameters, int fd) {
    // Initial state
    state state = START;

    // Create SET Buffer to send
    char readBuf[5], ua_buf[5];

    while(state != STOP) {
        read(fd,readBuf,1);

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
                if (readBuf[0] == END_SEND) {
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
                if (readBuf[0] == createBCC(END_SEND,SET)) {
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
    createSupervisionFrame(ua_buf, UA, LlRx);
    sendFrame(ua_buf, fd, sizeof(ua_buf));

    return 0; 
}
