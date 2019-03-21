#pragma once
#include "../core/dbj++core.h"
#include "../win/dbj_win32.h"
#include <filesystem>

namespace dbj::util 
{
	namespace fs = ::std::filesystem;
	using smart_buffer = ::dbj::buf::yanb;

	struct file_descriptor {
		virtual const char * suffix() const noexcept = 0;
		smart_buffer folder;
		smart_buffer basename;
		smart_buffer fullpath;
	};


	inline void make_file_descriptor(file_descriptor & descriptor_)
	{
		std::error_code ec_;
		smart_buffer programdata(dbj::core::util::program_data_path(ec_));

		_ASSERTE(!ec_);

		fs::path folder_ = programdata.data();
		folder_.concat("\\").concat(dbj::programdata_subfolder);

		smart_buffer base_name_ = ::dbj::win32::module_basename(NULL);

		fs::path full_path_(folder_);
		full_path_.concat(base_name_.data()).concat(descriptor_.suffix());

		// NOTE: WIN32 STL filesystem is wchar_t "oriented"
		// so we have to jump through more hoops
		auto narrow = [](fs::path wide_)
			-> std::string
		{
			std::wstring ws(wide_);
			return { ws.begin(), ws.end() };
		};

		std::string nf_
			= narrow(folder_);
		std::string nfp_
			= narrow(full_path_);

		descriptor_.folder = smart_buffer(nf_.data());
		descriptor_.basename = smart_buffer(base_name_.data());
		descriptor_.fullpath = smart_buffer(nfp_.data());
	}

} // dbj::util 
