struct MMap_Option：
It contains basic parameters used to determine memory mapping
{
	int32_t max_size_; // maximum mapping size
	int32_t first_size_; // the mapping size of the first time
	int32_t append_size_; // add more size of the mapping area when remap
}

class MMapFile:
Its object maps an open file to the memory using fd and struct MMap_Option as parameters

class FileOpertaion(Parent class):
It contains basic file operation based on disk 

class MMapFileOperation: public FileOperation (having a member of MMapFile)
It provides file operations via memory(optional)

class IndexHandle：
It contains fundamental operations of the index block 

the index block structure:
1 the index block header
struct IndexHeader
{
	BlockInfo block_info_;
	int32_t bucket_size_;
	int64_t data_offset_;
	int64_t meta_info_offset_;
	int64_t reuse_meta_info_offset;
};
2 the hush table (store the pointer of  the head of a bucket)
3 meta info nodes
struct MetaInfo
{
	uint32_t file_id_;
	int64_t offset_;
	int32_t nbytes_;
	int64_t next_;
};

Script file:
1 block_init:
generate the data block file and the index block file