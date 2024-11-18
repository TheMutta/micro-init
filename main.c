#define _GNU_SOURCE  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sched.h>
#include <string.h>
#include <sys/socket.h>
#include <assert.h>
#include <sys/un.h>

#define STACK_SIZE (64 * 1024)
#define SOCKET_PATH "/tmp/init.sock" 

pid_t create_thread(int (*func)(void*), void *arg) {
	return clone(func, malloc(STACK_SIZE) + STACK_SIZE, CLONE_VM | SIGCHLD, arg);
}

void emergency_shell() {
	/* Emergency shell */
	pid_t pid = fork();

	if (pid == 0) {
		char *prog[2] = { "/bin/sh", 0 };
		execv(prog[0], prog);
	} else {
		waitpid(pid, NULL, 0);
	}
}

int socket_handler() {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	struct sockaddr_un sock;
	sock.sun_family = AF_UNIX;
	strcpy(sock.sun_path, SOCKET_PATH);

	remove(SOCKET_PATH);
	assert(bind(sockfd, (struct sockaddr*)&sock, sizeof(sock)) != -1);

	listen(sockfd, 100);
	for (;;) {
		int acceptfd = accept(sockfd, NULL, NULL);
		assert(acceptfd != -1);


		close(acceptfd);
	}

	return EXIT_SUCCESS;
}


int init() {
	printf("Hello from child\n");

	pid_t pid = getpid();
    	printf("Current PID: %d\n", pid);

	clearenv();

	mount("none", "/dev", "devtmpfs", 0, "");
	mount("proc", "/proc", "proc", 0, "");
	mount("proc", "/proc", "proc", 0, "");
	mount("none", "/mnt", "tmpfs", 0, "");
    
        const char *new_hostname = "localhost";
        sethostname(new_hostname, strlen(new_hostname));

	create_thread(socket_handler, NULL);

	emergency_shell();

	return EXIT_SUCCESS;
}

int main() {
	pid_t pid = getpid();
    	printf("Current PID: %d\n", pid);

	/* If its not PID 1, we do a container PID 1 */
	if (pid != 1) {
		int clone_flags = 
				  CLONE_NEWNET |
				  CLONE_NEWPID |
				  CLONE_NEWNS |
				  CLONE_NEWUTS |
				  SIGCHLD;

		pid_t pid = 
			clone(init, malloc(STACK_SIZE) + STACK_SIZE, clone_flags, 0);
	
		waitpid(pid, NULL, 0);

		return EXIT_SUCCESS;
	} else {
		return init();
	}
}
