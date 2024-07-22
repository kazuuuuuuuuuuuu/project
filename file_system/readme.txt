# summary
This module focuses on efficient storage and rapid access to large-scale image files, using object-oriented design methods. It consists of data blocks and index blocks, where data blocks store the actual file content, and index blocks contain a hash table for storing file index information. I addressed hash collisions using linked lists and indexed files based on automatically distributed file numbers, implementing fundamental file operations such as insert, delete, update, and query. To accelerate index block access speed, I employed memory mapping. Additionally, I implemented features such as file fragment cleanup, error management, and index block metadata updating to support system stability and performance.

# detail
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

#test case:
1 block_init:
generate the data block file and the index block file

2 block_write:
write metainfo into the index block first and then write the actual file

3 block_read:
read metainfo from the index block and use it to index the actual file

4 block_delete:
delete metainfo and append it into the reuse linked list

5 block_compress:
clean up the deleted file fragments in the data block

6 block_stat:
show the header of the index block 
