#include <u.h>
#include <libc.h>

void
main(int argc, char *argv[])
{

	int fd, temp;
	char buf[1024];
	memset(buf, 0, 1024);

	temp = atoi(argv[2]);

	if(temp < 2200 || temp > 6500)
		sysfatal("temp range 2200 to 6500");

	fd = dial(netmkaddr(argv[1], "udp", "38899"), nil, nil, nil);

	fprint(fd, "{\"id\":1,\"method\":\"setPilot\",\"params\":{\"temp\":%d}}", temp);
	sleep(1);
	read(fd, buf, sizeof buf);
	print(buf);
	print("\n");

	close(fd);

	exits(nil);
}