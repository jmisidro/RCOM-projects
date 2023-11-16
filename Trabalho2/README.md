`A download application for the FTP and the configuration and study of a computer network`

For a more detailed description, check the project [report](docs/report-rcom-TP2-T12-G8.pdf). 

***

### Project Structure

- src/: Source code for the implementation of the FTP download application.
- main.c: Main file.
- app.c: Auxiliary functions to communicate with the FTP server.
- Makefile: Makefile to build the project and run the application.

### Instructions to Run the Project

1. Compile the application using the provided Makefile.
2. Run the download application (either by running the executable manually or using the Makefile target):
	```bash
	./download ftp://<user>:<password>@<host>/<url-path>
	make run_download
	```



