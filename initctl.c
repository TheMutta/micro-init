#include <sys/socket.h>
#include <assert.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/init.sock" 

int main() {
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	struct sockaddr_un sock;
	sock.sun_family = AF_UNIX;
	strcpy(sock.sun_path, SOCKET_PATH);

	assert(connect(sockfd, (struct sockaddr*)&sock, sizeof(sock)) != -1);
}
