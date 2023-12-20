#include "index_handle.h"

namespace file_system
{
	IndexHandle::IndexHandle(const uint32_t block_id)
	{
		std::stringstream tmp;
		tmp << INDEX_BLOCK_DIR_PREFIX << block_id;
		std::string index_block_path = tmp.str(); 

		file_op_ = new MMapFileOperation(index_block_path, O_RDWR|O_LARGEFILE|O_CREAT);
		block_id_ = block_id;
		is_mapped_ = false;
		
	}

	IndexHandle::~IndexHandle()
	{
		if(file_op_!=NULL)
		{
			delete(file_op_);
			file_op_ = NULL;	
		}
		is_mapped_ = false;
	}

	bool IndexHandle::create(const int32_t bucket_size)
	{
		if(debug) printf("create is running, block_id_: %u\n", block_id_);
		
		// 1 open the file 
		if(!file_op_->open_file()) 
		{
			fprintf(stderr, "create failed: open_file() failed\n");
			return false;
		}

		// 2 get the file size
		int64_t file_size = file_op_->get_file_size();
		if(file_size<0) 
		{
			fprintf(stderr, "create failed: get_file_size() failed\n");
			return false;
		}
		// 3 initialize the empty file
		else if(file_size==0)
		{
			// 1 initialize the index block header
			IndexHeader index_header; // have been zeroed out
			index_header.block_info_.block_id_ = block_id_;
			index_header.block_info_.next_file_no_ = 1;
			index_header.bucket_size_ = bucket_size;
			index_header.meta_info_offset_ = sizeof(IndexHeader) + bucket_size*sizeof(int64_t);
			
			// 2 Prepare the data buffer (index block header + hash table) to be written to the file
			char *buf = new char[index_header.meta_info_offset_];
			memcpy(buf, &index_header, sizeof(IndexHeader));
			memset(buf+sizeof(IndexHeader), 0, bucket_size*sizeof(int64_t));

			// 3 write the index block header and the hash table to the index block
			if(file_op_->pwrite_file(buf, index_header.meta_info_offset_, 0))
			{
				delete[] buf;
				if(file_op_->flush_file()) 
				{
					if(debug) printf("create success, block_id_: %u, bucket_size_: %d\n", block_id_, bucket_size);
					return true;
				}
				else
				{
					fprintf(stderr, "create failed: flush_file() failed\n");
					return false;
				}
			}
			else
			{
				delete[] buf;
				fprintf(stderr, "create failed: pread_file() failed\n");
				return false;
			}
		}
		else
		{
			fprintf(stderr, "create failed: index block already exists\n");
			return false;
		}
	}

	bool IndexHandle::load(const MMap_Option &mmap_option)
	{
		if(is_mapped_) return true;

		// 1 open the file
		if(!file_op_->open_file()) 
		{
			fprintf(stderr, "load failed: open_file() failed\n");
			return false;
		}

		// 2 get the file size
		int64_t file_size = file_op_->get_file_size();
		if(file_size<0) 
		{
			fprintf(stderr, "load failed: get_file_size() failed\n");
			return false;
		}
		else if(file_size==0)
		{
			fprintf(stderr, "load failed: file size is equal to 0\n");
			return false;
		}
		else 
		{
			// 3 map the index block
			if(file_op_->mmap_file(mmap_option))
			{
				is_mapped_ = true;
				int64_t meta_info_offset = sizeof(IndexHeader) + index_header()->bucket_size_ * sizeof(int64_t);
				if(file_size<meta_info_offset)
				{
					fprintf(stderr, "load failed: file is corrupted, file_size: %"__PRI64_PREFIX"d, minimum file_size: %"__PRI64_PREFIX"d\n",
						file_size, meta_info_offset);
					is_mapped_ = false;
					return false;
				}
				if(mmap_option.first_size_<meta_info_offset)
				{
					fprintf(stderr, "load failed: adjust the first mapping size, minimum file_size: %"__PRI64_PREFIX"d\n",
						meta_info_offset);
					is_mapped_ = false;
					return false;
				}
				if(debug) printf("load success, block_id_: %u\n", block_id_);
				return true;					
			}
			else
			{
				fprintf(stderr, "load failed: mmap_file() failed\n");
				return false;
			}
		}
	}

	void IndexHandle::show_info()
	{
		if(is_mapped_)
		{
			IndexHeader *index_header_p = index_header();
			BlockInfo *block_info_p = block_info();
			printf("\n");
			printf("block_id_: %u, version_: %d, file_count_: %d, size_: %d\n", 
				block_info_p->block_id_, block_info_p->version_, block_info_p->file_count_, block_info_p->size_); 
			printf("del_file_count_: %d, del_size_: %d, next_file_no_: %u\n",
				block_info_p->del_file_count_, block_info_p->del_size_, block_info_p->next_file_no_);
			printf("bucket_size_: %d, data_offset_: %"__PRI64_PREFIX"d, meta_info_offset_: %"__PRI64_PREFIX"d, reuse_meta_info_offset_: %"__PRI64_PREFIX"d\n",
				index_header_p->bucket_size_, index_header_p->data_offset_, index_header_p->meta_info_offset_, index_header_p->reuse_meta_info_offset_);
			printf("\n");
		}
		else fprintf(stderr, "conduct the load first\n");
	}

	bool IndexHandle::remove()
	{
		if(!file_op_->munmap_file())
		{
			fprintf(stderr, "remove failed: munmap_file() failed \n");
			return false;
		}
		if(!file_op_->unlink_file())
		{
			fprintf(stderr, "remove failed: unlink_file() failed \n");
			return false;
		}
		return true;
	}

	bool IndexHandle::flush()
	{
		return file_op_->flush_file();
	}

	int32_t IndexHandle::write_meta_info(MetaInfo &metainfo)
	{
		// check if the index block has been mapped
		if(!is_mapped_)
		{
			return INDEX_BLOCK_NEED_MAPPED;
		}

		// 1 check if fileid exists
		int64_t curr_offset = 0;
		int64_t prev_offset = 0;
		int ret = hash_search(metainfo.file_id_, curr_offset, prev_offset);
		if(ret==SUCCESS)
		{
			fprintf(stderr, "The file already exists\n");
			return FAILED;
		}
		// 2 if it doesn't exist, insert the metainfo
		else if(ret==METAINFO_NOT_FOUND)
		{
			ret = hash_insert(metainfo, prev_offset);
			if(ret==SUCCESS) 
			{
				return SUCCESS;
			}
			else
			{
				fprintf(stderr, "hash_insert failed, errno: %d\n", ret);
				return FAILED;
			}
		}
		else
		{
			fprintf(stderr, "hash_search failed, errno: %d\n", ret);
			return FAILED;
		}
	}

	int32_t IndexHandle::read_meta_info(MetaInfo &metainfo, const uint32_t file_id)
	{
		// check if the index block has been mapped
		if(!is_mapped_)
		{
			return INDEX_BLOCK_NEED_MAPPED;
		}

		// 1 check if fileid exists
		int64_t curr_offset = 0;
		int64_t prev_offset = 0;
		int ret = hash_search(file_id, curr_offset, prev_offset);
		if(ret!=SUCCESS)
		{
			fprintf(stderr, "hash_search failed, errno: %d\n", ret);
			return FAILED;
		}
		else
		{
			// 2 read the metainfo to the parameter passed
			if(!file_op_->pread_file(reinterpret_cast<char *>(&metainfo), sizeof(MetaInfo), curr_offset))
			{
				fprintf(stderr, "read_meta_info failed: file_op_->pread_file() failed\n");
				return FAILED;
			}
			return SUCCESS;
		}
	}

	int32_t IndexHandle::delete_meta_info(const uint32_t file_id)
	{
		// check if the index block has been mapped
		if(!is_mapped_)
		{
			return INDEX_BLOCK_NEED_MAPPED;
		}

		// 1 check if fileid exists
		int64_t curr_offset = 0;
		int64_t prev_offset = 0;
		int ret = hash_search(file_id, curr_offset, prev_offset);
		if(ret==METAINFO_NOT_FOUND)
		{
			fprintf(stderr, "the metainfo is not found\n");
			return METAINFO_NOT_FOUND;
		}
		else if(ret==SUCCESS)
		{
			// 1 read the current metainfo
			MetaInfo curr_meta;
			if(!file_op_->pread_file(reinterpret_cast<char *>(&curr_meta), sizeof(MetaInfo), curr_offset))
			{
				fprintf(stderr, "delete_meta_info failed: file_op_->pread_file() failed\n");
				return FAILED;
			}

			// 2 delete the metainfo -> modify the previous metainfo node or head
			if(prev_offset==0)
			{
				// 1 obtain the index of buckets where the metainfo should be stored
				int32_t index = static_cast<int32_t>(file_id) % index_header()->bucket_size_;
				
				// 2 obtain the head of the linked list
				hash_table()[index] = curr_meta.next_;
			}
			else
			{
				MetaInfo prev_meta;
				if(!file_op_->pread_file(reinterpret_cast<char *>(&prev_meta), sizeof(MetaInfo), prev_offset))
				{
					fprintf(stderr, "delete_meta_info failed: file_op_->pread_file() failed\n");
					return FAILED;
				}
				prev_meta.next_ = curr_meta.next_;
				if(!file_op_->pwrite_file(reinterpret_cast<char *>(&prev_meta), sizeof(MetaInfo), prev_offset))
				{
					fprintf(stderr, "delete_meta_info failed: file_op_->pwrite_file() failed\n");
					return FAILED;
				}		
			}
			
			// 3 Set Reusable List
			curr_meta.next_ = index_header()->reuse_meta_info_offset_;
			if(!file_op_->pwrite_file(reinterpret_cast<char *>(&curr_meta), sizeof(MetaInfo), curr_offset))
			{
				fprintf(stderr, "delete_meta_info failed: file_op_->pwrite_file() failed\n");
				return FAILED;
			}	
			index_header()->reuse_meta_info_offset_ = curr_offset;
			if(debug) printf("metainfo has been added to the reuse linked list, offset: %ld\n", curr_offset);

			// 4 update the index block header
			if(update_index_header(OPER_DELETE, curr_meta.nbytes_)!=SUCCESS)
			{
				fprintf(stderr, "delete_meta_info failed: update_index_header() failed\n");
				return FAILED;
			}

			return SUCCESS;
		}
		else
		{
			fprintf(stderr, "hash_search failed, errno: %d\n", ret);
			return FAILED;			
		}		
	}

	int32_t IndexHandle::update_index_header(const OperType opertype, const int64_t nbytes)
	{
		// check if the index block has been mapped
		if(!is_mapped_)
		{
			return INDEX_BLOCK_NEED_MAPPED;
		}
		if(opertype==OPER_INSERT)
		{
			block_info()->version_ ++;
			block_info()->file_count_ ++;
			block_info()->size_ += nbytes;
			block_info()->next_file_no_ ++;
			index_header()->data_offset_ += nbytes;
		}
		else if(opertype==OPER_DELETE)
		{
			block_info()->version_ ++;
			block_info()->file_count_ --;
			block_info()->size_ -= nbytes;
			block_info()->del_file_count_ ++;
			block_info()->del_size_ += nbytes;
		}
		else if(opertype==OPER_COMPRESS)
		{
			block_info()->version_ ++;
			block_info()->del_file_count_ = 0;
			block_info()->del_size_ = 0;	
			index_header()->data_offset_ = nbytes;	
		}
		if(debug) show_info();
		return SUCCESS;
	}

	IndexHeader *IndexHandle::index_header() const
	{
		if(!is_mapped_)
		{
			fprintf(stderr, "index_header failed: please load first\n");
			return NULL;
		}
		else
		{
			return reinterpret_cast<IndexHeader *>(file_op_->get_data_());
		}
	}

	BlockInfo *IndexHandle::block_info() const
	{
		if(!is_mapped_)
		{
			fprintf(stderr, "block_info failed: please load first\n");
			return NULL;
		}
		else
		{
			return reinterpret_cast<BlockInfo *>(file_op_->get_data_());
		}
	}

	int64_t *IndexHandle::hash_table() const
	{
		if(!is_mapped_)
		{
			fprintf(stderr, "hash_table failed: please load first\n");
			return NULL;
		}
		else
		{
			return reinterpret_cast<int64_t *>(reinterpret_cast<char *>(file_op_->get_data_())+sizeof(IndexHeader));
		}
	}

	int32_t IndexHandle::hash_search(uint32_t file_id, int64_t &curr_offset, int64_t &prev_offset) const
	{
		// check if the index block is mapped
		if(!is_mapped_)
		{
			return INDEX_BLOCK_NEED_MAPPED;
		}

		curr_offset = 0;
		prev_offset = 0;

		// 1 obtain the index of buckets where the metainfo should be stored
		int32_t index = static_cast<int32_t>(file_id) % index_header()->bucket_size_;
		
		// 2 obtain the head of the linked list
		curr_offset = hash_table()[index];

		// 3 traverse that linked list to find the metainfo with the file_id
		MetaInfo metainfo;
		while(curr_offset!=0)
		{
			if(!file_op_->pread_file(reinterpret_cast<char *>(&metainfo), sizeof(MetaInfo), curr_offset))
			{
				fprintf(stderr, "hash_search failed: file_op_->pread_file() failed\n");
				return FAILED;		
			}

			if(metainfo.file_id_==file_id)
			{
				return SUCCESS;	
			}

			prev_offset = curr_offset;
			curr_offset = metainfo.next_;
		}
		return METAINFO_NOT_FOUND;
	}

	int32_t IndexHandle::hash_insert(MetaInfo &metainfo, int64_t prev_offset) const
	{
		// check if the index block is mapped
		if(!is_mapped_)
		{
			return INDEX_BLOCK_NEED_MAPPED;
		} 

		// 1 determine where to write the metainfo
		int64_t curr_offset;
		if(index_header()->reuse_meta_info_offset_!=0)
		{
			curr_offset = index_header()->reuse_meta_info_offset_;
			MetaInfo temp;
			if(!file_op_->pread_file(reinterpret_cast<char *>(&temp), sizeof(MetaInfo), curr_offset))
			{
				fprintf(stderr, "hash_insert failed: file_op_->pread_file() failed\n");
				return INDEX_BLOCK_NEED_ROLL_BACK;
			}
			// update index header
			index_header()->reuse_meta_info_offset_ = temp.next_;
			if(debug) printf("reuse metainfo offset: %ld\n", curr_offset);
		}
		else
		{
			curr_offset = index_header()->meta_info_offset_;
			// update index header
			index_header()->meta_info_offset_ += sizeof(MetaInfo);
		}

		// 2 write the metainfo to the file
		metainfo.next_ = 0; // tail insertion
		if(!file_op_->pwrite_file(reinterpret_cast<char *>(&metainfo), sizeof(MetaInfo), curr_offset))
		{
			fprintf(stderr, "hash_insert failed: file_op_->pwrite_file() failed\n");
			return INDEX_BLOCK_NEED_ROLL_BACK;
		}

		// 3 link the metainfo to the linked list
		if(prev_offset!=0) // the previous node exists
		{
			MetaInfo temp;
			if(!file_op_->pread_file(reinterpret_cast<char *>(&temp), sizeof(MetaInfo), prev_offset))
			{
				fprintf(stderr, "hash_insert failed: file_op_->pread_file() failed\n");
				return INDEX_BLOCK_NEED_ROLL_BACK;
			}
			temp.next_ = curr_offset;
			if(!file_op_->pwrite_file(reinterpret_cast<char *>(&temp), sizeof(MetaInfo), prev_offset))
			{
				fprintf(stderr, "hash_insert failed: file_op_->pwrite_file() failed\n");
				return INDEX_BLOCK_NEED_ROLL_BACK;
			}
		}
		else // the previous node doesn't exist
		{
			int32_t index = static_cast<int32_t>(metainfo.file_id_) % index_header()->bucket_size_;
			hash_table()[index] = curr_offset;
		}
		return SUCCESS;
	}

	int32_t IndexHandle::hash_compress(const std::string file_path, const std::string copy_file_path)
	{
		// 0 check if the index block is mapped
		if(!is_mapped_)
		{
			return INDEX_BLOCK_NEED_MAPPED;
		} 

		FileOperation *old_file = new FileOperation(file_path, O_RDWR|O_LARGEFILE);
		FileOperation *new_file = new FileOperation(copy_file_path, O_RDWR|O_LARGEFILE|O_CREAT);

		int64_t curr_new_file_offset = 0;
		// 1 traverse each bucket
		for(int i=0;i<index_header()->bucket_size_;i++)
		{
			int64_t curr_offset = hash_table()[i];
			if(debug) printf("i: %d, curr_offset: %ld\n", i, curr_offset);
			while(curr_offset!=0)
			{
				// 1 read the metainfo
				MetaInfo temp;
				if(!file_op_->pread_file(reinterpret_cast<char *>(&temp), sizeof(MetaInfo), curr_offset))
				{
					fprintf(stderr, "hash_compress failed: file_op_->pread_file1() failed\n");
					return INDEX_BLOCK_NEED_ROLL_BACK;
				}

				// 2 read data from the old file
				char *buf = new char[temp.nbytes_];
				if(!old_file->pread_file(buf, temp.nbytes_, temp.offset_))
				{
					fprintf(stderr, "hash_compress failed: file_op_->pread_file2() failed\n");
					return INDEX_BLOCK_NEED_ROLL_BACK;
				}	

				// 3 write data to the new file			
				if(!new_file->pwrite_file(buf, temp.nbytes_, curr_new_file_offset))
				{
					fprintf(stderr, "hash_compress failed: file_op_->pwrite_file3() failed\n");
					return INDEX_BLOCK_NEED_ROLL_BACK;
				}	
				delete buf;

				// 4 update the metainfo
				temp.offset_ = curr_new_file_offset;
				if(!file_op_->pwrite_file(reinterpret_cast<char *>(&temp), sizeof(MetaInfo), curr_offset))
				{
					fprintf(stderr, "hash_compress failed: file_op_->pwrite_file4() failed\n");
					return INDEX_BLOCK_NEED_ROLL_BACK;
				}
				curr_new_file_offset += temp.nbytes_;

				// 5 traverse the next metainfo
				curr_offset = temp.next_;
			} 
		}

		delete(old_file);
		delete(new_file);

		// 2 update the index block header
		if(update_index_header(OPER_COMPRESS, curr_new_file_offset)!=SUCCESS)
		{
			return FAILED;
		}
		else
		{
			return SUCCESS;
		}
	}
}