#pragma once

#include "stdafx.h"

namespace dbj::sql {

	using namespace std;


	[[noreturn]] inline void dbj_terror(const char* msg_, const char* file_, const int line_)
	{
		_ASSERTE(msg_ && file_ && line_);
		std::fprintf(stderr, "\n\ndbj++sql Terminating error:%s\n%s (%d)", msg_, file_, line_);
		::exit(EXIT_FAILURE);
	}

#ifndef DBJ_VERIFY
#define DBJ_VERIFY_(x, file, line ) if (false == x ) ::dbj::sql::dbj_terror( #x ", failed", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)
#endif

#pragma region vector char_type buffer

	constexpr inline std::size_t DBJ_64KB = UINT16_MAX;
	/*
	for runtime buffering the most comfortable and in the same time fast
	solution is vector<char_type>

	only unique_ptr<char[]> is faster than vector of  chars, by a margin
	*/
	struct v_buffer final {

		using buffer_type		=  std::vector<char>;

		static
			buffer_type make(size_t count_)
		{
			_ASSERTE(count_ > 0);
			_ASSERTE(DBJ_64KB >= count_);
			buffer_type retval_(count_ + 1, char(0));
			return retval_;
		}

		static
			buffer_type make(std::basic_string_view<char> sview_)
		{
			_ASSERTE(sview_.size() > 0);
			_ASSERTE(DBJ_64KB >= sview_.size());
			buffer_type retval_(sview_.begin(), sview_.end());
			// terminate!
			retval_.push_back(char(0));
			return retval_;
		}

		static
			buffer_type make(std::unique_ptr<char[]> const& upc_)
		{
			return v_buffer::make(std::basic_string_view<char>(upc_.get()));
		}

		static
			buffer_type make(std::shared_ptr<char[]> const& upc_)
		{
			return v_buffer::make(std::basic_string_view<char>(upc_.get()));
		}
	}; // v_buffer

#pragma endregion
} // dbj::sql
