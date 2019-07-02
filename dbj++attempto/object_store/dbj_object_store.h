#pragma once
#include <string_view>
#include <any>
#include <map>

namespace dbj::odm 
{
	using namespace std;

	// keep string_view as the hash made from it
	// users provide object keys as strings
	struct object_key {
		using value_type = size_t; 
		//  typename hash<string_view>::result_type;

		object_key(string_view arg_) : key_(hash<string_view>()(arg_)) 
		{
		}

		bool operator == (const object_key& right) const noexcept {
			return this->key_ == right.key_;
		}

		bool operator < (const object_key& right) const noexcept {
			return this->key_ < right.key_;
		}

		const value_type& key() const noexcept { return key_; }

	private:
		value_type key_;
	};

	template<typename IMP>
	struct storage
	{
		struct transformer {
			transformer() = delete;
			transformer(const any& arg_) : val_(arg_) {}
			//throws exception on wrong type
			template<typename T>
			operator T () const { return any_cast<T>(val_); }
		private:
			any val_;
		};

		template <typename T>
		void store(string_view id, T value) {
			IMP::setDataImpl(id, any(value));
		}

		template <typename T>
		T load(string_view id) const {
			any res = IMP::getDataImpl(id);
			return any_cast<T>(res);
		}

		// simpler user code
		transformer operator () (string_view id) const {
			any res = IMP::getDataImpl(id);
			return { res };
		}
	};

	/*
	in memory object map
	must understand these two messages
	 void setDataImpl(string_view id, any const& value) ;
	 any getDataImpl(string_view id) ;
	*/
	struct storage_map 
	{
		inline static map< object_key, any > store_;

		static void setDataImpl(string_view id, any const& value)  
		{
			store_[object_key( id )] = value;
		}
		static any getDataImpl(string_view id) 
		{
			return store_[object_key(id)];
		}
	};

	DBJ_TEST_UNIT( object_storage_test ) {

		storage<storage_map> odb;

		odb.store("42", 42);
		int f42 = odb("42");
		DBJ_TEST_ATOM( f42 == 42);

		struct X {
			string name_;
		};
		X x{"dbj"};
		odb.store("dbj", x);
		X y =  odb("dbj") ;
		DBJ_TEST_ATOM (y.name_ == x.name_ );
	}

}