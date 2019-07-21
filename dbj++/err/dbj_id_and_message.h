#pragma once
#include <cassert>
#include <string_view>

namespace dbj::errc
{
#ifdef _MSC_VER
	#define DBJ_CONSTEXPR_ID_MESSAGE
#else
	#define DBJ_CONSTEXPR_ID_MESSAGE constexpr
#endif

	// id and message
	class idmessage_type final {
	public:
		using id_type = long;
		using message_type = std::string_view;
		using reference = std::reference_wrapper<idmessage_type>;

		idmessage_type() = delete;

		constexpr idmessage_type( id_type a_id_, const char* string_literal)
			: id_(a_id_), str_(string_literal)
		{}

		//
		template<size_t N>
		constexpr idmessage_type( id_type a_id_, const char(&string_literal)[N])
			: id_(a_id_), str_(string_literal)
		{}

		constexpr id_type id() const { return id_; };
		constexpr message_type message() const { return str_; };
	private:
		const id_type id_;
		message_type str_;
	};

	inline constexpr idmessage_type dbj_id_and_message_type_test_ () {

#ifdef _MSC_VER
		DBJ_CONSTEXPR_ID_MESSAGE auto ids_1 = idmessage_type(__LINE__, __FILE__);
#else
		// GCC and CLANG do it the standard way
		// https://wandbox.org/permlink/lc199NOd833zXSR4

		constexpr auto ids_1 = idmessage_type(__LINE__, __FILE__);
#endif
		DBJ_CONSTEXPR_ID_MESSAGE auto id = ids_1.id();
		DBJ_CONSTEXPR_ID_MESSAGE auto str = ids_1.message();

		return ids_1;
	}
} // dbj::errc

