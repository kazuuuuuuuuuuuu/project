#include "file_op.h"

using namespace std;
using namespace file_system;

int main(int argc, char const *argv[])
{
	string filename = "file_op.txt";
	FileOperation *fileop = new FileOperation(filename, O_RDWR|O_LARGEFILE|O_CREAT);

	bool ret;
	ret = fileop->open_file();
	if(!ret) return -1;

	char buffer[64];
	memset(buffer, '8', 64);
	fileop->pwrite_file(buffer, 64, 64);

	fileop->flush_file();

	char buffer2[65];
	memset(buffer2, 0, 64);
	buffer2[64] = '\0';
	ret = fileop->pread_file(buffer2, 64, 64);
	cout << "buffer2: " << buffer2 << endl;


	memset(buffer, '6', 64);
	fileop->write_file(buffer, 64);


	memset(buffer2, 0, 64);
	buffer2[64] = '\0';
	ret = fileop->pread_file(buffer2, 64, 0);
	cout << "buffer2_test2: " << buffer2 << endl;

	//fileop->unlink_file();
	fileop->close_file();
	return 0;
}