#pragma once

// does not require any include before
#include <mutex>

// Multi Threaded Build Switch
#define DBJ_AUTO_LOCK dbj::sync::lock_unlock __dbj_auto_lock__ 

namespace dbj {

	/// <summary>
	/// dbj synchronisation primitives
	/// see the licence blurb at eof
	/// </summary>
	namespace sync {

		/// <summary>
		/// automatic use of std::mutex 
		/// example of making a function
		/// thread safe
		/// <code>
		/// void safe_fun ( ) {
		/// dbj::sync::lock_unlock auto_lock_ ;
		/// or
		/// DBJ_AUTO_LOCK
		/// }
		/// </code>
		/// </summary>
		struct lock_unlock final {
			
			mutable std::mutex mux_;

			 lock_unlock() noexcept { mux_.lock(); }
			~lock_unlock() { mux_.unlock(); }
		};

		/// <summary>
		/// in presence of multiple threads
		/// guard value of type T
		/// example
		/// <code>
		/// static inline guardian<bool> signal_ ;
		/// </code>
		/// default bool is false
		/// <code>
		/// auto false_ = signal_.load() ;
		/// </code>
		/// switch to true
		/// <code>
		/// auto true_ = signal_.load(true) ;
		/// </code>
		/// </summary>
		template <typename T>
		struct __declspec(novtable) guardian final 
		{
			typedef T value_type;

			value_type & load() const noexcept  {
				lock_unlock locker_;
				return treasure_;
			}
			value_type & store(value_type new_value) const noexcept {
				lock_unlock locker_;
				return treasure_ = new_value;
			}
		private:
			mutable value_type treasure_ ;
		};


	} // sync
} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"