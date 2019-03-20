#pragma once

#include "dbj_buffer.h"

namespace dbj {
	namespace fmt {

		template<typename T>
		using rfr = std::reference_wrapper<T>;
		/*
		somewhat inspired with
		https://msdn.microsoft.com/en-us/magazine/dn913181.aspx
		*/
		template <typename T>
		inline T frm_arg( T value) noexcept
		{
			return value ;
		}
		// no pass by value allowed
		// that means: no temporaries
		// template <typename T> T & frm_arg( rfr<T> && value) = delete;
#if 1
#pragma region dbj buffer and friends

		// template <typename T>
		inline char * frm_arg(  rfr<std::unique_ptr<char[]>> value) noexcept
		{
			return value.get().get() ;
		}

		inline wchar_t * frm_arg( rfr< std::unique_ptr<wchar_t[]>> value) noexcept
		{
			return value.get().get() ;
		}

		inline char * frm_arg(  std::shared_ptr<char> value) noexcept
		{
			return value.get() ;
		}

		inline wchar_t * frm_arg( std::shared_ptr<wchar_t> value) noexcept
		{
			return value.get() ;
		}

		inline char const * frm_arg( ::dbj::buf::buffer::type value) noexcept
		{
			return value.data() ;
		}

		inline char const * frm_arg( ::dbj::buf::yanb::type value ) noexcept
		{
			return value.data() ;
		}

		inline wchar_t const * frm_arg( ::dbj::buf::yanwb::type value) noexcept
		{
			return value.data() ;
		}

#pragma endregion 

		inline char const * frm_arg( std::error_code value) noexcept
		{
			if (value.value() == 0)
				// MSVC STL does have an empty string here ...
				return "OK";
			else {
				auto str = value.message();
				return (str.empty() ? "Empty" : value.message().c_str());
			}
		}

		template <typename T>
		inline T const * frm_arg( std::basic_string<T> value) noexcept
		{
			return value.c_str();
		}

		template <typename T>
		inline T const * frm_arg( std::basic_string_view<T> value) noexcept
		{
			return value.data();
		}
#endif
		/*
		vaguely inspired by
		https://stackoverflow.com/a/39972671/10870835
		*/
		template<typename ... Args>
		inline dbj::buf::yanb
			to_buff(std::string_view format_, Args /*const &*/ ...args)
			noexcept
		{
			static_assert(sizeof...(args) < 255, "\n\nmax 255 arguments allowed\n");
			const auto fmt = format_.data();
			// 1: what is the size required
			size_t size = 1 + std::snprintf(nullptr, 0, fmt, frm_arg( args ) ...);
			assert(size > 0);
			// 2: use it at runtime
			auto buf = std::make_unique<char[]>(size + 1);
			// each arg becomes arg to the frm_arg() overload found
			size = std::snprintf(buf.get(), size, fmt, frm_arg( args ) ...);
			assert(size > 0);

			return {buf.get()};
		}
		// wide version
		template<typename ... Args>
		inline dbj::buf::yanwb
			to_buff(std::wstring_view format_, Args const & ...args)
			noexcept
		{
			static_assert(sizeof...(args) < 255, "\n\nmax 255 arguments allowed\n");
			const auto fmt = format_.data();
			// 1: what is the size required
			size_t size = 1 + std::swprintf(nullptr, 0, fmt, frm_arg(args) ...);
			assert(size > 0);
			// 2: use it at runtime
			auto buf = std::make_unique<wchar_t[]>(size + 1);
			// each arg becomes arg to the frm_arg() overload found
			size = std::swprintf(buf.get(), size, fmt, frm_arg(args) ...);
			assert(size > 0);

			return { buf.get() };
		}

/*
BIG NOTE: if you mistake the formating code probably everything
on the console and in the app will go *very* wrong, and UCRT
crash will be spectacular
*/
	template<typename ... Args>
	inline void
		print(std::string_view format_, Args /*const &*/ ... args)
		noexcept
	{
		std::wprintf(L"%S", fmt::to_buff(format_, args...).data());
	}

	} // fmt

} // dbj

namespace dbj {
	namespace core {
#if 0
		template <typename ... Args>
		inline std::wstring printf_to_buffer
		(wchar_t const * const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_buff(message, args...);
			return { buf_.get() };
		}

		template <typename ... Args>
		inline std::string printf_to_buffer
		(const char * const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_buff(message, args...);
			return { buf_.get() };
		}
#endif
		// DBJ::TRACE exist in release builds too
		template <typename ... Args>
		inline void trace(wchar_t const * const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_buff(message, args...);
			::OutputDebugStringW(buf_.data()	);
		}
		template <typename ... Args>
		inline void trace(const char * const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_buff(message, args...);
			::OutputDebugStringA(buf_.data()	);
		}

#pragma warning( push )
#pragma warning( disable: 4190 )

		using smart_buf_type = typename dbj::buf::buff_type;

		extern "C" {

			/*	transform path to filename,	delimiter is '\\' */
			inline	dbj::buf::yanb
				filename(std::string_view file_path, const char delimiter_ = '\\')
				noexcept
			{
				_ASSERTE(!file_path.empty());
				size_t pos = file_path.find_last_of(delimiter_);
				return 
					dbj::fmt::to_buff("%s",
					(std::string_view::npos != pos
						? file_path.substr(pos, file_path.size()) 
						: file_path )
				);
			}

			/*
			usual usage :
			DBJ::FILELINE( __FILE__, __LINE__, "some text") ;
			*/
			// inline std::string FILELINE(const std::string & file_path,
			inline 
				dbj::buf::yanb
				fileline (std::string_view file_path,
				          unsigned line_,
				          std::string_view suffix = "")
			{
				_ASSERTE(!file_path.empty());

				return 
					dbj::fmt::to_buff(
						"%s(%u)%s", filename(file_path), line_, (suffix.empty() ? "" : suffix.data())
					);
			}

		} // extern "C"
#pragma warning( pop )
	} //core
} // dbj

#include "../dbj_gpl_license.h"

#pragma comment( user, DBJ_BUILD_STAMP ) 
