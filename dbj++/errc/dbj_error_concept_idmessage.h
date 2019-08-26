#pragma once

/*

*/

namespace dbj::errc
{
	using smart_buffer_helper	 = ::dbj::vector_buffer<char>;
	using smart_buffer			 =  smart_buffer_helper::narrow;
	// id and message
	struct idmessage final
	{
		// used from stdint.h
		// not optional type
		// https://en.cppreference.com/w/c/types/integer
		using id_type = uint_fast64_t;

		// the only runtime buffer type that 
		// is (marginally) faster is unique<char[]>
		// vector<char> is infinitely easier to 
		// use and leads to much simpler code
		// note: it is important to use helper 
		// routines that will assure zero terminated
		// strings using this structure
		using message_type = vector< char >;

		id_type        id;
		message_type   message;

		// idmessage() = delete;

		// we declare id 0 as "not an id"
		// we create and keep it as an unique 
		// compile time value
		inline static constexpr id_type not_id{ 0 };
		static constexpr bool valid_id ( id_type const& id_) { return id_ > not_id; }
		static constexpr bool valid(idmessage const& idm_) { return idm_.id > not_id; }
	};

	inline namespace varnish {

		// user defined literal, creates vector<char> from string literal
		inline typename idmessage::message_type operator"" _buff(const char* str, size_t sze_) {
			idmessage::message_type rt_(sze_, char(0));
			std::copy(str, str + sze_, rt_.data());
			rt_.push_back(0);
			return rt_;
		}
	}

	namespace helpers
	{
		inline idmessage idmessage_make(idmessage::id_type id_, idmessage::message_type message_)
		{
#ifdef _DEBUG
			if (!idmessage::valid_id(id_))
			{
				// no excpetions throwing, sorry
				perror("bad message id, exiting");
				exit(42);
			}

			if (message_.size() < 1)
			{
				// no excpetions throwing, sorry
				perror("bad message, exiting");
				exit(42);
			}
#endif
			return idmessage{ id_, message_ };
		}

		inline idmessage idmessage_make(idmessage::id_type id_, const char* string_literal)
		{
			return idmessage_make(id_, smart_buffer_helper::make(string_literal));
		}

		// string views
		inline idmessage idmessage_make(idmessage::id_type id_, string_view sv_)
		{
			return idmessage_make(id_, smart_buffer_helper::make(sv_.data()));
		}
		// strings
		inline idmessage idmessage_make(idmessage::id_type id_, string str_) {
			return idmessage_make(id_, smart_buffer_helper::make(str_.c_str()));
		}

		// std array of chars
		template<size_t N>
		inline idmessage idmessage_make(idmessage::id_type id_, array<char, N> const& arr_)
		{
			return idmessage_make(id_, smart_buffer_helper::make(arr_.data()));
		}

		// unique ptr char array has no copy semantics
		inline idmessage idmessage_make(idmessage::id_type id_, unique_ptr<char[]> const& up_)
		{
			return idmessage_make(id_, smart_buffer_helper::make(up_.get()));
		}
		// shared ptr char array
		inline idmessage idmessage_make(idmessage::id_type id_, shared_ptr<char[]> const& sp_)
		{
			return idmessage_make(id_, smart_buffer_helper::make(sp_.get()));
		}

		// this one is a problem for MSVC
		template<size_t N>
		inline auto  idmessage_make(idmessage::id_type , const char(&string_literal)[N]) = delete;


		// dissalow references to temporaries
		template<size_t N>
		inline auto  idmessage_make(idmessage::id_type, char(&& string_literal)[N]) = delete;
	}

} // dbj::errc

