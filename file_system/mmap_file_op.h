#ifndef _MMAP_FILE_OP_H_
#define _MMAP_FILE_OP_H_

#include "common.h"
#include "file_op.h"
#include "mmap_file.h"

namespace file_system
{
	class MMapFileOperation: public FileOperation
	{
	private:
		// inherit from parent class 
		/*
		int fd_;
		std::string file_path_;
		int open_flags_;
		*/
		MMapFile *mmap_file_;
		bool is_mapped_;

	public:
		// inherit from parent class 
		/*
		bool open_file();
		bool close_file();
		*/

		// the constructor and destructor 
		MMapFileOperation(const std::string file_path, const int open_flags = O_RDWR|O_LARGEFILE|O_CREAT);
		~MMapFileOperation();

		// read and write precisely
		bool pread_file(char *buf, const int32_t nbytes, const int64_t offset);
		bool pwrite_file(const char *buf, const int32_t nbytes, const int64_t offset);
		bool flush_file();

		// map and unmap through the member variable of class MMapFile
		bool mmap_file(const MMap_Option &mmap_option);
		bool munmap_file();

		// get the mapped pointer
		void *get_data_() const;
	};
}
#endif