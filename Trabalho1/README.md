`A data link layer protocol, with a simple file transfer application to test it`

For a more detailed description, check the project [report](docs/report-rcom-TP1-T12-G8.pdf). 

***

### Project Structure

- bin/: Compiled binaries.
- src/: Source code for the implementation of the link-layer and application layer protocols. Students should edit these files to implement the project.
- include/: Header files of the link-layer and application layer protocols. These files must not be changed.
- cable/: Virtual cable program to help test the serial port. This file must not be changed.
- main.c: Main file. This file must not be changed.
- Makefile: Makefile to build the project and run the application.
- penguin.gif: Example file to be sent through the serial port.

### Instructions to Run the Project

1. Compile the application and the virtual cable program using the provided Makefile.
2. Run the virtual cable program (either by running the executable manually or using the Makefile target):
	```bash
	sudo ./bin/cable_app
	sudo make run_cable
	```

3. Test the protocol without cable disconnections and noise
	- Run the receiver (either by running the executable manually or using the Makefile target):
		```bash
		./bin/main /dev/ttyS11 rx penguin-received.gif
		make run_tx
		```

	- Run the transmitter (either by running the executable manually or using the Makefile target):

		```bash  
		./bin/main /dev/ttyS10 tx penguin.gif
		make run_rx
		```

	- Check if the file received matches the file sent, using the diff Linux command or using the Makefile target:

		```bash
		diff -s penguin.gif penguin-received.gif
		make check_files
		```

4. Test the protocol with cable disconnections and noise
	- Run receiver and transmitter again
	- Quickly move to the cable program console and press 0 for unplugging the cable, 2 to add noise, and 1 to normal
	- Check if the file received matches the file sent, even with cable disconnections or with noise


**Final Grade: 19.7**