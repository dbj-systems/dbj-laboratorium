#pragma once
/*
#include "dbj++.h"
*/
#include <map>
#include <functional>
#include <initializer_list>
/* Command pattern mechanism */
#pragma region "commands"
namespace dbj {
	
	/* 
	https://stackoverflow.com/questions/31838611/why-use-invoke-helper-rather-than-just-call-functor
	*/
	template<typename F, typename ... Args>
	inline decltype(auto) call(F &&f, Args &&... args) {
		return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
	}

	namespace cmd {
		
		template < typename T>
		struct less {
			typedef typename less type;
			constexpr bool operator()(const T &lhs, const T &rhs) const
			{
				return lhs < rhs;
			}
		};

		/*  CMD_ENUM defined commands id's
		CMD_FUN is function type to execute them. Whatever satisfies std::is_function<CMD_FUN>
		CMD_COMPARATOR is function to compare the CMD_ENUM elements
		*/
		template<
			typename CMD_ENUM, 
			typename CMD_FUN, 
			typename CMD_COMPARATOR = typename dbj::cmd::less<CMD_ENUM>::type,
			typename std::enable_if_t<std::is_function_v<CMD_FUN>>* = nullptr 
		>
		  /* specialization for functions */
			class Commander
			{
			public:
				using comparator_type = CMD_COMPARATOR;
				using executor_type = std::function<CMD_FUN>;
				/*
				typename bellow is necessary since compiler at the moment of compilation of the template
				does not know that executor_type::result_type exists, it can not predict that
				std::function<CMD_FUN> can be compiled in future instantiations and thus can not predict
				that ::result_type will be possible to use
				this 'typename' was not required before C++11
				this is required only when compiling templates even wothout any instantions of them

				DBJ 2017-10-25
				*/
				typedef typename executor_type::result_type executor_return_type;
				using command_map_type = std::map<	CMD_ENUM, executor_type, comparator_type >;

				/*	commander's function */
				const executor_return_type execute(const CMD_ENUM & command) const
				{
					try {
						return dbj::call(command_map_.at(command));
					}
					catch (std::out_of_range &) {
						throw  ::dbj::exception( __FUNCSIG__  " Unknown command?");
					}
				}

				/* register a function by key given, do not replace	*/
				template< typename F>
				const Commander & reg (const CMD_ENUM & command_, F fun_ ) const 
				{
					auto rez [[maybe_unused]] = command_map_.try_emplace(command_, fun_);
					return (*this);
				}
				/*
				Prefer this method for commands registration
				considering: 
					Commander< int, string (void) >  cmdr ;
                this would be the registration call
				cmdr.register( {{ 1, [](){return "A";} },
								{ 2, [](){return "B";} });

				command_map_type::value_type is a pair of the map we use in here
				*/
				typedef typename  command_map_type::value_type map_value_type;

				const Commander & reg (
					std::initializer_list<map_value_type> initlist
				) // const
				{
					for ( map_value_type kvp : initlist) {
						/*	kvp is a pair { k,v } of the  map */
						this->reg(kvp.first, kvp.second);
					}
					return (*this);
				}

				template< typename F>
				const Commander & replace(const CMD_ENUM & command_, F fun_ ) const
				{
					try {   // found it, so replace
						// auto function_found [[maybe_unused]] = command_map_.at(command_);
						// command_map_[command_] = ( executor_type{ fun_ } );
						command_map_.at(command_) = executor_type{ fun_ };
					}
					catch (std::out_of_range &) {
						// not found, just ignore
						__noop;
					}
					return (*this);
				}

				Commander() noexcept { this->command_map_.clear(); }

				friend void swap(Commander& c1, Commander& c2) {
					  c1.command_map_.swap(c2.command_map_);
				}
			private:
				mutable command_map_type command_map_{};

				Commander& operator=(Commander);   // not assignable
				Commander(Commander &);				// not copyable
		};
	} // cmd
} // dbj
#pragma endregion "commands"

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"