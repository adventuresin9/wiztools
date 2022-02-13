#include <u.h>
#include <libc.h>

void
main(int argc, char *argv[])
{

	int fd, state;
	char buf[1024];
	memset(buf, 0, 1024);

	state = atoi(argv[2]);

	if(state < 0 || state > 1)
		sysfatal("must be 0 or 1");

	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, "{\"id\":1,\"method\":\"setPilot\",\"params\":{\"state\":%d}}", state);
	sleep(1);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");

	close(fd);

	exits(nil);
}