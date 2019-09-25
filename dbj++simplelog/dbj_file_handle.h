/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#pragma once
#ifndef _DBJ_FILE_HANDLE_INC_
#define _DBJ_FILE_HANDLE_INC_

#ifdef __cplusplus

#include "dbj--nanolib/dbj++status.h"
#include <io.h>
#include <fcntl.h>

namespace dbj {
	namespace simplelog {

		using namespace std;

		// File Handle(r)
		class FH final
		{
			string name_;
			int file_descriptor_ = -1;

			FH(const char* filename, int descriptor_)
				: name_(filename), file_descriptor_(descriptor_)
			{
				/* only assure can reach here */
			}

		public:
			using posix = typename dbj::nanolib::posix_retval_service< FH >;
			using return_type =  posix::return_type; 
			/*
			return type is pair{ optional{ FH }, optional{ json string } }
			*/
			[[nodiscard]] static return_type  assure(const char* filename)
			{
				const char* fn = filename;

				_ASSERTE(fn);
				//			fn = "/dev/null";
				int fd = -1;
				errno_t rez = _sopen_s(&fd, fn, O_WRONLY | O_APPEND | O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);

				if (rez != 0) {
					/*
	errno value	Condition
	EACCES	The given path is a directory, or the file is read-only, but an open-for-writing operation was attempted.
	EEXIST	_O_CREAT and _O_EXCL flags were specified, but filename already exists.
	EINVAL	Invalid oflag, shflag, or pmode argument, or pfh or filename was a null pointer.
	EMFILE	No more file descriptors available.
	ENOENT	File or path not found.
					*/
					return DBJ_RETVAL_ERR(posix, std::errc(rez));
				}

				struct stat sb;
				fstat(fd, &sb);

				switch (sb.st_mode & S_IFMT) {
				case S_IFCHR:  //printf("character device\n");        break;
#ifndef _MSC_VER
				case S_IFIFO:  //printf("FIFO/pipe\n");               break;
				case S_IFLNK:  //printf("symlink\n");                 break;
#endif // !_MSC_VER
				case S_IFREG:  //printf("regular file\n");            break;
					break;
				default:
					return DBJ_RETVAL_ERR(posix, std::errc::no_such_device);
					break;
				}

				auto fh = FH(filename, fd);
				return DBJ_RETVAL_OK(posix, fh);

			};

			[[nodiscard]] static  return_type assure(::std::string_view filename)
			{
				// 1.copy FH instance
				// 2.move FH instance
				return FH::assure(filename.data());
			}

			const char* name() const noexcept { return name_.c_str(); };

			FILE* file_ptr(const char* options_ = "w") const noexcept
			{
				_ASSERTE(file_descriptor_ > -1);
				static FILE* file_ = _fdopen(file_descriptor_, options_);
				_ASSERTE(file_ != nullptr);
				_ASSERTE(0 == ferror(file_));
				// fprintf(file, "TRA LA LA LA LA!");
				return file_;
			}
		}; // FH
	} // simplelog
} // dbj

#endif // __cplusplus

#endif // !_DBJ_FILE_HANDLE_INC_

