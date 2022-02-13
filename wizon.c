#include <u.h>
#include <libc.h>

#define WIZON "{\"method\": \"setPilot\", \"id\": 24, \"params\": {\"state\": true}}"


void
main(int argc, char *argv[])
{

	int fd;
	char buf[1024];
	memset(buf, 0, 1024);


	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, WIZON);
	sleep(1);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");

	close(fd);

	exits(nil);
}