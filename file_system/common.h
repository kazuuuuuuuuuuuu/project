#ifndef _COMMON_H_
#define _COMMON_H_ 

static int debug = 1;

#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <cstdlib>
#include <sstream>

namespace file_system
{
	// status upon return
	static const int32_t SUCCESS = 1;
	static const int32_t FAILED = 0;
	static const int32_t INDEX_BLOCK_NEED_MAPPED = -8000;
	static const int32_t INDEX_BLOCK_NEED_ROLL_BACK = -8001;
	static const int32_t METAINFO_NOT_FOUND = -8002;

	// the parameters used for updating the index block header
	enum OperType
	{
		OPER_INSERT, OPER_DELETE, OPER_COMPRESS
	};

	// constant parameters
	static const std::string DATA_BLOCK_DIR_PREFIX = "./datablock/";
	static const std::string INDEX_BLOCK_DIR_PREFIX = "./indexblock/";

	struct MMap_Option
	{
		int32_t max_size_;
		int32_t first_size_;
		int32_t append_size_;
	};

	struct BlockInfo
	{
		uint32_t block_id_;
		int32_t version_;
		int32_t file_count_;
		int32_t size_;
		int32_t del_file_count_;
		int32_t del_size_;
		uint32_t next_file_no_;

		BlockInfo()
		{
			memset(this, 0, sizeof(BlockInfo));
		}

		inline bool operator==(const BlockInfo &rhs) const
		{
			return memcmp(this, &rhs, sizeof(BlockInfo))==0;
		}
	};

	struct MetaInfo
	{
		uint32_t file_id_;
		int64_t offset_;
		int32_t nbytes_;
		int64_t next_;

		MetaInfo()
		{
			memset(this, 0, sizeof(MetaInfo));
		}

		MetaInfo(const uint32_t file_id, const int64_t offset, const int32_t nbytes, const int64_t next)
		{
			file_id_ = file_id;
			offset_ = offset;
			nbytes_ = nbytes;
			next_ = next;
		}

		MetaInfo(const MetaInfo &metainfo) // copy constructor
		{
			memcpy(this, &metainfo, sizeof(MetaInfo));
		}

		MetaInfo& operator=(const MetaInfo &rhs)
		{
			if(&rhs==this)
				return *this;
			memcpy(this, &rhs, sizeof(MetaInfo));
			return *this;
		}

		bool operator==(const MetaInfo &rhs) const
		{
			return memcmp(this, &rhs, sizeof(MetaInfo))==0;
		}
	};
}
#endif