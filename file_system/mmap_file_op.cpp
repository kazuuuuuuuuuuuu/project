#include "mmap_file_op.h"

namespace file_system
{
	MMapFileOperation::MMapFileOperation(const std::string file_path, const int open_flags):
	FileOperation(file_path, open_flags), mmap_file_(NULL), is_mapped_(false) // call the constructor of the parent class first
	{
	}

	MMapFileOperation::~MMapFileOperation() // only deal with the part new added, the parent destructor will automatically run
	{
		if(is_mapped_)
		{
			delete(mmap_file_); // delete -> call the destructor of that object
			mmap_file_ = NULL;
			is_mapped_ = false;
		}
	}

	bool MMapFileOperation::pread_file(char *buf, const int32_t nbytes, const int64_t offset)
	{
		// mapping is not enough -> remap once
		if( is_mapped_&& ((offset+nbytes)>mmap_file_->get_size_()) )
		{
			if(debug) printf("MMapFileOperation::pread_file needs remap, bytes: %d, offset: %"__PRI64_PREFIX"d, mapping area size: %d\n", nbytes, offset, mmap_file_->get_size_());
			if(mmap_file_->remmap_file())
			{
				if(debug) printf("MMapFileOperation::pread_file->remap success, bytes: %d, offset: %"__PRI64_PREFIX"d, new mapping area size: %d\n", nbytes, offset, mmap_file_->get_size_());
			}
			else 
			{
				if(debug) printf("MMapFileOperation::pread_file->remap failed\n");
			}
		}
		// read from memory
		if( is_mapped_&& ((offset+nbytes)<=mmap_file_->get_size_()) )
		{
			if(debug) printf("MMapFileOperation::pread_file reads from memory, bytes: %d, offset: %"__PRI64_PREFIX"d, mapping area size: %d\n", nbytes, offset, mmap_file_->get_size_());
			memcpy(buf, (reinterpret_cast<char *>(mmap_file_->get_data_())) + offset, nbytes);
			return true;
		}
		// read from file -> call the method of the parent class
		if(debug) printf("MMapFileOperation::pread_file reads without mapping, bytes: %d, offset: %"__PRI64_PREFIX"d\n", nbytes, offset);
		return FileOperation::pread_file(buf, nbytes, offset);
	}

	bool MMapFileOperation::pwrite_file(const char *buf, const int32_t nbytes, const int64_t offset)
	{
		// mapping is not enough -> remap once
		if( is_mapped_&&(offset+nbytes)>mmap_file_->get_size_() )
		{
			if(debug) printf("MMapFileOperation::pwrite_file needs remap, bytes: %d, offset: %"__PRI64_PREFIX"d, mapping area size: %d\n", nbytes, offset, mmap_file_->get_size_());
			if(mmap_file_->remmap_file())
			{
				if(debug) printf("MMapFileOperation::pwrite_file->remap success, bytes: %d, offset: %"__PRI64_PREFIX"d, new mapping area size: %d\n", nbytes, offset, mmap_file_->get_size_());
			}
			else
			{
				if(debug) printf("MMapFileOperation::pwrite_file->remap failed\n");
			}
		}
		// read from memory
		if( is_mapped_&&(offset+nbytes)<=mmap_file_->get_size_() )
		{
			if(debug) printf("MMapFileOperation::pwrite_file writes via memory, bytes: %d, offset: %"__PRI64_PREFIX"d, mapping area size: %d\n", nbytes, offset, mmap_file_->get_size_());
			memcpy((reinterpret_cast<char *>(mmap_file_->get_data_())) + offset, buf, nbytes);
			return true;
		}
		

		// write to file direcrly
		if(debug) printf("MMapFileOperation::pwrite_file writes without mapping, bytes: %d, offset: %"__PRI64_PREFIX"d\n", nbytes, offset);
		return FileOperation::pwrite_file(buf, nbytes, offset);
	}
	
	bool MMapFileOperation::flush_file()
	{
		if(is_mapped_)
		{
			if(mmap_file_->sync_file())
			{
				if(FileOperation::flush_file())
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			if(FileOperation::flush_file())
			{
				return true;
			}
			return false;
		}
	}

	bool MMapFileOperation::mmap_file(const MMap_Option &mmap_option)
	{
		// 0 check if mmap_file_ already exists
		if(is_mapped_) return true;

		if(fd_<0) open_file();
		if(fd_>0)
		{	
			// 1 create object
			mmap_file_ = new MMapFile(mmap_option, fd_);
			// 2 conduct mmap using that object
			if(mmap_file_->mmap_file(true))
			{
				is_mapped_ = true;
				return true;
			}
			else
			{
				delete(mmap_file_);
				mmap_file_ = NULL;
				is_mapped_ = false;
				return false;
			}
		}
		return false;
	}
	
	bool MMapFileOperation::munmap_file()
	{
		if(!is_mapped_) return true;

		delete(mmap_file_); // ~MMapFile will sync and unmap the memory 
		is_mapped_ = false;
		return true;
	}

	void *MMapFileOperation::get_data_() const
	{
		if(is_mapped_)
		{
			return mmap_file_->get_data_();
		}
		return NULL;
	}
}