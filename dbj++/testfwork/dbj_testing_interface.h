#pragma once

#include "dbj_testing.h"
#include "../console/dbj_console.h"
// #include "../dbj_console_painter.h"

/*
   testing interface depends on the dbj::console
   therefore it is moved here so that the whole testing
   does not depend on dbj::console
*/
namespace dbj {
	namespace testing {

		typedef typename dbj::console::painter_command CMD;
		using dbj::console::print;

		template< typename ... Args >
		inline void text_line(
			CMD && cmd_,
			Args && ... args
		) {
			paint(cmd_);
			print("\n");
			if constexpr (sizeof...(args) < 1) {
				print(::dbj::LINE());
			}
			else {
				print(args...);
			}
			paint(CMD::text_color_reset);

		};

		inline auto white_line = [&](auto && ... args) {
			text_line(CMD::white, args...);
		};

		inline auto blue_line = [&](auto && ... args) {
			text_line(CMD::bright_blue, args...);
		};

		inline auto green_line = [&](auto && ... args) {
			text_line(CMD::green, args...);
		};

		inline auto red_line = [&](auto && ... args) {
			text_line(CMD::bright_red, args ...);
		};

		inline auto prefix(
			/*
			   since we do only unicode builds
			   argv[0] is wchar_t *
			 */
			const wchar_t * prog_full_path
		) {
			_ASSERTE(prog_full_path);

			auto narrow_path = dbj::fmt::to_buff("%S", prog_full_path);
			
			auto buff_ = dbj::fmt::to_buff(
			"\n%s"
			"\n%s"
			"\n%s by %s"
			"\nMSVC version: %d"
			"\n[%d] tests registered"
			"\nRunning: %s",
				::dbj::LINE(),
				::dbj::testing::TITLE,
				::dbj::YEAR(), ::dbj::COMPANY(),
				_MSC_FULL_VER,
				internal::tuset_instance().size(),
				narrow_path.data()
				);

			_ASSERTE(buff_);

			white_line(buff_.data());
		}

		inline auto suffix() {
			auto buff_ = dbj::fmt::to_buff(
				"\n%s"
				"\n%s\n",
				::dbj::LINE() , dbj::testing::ALLDN
			);
			_ASSERTE(buff_);
			white_line(buff_.data());
			// no needed --> print(CMD::text_color_reset);
		}

		inline auto unit_prefix(const char * name_) {
			auto buff_ = dbj::fmt::to_buff(
				"\n%s"
				"\nBegin test unit %s\n",
				::dbj::LINE(), name_
			);
			_ASSERTE(buff_);
			blue_line(buff_.data());
		}

		inline auto unit_suffix(const char * name_) {
			auto buff_ = dbj::fmt::to_buff(
				"\n%s"
				"\nEnd test unit %s\n",
				::dbj::LINE(), name_
			);
			_ASSERTE(buff_);
			blue_line(buff_.data());
		}

		inline auto space_prefix(const char * name_) {
			auto buff_ = dbj::fmt::to_buff(
				"\n%s"
				"\nRuntime started %s\n",
				::dbj::LINE(), name_
			);
			_ASSERTE(buff_);
			white_line(buff_.data());
		}

		inline auto space_suffix(const char * name_) {
			auto buff_ = dbj::fmt::to_buff(
				"\n%s"
				"\nRuntime finished %s\n",
				::dbj::LINE(), name_
			);
			_ASSERTE(buff_);
			white_line(buff_.data());
		}

		/*  execute all the tests collected  */
		inline void execute(
			const int		DBJ_MAYBE(argc),
			const wchar_t * DBJ_MAYBE(argv)[],
			const wchar_t * DBJ_MAYBE(envp)[]
		)
		{
			if (internal::tuset_instance().size() < 1) {
				white_line();
				white_line("No tests registered");
				white_line();
				return;
			}

			// here we catch only what was declared as 
			// part of std::exception hierarchy
			// everything else will go up the stack
			auto handle_eptr = [](std::exception_ptr eptr)
				// passing by value is ok
			{
				try {
					if (eptr) {
						std::rethrow_exception(eptr);
					}
				}
				catch (const std::exception & x_) {
					::dbj::console::print(x_);
				}
			};

			prefix(argv[0]);
			for (auto & tunode_ : internal::tuset_instance())
			{
				unit_prefix( tunode_.description.data() );
				try {
					white_line(" ");
					internal::unit_execute( tunode_.TU );
					white_line(" ");
				}
				catch (const std::error_code ec_) {
					// yes we are crazy like that
					// we throw std::error_code and then
					// we catch it by value
					// dbj@dbj.org 2019-02-02
					print("\nstd::error_code\n\t", ec_ );
				}
				catch (const std::system_error & x_) {
					print("\nstd::system_error\n\t", x_.what(), "\n\t", x_.code());
				}
				catch (const std::exception & x_) {
					print("\n\nstd::exception caught by dbj testing system\n\t");
					print( x_ );
					print("\n\n");
				}
				catch (...) {
					std::exception_ptr eptr{
						std::current_exception()
					}; // capture
					handle_eptr(eptr);
				}
				unit_suffix(tunode_.description.data());
			}
			suffix();
		}

#ifndef DBJ_TEST_ATOM

		// by default if result is bool, show it
		inline bool atom_show_true_rezult = true;

		struct flip_atom_show_true_rezult final {

			flip_atom_show_true_rezult() {
				atom_show_true_rezult = false;
			}

			~flip_atom_show_true_rezult() {
				atom_show_true_rezult = true;
			}

		} ;

		///<summary>
		/// usage example: 
		/// <code>
		/// auto any_ = DBJ_TEST_ATOM( dbj::any::range({ 42 }) );
		/// </code>
		/// argument of the macro gets printed as a string and then
		/// get's executed as a single expression 
		/// of the lambda body
		/// whose return value is returned
		/// </summary>
		template<typename return_type>
		// inline decltype(auto)
		inline return_type const &
			test_lambada(
				const char * expression,
				return_type const & anything,
				bool show_type = true) noexcept
		{
			using ::dbj::console::print;
			using ::dbj::console::painter_command;

			using no_const_no_ref_return_type 
				= std::remove_const_t< std::decay_t<return_type> > ;

			constexpr bool is_bool_return_type  
				= std::is_integral_v<return_type> &&
				std::is_same_v<no_const_no_ref_return_type, bool >;

			if constexpr (is_bool_return_type)
			{
				if (anything && (! atom_show_true_rezult) )
					return anything; 

				print(
					painter_command::green, "\n", ::dbj::LINE(),
					"\n- expression -> ", painter_command::text_color_reset, expression);
				if (show_type)
					print(
						painter_command::green, "\n\t- type-> ", painter_command::text_color_reset, typeid(anything).name()
					);

				if (false == anything) {
					print(
						painter_command::green, "\n\t- value -> ", painter_command::bright_red, anything, painter_command::text_color_reset
					);
				}
				else {
					print(
						painter_command::green, "\n\t- value -> ", painter_command::text_color_reset, anything
					);
				}
			}
			else {

				print(
					painter_command::green, "\n", ::dbj::LINE(),
					"\n- expression -> ", painter_command::text_color_reset, expression);
				if (show_type)
					print(
						painter_command::green, "\n\t- type-> ", painter_command::text_color_reset, typeid(anything).name()
					);

				print(
					painter_command::green, "\n\t- value -> ", painter_command::text_color_reset, anything
				);
			}

			return static_cast<return_type const &>(anything);
		};

		// #define DBJ_TEST_ATOM(x) dbj::testing::test_lambada( DBJ_EXPAND(x), [&] { return (x);}() ) 
#define DBJ_TEST_ATOM(x) ::dbj::testing::test_lambada( DBJ_EXPAND(x), (x) ) 
// same as above but does not show type
#define DBJ_ATOM_TEST(x) ::dbj::testing::test_lambada( DBJ_EXPAND(x), (x), false ) 

#define DBJ_ATOM_FLIP [[maybe_unused]] ::dbj::testing::flip_atom_show_true_rezult \
   flip_atom_show_true_rezult_instance{} 

#endif // DBJ_TEST_ATOM

	} // testing
} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"