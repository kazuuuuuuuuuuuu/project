#include "file_op.h"

namespace file_system
{
	FileOperation::FileOperation():
	fd_(-1)
	{
	}

	FileOperation::FileOperation(const std::string file_path, const int open_flags):
	fd_(-1), file_path_(file_path), open_flags_(open_flags)
	{
	}

	FileOperation::~FileOperation()
	{
		if(fd_>0)
		{
			flush_file();
			close_file();
		}
		fd_ = -1;
	}

	bool FileOperation::open_file()
	{
		if(fd_>0) return true;
		if(fd_<0)
		{
			fd_ = ::open(file_path_.c_str(), open_flags_, OPEN_MODE_);
			if(fd_<0)
			{
				fprintf(stderr, "open failed: %s\n", strerror(errno));
				return false;
			}
			else return true;
		}
	}

	bool FileOperation::close_file()
	{
		if(fd_<0) return true;
		if(fd_>0)
		{
			::close(fd_);
			fd_ = -1;
			return true;
		}
	}

	bool FileOperation::seek_file(const int64_t offset)
	{
		if(fd_<0) open_file();
		if(fd_>0)
		{
			if(lseek(fd_, offset, SEEK_SET)==0) return true; // SEEK_SET -> the file pointer is set to offset bytes
		}
		return false;
	}

	bool FileOperation::write_file(const char *buf, const int32_t nbytes)
	{
		if(fd_<0) open_file();
		if(fd_>0)
		{
			int32_t unwrite = nbytes;
			int32_t write;
			const char *p = buf;
			int i = 0;

			while(unwrite>0)
			{
				i ++;
				if(i>MAX_DISK_ACCESS_) break;

				write = ::write(fd_, p, unwrite);
				if(write<0)
				{
					write = errno;
					if(write==EINTR||write==EAGAIN) continue;
					else if(write==EBADF) fd_ = -1;

					fprintf(stderr, "write failed: %s\n", strerror(write));
					return false;
				}
				else
				{
					unwrite -= write;
					p += write;
				}
			}
			if(unwrite==0) return true;
			else
			{
				fprintf(stderr, "write_file failed: disk access incomplete\n");
				return false;
			}
		}
		return false;
	}

	bool FileOperation::pread_file(char *buf, const int32_t nbytes, const int64_t offset)
	{
		if(fd_<0) open_file();
		if(fd_>0)
		{
			char *p = buf;
			int32_t unread = nbytes;
			int32_t read;
			int64_t read_offset = offset;
			int i = 0;

			while(unread>0)
			{
				i ++;
				if(i>MAX_DISK_ACCESS_) break;

				read = pread(fd_, p, unread, read_offset);
				if(read<0)
				{
					read = errno;
					if(read==EINTR||read==EAGAIN) continue;
					else if(read==EBADF) fd_ = -1;

					fprintf(stderr, "pread64 failed: %s\n", strerror(read));
					return false;
				}
				else
				{		
					unread -= read;
					p += read;
					read_offset += read;
				}
			}
			if(unread==0) return true;
			else
			{
				fprintf(stderr, "pread_file failed: disk access incomplete\n");
				return false;
			}
		}
		return false;
	}

	bool FileOperation::pwrite_file(const char *buf, const int32_t nbytes, const int64_t offset)
	{
		if(fd_<0) open_file();
		if(fd_>0)
		{
			const char *p = buf;
			int32_t unwrite = nbytes;
			int32_t write;
			int64_t write_offset = offset;
			int i = 0;

			while(unwrite>0)
			{
				i ++;
				if(i>MAX_DISK_ACCESS_) break;

				write = pwrite64(fd_, p, unwrite, write_offset);
				if(write<0)
				{
					write = errno;
					if(write==EINTR||write==EAGAIN) continue;
					else if(write==EBADF) fd_ = -1;

					fprintf(stderr, "pwrite64 failed: %s\n", strerror(write));
					return false;
				}
				else
				{
					unwrite -= write;
					p += write;
					write_offset += write;
				}
			}
			if(unwrite==0) return true;
			else
			{
				fprintf(stderr, "pwrite_file failed: disk access incomplete\n");
				return false;				
			}
		}
		return false;
	}

	bool FileOperation::unlink_file()
	{
		close_file();
		if(unlink(file_path_.c_str())==0) return true; // delete a hard link
		return false;
	}

	bool FileOperation::flush_file()
	{
		if(fd_>0)
		{
			if((open_flags_&O_SYNC)==0)
			{
				if(fsync(fd_)==0) return true;
			}
		}
		return false;

	}

	bool FileOperation::truncate_file(const int64_t length)
	{
		if(fd_<0) open_file();
		if(fd_>0)
		{
			if(ftruncate(fd_, length)==0) return true;
		}
		return false;
	}

	int FileOperation::get_fd_()
	{
		return fd_;
	}

	int64_t FileOperation::get_file_size()
	{
		if(fd_<0) open_file();
		if(fd_>0)
		{	struct stat buf;
			if(fstat(fd_, &buf)==0) return buf.st_size;
		}
		return -1;
	}
}