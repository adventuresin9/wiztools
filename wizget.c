#include <u.h>
#include <libc.h>

#define GREET "{\"method\":\"getPilot\",\"params\":{}}"
#define WIZINFO "{\"method\":\"getSystemConfig\", \"params\":{}}"


void
main(int argc, char *argv[])
{

	int fd, n;
	char buf[1024];
	memset(buf, 0, 1024);

	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, GREET);
	sleep(10);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n\n");

	sleep(10);

	memset(buf, 0, 1024);
	fprint(fd, WIZINFO);
	sleep(10);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");


	close(fd);

	exits(nil);
}
	

	

