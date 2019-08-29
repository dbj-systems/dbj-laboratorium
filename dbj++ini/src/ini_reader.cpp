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

#define DBJ_INCLUDE_STD_
#include <dbj++/dbj++required.h>
#include <dbj++/dbj++.h>

#include "ini.h"
#include "../dbj++ini.h"

namespace dbj::ini 
{

	/*
   ini file descriptor
   folder -- %programdata%\\dbj\\module_base_name
   basename -- module_base_name + ".log"
*/
	struct ini_file_descriptor final :
		::dbj::util::file_descriptor
	{
		virtual const char* suffix() const noexcept override { return ".ini"; }
	};

	static ini_file_descriptor ini_file()
	{
		ini_file_descriptor ifd_;
		::dbj::util::make_file_descriptor(ifd_);
		return ifd_;
	}

	buffer_type legal_full_path_ini( std::string_view inifile_base_name  ) {
		
		ini_file_descriptor ifd = ini_file();

		buffer_type buffy_ = buffer::make(BUFSIZ);

		auto rez_ = std::snprintf( buffy_.data(), buffy_.size(),
			"%s\\%s", ifd.folder.data() , inifile_base_name.data()
		);

		if (rez_ < 1) dbj_terror("std::snprintf() failed", __FILE__, __LINE__);

		return buffy_;
	}

	using std::string;
	using std::string_view;

	// dbj added

	using hash_key_type = long;

	// this is NOT a good idea probably?
	// buffer_type is vector<char> right now
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
		string key(section.data()); key.append("="); key.append(name.data());

		return djb2_hash((unsigned char *)key.c_str());
	}

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
					return buffer::make(/*_strdup*/(default_value.data()));
				}
		}

		buffer_type get_string(string_view section, string_view name, string_view default_value) const
		{
			buffer_type str = get(section, name, "");
			return  (! str.data() ? buffer::make(/*_strdup*/(default_value.data())) : str);
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
		static int ValueHandler(
			void* user, 
			const char* section, 
			const char* name,
			const char* value)
		{
			ini_reader_engine* reader = static_cast<ini_reader_engine*>(user);
			hash_key_type key = make_key(section, name);
			/*
			dbj: ok this was original authors idea to handle multiple ini file
			entries under the same key, in the same section...

			[section]
			key=A
			key=B
			key=C

			key is: "section=key", value made as "A\nB\nC"

			DBJ: what happens if value is empty string?
			*/
			if (reader->kv_map_.count(key) > 0 ) 
			{
				string new_val(  map_value_as_pointer(reader->kv_map_, key)  );
				new_val.append("\n");
				new_val.append(value);
				reader->kv_map_[key] = buffer::make  (/*_strdup*/(new_val.data()));
			}
			else {
				reader->kv_map_[key] = buffer::make (/*_strdup*/(value));
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

	/*static*/ buffer_type ini_reader::default_inifile_path()
	{
		auto[folder, basename, fullpath] = ini_file ();
		return fullpath;
	}		 
	

} // inih