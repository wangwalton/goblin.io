#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <assert.h>
#include "ds.h"


int build_fd_set(struct server *sv, fd_set* readfds, int sockfd);
void initialize_server(struct server *sv);
char* my_itoa(int num, char *str);
int check_login_cred(char username[MAX_USER_LENGTH], char password[MAX_USER_LENGTH], struct server *sv);
void print_chatrooms(struct server* sv);
void print_clients(struct server* sv);

void message_handler(struct server* sv, struct message buf, int i);
int login(struct message* m, struct server* sv, unsigned int p);
void exit_server(struct server* sv, unsigned int p);
void join(struct message* m, struct server* sv, unsigned int p, int send_message);

// Specify which room to leave
// Also need to check if it's in anyroom
// if all == 1, message doesn't need to be specified
void leave_sess(struct message* m, struct server* sv, unsigned int p, char all);
void message(struct message* m, struct server* sv, unsigned int p);
void new_sess(struct message* m, struct server* sv, unsigned int p);
void query(struct server* sv, unsigned int p);
void invite(struct message* m, struct server* sv, unsigned int p);
void invite_accept(struct message* m, struct server* sv, unsigned int p);
void invite_reject(struct message* m, struct server* sv, unsigned int p);

int main(int argc, char *argv[]){

	// mapping a global variable to be shared across processes
	struct server *sv = malloc(sizeof(struct server));

	//initialize server with users and passwords
	initialize_server(sv);

	// Defining static variables
	int sockfd, newsockfd;
	if (argc < 2) {
		printf("Too few arguments!\n");
		exit(-1);
	}
	int port = atoi(argv[1]);
	struct sockaddr_in serv_addr, cli_addr;
	int clilen = sizeof(struct sockaddr_in);
	int pid;


	// Error variables
	int n;

	// Get socket 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// Error checking
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	// Bind socket to port
	// Preping serv_addr
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	// Binding error
	int yes = 1;
	while (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		port = port + 1;
		serv_addr.sin_port = htons(port);
	}
	// Make socket useable to get rid of "address already in use" error message
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));


	printf("Starting server on port %d...\n", port);	
	printf("Waiting for incoming connections...\n");

	// Start waiting for incoming connections, will go to sleep and wait for incoming connections
	listen(sockfd, MAX_CLIENTS);
	int nbytes;

	fd_set readfds;
	int currfd = -1;
	int newfd = -1;
	int fdmax = sockfd;

	int FD_ER = 0;
	socklen_t FD_ER_len = sizeof(FD_ER);
	struct message buf;
	// Main loop handling requests
	while (1) {

		// building fd set
		fdmax = build_fd_set(sv, &readfds, sockfd);

		if (select(fdmax+1, &readfds, NULL, NULL, NULL) == -1) {
			printf("Select error!\n");
			exit(-1);
		}

		// Server socket change, new client
		if (FD_ISSET(sockfd, &readfds)) {
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);


			printf("Received new client %d\n", newsockfd);

			if (newsockfd < 0) {
				perror("ERROR on accept");
				exit(1);
			}

			// Setting up users
			else {
				getpeername(newsockfd, (struct sockaddr *) &cli_addr, &clilen);
				char * client_ip_addr = inet_ntoa(cli_addr.sin_addr);
				int cli_port = ntohs(cli_addr.sin_port);

				for (int i = 0; i < MAX_CLIENTS; i++) {
					if (sv->users[i].socket == 0) {
						sv->users[i].socket = newsockfd;
						sv->users[i].port = cli_port;
						strcpy(sv->users[i].ip, client_ip_addr);

						// Manually calling loggin here
						memset(&buf, 0, sizeof(struct message));
						if ((nbytes = recv(newsockfd, &buf, sizeof(struct message), 0)) <= 0) {
							if (nbytes == 0) {
								printf("Socket %d already close connection\n", newsockfd);
							}

							else {
								printf("Recv failed\n");
							}
							exit_server(sv, i);
						}
						login(&buf,sv,  i);
						break;
					}
				}
			}
		}

		// One of the already established connections is ready
		for (int i = 0; i < MAX_CLIENTS; i++) {
			currfd = sv->users[i].socket;

			// Don't need to check if its greater than 0 because fd_isset if faster (checking bits
			if (FD_ISSET(currfd, &readfds)) {
				printf("Detected activity at socket %d, index %d\n", currfd, i);
				memset(&buf, 0, sizeof(struct message));
				if ((nbytes = recv(currfd, &buf, sizeof(struct message), 0)) <= 0) {
					if (nbytes == 0) {
						printf("Socket %d already closed connection\n", currfd);
					}

					else {
						printf("Recv failed\n");
					}

					exit_server(sv, i);
				}
				printf("About to execute message_handler\n");
				message_handler(sv, buf, i);

			}
		}
	}
}

void message_handler(struct server* sv, struct message buf, int i) {
	if (buf.type == LOGIN) {
		login(&buf, sv, i); 
	}
	else if (buf.type == EXIT) {
		exit_server(sv, i);
	}

	else if (buf.type == JOIN) {
		join(&buf, sv, i, 1);
	}

	else if (buf.type == LEAVE_SESS) {
		leave_sess(&buf, sv, i, 0);
	}

	else if (buf.type == NEW_SESS) {
		new_sess(&buf, sv, i);
	}

	else if (buf.type == MESSAGE) {
		message(&buf, sv, i);
	}

	else if (buf.type == QUERY) {
		query(sv, i);
	}
	else if (buf.type == INVITE) {
		invite(&buf, sv, i);
	}
	else if (buf.type == INV_ACCEPT) {
		invite_accept(&buf, sv, i);

	}
	else if (buf.type == INV_REJECT) {
		invite_reject(&buf, sv, i);
	}
}


//returns index if valid, -1 if wrong username, -2 if wrong password, -3 if already logged in
int check_login_cred(char username[MAX_USER_LENGTH], char password[MAX_USER_LENGTH], struct server *sv){

	int i;
	for( i=0; i<MAX_CLIENTS; i++){
		char userx[MAX_USER_LENGTH];
		char usery[MAX_USER_LENGTH];
		memset(&userx, '\0', sizeof(userx));
		memset(&usery, '\0', sizeof(usery));
		strcpy(userx,sv->login_c[i].username);
		strcpy(usery,username);
		if(strcmp(userx, usery)==0){
			break;
		}
	}
	if(i==MAX_CLIENTS) return -1;

	if(strcmp(sv->login_c[i].password, password)==0&& sv->login_c[i].status==0){
		return i;
	} else if(strcmp(sv->login_c[i].password, password)==0&& sv->login_c[i].status==1){
		return -3;
	} else {
		return -2;
	}

}

//set username and passwords to all users
void initialize_server(struct server *sv){

	memset(sv, 0, sizeof(struct server));

	// Generates user and passwords
	for(int i=0; i<MAX_CLIENTS; i++){

		char a[10];
		my_itoa(i,a);

		char username[MAX_USER_LENGTH]= "u";
		char password[MAX_USER_LENGTH]= "p";
		strcat(username,a);
		strcat(password,a);
		strcpy(sv->login_c[i].username, username);
		strcpy(sv->login_c[i].password, password);

		// Setting room_index  to 0;
		for (int j = 0; j < MAX_ROOMS; j++) {
			sv->users[i].room_index[j] = -1;
		}

		sv->users[i].user_id = -1;
	}
}

char* my_itoa(int num, char *str) {
	if(str == NULL)
	{
		return NULL;
	}
	sprintf(str, "%d", num);
	return str;
}

int login(struct message* m, struct server* sv, unsigned int p) {

	struct message out;
	memset(&out, 0, sizeof(struct message));

	// Parse message data into username and password
	char username[MAX_USER_LENGTH];
	char password[MAX_USER_LENGTH];

	memset(username, '\0', MAX_USER_LENGTH);
	memset(password, '\0', MAX_USER_LENGTH);

	int i = 0;
	int comma_indice = 0;

	// Note client has changed login structure
	for (int i = 0;  m->data[i] != '\0'; i++) {
		if (m->data[i] == ',') {
			comma_indice = i;
			continue;
		}

		if (comma_indice == 0) {
			username[i] = m->data[i];
		}

		else {
			password[i - comma_indice - 1] = m->data[i];
		}
	}

	int cred_res = check_login_cred(username, password, sv);

	int ret;	
	// Successful login
	if (cred_res >= 0) {
		printf("\tSuccessful login from %d\n", sv->users[p].socket);
		sv->login_c[cred_res].status = 1;
		sv->users[p].status = 1;
		sv->users[p].user_id = cred_res;

		out.type = LO_ACK;
		ret = 1;
	}

	// Wrong username
	if (cred_res == -1) {
		out.type = LO_NACK;
		strcpy(out.data, "Wrong Username");
		printf("%s\n", out.data);
		ret = 0;
	}

	// Wrong password
	if (cred_res == -2) {
		out.type = LO_NACK;
		strcpy(out.data, "Wrong Password");
		ret = 0;
	}

	// User already logged in
	if (cred_res == -3) {
		out.type = LO_NACK;
		strcpy(out.data, "Already logged in");
		printf("%s\n", out.data);
		ret = 0;
	}

	int sockfd = sv->users[p].socket;
	print_message(out, 1);
	send(sockfd, &out, sizeof(struct message), 0);
	return ret;
}

void exit_server(struct server* sv, unsigned int p) {

	// Leave room
	printf("Client %s has left the server\n", sv->login_c[sv->users[p].user_id].username); 
	leave_sess(0, sv, p, 1);
	sv->login_c[sv->users[p].user_id].status = 0;
	close(sv->users[p].socket);
	memset(&(sv->users[p]), 0, sizeof(struct user));
	sv->users[p].socket = 0;
	sv->users[p].user_id = -1;

	for (int i = 0; i < MAX_ROOMS; i++) {
		sv->users[p].room_index[i] = -1;
	}
	// TODO, update ip and port
	return;
}

void join(struct message* m, struct server* sv, unsigned int p, int send_message) {

	int sockfd = sv->users[p].socket;
	// Prepping out message
	struct message out;
	memset(&out, 0, sizeof(struct message));

	// Check if client already in a room
	// Originally defaults to NULL
	//	if (sv->users[p].room_index == -1) {
	// Clients can joing multiple rooms now
	//		out.type = JN_NACK;
	//		strcpy(out.data, "User already in room");
	//	}

	// Check if room exists
	int room_found = -1;
	for (int i = 0; i < MAX_ROOMS; i++) {
		if (strcmp(m->data, sv->cr[i].room_name) == 0) {
			room_found = i;
			break;
		}
	}

	// Entered a room that doesn't exit
	if (room_found == -1) {
		out.type = JN_NACK;
		strcpy(out.data, "Room doesn't exist");
	}

	// Room is found
	else {

		for (int j = 0; j < MAX_ROOMS; j++) {
			if (sv->users[p].room_index[j] == room_found) {
				out.type = JN_NACK;
				strcpy(out.data, "Already in room\n");
				if (send_message == 1) {
					send(sockfd, &out, sizeof(struct message), 0);
					print_message(out, 1);
				}
				return;
			}
		}
		// update client state
		// append sockfd to chatroom list
		// prep message

		// Finding free slot on client side
		int i;
		for (i = 0; i < MAX_ROOMS; i++) {
			if (sv->users[p].room_index[i] == -1) {
				sv->users[p].room_index[i] = room_found;
				break;
			}
		}
		assert(i != MAX_ROOMS);
		// Find next available spot to append sockfd
		for (int i = 0; i < MAX_USERS; i++) {
			if (sv->cr[room_found].participants[i] == 0) {
				sv->cr[room_found].participants[i] = sockfd;
				break;
			}
		}

		sv->cr[room_found].num_p++;

		// Prep message
		out.type = JN_ACK;
		strcpy(out.data, m->data);
	}

	if (send_message == 1) {
		send(sockfd, &out, sizeof(struct message), 0);
		print_message(out, 1);
	}
	return;
}

// TO DO: implement ack from successful leave
void leave_sess(struct message* m, struct server* sv, unsigned int p, char all) {

	int sockfd = sv->users[p].socket;
	// Leaving specific room
	if (all == 0) {

		// Find room index
		char* room_name = m->data;

		
		// i is the room_index
		// Looping over all the chatrooms
		for (int i = 0; i < MAX_ROOMS; i++) {
			if (strcmp(room_name, sv->cr[i].room_name) == 0) {

				// Updating chatroom state
				// Looping over all the participants in a chatrooms
				for (int j = 0; j < MAX_USERS; j++) {
					if (sv->cr[i].participants[j] == sockfd) {
						sv->cr[i].num_p--;
						sv->cr[i].participants[j] = 0;

						// No one left in the room, delete room
						if (sv->cr[i].num_p == 0) {
							memset(sv->cr[i].room_name, 0, MAX_ROOM_NAME);
						}
						
						// Updating the partipant's room_index array
						for ( int k = 0; k < MAX_ROOMS; k++ ) {
							if (sv->users[p].room_index[k] == i) {
								sv->users[p].room_index[k] = -1;
								return;
							}
						}
					}
				}
			}
		}
	}
	
	// Leave all rooms that the client is in
	else {
		// Looping over all of the room that a client is in
		for (int i = 0; i < MAX_ROOMS; i++) {
			if (sv->users[p].room_index[i] != -1) {

				// Leaving room index ri
				int ri = sv->users[p].room_index[i];
				sv->users[p].room_index[i] = -1;
				
				sv->cr[ri].num_p--;
				if (sv->cr[ri].num_p == 0) {
					memset(sv->cr[i].room_name, 0, MAX_ROOM_NAME);
				}

				// Declared outside for assert statement
				// Updating the chatroom partipant array
				int j;
				for (j = 0; j < MAX_USERS; j++) {
					if (sv->cr[ri].participants[j] == sockfd) {
						sv->cr[ri].participants[j] = 0;
						break;
					}
				}
				assert( j != MAX_USERS);
			}
		}
	}
}

void new_sess(struct message* m, struct server* sv, unsigned int p) {
	struct message out;
	int sockfd = sv->users[p].socket;
	memset(&out, 0, sizeof(struct message));

	int room_available = 0;

	//check if room exists already
	for (int i = 0; i < MAX_ROOMS; i++) {
		//if same name
		if (strcmp(sv->cr[i].room_name,m->data)==0) {
			join(m, sv, p, 0);
			out.type = NS_NACK;
			strcpy(out.data, "Room already exists, joined room");
			send(sockfd, &out, sizeof(struct message), 0);
			print_message(out, 1);
			return;
		}
	}

	// Updating chatroom
	for (int i = 0; i < MAX_ROOMS; i++) {

		// Finding free room
		if (sv->cr[i].room_name[0] == '\0') {
			strcpy(sv->cr[i].room_name, m->data);
			room_available = 1;
			break;
		}
	}

	// Nothing was updated, No more rooms available
	if (room_available == 0) {
		//printf("No more rooms available\n");
		out.type = NS_NACK;
		strcpy(out.data, "No more rooms available");
		send(sockfd, &out, sizeof(struct message), 0);
		print_message(out, 1);
	}

	else {
		join(m, sv, p, 0);
		out.type = NS_ACK;
		strcpy(out.data, m->data);
		send(sockfd, &out, sizeof(struct message), 0);
		print_message(out, 1);
	}
	return;
}

void message(struct message* m, struct server* sv, unsigned int p) {
	// Now m contains sessionID,DATA
	char room_broadcast[MAX_ROOM_NAME] = {0};
	int i = 0;
	while (m->data[i] != ':') {
		room_broadcast[i] = m->data[i];
		i++;
		if ( i == MAX_ROOM_NAME) {
			return;
		}
	}
	char* message = m->data + i + 1;
	
	// Check if user is in room
	char in_right_room = 0;
	int rii;
	for (int j = 0; j < MAX_ROOMS; j++) {
		rii = sv->users[p].room_index[j];
		if (rii != -1) {
			if (strcmp(room_broadcast, sv->cr[rii].room_name) == 0) {
				in_right_room = 1;
				break;
			}
		}
	}

	if (in_right_room == 0) {
		printf("Not in the right room.\n");
		return;
	}

	// Finding correct room that the client is sending (index)
	int room_index;
	for (room_index = 0; room_index < MAX_ROOMS; room_index++) {
		if (strcmp(room_broadcast, sv->cr[room_index].room_name) == 0) {
			break;
		}
	}

	if (room_index == MAX_ROOMS) {
		printf("No rooms named %s\n", room_broadcast);
		return;
	}

	int sockfd = sv->users[p].socket;

	printf("\tEveryone in the room: ");
	for (int i = 0; i < MAX_USERS; i++) {
		if (sv->cr[room_index].participants[i] != 0) {
			printf("%d ", sv->cr[room_index].participants[i]);
		}
	}
	printf(" \n");

	// Relay message to everyone in the chatroom
	for (int i = 0; i < MAX_USERS; i++) {
		if (sv->cr[room_index].participants[i] != 0) {
			if (sv->cr[room_index].participants[i] != sockfd) {

				send(sv->cr[room_index].participants[i], m, sizeof(struct message), 0);
				print_message(*m, 1);
				printf("\tTo: %d\n", sv->cr[room_index].participants[i]);
			}
		}
	}
	return;
}

void query(struct server* sv, unsigned int p) {
	int sockfd = sv->users[p].socket;
	struct message out;
	memset(&out, 0, sizeof(struct message));
	out.type = QU_ACK;

	// Looping over actual rooms
	for (int i = 0; i < MAX_ROOMS; i++) {

		// Checking if it's a opened room
		if (sv->cr[i].room_name[0] == '\0') continue;
		
		strcat(out.data, sv->cr[i].room_name);
		strcat(out.data, ":\t");

		// Looping over all clients in a room
		for (int j = 0; j < MAX_CLIENTS; j++) {

			if (sv->cr[i].participants[j] == 0) continue;

			// Finding user name given socket number
			for (int k = 0; k < MAX_USERS; k++) {

				if (sv->users[k].socket != 0 && sv->users[k].socket == sv->cr[i].participants[j]) {
					strcat(out.data, sv->login_c[sv->users[k].user_id].username);
					strcat(out.data, " ");
					break;
				}
			}
		}
		strcat(out.data, "\n");
	}

	send(sockfd, &out, sizeof(struct message), 0);
	print_message(out, 1);
	return;
}


void print_message(struct message m, int is_send) {
	if (is_send == 1) printf("\tSending message: ");
	else printf("\tReceiving message: ");
	if (m.type != QUERY) {
		printf("%d %s  %s", m.type, m.source, m.data);
		if (m.type != MESSAGE) {
			printf("\n");
		}
	}
	return;
}

void print_chatrooms(struct server* sv) {
	printf("\n\tPrinting Chatrooms: (%d)\n", getpid());
	for (int i = 0; i < MAX_ROOMS; i++) {
		if (sv->cr[i].room_name[0] == '\0') {
			;
		}
		else {
			printf("\t%s: ", sv->cr[i].room_name);

			for (int j = 0; j < MAX_USERS; j++) {
				if (sv->cr[i].participants[j] != 0) {
					printf("\t%d", sv->cr[i].participants[j]);
				}
			}
			printf("\n");
		}
	}
}

void print_clients(struct server* sv) {
	printf("\n\tUsers that are currently online: (%d) \n", getpid());
	printf("\n\tUsername\troomname\tsocket");
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (sv->users[i].status == 1) {
			//printf("%s\t%s\t%d", sv->login_c[sv->users[i].user_id].username, sv->users[i].room_index, sv->users[i].socket);
		}
	}
}
int build_fd_set(struct server *sv, fd_set* readfds, int sockfd) {
	FD_ZERO(readfds);
	FD_SET(sockfd, readfds);
	int currfd;
	int FD_ER;
	socklen_t FD_ER_len = sizeof(FD_ER);
	int fdmax = sockfd;

	for (int j = 0; j < MAX_CLIENTS; j++) {
		// Need to make a data struture that holds incoming connection sockets
		// Initalized to -1
		currfd = sv->users[j].socket;

		if (currfd > 0) {
			FD_ER = 0;

			// Getting fd state to check if client disconnected

			int fdstate = getsockopt(currfd, SOL_SOCKET, SO_ERROR, &FD_ER, &FD_ER_len);

			if (fdstate ==0 && FD_ER == 0) {
				FD_SET(currfd, readfds);
			}

			else {
				printf("Hard disconnect socket %d, RUDE! Please implement this funciton on serverside\n", currfd);
				close(currfd);
				sv->users[j].socket = 0;
			}

			if (currfd > fdmax) {
				fdmax = currfd;
			}
		}

	}
	return fdmax;
}


// An invite packet send by client1 directed to client 2;
// Should simply forward this message to client 2;
void invite(struct message* m, struct server* sv, unsigned int p){

	struct message out;
	memset(&out, 0, sizeof(struct message));
	char username_from[MAX_DATA];
	char username_to[MAX_DATA];
	char session_name[MAX_DATA];
	strcpy(username_from,m->source);
	sscanf(m->data, "%s %s", username_to , session_name);
	int sockfd;
	int to_number;
	int from_number;
	int userFound=0;
	int sessionFound=0;

	//check if client 2 is online/exists
	for(int i=0; i<MAX_CLIENTS; i++){
		if(strcmp(sv->login_c[i].username, username_to)==0&&sv->login_c[i].status==1){
			to_number=i;
			userFound=1;
		} else if(strcmp(sv->login_c[i].username, username_from)==0){
			from_number=i;
		}
	}
	
	//check is session exists
	for(int i=0; i<MAX_ROOMS; i++){
		if(strcmp(sv->cr[i].room_name, session_name)==0){
			sessionFound=1;
			break;
		}
	}

	//send msg either to client 1 or client 2 depending if client 2 was found or if session was found
	if(userFound==0&&sessionFound==1){
		
		for(int i=0; i<MAX_CLIENTS; i++){
			if(sv->users[i].user_id== from_number){
				sockfd= sv->users[i].socket;
				break;
			}
		}
		//send error msg back to sender
		
		char data[MAX_DATA]="User not online.\n";
		strcpy(out.data, data);
		out.type=INVITE_FAIL;
		int n= send(sockfd, &out, sizeof(struct message), 0);
		if(n==-1){
			printf("Error sending INVITE message\n");
		return;
		}
		print_message(out, 1);
	
	} else if(userFound==1&&sessionFound==1){

		for(int i=0; i<MAX_CLIENTS; i++){
			if(sv->users[i].user_id== to_number){
				sockfd= sv->users[i].socket;
				break;
			}
		}
		//relay msg to client 2
		int n= send(sockfd, m, sizeof(struct message), 0);
		if(n==-1){
			printf("Error sending INVITE message\n");
		return;
		}
		print_message(*m, 1);

	} else if(userFound==1&&sessionFound==0){
		
		for(int i=0; i<MAX_CLIENTS; i++){
			if(sv->users[i].user_id== from_number){
				sockfd= sv->users[i].socket;
				break;
			}
		}
		//send error msg back to sender
		
		char data[MAX_DATA]="Session does not exist.\n";
		strcpy(out.data, data);
		out.type=INVITE_FAIL;
		int n= send(sockfd, &out, sizeof(struct message), 0);
		if(n==-1){
			printf("Error sending INVITE message\n");
		return;
		}
		print_message(out, 1);
	
	} else if(userFound==0&&sessionFound==0){
		
		for(int i=0; i<MAX_CLIENTS; i++){
			if(sv->users[i].user_id== from_number){
				sockfd= sv->users[i].socket;
				break;
			}
		}
		//send error msg back to sender
		
		char data[MAX_DATA]="User not online and session does not exist.\n";
		strcpy(out.data, data);
		out.type=INVITE_FAIL;
		int n= send(sockfd, &out, sizeof(struct message), 0);
		if(n==-1){
			printf("Error sending INVITE message\n");
		return;
		}
		print_message(out, 1);
	
	} 
	
}

// Client 2's response to an invitation message, call join room
// Forward message to Client 1 to let them know that client 2 has joined
void invite_accept(struct message* m, struct server* sv, unsigned int p) {
	
	struct message out;
	memset(&out, 0, sizeof(struct message));
	char username_from[MAX_DATA];
	char username_to[MAX_DATA];
	char session_name[MAX_DATA];
	strcpy(username_from,m->source);
	sscanf(m->data, "%s %s", username_to , session_name);	
	int from_number;
	int to_number;
	int to_sockfd;
	int from_sockfd;
	
	//determine socket to send to
	for(int i=0; i<MAX_CLIENTS; i++){
		if(strcmp(sv->login_c[i].username, username_to)==0&&sv->login_c[i].status==1){
			to_number=i;
		} else if(strcmp(sv->login_c[i].username, username_from)==0){
			from_number=i;
		}
	}

	for(int i=0; i<MAX_CLIENTS; i++){
		if(sv->users[i].user_id== from_number){
			from_sockfd= sv->users[i].socket;
		} else if(sv->users[i].user_id== to_number){
			to_sockfd= sv->users[i].socket;
		}
	}

	//accepted invite so move user into specified session
	
	struct message temp;
	memset(&temp, 0, sizeof(struct message));
	strcpy(temp.data, session_name);

	
    join(&temp, sv, to_number, 0);



	out.type=INV_ACCEPT;
	int n= send(from_sockfd, &out, sizeof(struct message), 0);
	if(n==-1){
		printf("Error sending INVITE Accept/Reject message\n");
		return;
	}
	print_message(out, 1);

}

// Client 2's response to an invitation message, don't call join room
// Forward message to Client 1 to let them know that client 2 has joined
// This message is immedietely forwarded back to client 1 if client 2 is not online
void invite_reject(struct message* m, struct server* sv, unsigned int p) {

	struct message out;
	memset(&out, 0, sizeof(struct message));
	char username_from[MAX_DATA];
	char username_to[MAX_DATA];
	char session_name[MAX_DATA];
	strcpy(username_from,m->source);
	sscanf(m->data, "%s %s", username_to , session_name);	
	int from_number;
	int sockfd;
	
	//determine socket to send to
	for(int i=0; i<MAX_CLIENTS; i++){
		if(strcmp(sv->login_c[i].username, username_from)==0&&sv->login_c[i].status==1){
			from_number=i;
		} 
	}

	for(int i=0; i<MAX_CLIENTS; i++){
		if(sv->users[i].user_id== from_number){
			sockfd= sv->users[i].socket;
			break;
		}
	}

	//rejected invite so dont need to do anything

	out.type=INV_REJECT;
	int n= send(sockfd, &out, sizeof(struct message), 0);
	if(n==-1){
		printf("Error sending INVITE Accept/Reject message\n");
		return;
	}
	print_message(out, 1);
}
