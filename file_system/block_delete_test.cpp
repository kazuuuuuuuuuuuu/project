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
	int32_t ret = index_block->delete_meta_info(file_id);
	if(ret!=SUCCESS)
	{
		
		fprintf(stderr, "delete_meta_info() failed, errno: %d\n", ret);
		return -1;
	}
	else
	{
		// flush
		if(!index_block->flush())
		{
			delete(index_block);
			fprintf(stderr, "index_block->flush() failed\n");
			return -1;
		}
		if(debug) printf("delete file success\n");
		delete(index_block);
		return 0;
	}
}