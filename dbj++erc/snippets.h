#pragma once
#include "pch.h"
#include "dbj_simple_error.h"

namespace r_and_d {

	using namespace dbj::errc;
	using namespace std ;
	
		using sT = simple_error_type;

		constexpr auto err_42 = sT::make(42, "Right meaning");
		constexpr auto err_43 = sT::make(43, "Wrong meaning", _DBJ_CONSTEXPR_LINE, __FILE__);
		constexpr auto err_44 = sT::locate( err_42, _DBJ_CONSTEXPR_LINE, __FILE__);

		DBJ_TEST_UNIT(pragmatic_error_type_testing) {

			constexpr auto here1 = err_42;
			constexpr auto here2 = err_43;
			constexpr auto here3 = err_44;

			auto [err_id,err_msg,opt_err_loc,opt_err_file] = sT::flat(here3);

		}

	///////////////////////////////////////////////////////////////////////////////////////

	inline constexpr idmessage_type dbj_id_and_message_type_test_()
	{
#ifdef MSVC_LINE_IS_NOT_CONTEXPR
		// mistery bug is not a mistery
		constexpr auto LINE = _DBJ_CONSTEXPR_LINE;
		// constexpr long LINE = __LINE__;

		constexpr auto FILE = __FILE__;
		constexpr auto MSCVER = _MSC_VER;
		constexpr auto MSCFULLVER = _MSC_FULL_VER;
		constexpr auto MSCBUILD = _MSC_BUILD;

		static_assert((bool)noexcept(MSCBUILD, LINE, MSCFULLVER, MSCVER, FILE), "");
#endif // MSVC_LINE_IS_NOT_CONTEXPR

		constexpr auto ids_1 = idmessage_type(_DBJ_CONSTEXPR_LINE, __FILE__);
		constexpr auto id = ids_1.id();
		constexpr auto str = ids_1.message();

		return ids_1;
	}

	DBJ_TEST_UNIT(id_string_type)
	{
		using dbj::fmt::print;
		// making
		constexpr auto ids_1 = dbj_id_and_message_type_test_();
		constexpr auto ids_2 = idmessage_type(42, "Hola Lola!");
		// moving
		auto mover = [](auto ids_arg_) constexpr { return ids_arg_;  };
		constexpr auto ids_3 = mover(ids_1);

		// printing
		auto printer = [](idmessage_type ids) {
			print("\n\n idstring type\t%s\n\n{ \nid:\t%d,\n\nstr:\t'%s'\n}\n\n", typeid(ids).name(), ids.id(), ids.message());
		};

		printer(ids_1);
		printer(ids_2);
		printer(ids_3);
	}

	DBJ_TEST_UNIT(compile_time_sv_carier)
	{
		using dbj::fmt::print;
		using namespace std::literals;

#define sv_carrier(L_) [] () constexpr { constexpr auto sview = DBJ_CONCAT(L_,sv); return sview; }

		// but this also works
		constexpr auto carrier = sv_carrier("Hola Loyola!");
		print("\n\n%s", carrier().data());

#undef sv_carrier
	}

	// Convert array into std::array
	template<size_t N, std::size_t... I>
	inline constexpr decltype(auto) 
		charr_to_stdarr (const char (&charr)[N], std::index_sequence<I...>)
	{
		return std::array<char, N>{ { charr[I]...} };
	}

	/*
	from string literal make compile time char buffer inside std::array

	constexpr auto buffer_1 = inner::char_buff("Hola Lola!");
	constexpr std::array<char, buffer_1.size() > buffer_2 = buffer_1;
	*/
	template<size_t N, typename Indices = std::make_index_sequence<N> >
	constexpr decltype(auto)
		char_buff(const char(&sl_)[N])
	{
		//std::array<char, N + 1 > buffer_{ { 0 } };
		//size_t k = 0;
		//for (auto chr : sl_) { buffer_[k++] = chr; }
		//return buffer_;

		return charr_to_stdarr(sl_, Indices{} );
	}

	DBJ_TEST_UNIT(constexpr_charr_array_carier)
	{
#define buf_carrier(L_) [] () constexpr { constexpr auto char_arr = char_buff(L_); return char_arr; }

		constexpr auto chr_arr_buf = char_buff("Hola Ignacio!");
		using dbj::fmt::print;
		print("\n\n%s", chr_arr_buf.data());

#undef buf_carrier
	}
} // r+and+d
namespace dbj::samples {

	using namespace dbj::errc;

	DBJ_TEST_UNIT(dbj_errc_check_the_error_type_maleability)
	{
		using namespace dbj::fmt;
		error_type err = error_type::make(
			static_cast<error_type::id_type>(std::errc::argument_list_too_long),
			"Argument list too long"
		);
		print("\nno location\n\nerror json format: %s", error_type::json(err).get());
		error_type::locate(err, __LINE__, __FILE__);
		print("\n\nwith location\n\nerror json format: %s", error_type::json(err).get());
	}

	/*
	create return type for testing in here
	*/
	using my_errc_type =
		dbj::errc::error_concept< std::int64_t, dbj::errc::simple_error_type >;
	/*
	create error instances for testing in here
	in reality users will have them in namespaces

	notice how these predefined instances have no location in them

	notice we start id's from 1001
	this is to try and not clash with POSIX std::errc
	*/
	namespace posix {
		constexpr simple_error_type divide_by_zero =
			simple_error_type::make(1001, "Divide by Zero");

		constexpr simple_error_type integer_divide_overflows =
			simple_error_type::make(1002, "Integer Divide Overflows");

		constexpr simple_error_type not_integer_division =
			simple_error_type::make(1002, "Non Integer Division");

	} // posix errors


	auto error_return = [](my_errc_type::error_type E_, long L_, const char* F_) {
		return my_errc_type::make_err(
			simple_error_type::locate(E_, L_, F_)
		);
	};

	auto value_return = [](my_errc_type::value_type val_) {
		return my_errc_type::make_val(val_);
	};

	/*
	canonical example from
	http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0709r0.pdf
	page 21 onwards
	*/
	inline auto
		safe_divide = [](auto i, auto j)
	{
		if (j == 0)
			return error_return(posix::divide_by_zero, _DBJ_CONSTEXPR_LINE, __FILE__);

		if (i == INT_MIN && j == -1)
			return error_return(posix::integer_divide_overflows, _DBJ_CONSTEXPR_LINE, __FILE__);

		if (i % j != 0)
			return error_return(posix::not_integer_division, _DBJ_CONSTEXPR_LINE, __FILE__);

		return value_return((i / j));
	};

	DBJ_TEST_UNIT(dbj_errc_check_the_errc_maleability)
	{
		using dbj::fmt::print;

		auto analyzer = []
		(
			my_errc_type::value_type divider,
			my_errc_type::value_type divisor
		) 
		{
			auto [v, e] = safe_divide(divider, divisor);
			
			if(e)
			{
				auto [e_id, e_msg, loc_, loc_msg ] = (*e).flat(*e);
				print("\n\nError:\n\tid:%d\n\tmessag: %s\n\tline:%d\n\tfile: %s\n\n"
					, e_id, e_msg, loc_, loc_msg);
			}
			// always check the value
			if (v)
				print("\n\nValue returned: %d\n\n", int(*v));
		};

		analyzer(8, 2);
		analyzer(8, 0);
		analyzer(0.0, 42.13);

	}

} // dbj::samples
