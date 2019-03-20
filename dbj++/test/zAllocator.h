/*
This is mainly for "I found a better allocator!" people. 
Used in "dbj_buffer_testing.h". It yeilds (of course) slower results.

ps: Kevin, Jeffrey and David did a good job. For that time and that context. 
They clean the memory (::memset ) before delete, to make for a safe code. 
The primary objective here was safety not speed ...

dbj@dbj.org 2019 FEB
*/
/*
 * OWASP Enterprise Security API (ESAPI)
 *
 * This file is part of the Open Web Application Security Project (OWASP)
 * Enterprise Security API (ESAPI) project. For details, please see
 * <a href="http://www.owasp.org/index.php/ESAPI">http://www.owasp.org/index.php/ESAPI</a>.
 *
 * Copyright (c) 2011 - The OWASP Foundation
 *
 * @author Kevin Wall, kevin.w.wall@gmail.com
 * @author Jeffrey Walton, noloader@gmail.com
 * @author David Anderson, david.anderson@aspectsecurity.com
 */

#pragma once

// dbj removed -- #include "EsapiCommon.h"

#include <memory>
#include <cstring>
#include <limits>
#include <cassert>

 // For problems with allocator and its use, ask on comp.lang.c++
 // For problems with the libstd-c++ implementation and use of allocator, ask on the
 //   GCC mailing list (its easy to get off topic, use sparingly)
 // For problems with _Alloc_hider, see http://gcc.gnu.org/ml/gcc-help/2011-08/msg00199.html.

// dbj added
#define ESAPI_CXX_MSVC
#define ASSERT assert
// eof dbj added

#if defined(ESAPI_CXX_MSVC)
# pragma warning(push, 3)
# pragma warning(disable:4100)
#endif

namespace esapi
{
	template<typename T>
	class zallocator
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		// tame the optimizer
		static volatile void* g_dummy;

		template<typename U>
		struct rebind {
			typedef zallocator<U> other;
		};

		inline explicit zallocator() { }
		inline virtual ~zallocator() { }
		inline zallocator(zallocator const&) { }

		// Dropped explicit. See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=50118
		template<typename U>
		inline zallocator(zallocator<U> const&) { }

		// address
		inline pointer address(reference r) { return &r; }
		inline const_pointer address(const_reference r) { return &r; }

		// memory allocation
		inline pointer allocate(
			size_type cnt
			// dbj removed -- , typename std::allocator<void>::const_pointer = 0
		)
		{
			ASSERT(cnt > 0);
			ASSERT(cnt <= max_size());

			// Check for overflow/wrap. Standard containers only need to check for wrap in
			// vector::reserve. Other containers might check, but don't be fooled into complacency.
			// zero count allocation are legal, but near useless.
			if (cnt > max_size())
				throw std::bad_alloc();

			return reinterpret_cast<pointer>(::operator new(cnt * sizeof(T)));
		}

		inline void deallocate(pointer p, size_type cnt)
		{
			ASSERT(p);
			if (!p) return;

			// What to do on wrap here???
			ASSERT(cnt);
			ASSERT(!(cnt > max_size()));

			// Because 'p' is assigned to a static volatile pointer, the
			// optimizer currently does not optimize out the ::memset as dead
			// code. Set a breakpoint on the assignment to g_dummy in the
			// assembled code for verification.
			::memset(static_cast<void*>(p), 0x00, cnt * sizeof(T));
			g_dummy = p;
			::operator delete(p);
		}

		// size
		inline size_type max_size() const
		{
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		// construction/destruction
		inline void construct(pointer p, const T& t) { new(p) T(t); }
		inline void destroy(pointer p) { p->~T(); }

		inline bool operator==(zallocator const&) const { return true; }
		inline bool operator!=(zallocator const& a) const { return !operator==(a); }

//		// http://code.google.com/p/owasp-esapi-cplusplus/issues/detail?id=11
//#if defined(__GXX_EXPERIMENTAL_CXX0X__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
//		// Why is Apple defining __GXX_EXPERIMENTAL_CXX0X__
//# if !(defined(ESAPI_OS_APPLE) || defined(ESAPI_OS_ANDROID))
//		template<typename U, typename... Args>
//		void construct(U* p, Args&&... a)
//		{
//			::new ((void*)p) U(std::forward<Args>(a)...);
//		}
//
//		template<typename U>
//		void destroy(U* p)
//		{
//			ASSERT(p);
//			if (p)
//				p->~U();
//		}
//# endif
//#endif

	};

	// Storage and intialization
	template <class T>
	volatile void* zallocator<T>::g_dummy = nullptr;

#if defined(ESAPI_CXX_MSVC)
# pragma warning(default:4100)
# pragma warning(pop)
#endif

} // esapi
