/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/  
#pragma once

#ifndef DBJ_TEST_DB_INC
#define DBJ_TEST_DB_INC

#include "..\pch.h"

namespace dbj_sql_user {
	/*
	the dbj++sql namespace
	*/
	namespace sql = ::dbj::sql;

	/*
	return value is of this type : dbj::sql::status_and_location

	We use std::vector<char> as a char buffer
	std::unique_ptr<char[]> is the only other type we could use
	but it is only slightly faster and not that easy to use
	*/
	using buffer = typename dbj::nanolib::v_buffer;
	using buffer_type = typename buffer::buffer_type;

	// extern sqlite db file, to use

#include "..\db_file_path.inc"

	// in memory db for testing
	// we return an const reference to database singleton, made and hidden in here
	// notice the status_and_location ref. argument `status`
	// caller is laways reponisble to test it for the actual error state occurence
	// staus is for signaling the status  n ot just for the error events
	inline sql::database const& demo_db(sql::status_and_location & status)
		// no throwing from here
		noexcept
	{
		constexpr auto DEMO_DB_CREATE_SQL = "DROP TABLE IF EXISTS entries; "
			"CREATE TABLE entries ( Id int primary key, Name nvarchar(100) not null ); "
			"INSERT INTO entries values (1, 'LondonodnoL');"
			"INSERT INTO entries values (2, 'Glasgow');"
			"INSERT INTO entries values (3, 'CardifidraC');"
			"INSERT INTO entries values (4, 'Belgrade');"
			"INSERT INTO entries values (5, 'RomamoR');"
			"INSERT INTO entries values (6, 'Sarajevo');"
			"INSERT INTO entries values (7, 'Pregrevica');"
			"INSERT INTO entries values (8, 'ZemunumeZ');"
			"INSERT INTO entries values (9, 'Batajnica');";
		// this lambda is executed only on the first call 
		auto initor = [&]()
			-> const sql::database &
		{
			// sql::database constructor does not throw on error
			// it has the status to report 
			static sql::database db(":memory:", status);
			// if status is in the error state still return the db 
			// the caller will decide on the course of action
			if ( is_error(status)) return db;
			// create the database 
			// update the status
			status = db.exec( DEMO_DB_CREATE_SQL );
			return db;
		};
		// here we3 keep the single sql::database type instance
		static  sql::database const& instance_ = initor();
		return instance_;
	} // demo_db

}; // dbj_sql_user nspace

/* this is a cludge */
#define CHECK_RETURN if ( ::dbj::sql::is_error(status) ) { DBJ_FPRINTF(stdout, "\n\n%s\n\n",  ::dbj::sql::to_json( status).data() ); return; }


#endif // !DBJ_TEST_DB_INC

