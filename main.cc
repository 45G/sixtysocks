#include <stdlib.h>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "poller.hh"

using namespace std;

void usage()
{
	cerr << "usage: woolsocks [-j <thread count>] [-o <cpu offset>]" << endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	char c;
	opterr = 0;
	int numThreads = thread::hardware_concurrency();
	int cpuOffset = 0;
	
	while ((c = getopt(argc, argv, "j:o:")) != -1)
	{
		switch (c)
		{
		case 'j':
			numThreads = atoi(optarg);
			if (numThreads < 0)
				usage();
			break;
			
		case 'o':
			cpuOffset = atoi(optarg);
			if (cpuOffset < 0)
				usage();
			break;
			
		default:
			usage();
		}
	}
	if (c == -1)
		usage();
	
	if (cpuOffset + numThreads > (int)thread::hardware_concurrency())
		usage();
	
	Poller poller(numThreads, cpuOffset);
	
	return 0;
}
