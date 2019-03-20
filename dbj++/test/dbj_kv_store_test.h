#pragma once

// #include "../dbj_key_value_store.h"

#include <future>
#include <random>

DBJ_TEST_SPACE_OPEN(kv_storage_test)

using KVS =  ::dbj::storage::keyvalue_storage<int, std::string>;
// KVS has to be trivially constructible
inline dbj::sync::guardian<KVS> guarded_kvs_;

// https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
inline const int random_int ( int max_ ) {
	dbj::sync::lock_unlock __dbj_auto_lock__;
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(1 ,max_);
	return dis(gen);
}

inline size_t producer_( ) 
{
	dbj::sync::lock_unlock __dbj_auto_lock__;
	static const std::string key_base_ = "K";
	KVS & kvs = guarded_kvs_.load();
	const int next_bunch_size = random_int(64);
	for  ( int j = 0; j < next_bunch_size; j++)
	{
		kvs.add( 
			key_base_ + std::to_string(j), j
		);
	}
	return next_bunch_size;
}

DBJ_TEST_UNIT( dbj_kv_storage_test ) {

	auto handle_1 = std::async(std::launch::async, producer_);
	auto handle_2 = std::async(std::launch::async, producer_);

	size_t r1 = DBJ_TEST_ATOM( handle_1.get() );
	size_t r2 = DBJ_TEST_ATOM( handle_2.get() );

	const KVS & kvs = guarded_kvs_.load();
	bool DBJ_MAYBE(should_be_true) = (r1 + r2) == kvs.size();
	KVS::value_vector r3 = DBJ_TEST_ATOM( kvs.retrieve("K"));
	bool DBJ_MAYBE(should_be_true_too) = (r1 + r2) == r3.size();
}

DBJ_TEST_SPACE_CLOSE