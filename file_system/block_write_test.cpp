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

	// 2 write file and update the index block
	if(debug) printf("write file and update the index block...\n");
	std::stringstream tmp;
	tmp << DATA_BLOCK_DIR_PREFIX << block_id;
	string data_block_path = tmp.str();
	FileOperation *data_block = new FileOperation(data_block_path);

	int times;
	cout << "enter the write times: ";
	cin >> times;
	for(int i=0;i<times;i++)
	{
		
		// 0 prepare the file to be written
		char buffer[4096];
		memset(buffer, static_cast<char>('0'+i), sizeof(buffer));
	
		// 0 obtain write location
		int64_t data_offset = index_block->index_header()->data_offset_;
		
		// 1 write file to the data block
		if(!data_block->pwrite_file(buffer, sizeof(buffer), data_offset))
		{
			delete(data_block);
			delete(index_block);
			fprintf(stderr, "data_block->pwrite_file() failed\n");
			return -1;
		}

		// 2 write the metainfo to the index block
		uint32_t file_no = index_block->block_info()->next_file_no_;
		MetaInfo metainfo;
		metainfo.file_id_ = file_no;
		metainfo.offset_ = data_offset;
		metainfo.nbytes_ = sizeof(buffer);

		int32_t ret = index_block->write_meta_info(metainfo);
		if(ret!=SUCCESS)
		{
			delete(index_block);
			delete(data_block);
			fprintf(stderr, "index_block->write_meta_info() failed, errno: %d\n", ret);
			return -1;
		}

		// 3 update the index block header 
		ret = index_block->update_index_header(OPER_INSERT, sizeof(buffer));
		if(ret!=SUCCESS)
		{
			delete(index_block);
			delete(data_block);
			fprintf(stderr, "index_block->updata_block_info() failed, errno: %d\n", ret);
			return -1;
		}	
	}

	// flush
	if(!index_block->flush())
	{
		delete(index_block);
		delete(data_block);
		fprintf(stderr, "index_block->flush() failed\n");
		return -1;
	}

	// automatically close opened file descriptors
	delete(index_block);
	delete(data_block);
	if(debug) printf("write file success\n");
	return 0;
}