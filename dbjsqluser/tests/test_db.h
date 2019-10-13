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
	return value is of this type : dbj::sql::db_valstat

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
	// notice the status_type ref. argument `status`
	// caller is laways reponisble to test it for the actual error state occurence
	// staus is for signaling the status  n ot just for the error events
	inline  sql::db_valstat demo_db()
		// no throwing from here
		noexcept
	{
		sql::db_valstat demo_db_status;

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
		auto db_instance_holder = 
			[](sql::dbj_sql_valstat& sqlite3_status) 
			-> std::reference_wrapper<sql::database>
		{
			// here we keep the single sql::database type instance
			static sql::database db(":memory:", sqlite3_status);
			return db;
		};

		static sql::db_valstat instance_ 
			= sql::db_initor(db_instance_holder, DEMO_DB_CREATE_SQL);
		return instance_ ;
	} // demo_db

	/*
	-------------------------------------------------------------------------------------
	*/
	inline sql::db_valstat rezults_db() noexcept
	{
		constexpr auto REZULTS_DB_CREATE_SQL =
			"DROP TABLE IF EXISTS rezults;"
			"CREATE TABLE rezults ("
			"rank INT PRIMARY KEY NOT NULL, "
			"iterations INT NOT NULL,"
			" size      INT NOT NULL,"
			" rezult    REAL,"
			" comment VARCHAR(255) NOT NULL);"
			"INSERT INTO rezults VALUES (  1,  1000,   2047,  458.92, 'home made unique_ptr buffer' );"
			"INSERT INTO rezults VALUES (  2,  1000,   2047,  459.30, 'home made shared_ptr buffer' );"
			"INSERT INTO rezults VALUES (  3,  1000,   2047,  462.75, 'std::vector<char>' );"
			"INSERT INTO rezults VALUES (  4,  1000,   2047,  467.14, 'shared_ptr<char[]>' );"
			"INSERT INTO rezults VALUES (  5,  1000,   2047,  469.39, 'unique_ptr<char[]>' );"
			"INSERT INTO rezults VALUES (  6,  1000,   2047,  483.10, 'std::string' );"
			"INSERT INTO rezults VALUES (  7,  1000,   4095,  915.35, 'home made shared_ptr buffer' );"
			"INSERT INTO rezults VALUES (  8,  1000,   4095,  923.54, 'home made unique_ptr buffer' );"
			"INSERT INTO rezults VALUES (  9,  1000,   4095,  925.53, 'shared_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 10,  1000,   4095,  927.42, 'unique_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 11,  1000,   4095,  935.58, 'std::vector<char>' );"
			"INSERT INTO rezults VALUES ( 12,  1000,   4095,  945.35, 'std::string' );"
			"INSERT INTO rezults VALUES ( 13,  1000,   8191, 1821.79, 'home made shared_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 14,  1000,   8191, 1823.44, 'home made unique_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 15,  1000,   8191, 1839.44, 'shared_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 16,  1000,   8191, 1848.01, 'unique_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 17,  1000,   8191, 1854.97, 'std::vector<char>' );"
			"INSERT INTO rezults VALUES ( 18,  1000,   8191, 1889.98, 'std::string' );"
			"INSERT INTO rezults VALUES ( 19,  1000,  16383, 3661.54, 'shared_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 20,  1000,  16383, 3688.72, 'home made shared_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 21,  1000,  16383, 3692.42, 'unique_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 22,  1000,  16383, 3693.39, 'home made unique_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 23,  1000,  16383, 3704.24, 'std::vector<char>' );"
			"INSERT INTO rezults VALUES ( 24,  1000,  16383, 3797.33, 'std::string' );"
			"INSERT INTO rezults VALUES ( 25,  1000,  32767, 7288.90, 'home made shared_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 26,  1000,  32767, 7327.26, 'shared_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 27,  1000,  32767, 7357.97, 'home made unique_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 28,  1000,  32767, 7363.80, 'unique_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 29,  1000,  32767, 7403.32, 'std::vector<char>' );"
			"INSERT INTO rezults VALUES ( 30,  1000,  32767, 7483.89, 'std::string' );"
			"INSERT INTO rezults VALUES ( 31,  1000,  65535, 14570.30, 'home made shared_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 32,  1000,  65535, 14666.05, 'shared_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 33,  1000,  65535, 14697.39, 'home made unique_ptr buffer' );"
			"INSERT INTO rezults VALUES ( 34,  1000,  65535, 14698.30, 'std::vector<char>' );"
			"INSERT INTO rezults VALUES ( 35,  1000,  65535, 14841.03, 'unique_ptr<char[]>' );"
			"INSERT INTO rezults VALUES ( 36,  1000,  65535, 15075.27, 'std::string' );";

		// this lambda is executed only on the first call 
		auto db_instance_holder =
			[](sql::dbj_sql_valstat& sqlite3_status)
			-> std::reference_wrapper<sql::database>
		{
			// here we keep the single sql::database type instance
			static sql::database db(":memory:", sqlite3_status);
			return db;
		};

		static sql::db_valstat instance_ = sql::db_initor(db_instance_holder, REZULTS_DB_CREATE_SQL);
		return instance_;

	} // demo_db

}; // dbj_sql_user nspace

#endif // !DBJ_TEST_DB_INC

