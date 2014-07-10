#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include "cosi-bot.h"

cosi_instance_t * cosi_head = NULL;

static cosi_instance_t* ll_prepend(char * username)
{
	cosi_instance_t * newhead = malloc(sizeof(cosi_instance_t));
	strcpy(newhead->username, username);
	newhead->next = cosi_head;
	cosi_head = newhead;

	return newhead;
}


static cosi_instance_t* ll_find(char * username)
{
	cosi_instance_t * cur;
	for (cur = cosi_head; cur != NULL; cur = cur->next) {
		if (!strcmp(username,cur->username)) {
			return cur;
		}
	}
	
	return NULL;
}

/******** END LINKED LIST SECTION ********/

int cosi_check_instance(char * username)
{
	cosi_instance_t * check = ll_find(username);

	return ((check != NULL) ? 0 : 1);
}

int cosi_create_instance(char * username, char * readbuf)
{
	int write_to[2];
	int read_frm[2];
	int pid;
	cosi_instance_t * new;

	new = ll_prepend(username);

	pipe(write_to);
	pipe(read_frm);

	pid = fork();
	if (0 == pid) {
		// Child code
		close(write_to[1]);
		close(read_frm[0]);
		dup2(write_to[0], STDIN_FILENO);
		dup2(read_frm[1], STDOUT_FILENO);
		close(write_to[0]);
		close(read_frm[1]);
		execlp("stdbuf", "stdbuf", "-o", "0", "-i", "0", "./cosi-adventure", NULL);
		return 0;
	}
	close(write_to[0]);
	close(read_frm[1]);

	strcpy(new->username, username);
	new->writefd = write_to[1];
	new->readfd = read_frm[0];
	new->pid = pid;

	sleep(1);
	read(new->readfd, readbuf, COSI_READBUF_LEN);

	return 0;
}

int cosi_remove_instance(char * username)
{
	cosi_instance_t *cur, *prev;

	cur = cosi_head;
	while (cur != NULL) {
		if (!strcmp(username, cur->username)) {
			goto found;
		}
		prev = cur;
		cur = cur->next;
	}	
	return -1; // Could not find!
found:
	close(cur->writefd);
	close(cur->readfd);
	// free username?
	kill(cur->pid, SIGKILL);

	if (NULL == prev) { // Then its the head
		cosi_head = cur->next;
	} else {
		prev->next = cur->next;
	}
	free(cur);
	return 0;
}

// Send "sendbuf" to the game, and return "readbuf" to the bot
// Both buffers should be NULL-terminated, and note that readbuf should use COSI_READBUF_LEN
int cosi_send_instruction(char * username, char * sendbuf, char * readbuf)
{
	char modsendbuf[COSI_READBUF_LEN] = {0};
	cosi_instance_t * inst;
	int lastchar;

	inst = ll_find(username);

	strcpy(modsendbuf, sendbuf);
	lastchar = strlen(modsendbuf);
	modsendbuf[lastchar] = '\n';

	fprintf(stderr,"Writing input to game...\n");
	write(inst->writefd, modsendbuf, strlen(modsendbuf));	
	sleep(1);
	fprintf(stderr,"Reading game's output\n");
	read(inst->readfd, readbuf, COSI_READBUF_LEN);

	if (!strcmp(sendbuf, "quit")) {
		cosi_remove_instance(username);
	}

	return 0;
}
