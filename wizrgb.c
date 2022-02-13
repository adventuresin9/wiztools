#include <u.h>
#include <libc.h>

void
main(int argc, char *argv[])
{

	int fd, r, g, b;
	char buf[1024];
	memset(buf, 0, 1024);
	r = g = b = 0;

	r = atoi(argv[2]);
	g = atoi(argv[3]);
	b = atoi(argv[4]);

	if(r < 0 || r > 255)
		sysfatal("must be 0 to 255");

	if(g < 0 || g > 255)
		sysfatal("must be 0 to 255");

	if(b < 0 || b > 255)
		sysfatal("must be 0 to 255");

	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, "{\"id\":1,\"method\":\"setPilot\",\"params\":{\"r\":%d,\"g\":%d,\"b\":%d}}", r, g, b);
	sleep(1);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");

	close(fd);

	exits(nil);
}