#ifndef _DBJ_FORMAT_INC
#define _DBJ_FORMAT_INC

#include "dbj_buffers.h"

namespace dbj {
	namespace fmt {

		template<typename ... Args>
		inline auto to_wbuff
		(wchar_t const* format_, Args ... args)	noexcept -> std::vector<wchar_t> ;

		template<typename ... Args>
		inline auto	to_buff
		(char const* format_, Args ...args) noexcept -> std::vector<char> ;


		//////////////////////////////////////////////////////////////////////

		template<typename T>
		using rfr = std::reference_wrapper<T>;
		/*
		somewhat inspired with
		https://msdn.microsoft.com/en-us/magazine/dn913181.aspx
		*/
		template <typename T>
		inline T frm_arg(T value) noexcept
		{
			return value;
		}
		// no pass by value allowed
		// that means: no temporaries
		// template <typename T> T & frm_arg( rfr<T> && value) = delete;

		inline char* frm_arg( std::vector<char> const & value) noexcept
		{
			_ASSERTE( value.back() == char(0) ); // properly terminated?
			return const_cast<char *>( value.data() );
		}

		inline wchar_t* frm_arg( std::vector<wchar_t> const & value) noexcept
		{
			_ASSERTE(value.back() == wchar_t(0)); // properly terminated?
			return const_cast<wchar_t *>( value.data() ) ;
		}

#pragma region dbj buffer and friends

		/*
		this is fast and dangerous, we can not check if these are 
		properly zero terinated
		*/
		inline char* frm_arg(rfr<std::unique_ptr<char[]>> value) noexcept
		{
			return value.get().get();
		}

		inline wchar_t* frm_arg(rfr< std::unique_ptr<wchar_t[]>> value) noexcept
		{
			return value.get().get();
		}

		inline char* frm_arg(std::shared_ptr<char> value) noexcept
		{
			return value.get();
		}

		inline wchar_t* frm_arg(std::shared_ptr<wchar_t> value) noexcept
		{
			return value.get();
		}


#pragma endregion 

		inline char const* frm_arg(std::error_code const & value) noexcept
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
		inline T const* frm_arg(std::basic_string<T> const & value) noexcept
		{
			return value.c_str();
		}

		template <typename T>
		inline T const* frm_arg(std::basic_string_view<T> const & value) noexcept
		{
			return value.data();
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
			DBJ_VERIFY( format_.size() > 0  );
			static_assert( sizeof...(args) > 0  );
			// vectore move happens here
			std::vector<char> buffy{ fmt::to_buff(format_.data(), args...) } ;
			// here we basically are doing the transformation from narrow
			// to wide amd that is a performance hit
			std::wprintf(L"%S",  buffy.data() );
		}

	} // fmt

} // dbj

namespace dbj {
	namespace core {
#if 1
		template <typename ... Args>
		inline std::wstring printf_to_buffer
		(wchar_t const* const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_buff(message, args...);
			return { buf_.get() };
		}

		template <typename ... Args>
		inline std::string printf_to_buffer
		(const char* const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_buff(message, args...);
			return { buf_.get() };
		}
#endif
		// DBJ::TRACE exist in release builds too
		template <typename ... Args>
		inline void wtrace(wchar_t const* const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_wbuff(message, args...);
			::OutputDebugStringW(buf_.data());
		}
		template <typename ... Args>
		inline void trace(const char* const message, Args ... args) noexcept
		{
			auto buf_ = dbj::fmt::to_buff(message, args...);
			::OutputDebugStringA(buf_.data());
		}

#ifdef _DEBUG
#define DBJ_TRACE( msg_ , ...) ::dbj::core::wtrace( msg_, __VA_ARGS__ )
#else
#define DBJ_TRACE(...)
#endif


	} //core

	inline void errno_exit(errno_t errno_, const char* file, const int line, void(*ccb_)(void) = nullptr)
	{
		if (errno_t(0) == errno_)
			return;
		char err_msg_[BUFSIZ]{ 0 };
		strerror_s(err_msg_, BUFSIZ, errno_);
		::dbj::core::trace(
			"\n\n\n%s(%d)\n\nerrno message: %s\nerrno was: %d\n\nExiting..\n\n"
			, file, line, err_msg_, errno_
		);

		perror(err_msg_);

#ifdef _DEBUG
		::DebugBreak();
#endif // _DEBUG

		if (ccb_) ccb_();
		// NOTE: std::exit is *different*
		std::exit(errno_);
	}

#define	DBJ_ERRP(...) do { \
errno_t e_ = ::fprintf(stderr, __VA_ARGS__); \
if (e_ < 0) ::dbj::errno_exit( errno, __FILE__, __LINE__ ); \
} while (false)

#define	DBJ_WERRP(...) do { \
errno_t e_ = ::fwprintf(stderr, __VA_ARGS__); \
if (e_ < 0) ::dbj::errno_exit( errno, __FILE__, __LINE__ ); \
} while (false)

#define DBJ_FPRINTF(...) \
	do { if (errno_t result_ = ::fprintf(__VA_ARGS__); result_ < 0) \
		dbj::errno_exit(errno, __FILE__, __LINE__); } while(false)

}

namespace dbj::core {

#pragma warning( push )
#pragma warning( disable: 4190 )

	// using smart_buf_type = typename dbj::chr_buf::buff_type;

	extern "C" {

		/*	transform path to filename,	default delimiter is '\\' */
		inline	typename ::dbj::vector_buffer<char>::narrow
			filename(std::string_view file_path, const char delimiter_ = '\\')
			noexcept
		{
			_ASSERTE(!file_path.empty());
			size_t pos = file_path.find_last_of(delimiter_);
			return
				dbj::fmt::to_buff("%s",
				(std::string_view::npos != pos
					? file_path.substr(pos, file_path.size())
					: file_path)
				);
		}

		/*
		usual usage :
		DBJ::FILELINE( __FILE__, __LINE__, "some text") ;
		*/
		// inline std::string FILELINE(const std::string & file_path,
		inline
			::dbj::vector_buffer<char>::narrow  fileline
			(std::string_view file_path, unsigned line_, std::string_view suffix = "")
		{
			_ASSERTE(!file_path.empty());

			return
				dbj::fmt::to_buff(
					"%s(%u)%s", filename(file_path), line_, (suffix.empty() ? "" : suffix.data())
				);
		}

	} // extern "C"
#pragma warning( pop )
}
namespace dbj::fmt {
	
	/*  only vaguely inspired by
		https ://stackoverflow.com/a/39972671/10870835
	*/
	template<typename ... Args>
	inline auto	to_buff
	   ( char const * format_, Args ...args)
		 noexcept -> std::vector<char>
	{
		DBJ_VERIFY(format_ != nullptr );
		static_assert(sizeof...(args) < 255, "\n\nmax 255 arguments allowed\n");
		/*  ppf_arg's are called only once ,rezults are stored in a tuple */
		auto args_tup = make_tuple( frm_arg(args) ... );
		auto snprintf_tup_1 = make_tuple(nullptr, 0, format_);

		// obtain the buffer size required
		size_t size = apply(std::snprintf, tuple_cat(snprintf_tup_1, args_tup));
		DBJ_VERIFY(size > 0);

		std::vector<char> buf(size + 1);
		auto snprintf_tup_2 = make_tuple(buf.data(), buf.size(), format_);
		// populate the buffer, do not call the  ppf_arg's again
		size = apply(std::snprintf, tuple_cat(snprintf_tup_2, args_tup));
		DBJ_VERIFY(size > 0);
		// snprintf iz properly terminating the rezult
		// https://stackoverflow.com/a/13067917/10870835
		return buf;
	}

	// wide version
	//template<typename ... Args>
	//inline auto to_buff
	//      (wchar_t const * format_, Args ... args)
	//	noexcept -> std::vector<wchar_t>
	//{
	//	DBJ_VERIFY(format_ != nullptr);
	//	static_assert(sizeof...(args) < 255, "\n\nmax 255 arguments allowed\n");
	//	/*  ppf_arg's are called only once ,rezults are stored in a tuple */
	//	auto args_tup = std::make_tuple(frm_arg(args) ...);
	//	auto snprintf_tup_1 = make_tuple(nullptr, 0, format_);

	//	// obtain the buffer size required
	//	size_t size = std::apply(std::swprintf, tuple_cat(snprintf_tup_1, args_tup));
	//	DBJ_VERIFY(size > 0);

	//	std::vector<wchar_t> buf(size + 1);
	//	auto snprintf_tup_2 = make_tuple(buf.data(), buf.size(), format_);
	//	// populate the buffer, do not call the  ppf_arg's again
	//	size = std::apply(std::swprintf, tuple_cat(snprintf_tup_2, args_tup));
	//	DBJ_VERIFY(size > 0);
	//	// snprintf iz properly terminating the rezult
	//	// https://stackoverflow.com/a/13067917/10870835
	//	return buf;
	//}

	template<typename ... Args>
	inline auto to_wbuff
	(wchar_t const* format_, Args ... wargs_)
		noexcept -> std::vector<wchar_t>
	{
		DBJ_VERIFY(format_ != nullptr);
		static_assert(sizeof...(wargs_) < 255, "\n\nmax 255 arguments allowed\n");
#undef USE_TUPLE
#ifdef USE_TUPLE
		// for some reason as of today 2019-10-31, CL can not compile this
			/*  ppf_arg's are called only once ,rezults are stored in a tuple */
	auto args_tup = std::make_tuple(frm_arg(wargs_) ...);
	auto snprintf_tup_1 = make_tuple(nullptr, 0, format_);

	// obtain the buffer size required
	size_t size = std::apply(std::swprintf, std::tuple_cat(snprintf_tup_1, args_tup));
	DBJ_VERIFY(size > 0);

	std::vector<wchar_t> buf(size + 1);
	auto snprintf_tup_2 = make_tuple(buf.data(), buf.size(), format_);
	// populate the buffer, do not call the  ppf_arg's again
	size = std::apply(std::swprintf, std::tuple_cat(snprintf_tup_2, args_tup));
	DBJ_VERIFY(size > 0);
#else
		// 1: what is the size required
		size_t size = 1 + std::swprintf(nullptr, 0, format_, frm_arg(wargs_) ...);
		DBJ_VERIFY(size > 0);
		// 2: use it at runtime
		vector<wchar_t> buf(size + 1);
		// each arg becomes arg to the frm_arg() overload found
		size = std::swprintf(buf.data(), buf.size(), format_, frm_arg(wargs_) ...);
		DBJ_VERIFY(size > 0);
#endif
		return buf;
	}
}

#undef USE_TUPLE

#endif // !_DBJ_FORMAT_INC
