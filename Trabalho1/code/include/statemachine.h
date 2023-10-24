// State Machine file header.

#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

typedef enum{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP,
} State;

typedef struct {
    State state; /* Current state */
    unsigned char* expectedBytes; /* Array of possible bytes that are expected in the frame */
    int expectedBytesLength; /* Number of possible bytes that are expected in the frame */
    unsigned char addressByte; /* Address from which the frame is expected */
    int foundIndex; /* Index of the array where the byte was found; negative value otherwise */
    int dataLength; /* Length of the data packet sent from the application on the transmitter side (includes data packet + bcc2, with stuffing) */
} state_machine;


/**
 * Function to check if a byte is contained in the state machine's expectedBytes field, indicating it is expected
 * @param byte Byte to be checked
 * @param sm State machine for which to check
 * @return Index of the array where the byte was found; negative value otherwise
 */
int is_expected(unsigned char byte, state_machine* sm);


/**
 * Function to create a state machine, with the given attributes
 * @param expectedBytes Possible bytes that are expected in the frame
 * @param expectedBytesLength Number of possible bytes that are expected in the frame
 * @param addressByte Address from which the frame is expected
 * @return Pointer to the new state machine object
 */
state_machine* create_state_machine(unsigned char* expectedBytes, int expectedBytesLength, unsigned char addressByte);


/**
 * Function to update the state machine according to the bytes read
 * @param sm State machine to be updated
 * @param byte Last byte to have been read of the frame
 * @param frame Address where the frame is being stored
 * @param mode Type of frame (Supervision or Information)
 */
void event_handler(state_machine* sm, unsigned char byte, unsigned char* frame, int mode);


/**
 * Function to free the memory allocated to a state machine object
 * @param sm State machine to be destroyed
 */
void destroy_state_machine(state_machine* sm);

#endif // _STATE_MACHINE_H_
