#ifndef _MMAP_FILE_H_
#define _MMAP_FILE_H_ 

#include "common.h"

namespace file_system
{
	class MMapFile
	{
	private:
		int fd_;
		MMap_Option mmap_option_;
		void *data_;
		int32_t size_;
		
	public:
		// the constructor and destructor
		MMapFile();
		MMapFile(const MMap_Option &mmap_option, const int fd);
		~MMapFile();

		// operations of mapping file to the memory
		bool mmap_file(const bool write = false);
		bool munmap_file();
		bool remmap_file();

		// get info
		void *get_data_() const;
		int32_t get_size_() const;

		// sync memory contents to the file
		bool sync_file();

	private:
		// ensure that the file size meets the mapping size requirements
		bool ensure_file_size(const int32_t size); // called by mmap_file and remmap_file
	};
}
#endif