#pragma once

/*
	The key 'building block' type.

		+-----------------+
		|  ID             |
		+-----------------+
		|  MESSAGE        |
		+-----------------+

*/
namespace dbj {

	using buffer_helper	 = ::dbj::vector_buffer<char>;
	using buffer_type	 =  buffer_helper::buffer_type;


	struct id_message_type {

		// used from stdint.h
		// not optional 
		// https://en.cppreference.com/w/c/types/integer
		using id_type = uint_fast64_t;

		// the only runtime buffer type that 
		// is (marginally) faster is unique<char[]>
		// vector<char> is infinitely easier to 
		// use and leads to much simpler code
		// note: it is important to use helper 
		// routines that will assure zero terminated
		// strings using this structure
		using message_type  = buffer_type;

		// initial aka default state of the id is 0
		// by that we know instance is not valid
		// it has to valid id given and message with that
		id_type        id{} ;
		message_type   message{} ;

		// we declare id 0 as "not an id"
		// we create and keep it as an unique 
		// compile time value
		inline static constexpr id_type not_id{ 0 };

	}; // idemessage

} // dbj

namespace dbj::errc
{
	// id and message
	struct idmessage final : 
		 ::dbj::id_message_type
	{
		using base = ::dbj::id_message_type;

		 static constexpr bool valid_id( id_type id_) noexcept { return id_ > not_id; }

		 constexpr bool valid() const noexcept { return this->id > not_id; }

#pragma region convenience

		 static idmessage make (id_type id_, message_type message_)
		{
			if (!valid_id(id_))
			{
				// debug time pop-up
				_ASSERTE( false );
				// no runtime exceptions throwing, sorry
				perror(DBJ_ERR_PROMPT("bad message id, exiting"));
				exit(42);
			}

			if (message_.size() < 1)
			{
				// debug time pop-up
				_ASSERTE(false);
				// no runtime exceptions throwing, sorry again
				perror(DBJ_ERR_PROMPT("bad message, exiting"));
				exit(42);
			}

			return { id_ , message_ } ;
		}

		static idmessage make(id_type id_, const char* string_literal)
		{
			return { id_, buffer_helper::make(string_literal) };
		}

		// string views
		static idmessage make(id_type id_, string_view sv_)
		{
			return { id_, buffer_helper::make(sv_.data()) };
		}
		// strings
		static idmessage make(id_type id_, string str_) {
			return { id_, buffer_helper::make(str_.c_str()) };
		}

		// std array of chars
		template<size_t N>
		static idmessage make(id_type id_, array<char, N> const& arr_)
		{
			return { id_, buffer_helper::make(arr_.data()) };
		}

		// unique ptr char array has no copy semantics
		static idmessage make(id_type id_, unique_ptr<char[]> const& up_)
		{
			return { id_, buffer_helper::make(up_.get()) };
		}
		// shared ptr char array
		static idmessage make(id_type id_, shared_ptr<char[]> const& sp_)
		{
			return { id_, buffer_helper::make(sp_.get()) };
		}

		// this one is a problem for MSVC
		template<size_t N>
		static auto  make(id_type, const char(&string_literal)[N]) = delete;


		// dissalow references to temporaries
		template<size_t N>
		static auto  make(id_type, char(&& string_literal)[N]) = delete;

#pragma endregion

	}; // idmessage

#if 0
	// already defined in dbj++ core buffers 
	inline namespace literal {

		// user defined literal, creates vector<char> from string literal
		inline typename message_type operator"" _buff(const char* str, size_t sze_) {
			message_type rt_(sze_, char(0));
			std::copy(str, str + sze_, rt_.data());
			rt_.push_back(0);
			return rt_;
		}
	}
#endif



} // dbj::errc

