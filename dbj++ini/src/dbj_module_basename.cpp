
#include "../dbj++ini.h"

#include <dbj++/core/dbj++core.h>
#include <dbj++/console/dbj++con.h>
#include <dbj++/win/dbj++win.h>

#include <array>
#include <filesystem>
#include <cassert>
#include <memory>

// using smart_buffer = std::shared_ptr<char>;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

//static const char * dbj_get_envvar(char * varname_)
//{
//	_ASSERTE(varname_);
//	static std::array<char, 256>	bar{ {0} };
//	bar.fill(0); // zero the buffer
//	::SetLastError(0);
//	if (1 > ::GetEnvironmentVariableA(varname_, bar.data(), (DWORD)bar.size()))
//	{
//		std::error_code  er_ = std::error_code(::GetLastError(), std::system_category());
//
//		DBJ_VERIFY(!er_.message().c_str());
//	}
//	return bar.data();
//}
//
///*
//*/
//static std::filesystem::path dbj_program_data_path()
//{
//	namespace fs = std::filesystem;
//
//	std::string program_data{ dbj_get_envvar((char*)"ProgramData") };
//
//	static fs::path prog_data_path_;
//	prog_data_path_ = program_data;
//	return prog_data_path_;
//}


namespace dbj::ini
{
	/*
	   ini file descriptor

	   folder -- %programdata%\\dbj\\module_base_name
	   basename -- module_base_name + ".log"
	*/
	ini_file_descriptor ini_file() 
	{
		namespace fs = std::filesystem;
		std::error_code ec_;
		dbj::buf::yanb programdata = dbj::core::util::program_data_path(ec_);

		_ASSERTE(! ec_);

		fs::path folder_ = programdata.data();
		folder_.concat("\\").concat(dbj_programdata_subfolder);

		dbj::buf::yanb base_name_ = ::dbj::win32::module_basename(NULL);

		auto fp_ = folder_.concat(base_name_.data()).concat(".ini");

		// NOTE: WIN32 STL filesystem is wchar_t "oriented"
		// so we have to jump through more hoops
		auto narrow = [](wchar_t const * wide_)
			-> std::string
		{
			std::wstring ws(wide_);
			return { ws.begin(), ws.end() };
		};

		std::string nf_ = narrow(folder_.c_str());
		std::string nfp_ = narrow(fp_.c_str());

		return ini_file_descriptor{
			smart_buffer(_strdup(nf_.data())),
			smart_buffer(_strdup(base_name_.data())),
			smart_buffer(_strdup(nfp_.data()))
		};
	}
} // dbj::ini