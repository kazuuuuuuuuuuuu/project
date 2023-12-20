#include "mmap_file.h"

using namespace std;
using namespace file_system;

static const mode_t OPEN_MODE = 0644; // when O_CREAT is set, set permissions for the new file (6->110 4->100)
const static MMap_Option mmap_option = {10240000, 4096, 4096};

int Open(const char * file_name, int open_flags)
{
	int fd = open(file_name, open_flags, OPEN_MODE);
	if(fd<0)
	{
		return -errno; // return a minus value when something goes wrong (errno is a positive number)
	}
	return fd;
}

int main(int argc, char const *argv[])
{
	const char *file_name = "./test_file.txt";
	
	// 1 open a file
	int fd = Open(file_name, O_RDWR|O_LARGEFILE|O_CREAT);
	if(fd<0)
	{
		fprintf(stderr, "Open failed: %s, file_name: %s\n", strerror(-fd), file_name);
		return -1;
	}

	MMapFile *map_file = new MMapFile(mmap_option, fd);

	bool is_mapped = map_file->mmap_file(true);
	if(is_mapped)
	{
		map_file->remmap_file();
		memset(map_file->get_data_(), '1', map_file->get_size_());
		map_file->sync_file();
		map_file->munmap_file();
	}
	else
	{
		fprintf(stderr, "map_file failed\n");
	}
	
	close(fd);
	return 0;
}