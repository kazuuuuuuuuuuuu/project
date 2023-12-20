#include "index_handle.h"
#include "file_op.h"

using namespace std;
using namespace file_system;

static const MMap_Option MMAP_OPTION = {1024000, 8192, 4096};
static const uint32_t DATA_BLOCK_SIZE = 1024 * 1024 * 64;
static const uint32_t BUCKET_SIZE = 1000; 
static uint32_t block_id;

int main(int argc, char const *argv[])
{
	cout << "enter block_id: ";
	cin >> block_id;

	// 1 load the index block
	if(debug) printf("load the index block...\n");
	IndexHandle *index_block = new IndexHandle(block_id);
	if(!index_block->load(MMAP_OPTION))
	{
		delete(index_block);
		fprintf(stderr, "load() failed\n");
		return -1;
	}

	// show info
	index_block->show_info();

	// 2 read file 
	// 1 read metainfo
	uint32_t file_id;
	cout << "enter file_id: ";
	cin >> file_id;
	MetaInfo metainfo;
	int32_t ret = index_block->read_meta_info(metainfo, file_id);
	if(ret!=SUCCESS)
	{
		delete(index_block);
		fprintf(stderr, "read_meta_info() failed, errno: %d\n", ret);
		return -1;
	}
	else
	{
		printf("metainfo:\n");
		printf("file_id_: %u\n", metainfo.file_id_);
		printf("offset_: %ld\n", metainfo.offset_);
		printf("nbytes_: %d\n", metainfo.nbytes_);
		printf("next_: %ld\n", metainfo.next_);
	}

	// 2 read data based on the metainfo
	if(debug) printf("read from the data blcok...\n");
	std::stringstream tmp;
	tmp << DATA_BLOCK_DIR_PREFIX << block_id;
	string data_block_path = tmp.str();
	FileOperation *data_block = new FileOperation(data_block_path, O_RDWR|O_LARGEFILE);

	char *buf = new char[metainfo.nbytes_+1];
	if(!data_block->pread_file(buf, metainfo.nbytes_, metainfo.offset_))
	{
		delete buf;
		delete(index_block);
		delete(data_block);
		fprintf(stderr, "data_block->pread_file failed");
		return -1;
	}
	else
	{
		buf[metainfo.nbytes_] = '\0';
		printf("read: %s\n", buf);
		// automatically close opened file descriptors
		delete buf;
		delete(index_block);
		delete(data_block);
		if(debug) printf("read file success\n");
	}
	return 0;
}