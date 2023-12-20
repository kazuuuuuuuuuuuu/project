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

	// original
	stringstream tmp;
	tmp << DATA_BLOCK_DIR_PREFIX << block_id;

	// copy
	stringstream tmp_copy;
	tmp_copy << DATA_BLOCK_DIR_PREFIX << block_id << "_copy";

	if(index_block->hash_compress(tmp.str(), tmp_copy.str())!=SUCCESS)
	{
		printf("index_block->hash_compress() failed\n");
		delete(index_block);
		return -1;
	}
	
	// delete the old file
	FileOperation *old_file = new FileOperation(tmp.str());
	if(!old_file->unlink_file())
	{
		printf("old_file->unlink_file() failed\n");
		delete(index_block);
		delete(old_file);
	}
	
	// automatically close opened file descriptors
	delete(index_block);
	delete(old_file);

	// rename
	rename(tmp_copy.str().c_str(), tmp.str().c_str());
	if(debug) printf("compress file success\n");
	return 0;
}