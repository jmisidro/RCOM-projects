// Write to serial port in non-canonical mode
#include "ll_aux.h"

// global variables
// finish - TRUE when the alarm has completed all retransmissions so we close the connection
// num_retr - current number of retransmissions
int finish, num_retr, resendFrame;

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
        if(controlField == SET || controlField == DISC) {
            frame[1] = END_SEND;
        }
        else if(controlField == UA || controlField == RR_0 || controlField == REJ_0 || controlField == RR_1 || controlField == REJ_1 ) {
            frame[1] = END_REC;
        }
        else return -1;
    }
    else if(role == LlRx) {
        if(controlField == SET || controlField == DISC) {
            frame[1] = END_REC;
        }
        else if(controlField == UA || controlField == RR_0 || controlField == REJ_0 || controlField == RR_1 || controlField == REJ_1 ) {
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
    if( (write(fd, frame, length)) <= 0) {
        return -1;
    }

    return 1;
}

int readByte(unsigned char* byte, int fd) {

    if(read(fd, byte, sizeof(unsigned char)) <= 0)
        return -1;

    return 0;
}

int llOpenReceiver(int fd)
{
  unsigned char frame[BUF_SIZE_SUP], expectedByte[1];
  expectedByte[0] = SET;
  if (readSupervisionFrame(frame, fd, expectedByte, 1, END_SEND) == -1)
    return -1;

  printf("llopen: Received SET frame\n");

  if (createSupervisionFrame(frame, UA, LlRx) != 0)
    return -1;


  // send SET frame to receiver
  if (sendFrame(frame, fd, sizeof(frame)) == -1)
    return -1;

  printf("llopen: Sent UA frame\n");

  return fd;
}

int llOpenTransmitter(int fd)
{
  unsigned char frame[BUF_SIZE_SUP], responseBuffer[BUF_SIZE_SUP]; // buffer to read the response 


  // creates SET frame
  if (createSupervisionFrame(frame, SET, LlTx) != 0)
    return -1;


  // send SET frame to receiver
  if (sendFrame(frame, fd, sizeof(frame)) == -1)
    return -1;

  printf("llopen: Sent SET frame\n");

  int read_value = -1;
  finish = 0;
  num_retr = 0;
  resendFrame = FALSE;

  // Sets alarm
  alarm(ll.timeout);

  unsigned char expectedByte[1];
  expectedByte[0] = UA;

  while (finish != 1) {
    read_value = readSupervisionFrame(responseBuffer, fd, expectedByte, 1, END_SEND);
    if (resendFrame) {
      sendFrame(frame, fd, sizeof(frame));
      resendFrame = FALSE;
    }

    if (read_value >= 0) {
      // Cancels alarm
      alarm(0);
      finish = 1;
    }
  }

  if (read_value == -1) {
    printf("Closing file descriptor\n");
    return -1;
  }

  printf("llopen: Received UA frame\n");

  return fd;
}

int llCloseTransmitter(int fd)
{
  unsigned char frame[BUF_SIZE_SUP], responseBuffer[BUF_SIZE_SUP], expectedByte[1];

  // creates DISC frame
  if (createSupervisionFrame(frame, DISC, LlTx) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(frame, fd, sizeof(frame)) == -1)
    return -1;

  printf("Sent DISC frame\n");

  int read_value = -1;
  finish = 0;
  num_retr = 0;
  resendFrame = FALSE;

  // Sets alarm
  alarm(ll.timeout);

  expectedByte[0] = DISC;

  while (finish != 1) {
    read_value = readSupervisionFrame(responseBuffer, fd, expectedByte, 1, END_REC);

    if (resendFrame) {
      sendFrame(frame, fd, sizeof(frame));
      resendFrame = FALSE;
    }

    if (read_value >= 0) {
      // Cancels alarm
      alarm(0);
      finish = 1;
    }
  }

  if (read_value == -1) {
    printf("Closing file descriptor\n");
    return -1;
  }

  printf("llclose: Received DISC frame\n");

  // creates UA frame
  if (createSupervisionFrame(frame, UA, LlTx) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(frame, fd, sizeof(frame)) == -1)
    return -1;

  printf("llclose: Sent UA frame\n");

  return 0;
}

int llCloseReceiver(int fd)
{
  unsigned char frame[BUF_SIZE_SUP], responseBuffer[BUF_SIZE_SUP], expectedByte[1];
  expectedByte[0] = DISC;

  if (readSupervisionFrame(frame, fd, expectedByte, 1, END_SEND) == -1)
    return -1;

  printf("llclose: Received DISC frame\n");

  // creates DISC frame
  if (createSupervisionFrame(frame, DISC, LlRx) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(frame, fd, sizeof(frame)) == -1)
    return -1;

  printf("llclose: Sent DISC frame\n");

  int read_value = -1;
  finish = 0;
  num_retr = 0;
  resendFrame = FALSE;

  alarm(ll.timeout);

  expectedByte[0] = UA;

  while (finish != 1) {
    read_value = readSupervisionFrame(responseBuffer, fd, expectedByte, 1, END_REC);

    if (resendFrame)
    {
      sendFrame(frame, fd, sizeof(frame));
      resendFrame = FALSE;
    }

    if (read_value >= 0)
    {
      // Cancels alarm
      alarm(0);
      finish = 1;
    }
  }

  if (read_value == -1) {
    printf("Closing file descriptor\n");
    return -1;
  }

  printf("llclose: Received UA frame\n");

  return 0;
}
