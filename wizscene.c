#include <u.h>
#include <libc.h>

void
main(int argc, char *argv[])
{

	int fd, sceneid;
	char buf[1024];
	memset(buf, 0, 1024);

	sceneid = atoi(argv[2]);

	if(sceneid < 1 || sceneid > 32)
		sysfatal("must be 1 to 32");

	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, "{\"id\":1,\"method\":\"setPilot\",\"params\":{\"sceneid\":%d}}", sceneid);
	sleep(1);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");

	close(fd);

	exits(nil);
}
