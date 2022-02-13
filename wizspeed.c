#include <u.h>
#include <libc.h>

void
main(int argc, char *argv[])
{

	int fd, speed;
	char buf[1024];
	memset(buf, 0, 1024);

	speed = atoi(argv[2]);

	if(speed < 10 || speed > 200)
		sysfatal("must be 10 to 200");

	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, "{\"id\":1,\"method\":\"setPilot\",\"params\":{\"speed\":%d}}", speed);
	sleep(1);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");

	close(fd);

	exits(nil);
}