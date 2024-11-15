#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv [])
{
	openlog(NULL,0,LOG_USER);
	if (argc < 3){
		syslog(LOG_ERR, "Invalid number of arguments: %i", argc-1);
		return 1;
	}
	else{
		int fd;
		fd = creat (argv[1], 0644);
		if (fd == -1){
			syslog(LOG_ERR, "Error opening/creating file %s", argv[1]);
			return 1;
		}
		else{
			syslog(LOG_DEBUG, "Writing %s to file %s", argv[2], argv[1]);
		}
		strcat(argv[2], "\n");
		fd = write (fd, argv[2], strlen(argv[2]));
		if (fd == -1){
			syslog(LOG_ERR, "Error writing in file %s", argv[1]);
			return 1;
		}
	}
}
