#define COSI_READBUF_LEN 512

typedef struct cosi_instance_s {
	char username[128]; // Username of the player
	int pid;            // Process ID for the game
	int writefd;        // Write to the game's stdin
	int readfd;         // Read from the game's stdout
	// TODO: Put a timestamp in here for timeouts
	struct cosi_instance_s * next;
} cosi_instance_t;

int cosi_check_instance(char * username);
int cosi_create_instance(char * username, char * readbuf);
int cosi_remove_instance(char * username);
int cosi_send_instruction(char * username, char * sendbuf, char * readbuf);

