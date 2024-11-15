#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

bool signal_flag = false;
char *file_dir = "/var/tmp/aesdsocketdata";
int client_sockfd;
int sockfd;

void get_client_ip(struct sockaddr_storage *client_sockaddr, char* ipstr){
    if (client_sockaddr->ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&client_sockaddr;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
    } 
    else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_sockaddr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
    }
    syslog(LOG_INFO, "Accepted connection from %s", ipstr);
}

void close_all(void){
    close(client_sockfd);
    close(sockfd);
    closelog();
    remove(file_dir);
}

void signal_handler(int signum){
    if ((signum == SIGINT) || (signum == SIGTERM)){
        signal_flag = true;
	syslog(LOG_INFO, "Caught signal, exiting");
	close_all();
	exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct sockaddr_storage client_sockaddr;
    socklen_t client_sockaddr_size = sizeof(client_sockaddr);
    sockfd = -1;
    client_sockfd = -1;
    int ret = -1;
    char ipstr[INET6_ADDRSTRLEN];
    
    FILE *file = NULL;
    char *file_dir = "/var/tmp/aesdsocketdata";
    int bytes_read = 0;
    int bytes_write = 0;
    char data_read_buffer[1024];
    char data_write_buffer[1024];

    struct sigaction new_action = {0};
    new_action.sa_handler = signal_handler;
    sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGTERM, &new_action, NULL);

    bool daemon_flag = false;

    if (argc > 1){
        if (!strcmp(argv[1],"-d")){
	    daemon_flag = true;
	}
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    openlog(NULL,0,LOG_USER);

    ret = getaddrinfo(NULL, "9000", &hints, &servinfo);
    if (ret == -1){
        syslog(LOG_ERR, "ERROR getting address");
        freeaddrinfo(servinfo);
	close_all();
	return -1;
    }

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd == -1){
        syslog(LOG_ERR, "ERROR opening socket");
	freeaddrinfo(servinfo);
	close_all();
	return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        syslog(LOG_ERR, "ERROR setting socket");
        freeaddrinfo(servinfo);
	close_all();
	return -1;
    } 

    ret = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (ret == -1){
	syslog(LOG_ERR, "ERROR binding socket");
	freeaddrinfo(servinfo);
	close_all();
	return -1;
    }
    freeaddrinfo(servinfo);

    if (daemon_flag){
        daemon(0, 0);
    }

    ret = listen(sockfd, 10);
    if (ret == -1){
        syslog(LOG_ERR, "ERROR listening");
        return -1;
    }

    while (true){

        client_sockfd = accept(sockfd, (struct sockaddr *)&client_sockaddr, &client_sockaddr_size);
        if (client_sockfd < 0){
            syslog(LOG_ERR, "ERROR opening client socket");
	    return -1;
        }

        get_client_ip(&client_sockaddr, &ipstr);
	syslog(LOG_INFO, "Accepted connection from %s", ipstr);

        file = fopen(file_dir, "a");
	if (file == NULL){
	    syslog(LOG_ERR, "ERROR failed to open file for writing");
	    return -1;
	}

	memset(data_write_buffer, 0, sizeof(data_write_buffer));
	while ((bytes_write = recv(client_sockfd, data_write_buffer, sizeof(data_write_buffer) - 1, 0)) > 0){
	    fwrite(data_write_buffer, sizeof(char), bytes_write, file);
	    if (strchr(data_write_buffer, '\n') != NULL){
		break;
	    }
	}
	fclose(file);

        file = fopen(file_dir, "r");
	if (file == NULL){
            syslog(LOG_ERR, "ERROR failed to open file for reading");
            return -1;
        }

	memset(data_read_buffer, 0, sizeof(data_read_buffer));
        while ((bytes_read = fread(data_read_buffer, sizeof(char), sizeof(data_read_buffer)-1, file)) > 0){
	    send(client_sockfd, data_read_buffer, bytes_read, 0);
	}
	fclose(file);

        close(client_sockfd);
	get_client_ip(&client_sockaddr, &ipstr);
	syslog(LOG_INFO, "Closed connection from %s", ipstr);
    }
    close_all();
    return 0;
}
