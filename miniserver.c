#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void	error(char *msg) {
	write(2, msg, strlen(msg));
	exit(1);
}

int	set_socket(int port) {
	int	sokt, max;
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if ((sokt = socket(addr.sin_family, SOCK_STREAM, 0)) == -1)
		return 0;
	if (bind(sokt, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		return 0;
	if (listen(sokt, 128) == -1)
		return 0;
	//here logic of select
	fd_set current, ready;
	FD_ZERO(&current);
	FD_SET(sokt, &current);
	//i think i should make the sokt nonblocking
	int acc;
	max = acc;
	while (1) {
		ready = current;
		if (select(max + 1, &ready, 0, 0, 0) < 0)
			error("Fatal error\n");
		for (int i = 0; i < max + 1; i++) {
			if (FD_ISSET(i, &ready)) {
				if (i == sokt) {
					acc = accept(sokt, 0, 0);
					if (acc < 0)
						error("Fatal error\n");
					FD_SET(acc, &current);
					if (max < acc)
						max = acc;
					//accept connection and read and set it to the current
				}
				else {
					//char buff[100] = {0};
					//read(i, buff, 50);
					//printf("%s", buff);
					char res[78] = "HTTP/1.1 200 OK\nContent-Length: 0\nContent-Type: text/html\nConnection: Closed\n\n";
					write(i, res, strlen(res));
					FD_CLR(i, &current);
					close(i);
					//send data
					//remove from ready in current
				}
			}
		}
	}
	return 1;
}

int	main(int ac, char **av) {
	if (ac != 2)
		error("Wrong number of arguments\n");
	if (!set_socket(atoi(av[1])))
		error("Fatal error\n");
	return 0;
}
