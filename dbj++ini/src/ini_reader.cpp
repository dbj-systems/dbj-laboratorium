// Read an INI file into easy-to-access name/value pairs.

// inih and ini_reader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih

//#include <string_view>
//#include <algorithm>
//#include <cctype>
//#include <cstdlib>
//#include <set>
#include <map>

#include "ini.h"
#include "../dbj++ini.h"

namespace dbj::ini 
{
	using std::string;
	using std::string_view;

	// dbj added

	using hash_key_type = long;

	/*
	[section]
	key = value

	Idea is we keep "section" + "=" + "key", as the key 
	in the map where we keep the value of the key, from the ini file

	Thus we do not need a more complex data structure that will map
	the simple hierachy from the ini file.

	Assumption is all "section=key" combinations will be unique
	*/

	using map_type = typename std::map<hash_key_type, buffer_type>;

	inline buffer_type::value_type const * 
		map_value_as_pointer ( map_type & the_map_, hash_key_type  const & the_key_ )
	{
		return the_map_[the_key_].data();
	}

	extern "C" {
		static constexpr hash_key_type djb2_hash(unsigned char* str)
		{
			unsigned long hash = 5381;
			int c = 0;

			while (c = *str++)
				hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

			return hash;
		}
	}

	// dbj
	static hash_key_type make_key(string_view section, string_view name) /*const*/
	{
		/*
		std::string '+' operator is notoriously slow, thus the appends bellow

		string key(section.data()); key.append("="); key.append(name.data());

		considering the above, is this not cleaner, faster + smaller in size
		*/
		buffer_type key = buffer::format("%s=%s", section.data(), name.data() );

		return djb2_hash((unsigned char *)key.data());
	}

	/*
	ini_reader is interface
	ini_reader_engine is its implementation
	*/
	struct ini_reader_engine final : ini_reader
	{

		ini_reader_engine (string_view filename)
		{
			_error = ini_parse(filename.data(), &ValueHandler, this);
		}

		int parse_error() const
		{
			return _error;
		}

		buffer_type get(string_view section, string_view name, string_view default_value) const
		{
			hash_key_type key = make_key(section, name);

				if (kv_map_.count(key) > 0) 
				{
					return kv_map_.at(key);
				}
				else {
					return buffer::make((default_value.data()));
				}
		}

		buffer_type get_string(string_view section, string_view name, string_view default_value) const
		{
			buffer_type str = get(section, name, "");
			return  (! str.data() ? buffer::make((default_value.data())) : str);
		}

		long get_integer(string_view section, string_view name, long default_value) const
		{
			buffer_type valstr = get(section, name, "");
			const char* value = valstr.data() ;
			char* end;
			// This parses "1234" (decimal) and also "0x4D2" (hex)
			long n = strtol(value, &end, 0);
			return end > value ? n : default_value;
		}

		double get_real(string_view section, string_view name, double default_value) const
		{
			buffer_type valstr = get(section, name, "");
			const char* value = valstr.data()  ;
			char* end;
			double n = strtod(value, &end);
			return end > value ? n : default_value;
		}

		/*
		when storing boolean values in the ini file

		true , yes, on , 1  -- are considered boolean true
		false , no, off, 0  -- are considered boolean false

		if none of the above is found, default_value will be retuned
		*/
		bool get_bool(string_view section, string_view name, bool default_value) const
		{
			auto eq = [](char const * left_, char const * right_) {
				return 0 == std::strcmp(left_, right_);
			};
			buffer_type valstr = get(section, name, "");
			char const * val = valstr.data() ;
			// Convert to lower case to make string comparisons case-insensitive
			// std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
			if (eq(val, "true") || eq(val, "yes") || eq(val, "on") || eq(val, "1"))
				return true;
			else if (eq(val, "false") || eq(val, "no") || eq(val, "off") || eq(val, "0"))
				return false;
			else
				return default_value;

		}

		bool has_value(string_view section, string_view name) const
		{
			hash_key_type key = make_key(section, name);
			return ( kv_map_.count(key) > 0 ? true : false );
		}

		// dbj made into static
		// this is sent to the C code as a callback
		// thus the void *, instead of ini_reader_engine &
		// this is where map is populated
		static int ValueHandler
		(
			void		* user, 
			const char	* section, 
			const char	* name,
			const char	* value
		)
		{
			_ASSERTE( user && section && name && value );

			ini_reader_engine* reader = static_cast<ini_reader_engine*>(user);
			hash_key_type key = make_key(section, name);
			/*
			dbj: ok this was original authors idea to handle multiple ini file
			entries under the same key, in the same section...

			[section]
			key=A
			key=B
			key=C

			key is: "section=key", value will be stored as "A\nB\nC"

			DBJ: why is '\n' list delimiter, is not clear, probably we can have 
			values with comas embeded?
			*/
			if (reader->kv_map_.count(key) > 0 ) 
			{
				constexpr static char list_separator = '\n';
				/*
				string new_val(  map_value_as_pointer(reader->kv_map_, key)  );
				new_val.append("\n");
				new_val.append(value);

				Again, observe how simpler it is not to use the string and string.append
				*/
				buffer_type new_val = buffer::format( "%s%c%s",
					reader->kv_map_[key].data() , list_separator, value
				);
				/*
				reader->kv_map_[key] = buffer::make  ((new_val.data()));

				since we do not use the string we do not need the above
				*/
				reader->kv_map_[key] = new_val ;
			}
			else {
				// just store the first key, value combination from ini file
				reader->kv_map_[key] = buffer::make ((value));
			}
			return 1;
		}

#ifdef _DEBUG
		// clear the map oly in DEBUG mode 
		// it can be long?
		~ini_reader_engine() {
			kv_map_.clear();
		}
#endif

	private:

		// dbj: here hide def. ctor and make this no copy, no move class
		ini_reader_engine() = delete;
		ini_reader_engine(ini_reader_engine const & ) = delete;
		ini_reader_engine & operator = (ini_reader_engine const & ) = delete;
		ini_reader_engine(ini_reader_engine && ) = delete;
		ini_reader_engine & operator = (ini_reader_engine &&) = delete;

		int _error{};

		map_type kv_map_{};

	}; // ini_reader_engine

	ini_reader const & ini_reader_instance(string_view ini_file_name)
	{
		static ini_reader_engine singleton_(ini_file_name);
		return singleton_;
	}

} // inih