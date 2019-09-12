#pragma once

#ifndef DBJ_ERROR_CONCEPT_INC
#define DBJ_ERROR_CONCEPT_INC

#include <memory>

/*
no exception but return values

return value type can have or not have, sqlite status code and std::errc code

*/

namespace dbj::sql
{
	using namespace std;

	enum class status_code : int
	{
		/*
			https://sqlite.org/c3ref/c_abort.html
			note:
			this must match with sqlite3 # defines
			*/
		sqlite_ok = SQLITE_OK,	/* successful result */
		sqlite_error = SQLITE_ERROR, /* generic error */
		sqlite_internal = SQLITE_INTERNAL, /* internal logic error in sqlite */
		sqlite_perm = SQLITE_PERM, /* access permission denied */
		sqlite_abort = SQLITE_ABORT, /* callback routine requested an abort */
		sqlite_busy = SQLITE_BUSY, /* the database file is locked */
		sqlite_locked = SQLITE_LOCKED, /* a table in the database is locked */
		sqlite_nomem = SQLITE_NOMEM, /* a malloc() failed */
		sqlite_readonly = SQLITE_READONLY, /* attempt to write a readonly database */
		sqlite_interrupt = SQLITE_INTERRUPT, /* operation terminated by sqlite= 3 _interrupt()*/
		sqlite_ioerr = SQLITE_IOERR, /* some kind of disk i/o error occurred */
		sqlite_corrupt = SQLITE_CORRUPT, /* the database disk image is malformed */
		sqlite_notfound = SQLITE_NOTFOUND, /* unknown opcode in sqlite= 3 _file_control() */
		sqlite_full = SQLITE_FULL, /* insertion failed because database is full */
		sqlite_cantopen = SQLITE_CANTOPEN, /* unable to open the database file */
		sqlite_protocol = SQLITE_PROTOCOL, /* database lock protocol error */
		sqlite_empty = SQLITE_EMPTY, /* internal use only */
		sqlite_schema = SQLITE_SCHEMA, /* the database schema changed */
		sqlite_toobig = SQLITE_TOOBIG, /* string or blob exceeds size limit */
		sqlite_constraint = SQLITE_CONSTRAINT, /* abort due to constraint violation */
		sqlite_mismatch = SQLITE_MISMATCH, /* data type mismatch */
		sqlite_misuse = SQLITE_MISUSE, /* library used incorrectly */
		sqlite_nolfs = SQLITE_NOLFS, /* uses os features not supported on host */
		sqlite_auth = SQLITE_AUTH, /* authorization denied */
		sqlite_format = SQLITE_FORMAT, /* not used */
		sqlite_range = SQLITE_RANGE, /* = 2 nd parameter to sqlite= 3 _bind out of range */
		sqlite_notadb = SQLITE_NOTADB, /* file opened that is not a database file */
		sqlite_notice = SQLITE_NOTICE, /* notifications from sqlite= 3 _log() */
		sqlite_warning = SQLITE_WARNING, /* warnings from sqlite= 3 _log() */
		sqlite_row = SQLITE_ROW, /* sqlite= 3 _step() has another row ready */
		sqlite_done = SQLITE_DONE        /* sqlite= 3 _step() has finished executing */
	};                          // dbj_status_code/*

			/*
		The sqlite3_errstr() interface returns the English-language text that
		describes the result code, as UTF-8. Memory to hold the error message
		string is managed internally and must not be freed by the application.

		function argument is int so that native sqlite3 return values can be
		used also
		*/
	inline buffer_type err_message_sql(int sqlite_return_value)  noexcept
	{

		status_code sc_ = (status_code)sqlite_return_value;
		if (const char* mp_ = ::sqlite::sqlite3_errstr(int(sc_)); mp_ != nullptr)
		{
			return buffer::make(mp_);
		}
		else
		{
			return buffer::make("Unknown SQLITE status code");
		}
	}

	// essentially transform POSIX error code
	// into std::error_code message
	inline buffer_type err_message_errc(std::errc posix_retval)  noexcept
	{
		// std::errc posix_retval = * std_errc;
		::std::error_code ec = std::make_error_code(posix_retval);
		return buffer::make(ec.message().c_str());
	}

	/*
	this is the sqlite3 logic
	not all error codes mean errors
	*/
	inline constexpr bool is_sqlite_error(status_code const& sc_) noexcept
	{
		switch (sc_) {
		case status_code::sqlite_ok:
		case status_code::sqlite_row:
		case status_code::sqlite_done:
			return false;
		default:
			return true;
		};
	}


	/*-----------------------------------------------------------------
	DBJ DB status type -- what is the status after the call

	contains any combination of sqlite3 status codes and std::errc codes
	std::nullopt signals the absence

	absence or presence is the state consumers/callers use
	see the bool operator
	*/
#if 0
	struct dbj_db_status_type final
	{
		using sqlite_status_type = optional< ::dbj::sql::status_code >;
		using std_errc_type = optional< std::errc >;

		//using buffer		= typename dbj::sql::v_buffer ;
		//using buffer_type	= typename buffer::buffer_type ;
		using location_type = optional< buffer_type >;

		sqlite_status_type	sqlite_status_id;
		std_errc_type		std_errc;
		location_type		location;

		dbj_db_status_type() { clear(); };

		dbj_db_status_type(
			sqlite_status_type sqlite_status_,
			location_type      location_ = nullopt,
			std_errc_type std_errc_ = nullopt
		)
			: sqlite_status_id(sqlite_status_), location(location_), std_errc(std_errc_)
		{
		}

		/*
		from sqlite3 code int's are coming
		*/
		dbj_db_status_type(
			int sqlite_status_,
			buffer_type  location_,
			std_errc_type std_errc_ = nullopt
		)
			: sqlite_status_id(::dbj::sql::status_code(sqlite_status_))
			, location(location_)
			, std_errc(std_errc_)
		{
		}
		/*
		if this operator returns true some error has happened
		 usage:

		dbj_db_status_type status_ ;
				.... here do something with the dbj++sqlite api ...
						if ( status_ ) { .. we have an error state ... } ;
		*/
		operator bool() const
		{
			if (sqlite_status_id != nullopt)
			{
				if (is_sqlite_error(*sqlite_status_id))
					return true;
			}

			if (std_errc != nullopt)  return true;

			return false; // not in an error state
		}
		/*
		some people can not understand using the above. they need
		more explicti name, so ...
		*/
		bool is_error() const noexcept
		{
			return this->operator bool();
		}

		void clear()
		{
			sqlite_status_id = nullopt;
			location = nullopt;
			std_errc = nullopt;
		}

		optional<buffer_type> sql_err_message()  const noexcept
		{
			// not set
			if (!sqlite_status_id) return nullopt;

			// err_message_sql has an int argument type
			// so that it can be used with native sqlite3 return values
			return err_message_sql((int)* sqlite_status_id);
		}

		optional<buffer_type> std_err_message()  const noexcept
		{
			if (!std_errc) return nullopt;
			return err_message_errc(*std_errc);
		}

		// name of this method follows the std:: de-facto rule
		char const* c_str() const {
			static auto buffy_ = buffer::make(BUFSIZ);
			buffy_ = dbj_db_status_type::to_buffer(*this);
			return buffy_.data();
		}

		static buffer_type to_buffer(dbj_db_status_type const& status_)
		{
			buffer_type buffy_ = buffer::make(0xFF + 0xFF); // 512 aka POSIX BUFSIZ

			auto sql_status_id = status_.sqlite_status_id;
			auto sql_status_message_ = status_.sql_err_message();

			auto std_errc_id = status_.std_errc;
			auto std_status_message_ = status_.std_err_message();

			int rez_ = std::snprintf(
				buffy_.data(), buffy_.size(),
				"sqlite3 id:%d, message:%s, std::errc id:%d, message:%s, location: %s",
				(sql_status_id ? int(*sql_status_id) : 0),
				(sql_status_message_ ? (*sql_status_message_).data() : "SQLITE_OK"),
				(std_errc_id ? int(*std_errc_id) : 0),
				(std_status_message_ ? (*std_status_message_).data() : "POSIX_OK"),
				(status_.location ? (*status_.location).data() : "Location unknown")
			);

			if (rez_ < 0)
				::dbj::nanolib::dbj_terror("std::snprintf() failed", __FILE__, __LINE__);

			return buffy_;
		}

	}; // dbj_db_status_type
#endif
	/*
	I do not throw anything.
	I use type/subtype little hierarchy because on of my key principles is to develop as little as posible
	The objective is to use std:: services,features,types and mechanims as much as possible.

	I do not use std::system_error becuase it is overcomplicated design and poor implementation.
	I see nothing wrong with type bellow. Provided they are not thrown.
	Throw/Try/Catch are proven to make slow and bloated apps.

	This concept also solves the problem of error categories. The problem is, one has to implement
	them error categories. In this scenario type-name of the subtype is the error category

	*/

	struct status_base_type;
	struct status_base_type
	{
		using pointer = typename std::shared_ptr<status_base_type>;
		virtual char const* category() const = 0;
		virtual char const* what() const = 0;
		virtual int code() const = 0;

		/*
		JSON format is universaly recognised. It gives us the ability to encode/decode the status message.
		Changing the json format in turn gives us less rigid design/implementation, as not everything is 
		coded properties. 
		*/
		char const* json() const noexcept {
			json_ = buffer::format("{ \"category\" : \"%s\" , \"code\" : %d, \"message\" : \"%s\" }",
				category(), code(), what()
			);
			return json_.data();
		}

		/*
		if this operator returns true some error has happened
		 usage:

		dbj_db_status_type status_ ;
				.... here do something with the dbj++sqlite api ...
						if ( status_ ) { .. we have an error state ... } ;
		*/
		virtual operator bool() const = 0;
		/*
		some people are not comfortable using the above. they need
		more explicti name, so ...
		*/
		bool is_error() const noexcept
		{
			return this->operator bool();
		}

	protected:
		mutable buffer_type data_;
		mutable buffer_type json_;
	};

	struct sqlite3_status : status_base_type
	{
		typename dbj::sql::status_code code_{};
		virtual char const* what() const {
			this->data_ = err_message_sql((int)code_);
			return data_.data();
		}
		virtual int code() const { return static_cast<int>(code_); }
		virtual char const* category() const { return "sqlite3_status"; }

		virtual operator bool() const noexcept
		{
			if (is_sqlite_error(code_))
				return true;
			return false; // not in an error state
		}
	};

	struct posix_status : status_base_type
	{
		typename std::errc code_{};
		virtual char const* what() const {
			this->data_ = err_message_errc(code_);
			return data_.data();
		}
		virtual int code() const { return static_cast<int>(code_); }
		virtual char const* category() const { return "posix_status"; }

		virtual operator bool() const noexcept
		{
			return  static_cast<int>(code_) > 0;
		}
	};

	/*
	location is simillar to but is not kind-of-a status
	*/
	struct location_status /*: status_base_type*/
	{
		// logicaly 'what' should be 'where' for location
		virtual char const* where() const {
			return location_.data();
		}

		char const* code() const	 { return code_.data(); }
		char const* category() const { return "status_location"; }

		char const* json() const noexcept {
			this->json_ = buffer::format("{ \"category\" : \"%s\" , \"code\" : %d, \"location\" : \"%s\" }",
				category(), code(), where()
			);
			return json_.data();
		}
	protected:
		/*
		 on an app level location code is line no. in a source file
		 On the system level, it can be anything else thet
		 identifies the distributed module/node/service
		 like URI
		 */
		buffer_type code_;
		buffer_type location_;
		mutable buffer_type json_;

		friend location_status make_status(unsigned long line_, char const* file_);
	};

	/*
	-----------------------------------------------------------------------------------
	*/
	inline status_base_type::pointer make_status(typename dbj::sql::status_code code_) {

		sqlite3_status* s3s = new sqlite3_status;
		s3s->code_ = code_;
		return status_base_type::pointer((status_base_type*)s3s);
	}

	inline status_base_type::pointer make_status(typename std::errc code_) {

		posix_status* pos = new posix_status;
		pos->code_ = code_;
		return status_base_type::pointer((status_base_type*)pos);
	}

	/* remember: location is simillar but is not kind-of status */
	inline location_status make_status(unsigned long line_, char const * file_) {
		/* 0 is not a good line number */
		_ASSERTE( line_ > 0UL );
		/*
		https://docs.microsoft.com/en-gb/cpp/c-runtime-library/reference/itoa-s-itow-s?view=vs-2019

		errno_t _ultoa_s(unsigned long value, char* buffer, size_t size, int radix);
		*/
		buffer_type line_str = buffer::make( _MAX_U64TOSTR_BASE2_COUNT );
		if (errno_t rez = _ultoa_s( line_, line_str.data(), line_str.size(), 10 ); rez == 0 ) {
			dbj::nanolib::dbj_terror("_ultoa_s() failed!", __FILE__, __LINE__ );
		}
		location_status loc{};
		loc.code_ = line_str;
		loc.location_ = buffer::make( file_ );
		return loc;
	}

/*
to code the location json when generating status-es we use this macro
*/

	using status_and_location = typename pair< status_base_type::pointer, location_status >;

	/* sqlite3 codes are of a int type */
	inline status_and_location make_sl( int code_ , unsigned long line_, char const * file_ ) 
	{
		return make_pair(
			dbj::sql::make_status(static_cast<dbj::sql::status_code>(code_)),
			dbj::sql::make_status(line_, file_)
		);
	}

	inline status_and_location make_sl( std::errc code_ , unsigned long line_, char const * file_ ) 
	{
		return make_pair(
			dbj::sql::make_status(static_cast<std::errc>(code_)),
			dbj::sql::make_status(line_, file_)
		);
	}

#define DBJ_STATUS(X_)  ::dbj::sql::make_sl( X_ , __LINE__, __FILE__ )
#define DBJ_SQLOK ::dbj::sql::make_sl( SQLITE_OK, __LINE__ , __FILE__ ) 
#define DBJ_ERROK ::dbj::sql::make_sl( static_cast<std::errc>(0), __LINE__ , __FILE__ ) 

	inline status_base_type::pointer status( status_and_location const & sl_ ) noexcept
	{
		_ASSERTE(sl_.first.get());
		return sl_.first;
	}

	inline location_status location( status_and_location const & sl_ ) noexcept
	{
		return sl_.second;
	}

	inline bool is_error(status_and_location const & sl_ ) {
		status_base_type::pointer stat_ptr = status(sl_);
		_ASSERTE(stat_ptr.get());
		return stat_ptr->is_error();
	}

	inline buffer_type to_json(status_and_location const& sl_) {
		status_base_type::pointer stat_ptr = status(sl_);
		_ASSERTE(stat_ptr.get());
		location_status loc_ = location(sl_);

		return buffer::format("%s %s", stat_ptr->json(), loc_.json());
	}



	/*
	consume the return in a structured declaration

	auto [value,status] = my_function () ;

	 	 if ( ! value ) ... error state ...
	*/
	template<typename T_>
	using return_type = pair<
			optional< T_ > ,
			optional< status_and_location >
		> ;

	template <typename T_, typename S_, typename L_ >
	inline 
		return_type<T_> 
		make_retval
	(  optional<T_> value_, optional< pair< S_, L_> > status_and_location_pair_ )
	{
		return make_pair( value_ , status_and_location_pair_);
	}

} // namespace dbj::sqlite

#endif // !DBJ_ERROR_CONCEPT_INC
