#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ds.h"

#define MAX_INPUT_LEN 100
int login(char input[MAX_INPUT_LEN], char command[MAX_INPUT_LEN],
		char p1[MAX_INPUT_LEN], char p2[MAX_INPUT_LEN],
		char p3[MAX_INPUT_LEN], char p4[MAX_INPUT_LEN]);
int create_session_client(char *sessionID, int sockfd);
int join_session_client(char *sessionID, int sockfd);
int leave_session_client(char *sessionID, int sockfd);
int exit_client(int socket);
int query_client(int socket);
int quit(int socket);
int send_text(char*input, int socket);
char username[MAX_USER_LENGTH];
void message_handler(int sockfd); 
void stdin_handler(int socket, char input[MAX_INPUT_LEN], char command[MAX_INPUT_LEN],
		char p1[MAX_INPUT_LEN], char p2[MAX_INPUT_LEN],
		char p3[MAX_INPUT_LEN], char p4[MAX_INPUT_LEN]);
int print_db = 0;
void build_fd_sets(int sockfd, fd_set *read_fds);
void incoming_invite(int sockfd, struct message *buf);
void incoming_invite_fail(struct message *buf);
void incoming_invite_reject(struct message *buf);
void incoming_invite_accept(struct message *buf);
void invite_client(int socket, char input[MAX_INPUT_LEN], char command[MAX_INPUT_LEN],
		char p1[MAX_INPUT_LEN], char p2[MAX_INPUT_LEN]);

void incoming_message(struct message *buf);

// Returns positive integer if success (sockfd) returns negative
int login(char input[MAX_INPUT_LEN], char command[MAX_INPUT_LEN],
		char p1[MAX_INPUT_LEN], char p2[MAX_INPUT_LEN],
		char p3[MAX_INPUT_LEN], char p4[MAX_INPUT_LEN]) {

	// Make sure the first command issued is login
	int logged_in = 0;
	int port;
	int sockfd;
	struct message buf, out;
	int numbytes;

	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	do {	
		memset(input, 0, MAX_INPUT_LEN);
		memset(command, 0, MAX_INPUT_LEN);
		memset(p1, 0, MAX_INPUT_LEN);
		memset(p2, 0, MAX_INPUT_LEN);
		memset(p3, 0, MAX_INPUT_LEN);
		memset(p4, 0, MAX_INPUT_LEN);

		// Parse Input
		printf("> ");
		fgets(input, MAX_INPUT_LEN, stdin);
		sscanf(input, "%s", command);

		// Quit process if entered command is quit
		if (strcmp(command, "/quit") == 0) {
			printf("Exiting program...\n");
			exit(0);
		}

	} while ( strcmp(command, "/login") != 0);


	// Loading parameters
	sscanf(input, "%s %s %s %s %s", command, p1, p2, p3, p4);
	//printf("%s\n%s\n%s\n%s\n%s\n", command, p1, p2, p3, p4);
	strcpy(username, p1);

	if ((rv = getaddrinfo(p3, p4, &hints, &servinfo)) != 0) {
		perror("getaddrinfo fault\n");
		exit(0);
	}
	// Loop through results
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if ( connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		// successful connect
		break;
	}

	if (p == NULL) {
		perror("didn't connect\n");
		close(sockfd);
		return -4;
	}

	// Sending login info
	out.type = LOGIN;
	strcpy(out.data, p1);
	char *sep = ",";
	strcat(out.data, sep);
	strcat(out.data, p2);
	strcpy(out.source, username);
	if ((numbytes = send(sockfd, &out, sizeof(struct message), 0)) < sizeof(struct message)) {
		perror("send less bytes on login \n");
		exit(1);
	}
	print_message(out, 1);

	if ((numbytes = recv(sockfd, &buf, sizeof(struct message), 0)) < sizeof(struct message)) {
		perror("recveived less bytes on login \n");
		exit(1);
	}
	print_message(buf, 0);
	freeaddrinfo(servinfo);

	// Checking the message
	if (buf.type == LO_ACK) {
		printf("Logged in\n");
		return sockfd;
	}

	else if (buf.type == LO_NACK) {

		printf("Couldn't log in: %s\n", buf.data) ;
		close(sockfd);
		return -1;
	}
	else if (buf.type != LO_ACK && buf.type != LO_NACK) {
		printf("Didn't expect this message: %d\n", buf.type);
		close(sockfd);
		return -2;
	}
	close(sockfd);
	return -3;
}

int join_session_client(char* sessionID, int sockfd){

	struct message out, buf;

	//set out message 
	memset(&out, 0, sizeof(struct message));
	memset(&buf, 0, sizeof(struct message));
	out.type= JOIN;
	out.size= strlen(sessionID);
	strcpy(out.data, sessionID);
	strcpy(out.source, username);

	int n= send(sockfd, &out, sizeof(struct message), 0);
	print_message(out, 1);
	if(n==-1){
		printf("Error sending join message\n");
		return 0;
	}
	print_message(out, 1);

	n= recv(sockfd, &buf, sizeof(struct message),0);
	print_message(buf, 0);
	if(n==-1){
		printf("Error receiving join ACK/NACK\n");
		return 0;
	}

	if(buf.type==JN_ACK){
		return 1;
	} else {
		printf("%s",buf.data);
		return 0;
	}
}

int create_session_client(char* sessionID, int sockfd){

	struct message out, buf;

	//set out message 
	memset(&out, 0, sizeof(struct message));
	memset(&buf, 0, sizeof(struct message));
	out.type= NEW_SESS;
	out.size = strlen(sessionID);
	strcpy(out.data, sessionID);

	strcpy(out.source, username);
	int n= send(sockfd, &out, sizeof(struct message), 0);
	print_message(out, 1);
	if(n==-1){
		printf("Error sending NS message\n");
		return 0;
	}

	n= recv(sockfd, &buf, sizeof(struct message), 0);
	print_message(buf, 0);
	if(n==-1){
		printf("Error receiving NS ACK/NACK\n");
		return 0;
	}

	if(buf.type==NS_ACK){
		return 1;
	} else {
		printf("%s",buf.data);
		return 0;
	}    

}

int leave_session_client(char* sessionID, int sockfd){

	struct message out;

	memset(&out, 0, sizeof(struct message));

	out.type= LEAVE_SESS;
	out.size = strlen(sessionID);
	strcpy(out.data, sessionID);
	strcpy(out.source, username);

	int n= send(sockfd, &out, sizeof(struct message), 0);
	if(n==-1){
		printf("Error sending NS message\n");
		return 0;
	}
	print_message(out, 1);

	return 1;
}


int exit_client(int sockfd) {
	struct message out;
	memset(&out, 0, sizeof(struct message));

	out.type = EXIT;

	strcpy(out.source, username);
	int n = send(sockfd, &out, sizeof(struct message), 0);
	close(sockfd);
	if (n == -1) {

		printf("Error exiting\n");
		return 0;
	}
	print_message(out, 1);
	return 1;
}

int query_client(int sockfd){

	struct message out, in;
	memset(&out, 0, sizeof(struct message));
	memset(&in, 0, sizeof(struct message));

	out.type= QUERY;
	strcpy(out.source, username);

	int n= send(sockfd, &out, sizeof(struct message), 0);

	print_message(out, 1);
	if(n==-1){
		printf("Error sending NS message\n");
		return 0;
	}

	n= recv(sockfd, &in, sizeof(struct message), 0);
	if(n==-1){
		printf("Error receiving NS ACK/NACK\n");
		return 0;
	}
	print_message(in, 0);

	if (in.type==QU_ACK){
		printf("Listing Chatrooms and Users:\n%s",in.data);
	}

	return 1;
}

int quit(int socket) {
	;
}

int send_text(char*input, int socket) {

	struct message out;

	memset(&out, 0, sizeof(struct message));

	out.type= MESSAGE;
	out.size= strlen(input);
	strcpy(out.data, input);
	strcpy(out.source, username);

	int n= send(socket, &out, sizeof(struct message), 0);

	print_message(out, 1);
	if(n==-1){
		printf("Error sending text message\n");
		return 0;
	}

	return 1;
}

void build_fd_sets(int sockfd, fd_set *read_fds) {
	FD_ZERO(read_fds);
	FD_SET(STDIN_FILENO, read_fds);

	if (sockfd != -1) {
		FD_SET(sockfd, read_fds);
	}
	return;
}

int sockfd = -1;
int main(int argc, char *argv[]) {

	char input[MAX_INPUT_LEN];
	char command[MAX_INPUT_LEN];
	char p1[MAX_INPUT_LEN];
	char p2[MAX_INPUT_LEN];
	char p3[MAX_INPUT_LEN];
	char p4[MAX_INPUT_LEN];


	int fdmax = 0;
	fd_set readfds;
	// Should login here as it saves a lot of work
	do {
		sockfd = login(input, command, p1, p2, p3, p4);
	} while (sockfd < 0);

	fd_set read_fds;
	int activity;
	while (1) {

		memset(input, 0, MAX_INPUT_LEN);
		memset(command, 0, MAX_INPUT_LEN);
		memset(p1, 0, MAX_INPUT_LEN);
		memset(p2, 0, MAX_INPUT_LEN);
		memset(p3, 0, MAX_INPUT_LEN);
		memset(p4, 0, MAX_INPUT_LEN);

		build_fd_sets(sockfd, &read_fds);
		printf("> ");
		fflush(stdout);

		if (sockfd > fdmax) {
			fdmax = sockfd;
		}

		activity = select(fdmax + 1, &read_fds, NULL, NULL, NULL);

		switch (activity) {
			case -1:
				perror("select()");
				return 0;
			case 0:
				printf("select() returns 0 \n");
			default:
				/* All fd_set's should be checked. */
				if (FD_ISSET(STDIN_FILENO, &read_fds)) {
					stdin_handler(sockfd, input, command, p1, p2, p3, p4);
				}

				if (FD_ISSET(sockfd, &read_fds)) {
					message_handler(sockfd); 
				}
		}
	}

	return 0;

}

void message_handler(int sockfd) {
	int numbytes;
	struct message buf;

	if ((numbytes = recv(sockfd, &buf, sizeof(struct message), 0)) < sizeof(struct message)) {
		perror("received less bytes on login \n");	
		exit(-1);
	}

	if (buf.type == MESSAGE) {
		incoming_message(&buf);
	}

	else if (buf.type == INVITE) {
		incoming_invite(sockfd, &buf);
	}

	else if (buf.type == INV_ACCEPT) {
		incoming_invite_accept(&buf);
	}

	else if (buf.type == INV_REJECT) {
		incoming_invite_reject(&buf);
	}

	else if (buf.type == INVITE_FAIL) {
		incoming_invite_fail(&buf);
	}
	return;
}

// Deciphers the message into chatroom_name and actual message and displays it
void incoming_message(struct message *buf) {
	char chatroom_name[MAX_ROOM_NAME] = {0};
	int	i = 0;
	while (buf->data[i] != ':') {
		chatroom_name[i] = buf->data[i];
		i++;
	}

	char* message = buf->data + i  + 1;
	printf("(%s) %s: %s", chatroom_name, buf->source, message);
}

// Handles all stdin activity
void stdin_handler(int socket, char input[MAX_INPUT_LEN], char command[MAX_INPUT_LEN],
		char p1[MAX_INPUT_LEN], char p2[MAX_INPUT_LEN],
		char p3[MAX_INPUT_LEN], char p4[MAX_INPUT_LEN]) {	

	// Ensures the input is not just a new line
	do {
		fgets(input, MAX_INPUT_LEN, stdin);
		sscanf(input, "%s", command);
	} while (command[0] == '\n');

	// COMMAD Switching
	// TO DO: change abbreviations of commands to actual ones
	if (strcmp(command, "/createsession") == 0) {
		sscanf(input, "%s %s", command, p1);
		create_session_client(p1, socket);
	}

	else if (strcmp(command, "/leavesession") == 0) {
		sscanf(input, "%s %s", command, p1);
		leave_session_client(p1, socket);
	}

	else if (strcmp(command, "/joinsession") == 0) {
		sscanf(input, "%s %s", command, p1);
		join_session_client(p1, socket);
	}

	else if (strcmp(command, "/logout") == 0) {
		exit_client(socket);

		// Poll for login again
		do {
			socket = login(input, command, p1, p2, p3, p4);
		} while (socket < 0);
	}

	else if (strcmp(command, "/list") == 0) {
		query_client(socket);
	}

	else if (strcmp(command, "/quit") == 0) {
		exit_client(socket);
		exit(0);
	}

	else if (strcmp(command, "/invite") == 0) {
		invite_client(socket, input, command, p1, p2);
	}

	// Message was sent
	else {
		send_text(input, socket);
	}
}


void print_message(struct message m, int is_send) {
	if (print_db == 1) {
		if (is_send == 1) printf("Sending message: ");
		else printf("Receiving message: ");
		printf("%d\t %s\n", m.type, m.data);
		return;
	}
	return;
}

// Follow the login function to first memset commands and parameters p1 and p2
// load up the values from input with sscanf
// Sends a message with type invite, data: client_username,room_name
// Expected to be entered from p1,p2 respectively
// NOTE: THIS IS THE FUNCTION THAT HANDLES STDIN invite
// i.e. inviting another client
void invite_client(int socket, char input[MAX_INPUT_LEN], char command[MAX_INPUT_LEN],
		char p1[MAX_INPUT_LEN], char p2[MAX_INPUT_LEN]) {
	
	struct message out;
	memset(&out, 0, sizeof(struct message));
	
	memset(command, 0, MAX_INPUT_LEN);
	memset(p1, 0, MAX_INPUT_LEN);
	memset(p2, 0, MAX_INPUT_LEN);


	sscanf(input, "%s %s %s", command, p1, p2);
	
	char data_send[MAX_INPUT_LEN];
	strcpy(data_send,p1);
	strcat(data_send," ");
	strcat(data_send,p2);

	out.type= INVITE;
	strcpy(out.source, username);
	strcpy(out.data, data_send);

	int n= send(socket, &out, sizeof(struct message), 0);
	if(n==-1){
		printf("Error sending INVITE message\n");
		return;
	}
	print_message(out, 1);

}

// Select statement right now polls from the socket which connects to the server as well as STDIN
// Now the poll from the socket to the server only handles incoming messages from the chatroom that the client is in
// We want it to handle 3 more types of messages
// 1: an invitation to join a chatroom
// 		Once this invitation is received, the user's next command has to be yes or no, i.e. it no longer accepts commands such as /js, /cs
// 2: invitation accept
//		This message only appears if the client has previously sent out an invitation for another client, indicating whether or not
//		the client invited has joined or not
// 3: invitation reject
// NOTE: THIS IS THE FUNCTION THAT HANDLES INcoming invite
// i.e. getting invited
void incoming_invite(int sockfd, struct message *buf) {
	// decipher the incoming message and print respective incoming room and etc
	
	struct message out;
	memset(&out, 0, sizeof(struct message));
	
	char username_from[MAX_INPUT_LEN];
	char username_me[MAX_INPUT_LEN];
	char session_name[MAX_INPUT_LEN];
	strcpy(username_from,buf->source);
	sscanf(buf->data, "%s %s", username_me , session_name);

	char answer;
	char input[50];
	printf("Invite from %s to session: %s. Do you accept or reject? (y/n): ", username_from, session_name);
	fflush(stdout);

	fgets(input, MAX_INPUT_LEN, stdin);
	answer = input[0];

	// answer handling

	if(answer == 'y'){
		out.type=INV_ACCEPT;

	}else{
		out.type=INV_REJECT;
		
	}
	strcpy(out.source, buf->source);
	strcpy(out.data, buf->data);
	int n= send(sockfd, &out, sizeof(struct message), 0);
	if(n==-1){
		printf("Error sending INVITE Accept/Reject message\n");
		return;
	}
	print_message(out, 1);

}

void incoming_invite_accept(struct message *buf) {

	char username_from[MAX_INPUT_LEN];
	strcpy(username_from,buf->source);

	printf("User: %s accepted your invitation\n", username_from);
	
}

void incoming_invite_reject(struct message *buf) {

	char username_from[MAX_INPUT_LEN];
	strcpy(username_from,buf->source);

	printf("User: %s rejected your invitation\n", username_from);

}
void incoming_invite_fail(struct message *buf){
	
	char data[MAX_DATA];
	strcpy(data,buf->data);

	printf("%s", data);
}
