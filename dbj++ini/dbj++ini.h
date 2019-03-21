#pragma once
//
// Started from: 
// https://github.com/benhoyt/inih
// Released under the New BSD license
// Go to the project home page for more info
//

#ifndef __INIREADER_H__
#define __INIREADER_H__

#ifdef _MSVC_LANG
#if _MSVC_LANG < 201402L
#error "C++17 required ..."
#endif
#endif

#include <map>
#include <string>
#include <memory>
// string_view requires C++17
#include <string_view>

#include <dbj++/core/dbj++core.h>

namespace dbj::ini 
{
	using std::string_view;

	// SEMantic VERsioning
	constexpr inline const auto MAJOR = 1;
	constexpr inline const auto MINOR = 0;
	constexpr inline const auto PATCH = 0;

	// dbj: we expose the interface and hide the implementation
	// there is usually only one ini per one process
	// we implement this interface with a singleton
	// since that is a singleton it keeps itself in the memory
	// and thus it keeps it's data in the memory too
	// the outcome is we do not need to use std::string for return values
	// to keep them strings
	struct ini_reader;

	// NOTE: to avoid std::string we use ::dbj::buf::yanb
	using smart_buffer = ::dbj::buf::yanb;

	struct ini_file_descriptor final {
		smart_buffer folder;
		smart_buffer basename;
		smart_buffer fullpath;
	};

	ini_file_descriptor ini_file();

	// this is the factory method that delivers the reference 
	// of the implementation object aka instance
	ini_reader const & ini_reader_instance( string_view /*ini_file_name*/ );

	// this is the interface to the ini_reader
	struct ini_reader
	{
		static smart_buffer default_inifile_path();
		// Return the result of ini_parse(), i.e., 0 on success, line number of
		// first error on parse error, or -1 on file open error.
		virtual int parse_error() const = 0;

		// Get a string value from INI file, returning default_value if not found.
		virtual smart_buffer get( string_view section, string_view name,
			string_view default_value) const = 0;

		// Get a string value from INI file, returning default_value if not found,
		// empty, or contains only whitespace.
		virtual smart_buffer get_string(string_view section, string_view name,
			string_view default_value) const = 0;

		// Get an integer (long) value from INI file, returning default_value if
		// not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
		virtual long get_integer(string_view section, string_view name, long default_value) const = 0;

		// Get a real (floating point double) value from INI file, returning
		// default_value if not found or not a valid floating point value
		// according to strtod().
		virtual double get_real(string_view section, string_view name, double default_value) const = 0;

		// Get a boolean value from INI file, returning default_value if not found or if
		// not a valid true/false value. Valid true values are "true", "yes", "on", "1",
		// and valid false values are "false", "no", "off", "0" (not case sensitive).
		virtual bool get_bool(string_view section, string_view name, bool default_value) const = 0;

		// Return true if a value exists with the given section and field names.
		virtual bool has_value(string_view section, string_view name) const = 0;
	};


} // inih
#endif  // __INIREADER_H__
