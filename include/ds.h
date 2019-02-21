
#define MAX_ROOMS 15
#define MAX_USERS 10
#define MAX_DATA 1000
#define MAX_USER_LENGTH 100
#define MAX_CLIENTS 10
#define MAX_ROOM_NAME 50
#define MAX_IP 20

struct login_cred { 
	char username[MAX_USER_LENGTH];
	char password[MAX_USER_LENGTH];
	int status; // Purely for avoiding two clients logining in from the same account
};

struct user {
	int socket;
	char ip[MAX_IP];
	int port;
	
	int user_id;
	char status;

	int room_index[MAX_ROOMS];
};

struct chatroom {
	char room_name[MAX_ROOM_NAME];
	int num_p;
	int participants[MAX_USERS];
};

struct server {
	struct chatroom cr[MAX_ROOMS];
	struct login_cred login_c[MAX_CLIENTS];
	struct user users[MAX_CLIENTS];
	int lock;
};



struct client_state {
	char loggedIn;
	char inRoom;
};

struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_USER_LENGTH];
	unsigned char data[MAX_DATA];
};

enum {
	LOGIN = 0,
	LO_ACK = 1,
	LO_NACK = 2,
	EXIT = 3,
	JOIN = 4,
	JN_ACK = 5,
	JN_NACK = 6,
	LEAVE_SESS = 7,
	NEW_SESS = 8,
	NS_ACK = 9,
	NS_NACK = 10,
	MESSAGE = 11,
	QUERY = 12,
	QU_ACK =13,
	INVITE = 14,
	INV_ACCEPT = 15,
	INV_REJECT = 16,
	INVITE_FAIL = 17,
};

void print_message(struct message, int is_send);
