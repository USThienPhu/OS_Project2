#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

void cpu_intensive(int duration)
{
	int start = uptime();
	int volatile x = 0;
	while (uptime() - start < duration)
	{
		x++;
	}
}

int main(int argc, char *argv[])
{
	struct sysinfo info;
	int i, n;

	printf("Load average test\n");

	// Test 1: Idle system
	printf("\n=== Test 1: Idle system ===\n");
	sleep(20);
	sysinfo(&info);
	printf("Load average: %lu.%lu\n", info.loadavg / 100, info.loadavg % 100); // Test 2: Fork some CPU-intensive processes
	printf("\n=== Test 2: Creating 3 CPU-intensive processes ===\n");
	for (i = 0; i < 3; i++)
	{
		n = fork();
		if (n < 0)
		{
			printf("fork failed\n");
			exit(1);
		}
		if (n == 0)
		{
			// Child: do CPU-intensive work
			cpu_intensive(100);
			exit(0);
		}
	}

	// Check load average periodically
	for (i = 0; i < 5; i++)
	{
		sleep(20);
		sysinfo(&info);
		printf("Load average: %lu.%lu (nproc: %lu, freemem: %lu KB)\n",
					 info.loadavg / 100, info.loadavg % 100,
					 info.nproc, info.freemem / 1024);
	}

	// Wait for children
	for (i = 0; i < 3; i++)
	{
		wait(0);
	}

	// Test 3: After processes finish
	printf("\n=== Test 3: After processes finish ===\n");
	sleep(20);
	sysinfo(&info);
	printf("Load average: %lu.%lu\n", info.loadavg / 100, info.loadavg % 100);
	printf("\nLoad average test completed!\n");
	exit(0);
}
