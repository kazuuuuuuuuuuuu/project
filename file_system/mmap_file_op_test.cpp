#include "mmap_file_op.h"

using namespace std;
using namespace file_system;

const static MMap_Option mmap_option = {10240000, 4096, 4096};

int main(int argc, char const *argv[])
{
	printf("test begin\n");
	string filename = "mmap_file_op_test.txt";
	int open_flags = O_RDWR|O_LARGEFILE|O_CREAT;
	MMapFileOperation *mmfo = new MMapFileOperation(filename, open_flags);

	// open file
	if(!mmfo->open_file())
	{
		fprintf(stderr, "mmfo->open_file failed\n");
		exit(-1);
	}
	/*
	// map file or not
	if(!mmfo->mmap_file(mmap_option))
	{
		fprintf(stderr, "mmfo->mmap_file failed\n");
		mmfo->close_file();
		exit(-2);		
	}
	*/

	char buffer[65];
	memset(buffer, '1', 64);

	// write
	if(!mmfo->pwrite_file(buffer, 64, 1))
	{
		fprintf(stderr, "mmfo->pwrite_file failed\n");
	}

	memset(buffer, 0, 65);
	// read
	if(!mmfo->pread_file(buffer, 64, 1))
	{
		fprintf(stderr, "mmfo->pread_file failed\n");
	}
	buffer[64] = '\0';
	cout << "buffer: " << buffer;

	// flush
	if(!mmfo->flush_file())
	{
		fprintf(stderr, "flush_file failed: %s", strerror(errno));
	}

	// unmap
	mmfo->munmap_file();

	// close
	mmfo->close_file();

	return 0;
}