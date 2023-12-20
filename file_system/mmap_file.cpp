#include "mmap_file.h"

namespace file_system
{
	MMapFile::MMapFile():
	fd_(-1), data_(NULL), size_(0)
	{
		memset(&mmap_option_, 0, sizeof(MMap_Option));
	}

	MMapFile::MMapFile(const MMap_Option &mmap_option, const int fd):
	fd_(fd), data_(NULL), size_(0)
	{
		mmap_option_ = mmap_option;
	}

	MMapFile::~MMapFile()
	{
		if(debug) printf("~MMapFile is running, fd_: %d, data_: %p, size_: %d\n", fd_, data_, size_);
		if(data_!=NULL&&size_>0)
		{
			sync_file();
			munmap_file();
		}
		fd_ = -1;
		data_ = NULL;
		size_ = 0;
		memset(&mmap_option_, 0, sizeof(MMap_Option));
	}

	bool MMapFile::mmap_file(const bool write)
	{
		// 1 set read and write permissions for the mapping area
		int flags = PROT_READ;
		if(write) flags |= PROT_WRITE;

		// 2 check if the file is opened 
		if(fd_<0||data_!=NULL) return false;

		// 3 set the mapping size -> size_ 
		size_ = mmap_option_.first_size_;

		// 4 modify the file size (require file's size >= size_)
		if(ensure_file_size(size_)==false) return false;

		// 5 execute mmap
		data_ = mmap(0, size_, flags, MAP_SHARED, fd_, 0);
		if(data_==MAP_FAILED)
		{
			fprintf(stderr, "mmap_file->mmap failed: %s\n", strerror(errno));
			data_ = NULL;
			size_ = 0;
			return false;
		}
		else
		{
			if(debug) printf("mmap_file success, fd_: %d, data_: %p, size_: %d\n", fd_, data_, size_);
			return true;
		}
	}

	bool MMapFile::munmap_file()
	{
		if(munmap(data_, size_)<0)
		{
			fprintf(stderr, "munmap_file failed: %s\n", strerror(errno));
			return false;
		}
		return true;
	}
	
	bool MMapFile::remmap_file()
	{
		if(fd_<0||data_==NULL) return false;
	
		if(size_<mmap_option_.max_size_)
		{
			// 1 set the size of new mapping area
			int32_t new_size_;
			if(size_+mmap_option_.append_size_<=mmap_option_.max_size_)
			{
				new_size_ = size_ + mmap_option_.append_size_;
			}
			else
			{
				new_size_ = mmap_option_.max_size_;
			}

			// 2 modify the file size (require file's size >= size_)
			if(ensure_file_size(new_size_)==false) return false;

			// 3 execute mremap
			void *new_data_ = mremap(data_, size_, new_size_, MREMAP_MAYMOVE);
			if(new_data_==MAP_FAILED)
			{
				fprintf(stderr, "mremap failed: %s\n", strerror(errno));
				return false;
			}
			else
			{
				if(debug) printf("remmap_file success, old data_: %p, new data_: %p, old size_: %d, new size_: %d\n", data_, new_data_, size_, new_size_);
				// if successful, update
				data_ = new_data_;
				size_ = new_size_;
				return true;
			}
		}
		else
		{
			printf("the mapping size has reached the max value\n");
			return false;
		}
	}

	void *MMapFile::get_data_() const
	{
		return data_;
	}

	int32_t MMapFile::get_size_() const
	{
		return size_;
	}

	bool MMapFile::ensure_file_size(const int32_t size)
	{
		// 1 obtain the file info from fd_
		struct stat file_stat;
		if(fstat(fd_, &file_stat)<0)
		{
			fprintf(stderr, "ensure_file_size->fstat failed: %s\n", strerror(errno));
			return false;
		}

		// 2 modify the file size to the size
		if(file_stat.st_size<size)
		{
			if(ftruncate(fd_, size)<0)
			{
				fprintf(stderr, "ensure_file_size->ftruncate failed: %s\n", strerror(errno));
				return false;			
			}
		}
		return true;
	}

	bool MMapFile::sync_file()
	{
		if(data_!=NULL&&size_>0)
		{
			if(msync(data_, size_, MS_SYNC)<0) // MS_SYNC -> synchronous
			{
				fprintf(stderr, "sync_file failed: %s\n", strerror(errno));
				return false;
			}	
			else
			{
				return true;
			}
		}
		else
		{
			fprintf(stderr, "sync_file failed: map memory not yet\n");
			return false;
		}
		
	}
}