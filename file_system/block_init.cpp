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

	// 1 generate the index block
	if(debug) printf("generate the index block...\n");
	IndexHandle *index_block = new IndexHandle(block_id);
	if(!index_block->create(BUCKET_SIZE))
	{
		delete(index_block);
		fprintf(stderr, "create() failed\n");
		return -1;
	}

	// 2 load the index block
	if(!index_block->load(MMAP_OPTION))
	{
		delete(index_block);
		fprintf(stderr, "load() failed\n");
		return -1;
	}

	// 3 show info
	index_block->show_info();

	// 4 generate the data block
	if(debug) printf("generate the data block...\n");
	std::stringstream tmp;
	tmp << DATA_BLOCK_DIR_PREFIX << block_id;
	string data_block_path = tmp.str();
	FileOperation *data_block = new FileOperation(data_block_path);
	if(!data_block->open_file())
	{
		index_block->remove();
		delete(data_block);
		delete(index_block);
		fprintf(stderr, "data_block->open_file() failed\n");
		return -1;
	}
	if(!data_block->truncate_file(DATA_BLOCK_SIZE))
	{
		index_block->remove();
		delete(data_block);
		delete(index_block);
		fprintf(stderr, "data_block->truncate_file() failed\n");
		return -1;
	}

	index_block->flush();
	delete(index_block);
	delete(data_block);
	if(debug) printf("the data block and the index block create success\n");
	return 0;
}