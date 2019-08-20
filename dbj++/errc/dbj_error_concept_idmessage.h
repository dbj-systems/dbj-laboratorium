#pragma once

//#include <cstdint>
//#include <cassert>
//#include <string_view>

namespace dbj::errc
{
	using smart_buffer_helper	 = ::dbj::vector_buffer<char> ;
	using smart_buffer			 = smart_buffer_helper::narrow ;
	// id and message
	struct idmessage_type final 
	{
		using id_type		= typename std::uint_fast32_t;
		using message_type	= typename smart_buffer;
		using char_type		= typename smart_buffer::value_type ;
		using reference		= typename std::reference_wrapper<idmessage_type>;
	private:
		const id_type		id_;
		const message_type	message_;
	public:

		idmessage_type() = delete;

		constexpr idmessage_type( id_type a_id_, const char_type* string_literal)
			: id_(a_id_), message_( smart_buffer_helper::make(string_literal))
		{
			DBJ_ASSERT(string_literal);
		}

		//
		template<size_t N>
		constexpr idmessage_type( id_type a_id_, const char_type(&string_literal)[N])
			: id_(a_id_), message_( smart_buffer_helper::make(string_literal))
		{}

		// dissalow references to temporaries
		template<size_t N>
		constexpr idmessage_type(id_type a_id_, char_type(&& string_literal)[N]) = delete;

		constexpr id_type id() const { return id_; }
		message_type message() const { return message_ ; }
		char_type const * data() const { return message_.data(); }

	};

} // dbj::errc

