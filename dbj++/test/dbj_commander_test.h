#pragma once
/*
  commander testing is using testing mechanism so we move it here 
  so that commander itself does not depened on testing

  NOTE: we do not include testing headers into the dbj++.h
*/
DBJ_TEST_SPACE_OPEN(dbj_cmd_testing )
			/*
			Define comand id's first
			*/
			enum class CMD : unsigned {
				white = 0, red, green, blue, cyan,
				yellow, grey, bright_red, text_color_reset,
				nop = (unsigned)-1
			};

			DBJ_TEST_UNIT(dbj_commander) {

				using namespace dbj::cmd;
				/*
				observe CAREFULY!
				declaration of a function type, NOT function pointer type
				*/
				using cmd_fun_t = bool(void);
				/*
				BIG NOTE! [&] scoping operator '&' MUST exist
				*/
				auto paint_it_red = [&]() {
					dbj::console::print("\n", __FUNCSIG__, " called!");
					return true;
				};
				dbj::cmd::Commander<CMD, cmd_fun_t > commander_;
				try {
					commander_.reg(CMD::red, paint_it_red);
					bool r = commander_.execute(CMD::red);
				}
				catch (dbj::exception & x) {
					dbj::console::print(x);
				}
			}

DBJ_TEST_SPACE_CLOSE
