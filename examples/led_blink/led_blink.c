#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
	int fd = open("/proc/gpio", O_RDWR);

	write(fd, "mode,21,1", 9);
	while (1) {
		write(fd, "write,21,1", 10);
		usleep(500000);
		write(fd, "write,21,0", 10);
		usleep(500000);
	}
}
