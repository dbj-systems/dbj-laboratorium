#pragma once
#include <filesystem>
// core always required --> #include "../core/dbj++core.h"
#include "../win/dbj_win32.h"
/*
things in here are mainly for helping managing 
local ini files, log files and a sutch.

Schema is:

	%programdata%/dbj				-- the root of all dbj system files
	%modulename%					-- module base name with no suffix
	%programdata%/dbj/%modulename%	-- for each dbj system module

	each module has two default files

	%programdata%/dbj/%modulename%/%modulename%.ini
	%programdata%/dbj/%modulename%/%modulename%.log
*/
namespace dbj::util
{
	namespace fs = ::std::filesystem;

	using smart_buffer = typename ::dbj::chr_buf::yanb ;

	// NOTE: WIN32 STL filesystem is wchar_t "oriented"
	// so we have to jump through more hoops
	inline auto narrow (fs::path wide_)
		-> smart_buffer
	{
		return { wide_.generic_string().data() };
	}

	/*
	by making the folder,basename and full path in one place we actually define
	the app file locations policy system wide
	*/
	struct file_descriptor 
	{
		using buff_t = ::dbj::chr_buf::yanb;
		virtual const char * suffix() const noexcept = 0;
		buff_t folder;
		buff_t basename;
		buff_t fullpath;
	};
	/*

	Usage:

	for a particular ap system file	inherit a concrete f descriptor and give 
	it a concrete suffix for example:

	struct log_file final : file_descriptor {
		virtual const char * suffix() const noexcept { return ".log"; }
	};

	function bellow does the rest.

	We do not do all of this all in one struct, so that we can decouple ini from log
	from whatever else app or user might require.
	*/
	inline void make_file_descriptor(file_descriptor & descriptor_)
	{
		std::error_code ec_;

		// with no suffix too
		// 	using smart_buffer = typename ::dbj::chr_buf::yanb ;
		auto base_nme_ = ::dbj::win32::module_basename(NULL) ;

		auto  programdata = dbj::core::util::program_data_path(ec_);

		_ASSERTE(!ec_);

		fs::path folder_ = programdata.get();
		folder_.concat("\\").concat(dbj::dbj_programdata_subfolder)
		       .concat("\\").concat(base_nme_.get());

		fs::path full_path_(folder_);
		full_path_.concat(base_nme_.get()).concat(descriptor_.suffix());

		descriptor_.folder = narrow(folder_);
		descriptor_.basename.assign( base_nme_ );
		descriptor_.fullpath = narrow(full_path_);
	}

	/*
	currently we just truncate the files. old versons are lost.
	*/
	// return true on truncation
	inline bool truncate_if_oversize(
		smart_buffer	file_path_,
		std::uintmax_t  max_allowed_size )
	{
		std::uintmax_t size_{};
		fs::path fp(file_path_.data());
		try {
			if (!exists(fp))
				return true; // will signal to the caller to write the top header

			// The size of the file, in bytes.
			size_ = fs::file_size(fp);

			if (size_ > max_allowed_size) {
				// truncate
				FILE *log_file_{};
				if (fopen_s(&log_file_, file_path_.data(), "w") != 0)
				{
					puts("\n\nCan't truncate the file -- " __FUNCSIG__ "\n\n");
					puts(file_path_.data());
					exit(1);
				}
			}
			else {
				// file is there and was not truncated
				return false;
			}
		}
		catch (fs::filesystem_error& e)
		{
			perror("\n\n");
			perror(e.what());
			perror("\n\n");
#ifdef _DEBUG
			::system("@echo.");
			::system("@pause");
			::system("@echo.");
#endif
			exit(1);
		}
		return true;
	}

	/*
	assure the presence of the folder, make it if not here
	*/
	inline void
		assure_folder( file_descriptor const & f_descriptor_ )
		noexcept
	{
		error_code e;
		// creates al the subdirs if required
		// it seems all is ok here if dir exist already
		fs::create_directories(f_descriptor_.folder.data(), e);
			_ASSERTE(!e);

		// let's try once more to be sure all is made ok
		e.clear(); fs::directory_entry dir(f_descriptor_.folder.data(), e);
		e.clear(); dir.refresh(e); // must do refresh
		e.clear(); bool rez = dir.is_directory(e); noexcept(rez);
		_ASSERTE(!e);
	}

} // dbj::util 
