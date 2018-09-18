#pragma once

#include "pch.h"
/*
 key entity is command line argument ( dbj::cl::arg )

 cl example: exe_module -a v1 v2 v3 --b v4 /c v5
 space  := all adjacent spaces reduced to a single space
 legal key tag's  are space+'-', space+'--' or space+'/'
 key	:= key_tag + word  // no space
 value	:= key_tag + (<space> + word)[1..N]

*/
namespace dbj::samples {

	template<typename C> 
	inline C single_space(C && ) {
		using CH = dbj::tt::remove_cvref_t<C>;

		if constexpr (std::is_same_v< char, CH>) {
			return ' ';
		}
		else if constexpr (std::is_same_v< wchar_t, CH>) {
			return L' ';
		}
		else {
			throw __FUNCSIG__ " only char or wchar_t are served currently";
		}
	}

	// all adjacent spaces to a single space
	template< typename C , typename string = std::basic_string<C> >
	inline string
		compress_spaces(string && str )
	{
		auto two_spaces = [](char lhs, char rhs) { return (lhs == rhs) && (lhs == single_space<C>() ); };

		string::iterator new_end = std::unique(str.begin(), str.end(), two_spaces);
		str.erase(new_end, str.end());
		return str;
	}

	template< typename C >
	inline string
		compress_spaces(const C * cp_ )
	{
		return compress_spaces(std::basic_string<dbj::tt::remove_cvref_t<C>>(cp_));
	}

	namespace cl {

			using vals_vec = std::vector<std::wstring>;
			enum class key_tag_idx { hyphen = 0, double_hyphen = 1 , slash = 2 };

			constexpr inline const char * key_tag(key_tag_idx pos_ ) noexcept {
				constexpr const char * key_mark_[]{ "-", "--", "/" };
				return key_mark_[size_t(pos_)];
			}

			constexpr inline const bool is_key_tag( const char * ch_ ) noexcept {

				if ( dbj::str::is_prefix(ch_, key_tag(key_tag_idx::hyphen))) return true ;
				if ( dbj::str::is_prefix(ch_, key_tag(key_tag_idx::double_hyphen))) return true ;
				if ( dbj::str::is_prefix(ch_, key_tag(key_tag_idx::slash))) return true ;
				return false;
			}

			class arg final {
				
			struct key final {
				const static char * mark_;
				const static std::string tag_;

				static key parse_key( const char * text_ ) {
					std::string txt( text_ );
					txt = dbj::str::ltrim(txt);

					if (!is_key_tag(txt.c_str()))
						throw __FUNCSIG__ " argument appears not to be a valid key?";
				}
			};

			struct vals final {
				mutable std::vector<std::wstring> vals_;
			};

			key key_;
			vals parsed_vals_ ;
			vals default_vals_;
		public:

			arg(const char * key, vals_vec default_values) 
			{
				// is this legal key syntax
			}

		}; // arg 

	} // namespace cl::arg {

} // namespace dbj::samples 

DBJ_TEST_SPACE_OPEN( dbj_samples_cl )

DBJ_TEST_UNIT(dbj_samples_cl_arg ) {

	// app_cl is set of cl arg's
	// searchable by arg key

}

DBJ_TEST_SPACE_CLOSE



