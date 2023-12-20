#ifndef _FILE_OP_H_
#define _FILE_OP_H_

#include "common.h"

namespace file_system
{
	class FileOperation
	{
	protected:
		int fd_;
		std::string file_path_;
		int open_flags_;

	protected:
		static const mode_t OPEN_MODE_ = 0644;
		static const int MAX_DISK_ACCESS_ = 5;

	public:
		// the constructor and destructor
		FileOperation();
		FileOperation(const std::string file_path, const int open_flags = O_RDWR|O_LARGEFILE|O_CREAT);
		~FileOperation();

		// open and close
		bool open_file();
		bool close_file();

		// change the file pointer and write
		bool seek_file(const int64_t offset);
		bool write_file(const char *buf, const int32_t nbytes);

		// read and write precisely (without moving the file pointer)
		virtual bool pread_file(char *buf, const int32_t nbytes, const int64_t offset);
		virtual bool pwrite_file(const char *buf, const int32_t nbytes, const int64_t offset);

		// unlink(delete a hard link) and flush and modify the file size
		bool unlink_file();
		virtual bool flush_file();
		bool truncate_file(const int64_t length);

		// get info
		int get_fd_();
		int64_t get_file_size(); // obtain the file size
	};
}
#endif