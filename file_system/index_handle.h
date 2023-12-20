#ifndef _INDEX_HANDLE_H_
#define _INDEX_HANDLE_H_ 

#include "mmap_file_op.h"

namespace file_system
{
	struct IndexHeader
	{
		BlockInfo block_info_;
		int32_t bucket_size_;
		int64_t data_offset_;
		int64_t meta_info_offset_;
		int64_t reuse_meta_info_offset_;

		IndexHeader()
		{
			memset(this, 0, sizeof(IndexHeader));
		}
	};

	class IndexHandle
	{
	private:
		MMapFileOperation *file_op_;
		uint32_t block_id_;
		bool is_mapped_;
		
	public:
		// the constructor and destructor
		IndexHandle(const uint32_t index_block_id);
		~IndexHandle();
		
		// initialization operations
		bool create(const int32_t bucket_size); // create a index block
		bool load(const MMap_Option &mmap_option); // mmap the index block
		void show_info(); // show the index header of the index block
		bool remove();
		bool flush();

		// update the index block 
		int32_t write_meta_info(MetaInfo &metainfo);
		int32_t read_meta_info(MetaInfo &metainfo, const uint32_t file_id);
		int32_t delete_meta_info(const uint32_t file_id);
		int32_t update_index_header(const OperType opertype, const int64_t nbytes);

		// get info or pointers
		IndexHeader *index_header() const;
		BlockInfo *block_info() const;
		int64_t *hash_table() const;
		
		// hash functions
		int32_t hash_search(uint32_t file_id, int64_t &curr_offset, int64_t &prev_offset) const;
		int32_t hash_insert(MetaInfo &metainfo, int64_t prev_offset) const;	// prev_offset must be the last node
		int32_t hash_compress(const std::string file_path, const std::string copy_file_path);
	};
}
#endif