#include <u.h>
#include <libc.h>

void
main(int argc, char *argv[])
{

	int fd, dim;
	char buf[1024];
	memset(buf, 0, 1024);

	dim = atoi(argv[2]);

	if(dim < 10)
		dim = 10;

	if(dim > 100)
		dim = 100;

	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, "{\"id\":1,\"method\":\"setPilot\",\"params\":{\"dimming\":%d}}", dim);
	sleep(1);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");

	close(fd);

	exits(nil);
}
