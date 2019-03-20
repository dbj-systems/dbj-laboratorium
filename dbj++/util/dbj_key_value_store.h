#pragma once
#include <map>
#include "../core/dbj_synchro.h"
#include "dbj_string_util.h"

#if _HAS_CXX17
#else
#error "c++17 is required, did you forgot /std:c++17 , in project settings perhaps?"
#endif

/// <summary>
/// k/v storage using std unordered_map
/// key type is wstring (this is for speed on MSVC platforms)
/// one key can "hold" multiple values
/// </summary>
namespace dbj::storage {

	using namespace std;

	template <typename value_type, typename key_type = std::wstring >
	class __declspec(novtable)  keyvalue_storage final
	{
#ifdef _DBJ_MT_
		using lock_unlock = dbj::sync::lock_unlock;
#endif
	public:

		static_assert(
			dbj::is_std_string_v<key_type> , "dbj::storage requires key to be of std string type"
			);

		using	storage_type = std::multimap< key_type, value_type >;
		using   iterator = typename storage_type::iterator;
		using	value_vector = std::vector< value_type >;

		/// <summary>
		/// the aim is to be able to use instances of this class as values
		/// c++ compiler will generate all the things necessary
		/// and we will provide the "swap" method to be used for move semantics		 
		/// </summary>
		friend void swap(keyvalue_storage & left_, keyvalue_storage & right_)	noexcept
		{
#ifdef _DBJ_MT_
			lock_unlock padlock{};
#endif
			std::swap(left_.key_value_storage_, right_.key_value_storage_);
		}

		/// <summary>
		/// clear the storage held
		/// </summary>
		void clear() const {
#ifdef _DBJ_MT_
			lock_unlock padlock{};
#endif
			key_value_storage_.clear();
		}

		const size_t size() const noexcept {
#ifdef _DBJ_MT_
			lock_unlock padlock{};
#endif
			return this->key_value_storage_.size();
		}

		/// <summary>
		/// the argument of the callback is the 
		/// storage_type::value_type
		/// which in turn is pair 
		/// wstring is the key, T is the value
		/// NOTE: no exception caught in here
		/// </summary>
		template< typename F>
		const auto for_each(F callback_) const noexcept {
#ifdef _DBJ_MT_
			lock_unlock padlock{};
#endif
			return
				std::for_each(
					this->key_value_storage_.begin(),
					this->key_value_storage_.end(),
					callback_
				);
		}

		/// <summary>
		/// is the storage held empty?
		/// </summary>
		const bool empty() const noexcept {
#ifdef _DBJ_MT_
			lock_unlock padlock{};
#endif
			return this->key_value_storage_.size() < 1;
		}

		/// <summary>
		/// add new K/V pair
		/// return the iterator to them just inserted
		/// </summary>
		auto add( const key_type & key, const value_type & value) const
		{
#ifdef _DBJ_MT_
			lock_unlock padlock{};
#endif
			_ASSERTE(false == key.empty());
			return key_value_storage_.insert( std::make_pair(key,value) );
		}

		/// <summary>
		/// multi map is already sorted
		/// </summary>
		void sort() const noexcept = delete;

		/// <summary>
		/// if find_by_prefix is false the exact key match should  be performed
		/// if true all the keys matching are going into the result
		/// </summary>
		value_vector
			retrieve(
				key_type		query,
				bool			find_by_prefix = true,
				/* currently we ignore maxResults */
				unsigned		DBJ_MAYBE(maxResults) = ULONG_MAX
			) const
		{
#ifdef _DBJ_MT_
			lock_unlock padlock{};
#endif
			value_vector retval_{};
			if (key_value_storage_.size() < 1)
				return retval_;

			if (true == find_by_prefix) {
				retval_ = prefix_match_query(query);

				if (retval_.size() < 1)
					find_by_prefix = false;
			}

			if (false == find_by_prefix) {
				retval_ = exact_match_query(query);
			}
			return retval_;
		}

	private:
		/* do not use as public in this form */
		value_vector
			exact_match_query(
				// must be lower case!
				key_type query
			) const
		{
			/// <summary>
			/// general question is why is vector of values returned?
			/// vector of keys is lighter
			/// </summary>
			value_vector retvec{};

			auto range = key_value_storage_.equal_range(query);
			if (range.first == key_value_storage_.end()) return retvec;

			std::transform(
				range.first,
				range.second,
				std::back_inserter(retvec),
				[](typename storage_type::value_type element) { return element.second; }
			);
			return retvec;
		}

		/* do not use as public in this form */
		value_vector
			prefix_match_query(	key_type prefix_ ) const
		{
			/// <summary>
			/// general question is why is vector of values returned?
			/// vector of keys is lighter
			/// </summary>
			value_vector retvec{};
			typename storage_type::iterator && walker_ 
				= this->key_value_storage_.upper_bound(prefix_);

			while (walker_ != key_value_storage_.end())
			{
				// if prefix of the current key add its value to the result
				const key_type & walker_key = walker_->first;
				if (
					dbj::str::is_prefix( prefix_.c_str(), walker_key.c_str())
					) 
				{
					retvec.push_back(walker_->second);
					// advance the iterator 
					walker_++;
				}
				else {
					break;
				}
			}
			return retvec;
		}
		// and at last the storage itself
		mutable storage_type key_value_storage_{};

	}; // eof keyvalue_storage 

} //  namespace

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"