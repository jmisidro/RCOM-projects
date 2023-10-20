// Link layer protocol auxiliary funtions

#include "ll_aux.h"

/**
 * Handles the alarm signal
 * @param signal Signal that is received
 */
void alarmHandler(int signal) {

  if (num_retr < ll.nRetransmissions) {
    resendFrame = TRUE;
    printf("Alarm: Timeout, Sending frame again... (Number of retransmissions: %d)\n", num_retr+1);
    alarm(ll.timeout);
    num_retr++;

  }
  else {
    printf("Alarm: Number of retransmissions exceeded\n");
    finish = TRUE;
  }
}

void alarmHandlerInstaller() {
    // Set alarm function handler
    (void)signal(SIGALRM, alarmHandler);
}


int openNonCanonical(int vtime, int vmin) {
    // Program usage: Uses either COM1 or COM2
    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(ll.serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(ll.serialPort);
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

    newtio.c_cflag = ll.baudRate | CS8 | CLOCAL | CREAD;
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

    printf("openNonCanonical: new termios structure set\n");

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

int byteStuffing(unsigned char* frame, int length) {

    // allocates space for aux buffer (length of the packet + 6 bytes for the frame header and tail)
    unsigned char aux[length + 6];

    // transfers info from the frame to aux
    for(int i = 0; i < (length + 6); i++){
      aux[i] = frame[i];
    }
    
    int finalLength = DATA_START;
    // parses aux buffer, and fills in correctly the frame buffer
    for(int i = DATA_START; i < (length + 6); i++){
      // If the octet 0x7E (FLAG) occurs inside the frame, the octet is replaced by the sequence: 0x7D 0x5E (ESCAPE_BYTE BYTE_STUFFING_FLAG)
      if(aux[i] == FLAG && i != (length + 5)) {
        frame[finalLength] = ESCAPE_BYTE;
        frame[finalLength+1] = BYTE_STUFFING_FLAG;
        finalLength = finalLength + 2;
      }
      // If the octet 0x7D (ESCAPE_BYTE) occurs inside the frame, the octet is replaced by the sequence: 0x7D 0x5D (ESCAPE_BYTE BYTE_STUFFING_ESCAPE)
      else if(aux[i] == ESCAPE_BYTE && i != (length + 5)) {
        frame[finalLength] = ESCAPE_BYTE;
        frame[finalLength+1] = BYTE_STUFFING_ESCAPE;
        finalLength = finalLength + 2;
      }
      else{
        frame[finalLength] = aux[i];
        finalLength++;
      }
    }

    return finalLength;
}

int byteDestuffing(unsigned char* frame, int length) {

  // allocates space for the max possible frame length read (length of the data packet + bcc2, already with stuffing, plus the other 5 bytes in the frame)
  unsigned char aux[length + 5];

  // copies the content of the frame (with stuffing) to the aux frame
  for(int i = 0; i < (length + 5); i++) {
    aux[i] = frame[i];
  }

  int finalLength = DATA_START;

  // iterates through the aux buffer, and fills the frame buffer with destuffed content
  for(int i = DATA_START; i < (length + 5); i++) {

    if(aux[i] == ESCAPE_BYTE){
      // If the octet 0x5D (BYTE_STUFFING_ESCAPE) occurs inside the frame, the octet is replaced by the escape byte: 0x7D
      if (aux[i+1] == BYTE_STUFFING_ESCAPE) {
        frame[finalLength] = ESCAPE_BYTE;
      }
      // If the octet 0x5E (BYTE_STUFFING_FLAG) occurs inside the frame, the octet is replaced by the flag: 0x7E
      else if(aux[i+1] == BYTE_STUFFING_FLAG) {
        frame[finalLength] = FLAG;
      }
      i++;
      finalLength++;
    }
    else{
      frame[finalLength] = aux[i];
      finalLength++;
    }
  }

  return finalLength;
}

int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role) {

    frame[0] = FLAG;

    /* ADDRESS FIELD */
    if(role == LlTx) {
        // If the frame is sent by the Sender, the Address field will be ADD_SEND (0x03)
        if(controlField == SET || controlField == DISC) {
          frame[1] = ADD_SEND;
        }
        // If the frame is an answer from the Sender, the Address field will be ADD_REC (0x01)
        else if(controlField == UA || controlField == RR_0 || controlField == REJ_0 || controlField == RR_1 || controlField == REJ_1 ) {
          frame[1] = ADD_REC;
        }
        else return -1;
    }
    else if(role == LlRx) {
        // If the frame is sent by the Receiver, the Address field will be ADD_REC (0x01)
        if(controlField == SET || controlField == DISC) {
          frame[1] = ADD_REC;
        }
        // If the frame is an answer from the Receiver, the Address field will be ADD_SEND (0x03)
        else if(controlField == UA || controlField == RR_0 || controlField == REJ_0 || controlField == RR_1 || controlField == REJ_1 ) {
          frame[1] = ADD_SEND;
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

  frame[1] = ADD_SEND;

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

    // dataLength = length of the data packet sent from the application on the transmitter side (includes data packet + bcc2, with stuffing)
    int ret = st->dataLength;

    destroy_st(st);

    return ret;
}


int sendFrame(unsigned char* frame, int fd, int length) {
    int n;

    if( (n = write(fd, frame, length)) <= 0){
        return -1;
    }

    return n;
}

int readByte(unsigned char* byte, int fd) {

    if(read(fd, byte, sizeof(unsigned char)) <= 0)
      return -1;

    return 0;
}

