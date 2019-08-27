#pragma once 

 
#ifndef SYSTEM_ERROR2_HPP 
#define SYSTEM_ERROR2_HPP 
 
 
#ifndef SYSTEM_ERROR2_SYSTEM_ERROR_HPP 
#define SYSTEM_ERROR2_SYSTEM_ERROR_HPP 
 
 
#ifndef SYSTEM_ERROR2_POSIX_CODE_HPP 
#define SYSTEM_ERROR2_POSIX_CODE_HPP 
 
 
#ifndef SYSTEM_ERROR2_GENERIC_CODE_HPP 
#define SYSTEM_ERROR2_GENERIC_CODE_HPP 
 
 
#ifndef SYSTEM_ERROR2_STATUS_ERROR_HPP 
#define SYSTEM_ERROR2_STATUS_ERROR_HPP 
 
 
#ifndef SYSTEM_ERROR2_STATUS_CODE_HPP 
#define SYSTEM_ERROR2_STATUS_CODE_HPP 
 
 
#ifndef SYSTEM_ERROR2_STATUS_CODE_DOMAIN_HPP 
#define SYSTEM_ERROR2_STATUS_CODE_DOMAIN_HPP 
 
 
#ifndef SYSTEM_ERROR2_CONFIG_HPP 
#define SYSTEM_ERROR2_CONFIG_HPP 
 
#endif 

#include <atomic> 
#include <cassert> 
#include <cstddef> // for size_t 
#include <cstdlib> // for malloc 
#include <new> 
#include <type_traits> 
 
namespace system_error2 {  
 
/*! The main workhorse of the system_error2 library, can be typed (`status_code<DomainType>`), erased-immutable (`status_code<void>`) or erased-mutable (`status_code<erased<T>>`). 
 
Be careful of placing these into containers! Equality and inequality operators are 
*semantic* not exact. Therefore two distinct items will test true! To help prevent 
surprise on this, `operator<` and `std::hash<>` are NOT implemented in order to 
trap potential incorrectness. Define your own custom comparison functions for your 
container which perform exact comparisons. 
*/ 
 
template <class DomainType> class status_code; 
class _generic_code_domain; 
//! The generic code is a status code with the generic code domain, which is that of `errc` (POSIX). 
using generic_code = status_code<_generic_code_domain>; 
 
namespace detail 
{ 
	inline constexpr size_t cstrlen(const char* str) 
	{ 
		const char* end = nullptr; 
		for (end = str; *end != 0; ++end) // NOLINT 
			; 
		return end - str; 
	} 
 
	template <class T> struct status_code_sizer 
	{ 
		void* a; 
		T b; 
	}; 
	template <class To, class From> struct type_erasure_is_safe 
	{ 
		static constexpr bool value = std::is_trivially_copyable<From>::value // 
			&& (sizeof(status_code_sizer<From>) <= sizeof(status_code_sizer<To>)); 
	}; 
} // namespace detail 
 
/*! Abstract base class for a coding domain of a status code. 
*/ 
 
class status_code_domain 
{ 
	template <class DomainType> friend class status_code; 
 
public: 
	//! Type of the unique id for this domain. 
	using unique_id_type = unsigned long long; 
	/*! Thread safe reference to a message string. 
 
	Be aware that you cannot add payload to implementations of this class. 
	You get exactly the `void *[2]` array to keep state, this is usually 
	sufficient for a `std::shared_ptr<>` or a `std::string`. 
	*/ 
 
	class string_ref 
	{ 
	public: 
		//! The value type 
		using value_type = const char; 
		//! The size type 
		using size_type = size_t; 
		//! The pointer type 
		using pointer = const char*; 
		//! The const pointer type 
		using const_pointer = const char*; 
		//! The iterator type 
		using iterator = const char*; 
		//! The const iterator type 
		using const_iterator = const char*; 
 
	protected: 
		enum class _thunk_op 
		{ 
			copy, 
			move, 
			destruct 
		}; 
		using _thunk_spec = void (*)(string_ref* dest, const string_ref* src, _thunk_op op); 
		static void _static_string_thunk(string_ref* dest, const string_ref* src, _thunk_op /*unused*/) 
		{ 
			(void)dest; 
			(void)src; 
			assert(dest->_thunk == _static_string_thunk); 
			assert(src == nullptr || src->_thunk == _static_string_thunk); 
			// do nothing 
		} 
		struct _allocated_msg 
		{ 
			mutable std::atomic<unsigned> count; 
		}; 
		_allocated_msg*& _msg() { return reinterpret_cast<_allocated_msg * &>(this->_state[0]); } // NOLINT 
		const _allocated_msg* _msg() const { return reinterpret_cast<const _allocated_msg*>(this->_state[0]); } // NOLINT 
		static void _refcounted_string_thunk(string_ref* dest, const string_ref* src, _thunk_op op) 
		{ 
			(void)src; 
			assert(dest->_thunk == _refcounted_string_thunk); 
			assert(src == nullptr || src->_thunk == _refcounted_string_thunk); 
			switch (op) 
			{ 
			case _thunk_op::copy: 
			case _thunk_op::move: 
			{ 
				if (dest->_msg() != nullptr) 
				{ 
					auto count = dest->_msg()->count.fetch_add(1); 
					assert(count != 0); 
				} 
				return; 
			} 
			case _thunk_op::destruct: 
			{ 
				if (dest->_msg() != nullptr) 
				{ 
					auto count = dest->_msg()->count.fetch_sub(1); 
					if (count == 1) 
					{ 
						free((void*)dest->_begin); // NOLINT 
						delete dest->_msg(); 
					} 
				} 
			} 
			} 
		} 
 
		pointer _begin{}, _end{}; 
		void* _state[3]{}; // at least the size of a shared_ptr 
		_thunk_spec _thunk; 
 
		constexpr explicit string_ref(_thunk_spec thunk) 
			: _thunk(thunk) 
		{ 
		} 
 
	public: 
		//! Construct from a C string literal 
		constexpr explicit string_ref(const char* str, _thunk_spec thunk = _static_string_thunk) 
			: _begin(str) 
			, _end(str + detail::cstrlen(str)) // NOLINT 
			, _thunk(thunk) 
		{ 
		} 
		//! Copy construct the derived implementation. 
		string_ref(const string_ref& o) 
			: _begin(o._begin) 
			, _end(o._end) 
			, _state{ o._state[0], o._state[1], o._state[2] } 
			, _thunk(o._thunk) 
		{ 
			_thunk(this, &o, _thunk_op::copy); 
		} 
		//! Move construct the derived implementation. 
		string_ref(string_ref&& o) noexcept : _begin(o._begin), _end(o._end), _state{ o._state[0], o._state[1], o._state[2] }, _thunk(o._thunk) { _thunk(this, &o, _thunk_op::move); } 
		//! Copy assignment 
		string_ref& operator=(const string_ref& o) 
		{ 
			this->~string_ref(); 
			new(this) string_ref(o); 
			return *this; 
		} 
		//! Move assignment 
		string_ref& operator=(string_ref&& o) noexcept 
		{ 
			this->~string_ref(); 
			new(this) string_ref(static_cast<string_ref&&>(o)); 
			return *this; 
		} 
		//! Destruction 
		~string_ref() 
		{ 
			_thunk(this, nullptr, _thunk_op::destruct); 
			_begin = _end = nullptr; 
		} 
 
		//! Returns whether the reference is empty or not 
		bool empty() const noexcept { return _begin == _end; } 
		//! Returns the size of the string 
		size_type size() const { return _end - _begin; } 
		//! Returns a null terminated C string 
		value_type* c_str() const { return _begin; } 
		//! Returns the beginning of the string 
		iterator begin() { return _begin; } 
		//! Returns the beginning of the string 
		const_iterator begin() const { return _begin; } 
		//! Returns the beginning of the string 
		const_iterator cbegin() const { return _begin; } 
		//! Returns the end of the string 
		iterator end() { return _end; } 
		//! Returns the end of the string 
		const_iterator end() const { return _end; } 
		//! Returns the end of the string 
		const_iterator cend() const { return _end; } 
	}; 
 
private: 
	unique_id_type _id; 
 
protected: 
	/*! Use [https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h](https://www.random.org/cgi-bin/randbyte?nbytes=8&format=h) to get a random 64 bit id. 
 
	Do NOT make up your own value. Do NOT use zero. 
	*/ 
 
	constexpr explicit status_code_domain(unique_id_type id) noexcept : _id(id) {} 
	//! No public copying at type erased level 
	status_code_domain(const status_code_domain&) = default; 
	//! No public moving at type erased level 
	status_code_domain(status_code_domain&&) = default; 
	//! No public assignment at type erased level 
	status_code_domain& operator=(const status_code_domain&) = default; 
	//! No public assignment at type erased level 
	status_code_domain& operator=(status_code_domain&&) = default; 
	//! No public destruction at type erased level 
	~status_code_domain() = default; 
 
public: 
	//! True if the unique ids match. 
	constexpr bool operator==(const status_code_domain& o) const noexcept { return _id == o._id; } 
	//! True if the unique ids do not match. 
	constexpr bool operator!=(const status_code_domain& o) const noexcept { return _id != o._id; } 
	//! True if this unique is lower than the other's unique id. 
	constexpr bool operator<(const status_code_domain& o) const noexcept { return _id < o._id; } 
 
	//! Returns the unique id used to identify identical category instances. 
	constexpr unique_id_type id() const noexcept { return _id; } 
	//! Name of this category. 
	virtual string_ref name() const noexcept = 0; 
 
protected: 
	//! True if code means failure. 
	virtual bool _failure(const status_code<void>& code) const noexcept = 0; 
	//! True if code is (potentially non-transitively) equivalent to another code in another domain. 
	virtual bool _equivalent(const status_code<void>& code1, const status_code<void>& code2) const noexcept = 0; 
	//! Returns the generic code closest to this code, if any. 
	virtual generic_code _generic_code(const status_code<void>& code) const noexcept = 0; 
	//! Return a reference to a string textually representing a code. 
	virtual string_ref _message(const status_code<void>& code) const noexcept = 0; 
	//! Throw a code as a C++ exception. 
	virtual void _throw_exception(const status_code<void>& code) const = 0; 
}; 
 
} 
 
#endif 
#include <exception> // for std::exception 
#include <initializer_list> 
 
#if __cplusplus >= 201700 || _HAS_CXX17 
#include <utility> // for in_place 
 
namespace system_error2 {  
using in_place_t = std::in_place_t; 
using std::in_place; 
} 
 
#else 
 
namespace system_error2 {  
//! Aliases `std::in_place_t` if on C++ 17 or later, else defined locally. 
struct in_place_t 
{ 
	explicit in_place_t() = default; 
}; 
//! Aliases `std::in_place` if on C++ 17 or later, else defined locally. 
constexpr in_place_t in_place{}; 

} 

#endif 
 
namespace system_error2 {  
 
/*! A tag for an erased value type for `status_code<D>`. 
  Available only if `ErasedType` is an integral type. 
  */ 
	template <class ErasedType, // 
	typename std::enable_if<std::is_integral<ErasedType>::value, bool>::type = true> 
	struct erased 
{ 
	using value_type = ErasedType; 
}; 
 
namespace detail 
{ 
 
	template <class T> struct is_erased_status_code 
	{ 
		static constexpr bool value = false; 
	}; 
	template <class T> struct is_erased_status_code<status_code<erased<T>>> 
	{ 
		static constexpr bool value = true; 
	}; 
} 
 
/*! A type erased lightweight status code reflecting empty, success, or failure. 
Differs from `status_code<erased<>>` by being always available irrespective of 
the domain's value type, but cannot be copied, moved, nor destructed. Thus one 
always passes this around by const lvalue reference. 
*/ 
template <> class status_code<void> 
{ 
	template <class T> friend class status_code; 
 
public: 
	//! The type of the domain. 
	using domain_type = status_code_domain; 
	//! The type of the status code. 
	using value_type = void; 
	//! The type of a reference to a message string. 
	using string_ref = typename domain_type::string_ref; 
 
protected: 
	const status_code_domain* _domain{ nullptr }; 
 
protected: 
	//! No default construction at type erased level 
	status_code() = default; 
	//! No public copying at type erased level 
	status_code(const status_code&) = default; 
	//! No public moving at type erased level 
	status_code(status_code&&) = default; 
	//! No public assignment at type erased level 
	status_code& operator=(const status_code&) = default; 
	//! No public assignment at type erased level 
	status_code& operator=(status_code&&) = default; 
	//! No public destruction at type erased level 
	~status_code() = default; 
 
	//! Used to construct a non-empty type erased status code 
	constexpr explicit status_code(const status_code_domain* v) 
		: _domain(v) 
	{ 
	} 
 
public: 
	//! Return the status code domain. 
	constexpr const status_code_domain& domain() const noexcept { return *_domain; } 
	//! True if the status code is empty. 
	constexpr bool empty() const noexcept { return _domain == nullptr; } 
 
	//! Return a reference to a string textually representing a code. 
	string_ref message() const noexcept { return (_domain != nullptr) ? _domain->_message(*this) : string_ref("(empty)"); } 
	//! True if code means success. 
	bool success() const noexcept { return (_domain != nullptr) ? !_domain->_failure(*this) : false; } 
	//! True if code means failure. 
	bool failure() const noexcept { return (_domain != nullptr) ? _domain->_failure(*this) : false; } 
	/*! True if code is strictly (and potentially non-transitively) semantically equivalent to another code in another domain. 
	Note that usually non-semantic i.e. pure value comparison is used when the other status code has the same domain. 
	As `equivalent()` will try mapping to generic code, this usually captures when two codes have the same semantic 
	meaning in `equivalent()`. 
	*/ 
	template <class T> bool strictly_equivalent(const status_code<T>& o) const noexcept 
	{ 
		if (_domain && o._domain) 
			return _domain->_equivalent(*this, o); 
		// If we are both empty, we are equivalent 
		if (!_domain && !o._domain) 
			return true; 
		// Otherwise not equivalent 
		return false; 
	} 
	/*! True if code is equivalent, by any means, to another code in another domain (guaranteed transitive). 
	Firstly `strictly_equivalent()` is run in both directions. If neither succeeds, each domain is asked 
	for the equivalent generic code and those are compared. 
	*/ 
 
	template <class T> inline bool equivalent(const status_code<T>& o) const noexcept; 
	//! Throw a code as a C++ exception. 
	void throw_exception() const { _domain->_throw_exception(*this); } 
}; 
 
/*! A lightweight, typed, status code reflecting empty, success, or failure. 
This is the main workhorse of the system_error2 library. 
*/ 
template <class DomainType> class status_code : public status_code<void> 
{ 
	template <class T> friend class status_code; 
	using _base = status_code<void>; 
 
public: 
	//! The type of the domain. 
	using domain_type = DomainType; 
	//! The type of the status code. 
	using value_type = typename domain_type::value_type; 
	//! The type of a reference to a message string. 
	using string_ref = typename domain_type::string_ref; 
 
protected: 
	value_type _value{}; 
 
public: 
	//! Default construction to empty 
	status_code() = default; 
	//! Copy constructor 
	status_code(const status_code&) = default; 
	//! Move constructor 
	status_code(status_code&&) = default; // NOLINT 
	//! Copy assignment 
	status_code& operator=(const status_code&) = default; 
	//! Move assignment 
	status_code& operator=(status_code&&) = default; // NOLINT 
	~status_code() = default; 
 
	//! Implicit construction from any type where an ADL discovered `make_status_code(T &&)` returns a `status_code`. 
	template <class T, // 
		typename std::enable_if<!std::is_same<typename std::decay<T>::type, status_code>::value // 
		&& std::is_same<typename std::decay<decltype(make_status_code(std::declval<T>()))>::type, status_code>::value, 
		bool>::type = true> 
		constexpr status_code(T && v) noexcept(noexcept(make_status_code(std::declval<T>()))) // NOLINT 
		: status_code(make_status_code(static_cast<T&&>(v))) 
	{ 
	} 
	//! Explicit in-place construction. 
	template <class... Args> 
	constexpr explicit status_code(in_place_t /*unused */, Args&& ... args) noexcept(std::is_nothrow_constructible<value_type, Args&& ...>::value) 
		: _base(domain_type::get()) 
		, _value(static_cast<Args&&>(args)...) 
	{ 
	} 
	//! Explicit in-place construction from initialiser list. 
	template <class T, class... Args> 
	constexpr explicit status_code(in_place_t /*unused */, std::initializer_list<T> il, Args&& ... args) noexcept(std::is_nothrow_constructible<value_type, std::initializer_list<T>, Args&& ...>::value) 
		: _base(domain_type::get()) 
		, _value(il, static_cast<Args&&>(args)...) 
	{ 
	} 
	//! Explicit copy construction from a `value_type`. 
	constexpr explicit status_code(const value_type& v) noexcept(std::is_nothrow_copy_constructible<value_type>::value) 
		: _base(domain_type::get()) 
		, _value(v) 
	{ 
	} 
	//! Explicit move construction from a `value_type`. 
	constexpr explicit status_code(value_type&& v) noexcept(std::is_nothrow_copy_constructible<value_type>::value) 
		: _base(domain_type::get()) 
		, _value(static_cast<value_type&&>(v)) 
	{ 
	} 
	/*! Explicit construction from an erased status code. Available only if 
	`value_type` is trivially destructible and `sizeof(status_code) <= sizeof(status_code<erased<>>)`. 
	Does not check if domains are equal. 
	*/ 
 
	template <class ErasedType, // 
		typename std::enable_if<detail::type_erasure_is_safe<ErasedType, value_type>::value, bool>::type = true> 
		constexpr explicit status_code(const status_code<erased<ErasedType>> & v) 
		: status_code(reinterpret_cast<const value_type&>(v._value)) // NOLINT 
	{ 
#if __cplusplus >= 201400 
		assert(v.domain() == domain()); 
#endif 
	} 
 
	//! Assignment from a `value_type`. 
	constexpr status_code& operator=(const value_type& v) noexcept(std::is_nothrow_copy_assignable<value_type>::value) 
	{ 
		_value = v; 
		return *this; 
	} 
 
	// Replace the type erased implementations with type aware implementations for better codegen 
	//! Return the status code domain. 
	constexpr const domain_type& domain() const noexcept { return *static_cast<const domain_type*>(this->_domain); } 
	//! Return a reference to a string textually representing a code. 
	string_ref message() const noexcept { return this->_domain ? string_ref(domain()._message(*this)) : string_ref("(empty)"); } 
 
	//! Reset the code to empty. 
	constexpr void clear() { *this = status_code(); } 
 
#if __cplusplus >= 201400 || _MSC_VER >= 1910 /* VS2017 */ 
	//! Return a reference to the `value_type`. 
	constexpr value_type & value() & noexcept { return _value; } 
	//! Return a reference to the `value_type`. 
	constexpr value_type&& value() && noexcept { return _value; } 
#endif 
	//! Return a reference to the `value_type`. 
	constexpr const value_type& value() const & noexcept { return _value; } 
	//! Return a reference to the `value_type`. 
	constexpr const value_type&& value() const && noexcept { return _value; } 
}; 
 
/*! Type erased status_code, but copyable/movable/destructible unlike `status_code<void>`. Available 
only if `erased<>` is available, which is when the domain's type is trivially 
copyable, and if the size of the domain's typed error code is less than or equal to 
this erased error code. 
*/ 
template <class ErasedType> class status_code<erased<ErasedType>> : public status_code<void> 
{ 
	template <class T> friend class status_code; 
	using _base = status_code<void>; 
 
public: 
	//! The type of the domain (void, as it is erased). 
	using domain_type = void; 
	//! The type of the erased status code. 
	using value_type = ErasedType; 
	//! The type of a reference to a message string. 
	using string_ref = typename _base::string_ref; 
 
protected: 
	value_type _value{}; 
 
public: 
	//! Default construction to empty 
	status_code() = default; 
	//! Copy constructor 
	status_code(const status_code&) = default; 
	//! Move constructor 
	status_code(status_code&&) = default; // NOLINT 
	//! Copy assignment 
	status_code& operator=(const status_code&) = default; 
	//! Move assignment 
	status_code& operator=(status_code&&) = default; // NOLINT 
	~status_code() = default; 
 
	//! Implicit copy construction from any other status code if its type is trivially copyable and it would fit into our storage 
	template <class DomainType, // 
		typename std::enable_if<detail::type_erasure_is_safe<value_type, typename DomainType::value_type>::value, bool>::type = true> 
		constexpr status_code(const status_code<DomainType> & v) noexcept : _base(v), _value(reinterpret_cast<const value_type&>(v.value())) // NOLINT 
	{ 
	} 
	//! Return the erased `value_type` by value. 
	constexpr value_type value() const noexcept { return _value; } 
}; 
 
} 
 
#endif 
#include <exception> // for std::exception 
 
namespace system_error2 {  
 
/*! Exception type representing a thrown status_code 
  */ 
 
	template <class DomainType> class status_error : public std::exception 
{ 
	status_code<DomainType> _code; 
	typename DomainType::string_ref _msgref; 
 
public: 
	//! The type of the status domain 
	using domain_type = DomainType; 
	//! The type of the status code 
	using status_code_type = status_code<DomainType>; 
 
	//! Constructs an instance 
	explicit status_error(status_code<DomainType> code) 
		: _code(static_cast<status_code<DomainType>&&>(code)) 
		, _msgref(_code.message()) 
	{ 
	} 
 
	//! Return an explanatory string 
	virtual const char* what() const noexcept override { return _msgref.c_str(); } // NOLINT 
	//! Returns a reference to the code 
	const status_code_type& code() const & { return _code; } 
	//! Returns a reference to the code 
	status_code_type& code() & { return _code; } 
	//! Returns a reference to the code 
	const status_code_type&& code() const && { return _code; } 
	//! Returns a reference to the code 
	status_code_type&& code() && { return _code; } 
}; 
 
} 
 
#endif 
#include <cerrno> // for error constants 
 
namespace system_error2 {  
 
//! The generic error coding (POSIX) 
enum class errc : int 
{ 
	success = 0, 
	unknown = -1, 
 
	address_family_not_supported = EAFNOSUPPORT, 
	address_in_use = EADDRINUSE, 
	address_not_available = EADDRNOTAVAIL, 
	already_connected = EISCONN, 
	argument_list_too_long = E2BIG, 
	argument_out_of_domain = EDOM, 
	bad_address = EFAULT, 
	bad_file_descriptor = EBADF, 
	bad_message = EBADMSG, 
	broken_pipe = EPIPE, 
	connection_aborted = ECONNABORTED, 
	connection_already_in_progress = EALREADY, 
	connection_refused = ECONNREFUSED, 
	connection_reset = ECONNRESET, 
	cross_device_link = EXDEV, 
	destination_address_required = EDESTADDRREQ, 
	device_or_resource_busy = EBUSY, 
	directory_not_empty = ENOTEMPTY, 
	executable_format_error = ENOEXEC, 
	file_exists = EEXIST, 
	file_too_large = EFBIG, 
	filename_too_long = ENAMETOOLONG, 
	function_not_supported = ENOSYS, 
	host_unreachable = EHOSTUNREACH, 
	identifier_removed = EIDRM, 
	illegal_byte_sequence = EILSEQ, 
	inappropriate_io_control_operation = ENOTTY, 
	interrupted = EINTR, 
	invalid_argument = EINVAL, 
	invalid_seek = ESPIPE, 
	io_error = EIO, 
	is_a_directory = EISDIR, 
	message_size = EMSGSIZE, 
	network_down = ENETDOWN, 
	network_reset = ENETRESET, 
	network_unreachable = ENETUNREACH, 
	no_buffer_space = ENOBUFS, 
	no_child_process = ECHILD, 
	no_link = ENOLINK, 
	no_lock_available = ENOLCK, 
	no_message = ENOMSG, 
	no_protocol_option = ENOPROTOOPT, 
	no_space_on_device = ENOSPC, 
	no_stream_resources = ENOSR, 
	no_such_device_or_address = ENXIO, 
	no_such_device = ENODEV, 
	no_such_file_or_directory = ENOENT, 
	no_such_process = ESRCH, 
	not_a_directory = ENOTDIR, 
	not_a_socket = ENOTSOCK, 
	not_a_stream = ENOSTR, 
	not_connected = ENOTCONN, 
	not_enough_memory = ENOMEM, 
	not_supported = ENOTSUP, 
	operation_cancelled = ECANCELED, 
	operation_in_progress = EINPROGRESS, 
	operation_not_permitted = EPERM, 
	operation_not_supported = EOPNOTSUPP, 
	operation_would_block = EWOULDBLOCK, 
	owner_dead = EOWNERDEAD, 
	permission_denied = EACCES, 
	protcol_error = EPROTO, 
	protocol_not_supported = EPROTONOSUPPORT, 
	read_only_file_system = EROFS, 
	resource_deadlock_would_occur = EDEADLK, 
	resource_unavailable_try_again = EAGAIN, 
	result_out_of_range = ERANGE, 
	state_not_recoverable = ENOTRECOVERABLE, 
	stream_timeout = ETIME, 
	text_file_busy = ETXTBSY, 
	timed_out = ETIMEDOUT, 
	too_many_files_open_in_system = ENFILE, 
	too_many_files_open = EMFILE, 
	too_many_links = EMLINK, 
	too_many_symbolic_link_levels = ELOOP, 
	value_too_large = EOVERFLOW, 
	wrong_protocol_type = EPROTOTYPE 
}; 
 
namespace detail 
{ 
	struct generic_code_messages 
	{ 
		const char* msgs[256]; 
		constexpr size_t size() const { return sizeof(msgs) / sizeof(*msgs); } 
		constexpr const char* operator[](int i) const { return (i < 0 || i >= static_cast<int>(size()) || nullptr == msgs[i]) ? "unknown" : msgs[i]; } // NOLINT 
		constexpr generic_code_messages() 
			: msgs{} 
		{ 
			msgs[0] = "Success"; 
 
			msgs[EAFNOSUPPORT] = "Address family not supported by protocol"; 
			msgs[EADDRINUSE] = "Address already in use"; 
			msgs[EADDRNOTAVAIL] = "Cannot assign requested address"; 
			msgs[EISCONN] = "Transport endpoint is already connected"; 
			msgs[E2BIG] = "Argument list too long"; 
			msgs[EDOM] = "Numerical argument out of domain"; 
			msgs[EFAULT] = "Bad address"; 
			msgs[EBADF] = "Bad file descriptor"; 
			msgs[EBADMSG] = "Bad message"; 
			msgs[EPIPE] = "Broken pipe"; 
			msgs[ECONNABORTED] = "Software caused connection abort"; 
			msgs[EALREADY] = "Operation already in progress"; 
			msgs[ECONNREFUSED] = "Connection refused"; 
			msgs[ECONNRESET] = "Connection reset by peer"; 
			msgs[EXDEV] = "Invalid cross-device link"; 
			msgs[EDESTADDRREQ] = "Destination address required"; 
			msgs[EBUSY] = "Device or resource busy"; 
			msgs[ENOTEMPTY] = "Directory not empty"; 
			msgs[ENOEXEC] = "Exec format error"; 
			msgs[EEXIST] = "File exists"; 
			msgs[EFBIG] = "File too large"; 
			msgs[ENAMETOOLONG] = "File name too long"; 
			msgs[ENOSYS] = "Function not implemented"; 
			msgs[EHOSTUNREACH] = "No route to host"; 
			msgs[EIDRM] = "Identifier removed"; 
			msgs[EILSEQ] = "Invalid or incomplete multibyte or wide character"; 
			msgs[ENOTTY] = "Inappropriate ioctl for device"; 
			msgs[EINTR] = "Interrupted system call"; 
			msgs[EINVAL] = "Invalid argument"; 
			msgs[ESPIPE] = "Illegal seek"; 
			msgs[EIO] = "Input/output error"; 
			msgs[EISDIR] = "Is a directory"; 
			msgs[EMSGSIZE] = "Message too long"; 
			msgs[ENETDOWN] = "Network is down"; 
			msgs[ENETRESET] = "Network dropped connection on reset"; 
			msgs[ENETUNREACH] = "Network is unreachable"; 
			msgs[ENOBUFS] = "No buffer space available"; 
			msgs[ECHILD] = "No child processes"; 
			msgs[ENOLINK] = "Link has been severed"; 
			msgs[ENOLCK] = "No locks available"; 
			msgs[ENOMSG] = "No message of desired type"; 
			msgs[ENOPROTOOPT] = "Protocol not available"; 
			msgs[ENOSPC] = "No space left on device"; 
			msgs[ENOSR] = "Out of streams resources"; 
			msgs[ENXIO] = "No such device or address"; 
			msgs[ENODEV] = "No such device"; 
			msgs[ENOENT] = "No such file or directory"; 
			msgs[ESRCH] = "No such process"; 
			msgs[ENOTDIR] = "Not a directory"; 
			msgs[ENOTSOCK] = "Socket operation on non-socket"; 
			msgs[ENOSTR] = "Device not a stream"; 
			msgs[ENOTCONN] = "Transport endpoint is not connected"; 
			msgs[ENOMEM] = "Cannot allocate memory"; 
			msgs[ENOTSUP] = "Operation not supported"; 
			msgs[ECANCELED] = "Operation canceled"; 
			msgs[EINPROGRESS] = "Operation now in progress"; 
			msgs[EPERM] = "Operation not permitted"; 
			msgs[EOPNOTSUPP] = "Operation not supported"; 
			msgs[EWOULDBLOCK] = "Resource temporarily unavailable"; 
			msgs[EOWNERDEAD] = "Owner died"; 
			msgs[EACCES] = "Permission denied"; 
			msgs[EPROTO] = "Protocol error"; 
			msgs[EPROTONOSUPPORT] = "Protocol not supported"; 
			msgs[EROFS] = "Read-only file system"; 
			msgs[EDEADLK] = "Resource deadlock avoided"; 
			msgs[EAGAIN] = "Resource temporarily unavailable"; 
			msgs[ERANGE] = "Numerical result out of range"; 
			msgs[ENOTRECOVERABLE] = "State not recoverable"; 
			msgs[ETIME] = "Timer expired"; 
			msgs[ETXTBSY] = "Text file busy"; 
			msgs[ETIMEDOUT] = "Connection timed out"; 
			msgs[ENFILE] = "Too many open files in system"; 
			msgs[EMFILE] = "Too many open files"; 
			msgs[EMLINK] = "Too many links"; 
			msgs[ELOOP] = "Too many levels of symbolic links"; 
			msgs[EOVERFLOW] = "Value too large for defined data type"; 
			msgs[EPROTOTYPE] = "Protocol wrong type for socket"; 
		} 
	}; 
} // namespace detail 
 
/*! The implementation of the domain for generic status codes, those mapped by `errc` (POSIX). 
*/ 
 
class _generic_code_domain : public status_code_domain 
{ 
	template <class> friend class status_code; 
	using _base = status_code_domain; 
 
public: 
	//! The value type of the generic code, which is an `errc` as per POSIX. 
	using value_type = errc; 
	using string_ref = _base::string_ref; 
 
public: 
	//! Default constructor 
	constexpr _generic_code_domain() noexcept : _base(0x746d6354f4f733e9) {} 
	_generic_code_domain(const _generic_code_domain&) = default; 
	_generic_code_domain(_generic_code_domain&&) = default; 
	_generic_code_domain& operator=(const _generic_code_domain&) = default; 
	_generic_code_domain& operator=(_generic_code_domain&&) = default; 
	~_generic_code_domain() = default; 
 
	//! Constexpr singleton getter. Returns the address of the constexpr generic_code_domain variable. 
	static inline constexpr const _generic_code_domain* get(); 
 
	virtual _base::string_ref name() const noexcept override final { return string_ref("generic domain"); } // NOLINT 
protected: 
	virtual bool _failure(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		return static_cast<const generic_code&>(code).value() != errc::success; // NOLINT 
	} 
	virtual bool _equivalent(const status_code<void>& code1, const status_code<void>& code2) const noexcept override final // NOLINT 
	{ 
		assert(code1.domain() == *this); 
		const auto& c1 = static_cast<const generic_code&>(code1); // NOLINT 
		if (code2.domain() == *this) 
		{ 
			const auto& c2 = static_cast<const generic_code&>(code2); // NOLINT 
			return c1.value() == c2.value(); 
		} 
		return false; 
	} 
	virtual generic_code _generic_code(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		return static_cast<const generic_code&>(code); // NOLINT 
	} 
	virtual _base::string_ref _message(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const generic_code&>(code); // NOLINT 
		static constexpr detail::generic_code_messages msgs; 
		return string_ref(msgs[static_cast<int>(c.value())]); 
	} 
	virtual void _throw_exception(const status_code<void>& code) const override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const generic_code&>(code); // NOLINT 
		throw status_error<_generic_code_domain>(c); 
	} 
}; 
//! A constexpr source variable for the generic code domain, which is that of `errc` (POSIX). Returned by `_generic_code_domain::get()`. 
constexpr _generic_code_domain generic_code_domain; 
inline constexpr const _generic_code_domain* _generic_code_domain::get() 
{ 
	return &generic_code_domain; 
} 
// Enable implicit construction of generic_code from errc 
constexpr inline generic_code make_status_code(errc c) noexcept 
{ 
	return generic_code(in_place, c); 
} 
/*************************************************************************************************************/ 
template <class T> inline bool status_code<void>::equivalent(const status_code<T>& o) const noexcept 
{ 
	if (_domain && o._domain) 
	{ 
		if (_domain->_equivalent(*this, o)) 
		{ 
			return true; 
		} 
		if (o._domain->_equivalent(o, *this)) 
		{ 
			return true; 
		} 
		generic_code c1 = o._domain->_generic_code(o); 
		if (c1.value() != errc::unknown && _domain->_equivalent(*this, c1)) 
		{ 
			return true; 
		} 
		generic_code c2 = _domain->_generic_code(*this); 
		if (c2.value() != errc::unknown && o._domain->_equivalent(o, c2)) 
		{ 
			return true; 
		} 
	} 
	// If we are both empty, we are equivalent 
	if (!_domain && !o._domain) 
	{ 
		return true; 
	} 
	// Otherwise not equivalent 
	return false; 
} 
//! True if the status code's are semantically equal via `equivalent()`. 
template <class DomainType1, class DomainType2> inline bool operator==(const status_code<DomainType1>& a, const status_code<DomainType2>& b) noexcept 
{ 
	return a.equivalent(b); 
} 
//! True if the status code's are not semantically equal via `equivalent()`. 
template <class DomainType1, class DomainType2> inline bool operator!=(const status_code<DomainType1>& a, const status_code<DomainType2>& b) noexcept 
{ 
	return !a.equivalent(b); 
} 
//! True if the status code's are semantically equal via `equivalent()` to the generic code. 
template <class DomainType1> inline bool operator==(const status_code<DomainType1>& a, errc b) noexcept 
{ 
	return a.equivalent(generic_code(b)); 
} 
//! True if the status code's are semantically equal via `equivalent()` to the generic code. 
template <class DomainType1> inline bool operator==(errc a, const status_code<DomainType1>& b) noexcept 
{ 
	return b.equivalent(generic_code(a)); 
} 
//! True if the status code's are not semantically equal via `equivalent()` to the generic code. 
template <class DomainType1> inline bool operator!=(const status_code<DomainType1>& a, errc b) noexcept 
{ 
	return !a.equivalent(generic_code(b)); 
} 
//! True if the status code's are not semantically equal via `equivalent()` to the generic code. 
template <class DomainType1> inline bool operator!=(errc a, const status_code<DomainType1>& b) noexcept 
{ 
	return !b.equivalent(generic_code(a)); 
} 
 
} 
 
#endif 
#include <cstring> // for strchr and strerror_r 
 
namespace system_error2 {  
 
class _posix_code_domain; 
//! A POSIX error code, those returned by `errno`. 
using posix_code = status_code<_posix_code_domain>; 
 
/*! The implementation of the domain for POSIX error codes, those returned by `errno`. 
*/ 
 
class _posix_code_domain : public status_code_domain 
{ 
	template <class DomainType> friend class status_code; 
	using _base = status_code_domain; 
 
public: 
	//! The value type of the POSIX code, which is an `int` 
	using value_type = int; 
	//! Thread safe reference to a message string fetched by `strerror_r()` 
	class string_ref : public _base::string_ref 
	{ 
	public: 
		explicit string_ref(const _base::string_ref& o) 
			: _base::string_ref(o) 
		{ 
		} 
		explicit string_ref(_base::string_ref&& o) 
			: _base::string_ref(std::move(o)) 
		{ 
		} 
		constexpr string_ref() 
			: _base::string_ref(_base::string_ref::_refcounted_string_thunk) 
		{ 
		} 
		constexpr explicit string_ref(const char* str) 
			: _base::string_ref(str, _base::string_ref::_refcounted_string_thunk) 
		{ 
		} 
		string_ref(const string_ref&) = default; 
		string_ref(string_ref&&) = default; 
		string_ref& operator=(const string_ref&) = default; 
		string_ref& operator=(string_ref&&) = default; 
		~string_ref() = default; 
		//! Construct from a POSIX error code 
		explicit string_ref(int c) 
			: _base::string_ref(_base::string_ref::_refcounted_string_thunk) 
		{ 
			char buffer[1024] = ""; 
#ifdef _WIN32 
			strerror_s(buffer, sizeof(buffer), c); 
#elif defined(__linux__) 
			char* s = strerror_r(c, buffer, sizeof(buffer)); 
			if (s != nullptr) 
			{ 
				strncpy(buffer, s, sizeof(buffer)); 
				buffer[1023] = 0; 
			} 
#else 
			strerror_r(c, buffer, sizeof(buffer)); 
#endif 
			size_t length = strlen(buffer); 
			auto* p = static_cast<char*>(malloc(length + 1)); // NOLINT 
			if (p == nullptr) 
			{ 
				goto failure; 
			} 
			memcpy(p, buffer, length + 1); 
			this->_begin = p; 
			this->_end = p + length; // NOLINT 
			_msg() = static_cast<_allocated_msg*>(calloc(1, sizeof(_allocated_msg))); // NOLINT 
			if (_msg() == nullptr) 
			{ 
				free((void*)this->_begin); // NOLINT 
				goto failure; 
			} 
			++_msg()->count; 
			return; 
		failure: 
			_msg() = nullptr; // disabled 
			this->_begin = "failed to get message from system"; 
			this->_end = strchr(this->_begin, 0); 
		} 
	}; 
 
public: 
	//! Default constructor 
	constexpr _posix_code_domain() noexcept : _base(0xa59a56fe5f310933) {} 
	_posix_code_domain(const _posix_code_domain&) = default; 
	_posix_code_domain(_posix_code_domain&&) = default; 
	_posix_code_domain& operator=(const _posix_code_domain&) = default; 
	_posix_code_domain& operator=(_posix_code_domain&&) = default; 
	~_posix_code_domain() = default; 
 
	//! Constexpr singleton getter. Returns the address of the constexpr posix_code_domain variable. 
	static inline constexpr const _posix_code_domain* get(); 
 
	virtual _base::string_ref name() const noexcept override final { return _base::string_ref("posix domain"); } // NOLINT 
protected: 
	virtual bool _failure(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		return static_cast<const posix_code&>(code).value() != 0; // NOLINT 
	} 
	virtual bool _equivalent(const status_code<void>& code1, const status_code<void>& code2) const noexcept override final // NOLINT 
	{ 
		assert(code1.domain() == *this); 
		const auto& c1 = static_cast<const posix_code&>(code1); // NOLINT 
		if (code2.domain() == *this) 
		{ 
			const auto& c2 = static_cast<const posix_code&>(code2); // NOLINT 
			return c1.value() == c2.value(); 
		} 
		if (code2.domain() == generic_code_domain) 
		{ 
			const auto& c2 = static_cast<const generic_code&>(code2); // NOLINT 
			if (static_cast<int>(c2.value()) == c1.value()) 
			{ 
				return true; 
			} 
		} 
		return false; 
	} 
	virtual generic_code _generic_code(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const posix_code&>(code); // NOLINT 
		return generic_code(static_cast<errc>(c.value())); 
	} 
	virtual _base::string_ref _message(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const posix_code&>(code); // NOLINT 
		return string_ref(c.value()); 
	} 
	virtual void _throw_exception(const status_code<void>& code) const override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const posix_code&>(code); // NOLINT 
		throw status_error<_posix_code_domain>(c); 
	} 
}; 
//! A constexpr source variable for the POSIX code domain, which is that of `errno`. Returned by `_posix_code_domain::get()`. 
constexpr _posix_code_domain posix_code_domain; 
inline constexpr const _posix_code_domain* _posix_code_domain::get() 
{ 
	return &posix_code_domain; 
} 
 
} 
 
#endif 
#if defined(_WIN32) || 0 
 
 
#ifndef SYSTEM_ERROR2_NT_CODE_HPP 
#define SYSTEM_ERROR2_NT_CODE_HPP 
 
#if !defined(_WIN32) && !0 
#error This file should only be included on Windows 
#endif 
 
 
#ifndef SYSTEM_ERROR2_WIN32_CODE_HPP 
#define SYSTEM_ERROR2_WIN32_CODE_HPP 
 
#if !defined(_WIN32) && !0 
#error This file should only be included on Windows 
#endif 
 
#include <cstring> // for strchr 
 
namespace system_error2 {  
 
//! \exclude 
namespace win32 
{ 
	// A Win32 DWORD 
	using DWORD = unsigned long; 
	// Used to retrieve the current Win32 error code 
	extern "C" DWORD __stdcall GetLastError(); 
	// Used to retrieve a locale-specific message string for some error code 
	extern "C" DWORD __stdcall FormatMessageW(DWORD dwFlags, const void* lpSource, DWORD dwMessageId, DWORD dwLanguageId, wchar_t* lpBuffer, DWORD nSize, void /*va_list*/* Arguments); 
	// Converts UTF-16 message string to UTF-8 
	extern "C" int __stdcall WideCharToMultiByte(unsigned int CodePage, DWORD dwFlags, const wchar_t* lpWideCharStr, int cchWideChar, char* lpMultiByteStr, int cbMultiByte, const char* lpDefaultChar, int* lpUsedDefaultChar); 
#pragma comment(lib, "kernel32.lib") 
} // namespace win32 
 
class _win32_code_domain; 
class _com_code_domain; 
//! (Windows only) A Win32 error code, those returned by `GetLastError()`. 
using win32_code = status_code<_win32_code_domain>; 
 
/*! (Windows only) The implementation of the domain for Win32 error codes, those returned by `GetLastError()`. 
*/ 
 
class _win32_code_domain : public status_code_domain 
{ 
	template <class DomainType> friend class status_code; 
	friend class _com_code_domain; 
	using _base = status_code_domain; 
	static int _win32_code_to_errno(win32::DWORD c) 
	{ 
		switch (c) 
		{ 
		case 0: 
			return 0; 
		case 0x1: return ENOSYS; 
		case 0x2: return ENOENT; 
		case 0x3: return ENOENT; 
		case 0x4: return EMFILE; 
		case 0x5: return EACCES; 
		case 0x6: return EINVAL; 
		case 0x8: return ENOMEM; 
		case 0xc: return EACCES; 
		case 0xe: return ENOMEM; 
		case 0xf: return ENODEV; 
		case 0x10: return EACCES; 
		case 0x11: return EXDEV; 
		case 0x13: return EACCES; 
		case 0x14: return ENODEV; 
		case 0x15: return EAGAIN; 
		case 0x19: return EIO; 
		case 0x1d: return EIO; 
		case 0x1e: return EIO; 
		case 0x20: return EACCES; 
		case 0x21: return ENOLCK; 
		case 0x27: return ENOSPC; 
		case 0x37: return ENODEV; 
		case 0x50: return EEXIST; 
		case 0x52: return EACCES; 
		case 0x57: return EINVAL; 
		case 0x6e: return EIO; 
		case 0x6f: return ENAMETOOLONG; 
		case 0x70: return ENOSPC; 
		case 0x7b: return EINVAL; 
		case 0x83: return EINVAL; 
		case 0x8e: return EBUSY; 
		case 0x91: return ENOTEMPTY; 
		case 0xaa: return EBUSY; 
		case 0xb7: return EEXIST; 
		case 0xd4: return ENOLCK; 
		case 0x10b: return EINVAL; 
		case 0x3e3: return ECANCELED; 
		case 0x3e6: return EACCES; 
		case 0x3f3: return EIO; 
		case 0x3f4: return EIO; 
		case 0x3f5: return EIO; 
		case 0x4d5: return EAGAIN; 
		case 0x961: return EBUSY; 
		case 0x964: return EBUSY; 
		case 0x2714: return EINTR; 
		case 0x2719: return EBADF; 
		case 0x271d: return EACCES; 
		case 0x271e: return EFAULT; 
		case 0x2726: return EINVAL; 
		case 0x2728: return EMFILE; 
		case 0x2733: return EWOULDBLOCK; 
		case 0x2734: return EINPROGRESS; 
		case 0x2735: return EALREADY; 
		case 0x2736: return ENOTSOCK; 
		case 0x2737: return EDESTADDRREQ; 
		case 0x2738: return EMSGSIZE; 
		case 0x2739: return EPROTOTYPE; 
		case 0x273a: return ENOPROTOOPT; 
		case 0x273b: return EPROTONOSUPPORT; 
		case 0x273d: return EOPNOTSUPP; 
		case 0x273f: return EAFNOSUPPORT; 
		case 0x2740: return EADDRINUSE; 
		case 0x2741: return EADDRNOTAVAIL; 
		case 0x2742: return ENETDOWN; 
		case 0x2743: return ENETUNREACH; 
		case 0x2744: return ENETRESET; 
		case 0x2745: return ECONNABORTED; 
		case 0x2746: return ECONNRESET; 
		case 0x2747: return ENOBUFS; 
		case 0x2748: return EISCONN; 
		case 0x2749: return ENOTCONN; 
		case 0x274c: return ETIMEDOUT; 
		case 0x274d: return ECONNREFUSED; 
		case 0x274f: return ENAMETOOLONG; 
		case 0x2751: return EHOSTUNREACH; 
		} 
		return -1; 
	} 
 
public: 
	//! The value type of the win32 code, which is a `win32::DWORD` 
	using value_type = win32::DWORD; 
	//! Thread safe reference to a message string fetched by `FormatMessage()` 
	class string_ref : public _base::string_ref 
	{ 
	public: 
		explicit string_ref(const _base::string_ref& o) 
			: _base::string_ref(o) 
		{ 
		} 
		explicit string_ref(_base::string_ref&& o) 
			: _base::string_ref(std::move(o)) 
		{ 
		} 
		constexpr string_ref() 
			: _base::string_ref(_base::string_ref::_refcounted_string_thunk) 
		{ 
		} 
		constexpr explicit string_ref(const char* str) 
			: _base::string_ref(str, _base::string_ref::_refcounted_string_thunk) 
		{ 
		} 
		string_ref(const string_ref&) = default; 
		string_ref(string_ref&&) = default; 
		string_ref& operator=(const string_ref&) = default; 
		string_ref& operator=(string_ref&&) = default; 
		~string_ref() = default; 
		//! Construct from a Win32 error code 
		explicit string_ref(win32::DWORD c) 
			: _base::string_ref(_base::string_ref::_refcounted_string_thunk) 
		{ 
			wchar_t buffer[32768]; 
			win32::DWORD wlen = win32::FormatMessageW(0x00001000 /*FORMAT_MESSAGE_FROM_SYSTEM*/ | 0x00000200 /*FORMAT_MESSAGE_IGNORE_INSERTS*/, nullptr, c, 0, buffer, 32768, nullptr); 
			size_t allocation = wlen + (wlen >> 1); 
			win32::DWORD bytes; 
			if (wlen == 0) 
			{ 
				goto failure; 
			} 
			for (;;) 
			{ 
				auto* p = static_cast<char*>(malloc(allocation)); // NOLINT 
				if (p == nullptr) 
				{ 
					goto failure; 
				} 
				bytes = win32::WideCharToMultiByte(65001 /*CP_UTF8*/, 0, buffer, wlen + 1, p, allocation, nullptr, nullptr); 
				if (bytes != 0) 
				{ 
					this->_begin = p; 
					this->_end = strchr(p, 0); 
					while (this->_end[-1] == 10 || this->_end[-1] == 13) 
					{ 
						--this->_end; 
					} 
					*const_cast<char*>(this->_end) = 0; // NOLINT 
					break; 
				} 
				free(p); // NOLINT 
				if (win32::GetLastError() == 0x7a /*ERROR_INSUFFICIENT_BUFFER*/) 
				{ 
					allocation += allocation >> 2; 
					continue; 
				} 
				goto failure; 
			} 
			_msg() = (_allocated_msg*)calloc(1, sizeof(_allocated_msg)); // NOLINT 
			if (_msg() == nullptr) 
			{ 
				free((void*)this->_begin); // NOLINT 
				goto failure; 
			} 
			++_msg()->count; 
			return; 
		failure: 
			_msg() = nullptr; // disabled 
			this->_begin = "failed to get message from system"; 
			this->_end = strchr(this->_begin, 0); 
		} 
	}; 
 
public: 
	//! Default constructor 
	constexpr _win32_code_domain() noexcept : _base(0x8cd18ee72d680f1b) {} 
	_win32_code_domain(const _win32_code_domain&) = default; 
	_win32_code_domain(_win32_code_domain&&) = default; 
	_win32_code_domain& operator=(const _win32_code_domain&) = default; 
	_win32_code_domain& operator=(_win32_code_domain&&) = default; 
	~_win32_code_domain() = default; 
 
	//! Constexpr singleton getter. Returns the address of the constexpr win32_code_domain variable. 
	static inline constexpr const _win32_code_domain* get(); 
 
	virtual _base::string_ref name() const noexcept override final { return _base::string_ref("win32 domain"); } // NOLINT 
protected: 
	virtual bool _failure(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		return static_cast<const win32_code&>(code).value() != 0; // NOLINT 
	} 
	virtual bool _equivalent(const status_code<void>& code1, const status_code<void>& code2) const noexcept override final // NOLINT 
	{ 
		assert(code1.domain() == *this); 
		const auto& c1 = static_cast<const win32_code&>(code1); // NOLINT 
		if (code2.domain() == *this) 
		{ 
			const auto& c2 = static_cast<const win32_code&>(code2); // NOLINT 
			return c1.value() == c2.value(); 
		} 
		if (code2.domain() == generic_code_domain) 
		{ 
			const auto& c2 = static_cast<const generic_code&>(code2); // NOLINT 
			if (static_cast<int>(c2.value()) == _win32_code_to_errno(c1.value())) 
			{ 
				return true; 
			} 
		} 
		return false; 
	} 
	virtual generic_code _generic_code(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const win32_code&>(code); // NOLINT 
		return generic_code(static_cast<errc>(_win32_code_to_errno(c.value()))); 
	} 
	virtual _base::string_ref _message(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const win32_code&>(code); // NOLINT 
		return string_ref(c.value()); 
	} 
	virtual void _throw_exception(const status_code<void>& code) const override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const win32_code&>(code); // NOLINT 
		throw status_error<_win32_code_domain>(c); 
	} 
}; 
//! (Windows only) A constexpr source variable for the win32 code domain, which is that of `GetLastError()` (Windows). Returned by `_win32_code_domain::get()`. 
constexpr _win32_code_domain win32_code_domain; 
inline constexpr const _win32_code_domain* _win32_code_domain::get() 
{ 
	return &win32_code_domain; 
} 
 
} 
 
#endif 
namespace system_error2 {  
 
//! \exclude 
namespace win32 
{ 
	// A Win32 NTSTATUS 
	using NTSTATUS = long; 
	// A Win32 HMODULE 
	using HMODULE = void*; 
	// Used to retrieve where the NTDLL DLL is mapped into memory 
	extern "C" HMODULE __stdcall GetModuleHandleW(const wchar_t* lpModuleName); 
} 
 
class _nt_code_domain; 
//! (Windows only) A NT error code, those returned by NT kernel functions. 
using nt_code = status_code<_nt_code_domain>; 
 
/*! (Windows only) The implementation of the domain for NT error codes, those returned by NT kernel functions. 
*/ 
 
class _nt_code_domain : public status_code_domain 
{ 
	template <class DomainType> friend class status_code; 
	friend class _com_code_domain; 
	using _base = status_code_domain; 
	static int _nt_code_to_errno(win32::NTSTATUS c) 
	{ 
		if (c >= 0) 
		{ 
			return 0; // success 
		} 
		switch (static_cast<unsigned>(c)) 
		{ 
		case 0x80000002: return EACCES; 
		case 0x8000000f: return EAGAIN; 
		case 0x80000010: return EAGAIN; 
		case 0x80000011: return EBUSY; 
		case 0xc0000002: return ENOSYS; 
		case 0xc0000005: return EACCES; 
		case 0xc0000008: return EINVAL; 
		case 0xc000000e: return ENOENT; 
		case 0xc000000f: return ENOENT; 
		case 0xc0000010: return ENOSYS; 
		case 0xc0000013: return EAGAIN; 
		case 0xc0000017: return ENOMEM; 
		case 0xc000001c: return ENOSYS; 
		case 0xc000001e: return EACCES; 
		case 0xc000001f: return EACCES; 
		case 0xc0000021: return EACCES; 
		case 0xc0000022: return EACCES; 
		case 0xc0000024: return EINVAL; 
		case 0xc0000033: return EINVAL; 
		case 0xc0000034: return ENOENT; 
		case 0xc0000035: return EEXIST; 
		case 0xc0000037: return EINVAL; 
		case 0xc000003a: return ENOENT; 
		case 0xc0000040: return ENOMEM; 
		case 0xc0000041: return EACCES; 
		case 0xc0000042: return EINVAL; 
		case 0xc0000043: return EACCES; 
		case 0xc000004b: return EACCES; 
		case 0xc0000054: return ENOLCK; 
		case 0xc0000055: return ENOLCK; 
		case 0xc0000056: return EACCES; 
		case 0xc000007f: return ENOSPC; 
		case 0xc0000087: return ENOMEM; 
		case 0xc0000097: return ENOMEM; 
		case 0xc000009b: return ENOENT; 
		case 0xc000009e: return EAGAIN; 
		case 0xc00000a2: return EACCES; 
		case 0xc00000a3: return EAGAIN; 
		case 0xc00000af: return ENOSYS; 
		case 0xc00000ba: return EACCES; 
		case 0xc00000c0: return ENODEV; 
		case 0xc00000d4: return EXDEV; 
		case 0xc00000d5: return EACCES; 
		case 0xc00000fb: return ENOENT; 
		case 0xc0000101: return ENOTEMPTY; 
		case 0xc0000103: return EINVAL; 
		case 0xc0000107: return EBUSY; 
		case 0xc0000108: return EBUSY; 
		case 0xc000010a: return EACCES; 
		case 0xc000011f: return EMFILE; 
		case 0xc0000120: return ECANCELED; 
		case 0xc0000121: return EACCES; 
		case 0xc0000123: return EACCES; 
		case 0xc0000128: return EINVAL; 
		case 0xc0000189: return EACCES; 
		case 0xc00001ad: return ENOMEM; 
		case 0xc000022d: return EAGAIN; 
		case 0xc0000235: return EINVAL; 
		case 0xc000026e: return EAGAIN; 
		case 0xc000028a: return EACCES; 
		case 0xc000028b: return EACCES; 
		case 0xc000028d: return EACCES; 
		case 0xc000028e: return EACCES; 
		case 0xc000028f: return EACCES; 
		case 0xc0000290: return EACCES; 
		case 0xc000029c: return ENOSYS; 
		case 0xc00002c5: return EACCES; 
		case 0xc00002d3: return EAGAIN; 
		case 0xc00002ea: return EACCES; 
		case 0xc00002f0: return ENOENT; 
		case 0xc0000373: return ENOMEM; 
		case 0xc0000416: return ENOMEM; 
		case 0xc0000433: return EBUSY; 
		case 0xc0000434: return EBUSY; 
		case 0xc0000455: return EINVAL; 
		case 0xc0000467: return EACCES; 
		case 0xc0000491: return ENOENT; 
		case 0xc0000495: return EAGAIN; 
		case 0xc0000503: return EAGAIN; 
		case 0xc0000507: return EBUSY; 
		case 0xc0000512: return EACCES; 
		case 0xc000070a: return EINVAL; 
		case 0xc000070b: return EINVAL; 
		case 0xc000070c: return EINVAL; 
		case 0xc000070d: return EINVAL; 
		case 0xc000070e: return EINVAL; 
		case 0xc000070f: return EINVAL; 
		case 0xc0000710: return ENOSYS; 
		case 0xc0000711: return ENOSYS; 
		case 0xc0000716: return EINVAL; 
		case 0xc000071b: return ENOSYS; 
		case 0xc000071d: return ENOSYS; 
		case 0xc000071e: return ENOSYS; 
		case 0xc000071f: return ENOSYS; 
		case 0xc0000720: return ENOSYS; 
		case 0xc0000721: return ENOSYS; 
		case 0xc000080f: return EAGAIN; 
		case 0xc000a203: return EACCES; 
		} 
		return -1; 
	} 
	static win32::DWORD _nt_code_to_win32_code(win32::NTSTATUS c) // NOLINT 
	{ 
		if (c >= 0) 
		{ 
			return 0; // success 
		} 
		switch (static_cast<unsigned>(c)) 
		{ 
		case 0x80000002: return 0x3e6; 
		case 0x80000005: return 0xea; 
		case 0x80000006: return 0x12; 
		case 0x80000007: return 0x2a3; 
		case 0x8000000a: return 0x2a4; 
		case 0x8000000b: return 0x56f; 
		case 0x8000000c: return 0x2a8; 
		case 0x8000000d: return 0x12b; 
		case 0x8000000e: return 0x1c; 
		case 0x8000000f: return 0x15; 
		case 0x80000010: return 0x15; 
		case 0x80000011: return 0xaa; 
		case 0x80000012: return 0x103; 
		case 0x80000013: return 0xfe; 
		case 0x80000014: return 0xff; 
		case 0x80000015: return 0xff; 
		case 0x80000016: return 0x456; 
		case 0x80000017: return 0x2a5; 
		case 0x80000018: return 0x2a6; 
		case 0x8000001a: return 0x103; 
		case 0x8000001b: return 0x44d; 
		case 0x8000001c: return 0x456; 
		case 0x8000001d: return 0x457; 
		case 0x8000001e: return 0x44c; 
		case 0x8000001f: return 0x44e; 
		case 0x80000020: return 0x2a7; 
		case 0x80000021: return 0x44f; 
		case 0x80000022: return 0x450; 
		case 0x80000023: return 0x702; 
		case 0x80000024: return 0x713; 
		case 0x80000025: return 0x962; 
		case 0x80000026: return 0x2aa; 
		case 0x80000027: return 0x10f4; 
		case 0x80000028: return 0x2ab; 
		case 0x80000029: return 0x2ac; 
		case 0x8000002a: return 0x2ad; 
		case 0x8000002b: return 0x2ae; 
		case 0x8000002c: return 0x2af; 
		case 0x8000002d: return 0x2a9; 
		case 0x8000002e: return 0x321; 
		case 0x8000002f: return 0x324; 
		case 0x80000030: return 0xab; 
		case 0x80000032: return 0xeb; 
		case 0x80000288: return 0x48d; 
		case 0x80000289: return 0x48e; 
		case 0x80000803: return 0x1abb; 
		case 0x8000a127: return 0x3bdf; 
		case 0x8000cf00: return 0x16e; 
		case 0x8000cf04: return 0x16d; 
		case 0x8000cf05: return 0x176; 
		case 0x80130001: return 0x13c5; 
		case 0x80130002: return 0x13c6; 
		case 0x80130003: return 0x13c7; 
		case 0x80130004: return 0x13c8; 
		case 0x80130005: return 0x13c9; 
		case 0x80190009: return 0x19e5; 
		case 0x80190029: return 0x1aa0; 
		case 0x80190031: return 0x1aa2; 
		case 0x80190041: return 0x1ab3; 
		case 0x80190042: return 0x1ab4; 
		case 0x801c0001: return 0x7a; 
		case 0xc0000001: return 0x1f; 
		case 0xc0000002: return 0x1; 
		case 0xc0000003: return 0x57; 
		case 0xc0000004: return 0x18; 
		case 0xc0000005: return 0x3e6; 
		case 0xc0000006: return 0x3e7; 
		case 0xc0000007: return 0x5ae; 
		case 0xc0000008: return 0x6; 
		case 0xc0000009: return 0x3e9; 
		case 0xc000000a: return 0xc1; 
		case 0xc000000b: return 0x57; 
		case 0xc000000c: return 0x21d; 
		case 0xc000000d: return 0x57; 
		case 0xc000000e: return 0x2; 
		case 0xc000000f: return 0x2; 
		case 0xc0000010: return 0x1; 
		case 0xc0000011: return 0x26; 
		case 0xc0000012: return 0x22; 
		case 0xc0000013: return 0x15; 
		case 0xc0000014: return 0x6f9; 
		case 0xc0000015: return 0x1b; 
		case 0xc0000016: return 0xea; 
		case 0xc0000017: return 0x8; 
		case 0xc0000018: return 0x1e7; 
		case 0xc0000019: return 0x1e7; 
		case 0xc000001a: return 0x57; 
		case 0xc000001b: return 0x57; 
		case 0xc000001c: return 0x1; 
		case 0xc000001e: return 0x5; 
		case 0xc000001f: return 0x5; 
		case 0xc0000020: return 0xc1; 
		case 0xc0000021: return 0x5; 
		case 0xc0000022: return 0x5; 
		case 0xc0000023: return 0x7a; 
		case 0xc0000024: return 0x6; 
		case 0xc0000027: return 0x21e; 
		case 0xc0000028: return 0x21f; 
		case 0xc0000029: return 0x220; 
		case 0xc000002a: return 0x9e; 
		case 0xc000002c: return 0x1e7; 
		case 0xc000002d: return 0x1e7; 
		case 0xc000002e: return 0x221; 
		case 0xc000002f: return 0x222; 
		case 0xc0000030: return 0x57; 
		case 0xc0000031: return 0x223; 
		case 0xc0000032: return 0x571; 
		case 0xc0000033: return 0x7b; 
		case 0xc0000034: return 0x2; 
		case 0xc0000035: return 0xb7; 
		case 0xc0000036: return 0x72a; 
		case 0xc0000037: return 0x6; 
		case 0xc0000038: return 0x224; 
		case 0xc0000039: return 0xa1; 
		case 0xc000003a: return 0x3; 
		case 0xc000003b: return 0xa1; 
		case 0xc000003c: return 0x45d; 
		case 0xc000003d: return 0x45d; 
		case 0xc000003e: return 0x17; 
		case 0xc000003f: return 0x17; 
		case 0xc0000040: return 0x8; 
		case 0xc0000041: return 0x5; 
		case 0xc0000042: return 0x6; 
		case 0xc0000043: return 0x20; 
		case 0xc0000044: return 0x718; 
		case 0xc0000045: return 0x57; 
		case 0xc0000046: return 0x120; 
		case 0xc0000047: return 0x12a; 
		case 0xc0000048: return 0x57; 
		case 0xc0000049: return 0x57; 
		case 0xc000004a: return 0x9c; 
		case 0xc000004b: return 0x5; 
		case 0xc000004c: return 0x57; 
		case 0xc000004d: return 0x57; 
		case 0xc000004e: return 0x57; 
		case 0xc000004f: return 0x11a; 
		case 0xc0000050: return 0xff; 
		case 0xc0000051: return 0x570; 
		case 0xc0000052: return 0x570; 
		case 0xc0000053: return 0x570; 
		case 0xc0000054: return 0x21; 
		case 0xc0000055: return 0x21; 
		case 0xc0000056: return 0x5; 
		case 0xc0000057: return 0x32; 
		case 0xc0000058: return 0x519; 
		case 0xc0000059: return 0x51a; 
		case 0xc000005a: return 0x51b; 
		case 0xc000005b: return 0x51c; 
		case 0xc000005c: return 0x51d; 
		case 0xc000005d: return 0x51e; 
		case 0xc000005e: return 0x51f; 
		case 0xc000005f: return 0x520; 
		case 0xc0000060: return 0x521; 
		case 0xc0000061: return 0x522; 
		case 0xc0000062: return 0x523; 
		case 0xc0000063: return 0x524; 
		case 0xc0000064: return 0x525; 
		case 0xc0000065: return 0x526; 
		case 0xc0000066: return 0x527; 
		case 0xc0000067: return 0x528; 
		case 0xc0000068: return 0x529; 
		case 0xc0000069: return 0x52a; 
		case 0xc000006a: return 0x56; 
		case 0xc000006b: return 0x52c; 
		case 0xc000006c: return 0x52d; 
		case 0xc000006d: return 0x52e; 
		case 0xc000006e: return 0x52f; 
		case 0xc000006f: return 0x530; 
		case 0xc0000070: return 0x531; 
		case 0xc0000071: return 0x532; 
		case 0xc0000072: return 0x533; 
		case 0xc0000073: return 0x534; 
		case 0xc0000074: return 0x535; 
		case 0xc0000075: return 0x536; 
		case 0xc0000076: return 0x537; 
		case 0xc0000077: return 0x538; 
		case 0xc0000078: return 0x539; 
		case 0xc0000079: return 0x53a; 
		case 0xc000007a: return 0x7f; 
		case 0xc000007b: return 0xc1; 
		case 0xc000007c: return 0x3f0; 
		case 0xc000007d: return 0x53c; 
		case 0xc000007e: return 0x9e; 
		case 0xc000007f: return 0x70; 
		case 0xc0000080: return 0x53d; 
		case 0xc0000081: return 0x53e; 
		case 0xc0000082: return 0x44; 
		case 0xc0000083: return 0x103; 
		case 0xc0000084: return 0x53f; 
		case 0xc0000085: return 0x103; 
		case 0xc0000086: return 0x9a; 
		case 0xc0000087: return 0xe; 
		case 0xc0000088: return 0x1e7; 
		case 0xc0000089: return 0x714; 
		case 0xc000008a: return 0x715; 
		case 0xc000008b: return 0x716; 
		case 0xc0000095: return 0x216; 
		case 0xc0000097: return 0x8; 
		case 0xc0000098: return 0x3ee; 
		case 0xc0000099: return 0x540; 
		case 0xc000009a: return 0x5aa; 
		case 0xc000009b: return 0x3; 
		case 0xc000009c: return 0x17; 
		case 0xc000009d: return 0x48f; 
		case 0xc000009e: return 0x15; 
		case 0xc000009f: return 0x1e7; 
		case 0xc00000a0: return 0x1e7; 
		case 0xc00000a1: return 0x5ad; 
		case 0xc00000a2: return 0x13; 
		case 0xc00000a3: return 0x15; 
		case 0xc00000a4: return 0x541; 
		case 0xc00000a5: return 0x542; 
		case 0xc00000a6: return 0x543; 
		case 0xc00000a7: return 0x544; 
		case 0xc00000a8: return 0x545; 
		case 0xc00000a9: return 0x57; 
		case 0xc00000aa: return 0x225; 
		case 0xc00000ab: return 0xe7; 
		case 0xc00000ac: return 0xe7; 
		case 0xc00000ad: return 0xe6; 
		case 0xc00000ae: return 0xe7; 
		case 0xc00000af: return 0x1; 
		case 0xc00000b0: return 0xe9; 
		case 0xc00000b1: return 0xe8; 
		case 0xc00000b2: return 0x217; 
		case 0xc00000b3: return 0x218; 
		case 0xc00000b4: return 0xe6; 
		case 0xc00000b5: return 0x79; 
		case 0xc00000b6: return 0x26; 
		case 0xc00000b7: return 0x226; 
		case 0xc00000b8: return 0x227; 
		case 0xc00000b9: return 0x228; 
		case 0xc00000ba: return 0x5; 
		case 0xc00000bb: return 0x32; 
		case 0xc00000bc: return 0x33; 
		case 0xc00000bd: return 0x34; 
		case 0xc00000be: return 0x35; 
		case 0xc00000bf: return 0x36; 
		case 0xc00000c0: return 0x37; 
		case 0xc00000c1: return 0x38; 
		case 0xc00000c2: return 0x39; 
		case 0xc00000c3: return 0x3a; 
		case 0xc00000c4: return 0x3b; 
		case 0xc00000c5: return 0x3c; 
		case 0xc00000c6: return 0x3d; 
		case 0xc00000c7: return 0x3e; 
		case 0xc00000c8: return 0x3f; 
		case 0xc00000c9: return 0x40; 
		case 0xc00000ca: return 0x41; 
		case 0xc00000cb: return 0x42; 
		case 0xc00000cc: return 0x43; 
		case 0xc00000cd: return 0x44; 
		case 0xc00000ce: return 0x45; 
		case 0xc00000cf: return 0x46; 
		case 0xc00000d0: return 0x47; 
		case 0xc00000d1: return 0x48; 
		case 0xc00000d2: return 0x58; 
		case 0xc00000d3: return 0x229; 
		case 0xc00000d4: return 0x11; 
		case 0xc00000d5: return 0x5; 
		case 0xc00000d6: return 0xf0; 
		case 0xc00000d7: return 0x546; 
		case 0xc00000d8: return 0x22a; 
		case 0xc00000d9: return 0xe8; 
		case 0xc00000da: return 0x547; 
		case 0xc00000db: return 0x22b; 
		case 0xc00000dc: return 0x548; 
		case 0xc00000dd: return 0x549; 
		case 0xc00000de: return 0x54a; 
		case 0xc00000df: return 0x54b; 
		case 0xc00000e0: return 0x54c; 
		case 0xc00000e1: return 0x54d; 
		case 0xc00000e2: return 0x12c; 
		case 0xc00000e3: return 0x12d; 
		case 0xc00000e4: return 0x54e; 
		case 0xc00000e5: return 0x54f; 
		case 0xc00000e6: return 0x550; 
		case 0xc00000e7: return 0x551; 
		case 0xc00000e8: return 0x6f8; 
		case 0xc00000e9: return 0x45d; 
		case 0xc00000ea: return 0x22c; 
		case 0xc00000eb: return 0x22d; 
		case 0xc00000ec: return 0x22e; 
		case 0xc00000ed: return 0x552; 
		case 0xc00000ee: return 0x553; 
		case 0xc00000ef: return 0x57; 
		case 0xc00000f0: return 0x57; 
		case 0xc00000f1: return 0x57; 
		case 0xc00000f2: return 0x57; 
		case 0xc00000f3: return 0x57; 
		case 0xc00000f4: return 0x57; 
		case 0xc00000f5: return 0x57; 
		case 0xc00000f6: return 0x57; 
		case 0xc00000f7: return 0x57; 
		case 0xc00000f8: return 0x57; 
		case 0xc00000f9: return 0x57; 
		case 0xc00000fa: return 0x57; 
		case 0xc00000fb: return 0x3; 
		case 0xc00000fc: return 0x420; 
		case 0xc00000fd: return 0x3e9; 
		case 0xc00000fe: return 0x554; 
		case 0xc00000ff: return 0x22f; 
		case 0xc0000100: return 0xcb; 
		case 0xc0000101: return 0x91; 
		case 0xc0000102: return 0x570; 
		case 0xc0000103: return 0x10b; 
		case 0xc0000104: return 0x555; 
		case 0xc0000105: return 0x556; 
		case 0xc0000106: return 0xce; 
		case 0xc0000107: return 0x961; 
		case 0xc0000108: return 0x964; 
		case 0xc000010a: return 0x5; 
		case 0xc000010b: return 0x557; 
		case 0xc000010c: return 0x230; 
		case 0xc000010d: return 0x558; 
		case 0xc000010e: return 0x420; 
		case 0xc000010f: return 0x21a; 
		case 0xc0000110: return 0x21a; 
		case 0xc0000111: return 0x21a; 
		case 0xc0000112: return 0x21a; 
		case 0xc0000113: return 0x21a; 
		case 0xc0000114: return 0x21a; 
		case 0xc0000115: return 0x21a; 
		case 0xc0000116: return 0x21a; 
		case 0xc0000117: return 0x5a4; 
		case 0xc0000118: return 0x231; 
		case 0xc0000119: return 0x233; 
		case 0xc000011a: return 0x234; 
		case 0xc000011b: return 0xc1; 
		case 0xc000011c: return 0x559; 
		case 0xc000011d: return 0x55a; 
		case 0xc000011e: return 0x3ee; 
		case 0xc000011f: return 0x4; 
		case 0xc0000120: return 0x3e3; 
		case 0xc0000121: return 0x5; 
		case 0xc0000122: return 0x4ba; 
		case 0xc0000123: return 0x5; 
		case 0xc0000124: return 0x55b; 
		case 0xc0000125: return 0x55c; 
		case 0xc0000126: return 0x55d; 
		case 0xc0000127: return 0x55e; 
		case 0xc0000128: return 0x6; 
		case 0xc0000129: return 0x235; 
		case 0xc000012a: return 0x236; 
		case 0xc000012b: return 0x55f; 
		case 0xc000012c: return 0x237; 
		case 0xc000012d: return 0x5af; 
		case 0xc000012e: return 0xc1; 
		case 0xc000012f: return 0xc1; 
		case 0xc0000130: return 0xc1; 
		case 0xc0000131: return 0xc1; 
		case 0xc0000132: return 0x238; 
		case 0xc0000133: return 0x576; 
		case 0xc0000134: return 0x239; 
		case 0xc0000135: return 0x7e; 
		case 0xc0000136: return 0x23a; 
		case 0xc0000137: return 0x23b; 
		case 0xc0000138: return 0xb6; 
		case 0xc0000139: return 0x7f; 
		case 0xc000013a: return 0x23c; 
		case 0xc000013b: return 0x40; 
		case 0xc000013c: return 0x40; 
		case 0xc000013d: return 0x33; 
		case 0xc000013e: return 0x3b; 
		case 0xc000013f: return 0x3b; 
		case 0xc0000140: return 0x3b; 
		case 0xc0000141: return 0x3b; 
		case 0xc0000142: return 0x45a; 
		case 0xc0000143: return 0x23d; 
		case 0xc0000144: return 0x23e; 
		case 0xc0000145: return 0x23f; 
		case 0xc0000146: return 0x240; 
		case 0xc0000147: return 0x242; 
		case 0xc0000148: return 0x7c; 
		case 0xc0000149: return 0x56; 
		case 0xc000014a: return 0x243; 
		case 0xc000014b: return 0x6d; 
		case 0xc000014c: return 0x3f1; 
		case 0xc000014d: return 0x3f8; 
		case 0xc000014e: return 0x244; 
		case 0xc000014f: return 0x3ed; 
		case 0xc0000150: return 0x45e; 
		case 0xc0000151: return 0x560; 
		case 0xc0000152: return 0x561; 
		case 0xc0000153: return 0x562; 
		case 0xc0000154: return 0x563; 
		case 0xc0000155: return 0x564; 
		case 0xc0000156: return 0x565; 
		case 0xc0000157: return 0x566; 
		case 0xc0000158: return 0x567; 
		case 0xc0000159: return 0x3ef; 
		case 0xc000015a: return 0x568; 
		case 0xc000015b: return 0x569; 
		case 0xc000015c: return 0x3f9; 
		case 0xc000015d: return 0x56a; 
		case 0xc000015e: return 0x245; 
		case 0xc000015f: return 0x45d; 
		case 0xc0000160: return 0x4db; 
		case 0xc0000161: return 0x246; 
		case 0xc0000162: return 0x459; 
		case 0xc0000163: return 0x247; 
		case 0xc0000164: return 0x248; 
		case 0xc0000165: return 0x462; 
		case 0xc0000166: return 0x463; 
		case 0xc0000167: return 0x464; 
		case 0xc0000168: return 0x465; 
		case 0xc0000169: return 0x466; 
		case 0xc000016a: return 0x467; 
		case 0xc000016b: return 0x468; 
		case 0xc000016c: return 0x45f; 
		case 0xc000016d: return 0x45d; 
		case 0xc000016e: return 0x249; 
		case 0xc0000172: return 0x451; 
		case 0xc0000173: return 0x452; 
		case 0xc0000174: return 0x453; 
		case 0xc0000175: return 0x454; 
		case 0xc0000176: return 0x455; 
		case 0xc0000177: return 0x469; 
		case 0xc0000178: return 0x458; 
		case 0xc000017a: return 0x56b; 
		case 0xc000017b: return 0x56c; 
		case 0xc000017c: return 0x3fa; 
		case 0xc000017d: return 0x3fb; 
		case 0xc000017e: return 0x56d; 
		case 0xc000017f: return 0x56e; 
		case 0xc0000180: return 0x3fc; 
		case 0xc0000181: return 0x3fd; 
		case 0xc0000182: return 0x57; 
		case 0xc0000183: return 0x45d; 
		case 0xc0000184: return 0x16; 
		case 0xc0000185: return 0x45d; 
		case 0xc0000186: return 0x45d; 
		case 0xc0000187: return 0x24a; 
		case 0xc0000188: return 0x5de; 
		case 0xc0000189: return 0x13; 
		case 0xc000018a: return 0x6fa; 
		case 0xc000018b: return 0x6fb; 
		case 0xc000018c: return 0x6fc; 
		case 0xc000018d: return 0x6fd; 
		case 0xc000018e: return 0x5dc; 
		case 0xc000018f: return 0x5dd; 
		case 0xc0000190: return 0x6fe; 
		case 0xc0000191: return 0x24b; 
		case 0xc0000192: return 0x700; 
		case 0xc0000193: return 0x701; 
		case 0xc0000194: return 0x46b; 
		case 0xc0000195: return 0x4c3; 
		case 0xc0000196: return 0x4c4; 
		case 0xc0000197: return 0x5df; 
		case 0xc0000198: return 0x70f; 
		case 0xc0000199: return 0x710; 
		case 0xc000019a: return 0x711; 
		case 0xc000019b: return 0x712; 
		case 0xc000019c: return 0x24c; 
		case 0xc000019d: return 0x420; 
		case 0xc000019e: return 0x130; 
		case 0xc000019f: return 0x131; 
		case 0xc00001a0: return 0x132; 
		case 0xc00001a1: return 0x133; 
		case 0xc00001a2: return 0x325; 
		case 0xc00001a3: return 0x134; 
		case 0xc00001a4: return 0x135; 
		case 0xc00001a5: return 0x136; 
		case 0xc00001a6: return 0x137; 
		case 0xc00001a7: return 0x139; 
		case 0xc00001a8: return 0x1abb; 
		case 0xc00001a9: return 0x32; 
		case 0xc00001aa: return 0x3d54; 
		case 0xc00001ab: return 0x329; 
		case 0xc00001ac: return 0x678; 
		case 0xc00001ad: return 0x8; 
		case 0xc00001ae: return 0x2f7; 
		case 0xc00001af: return 0x32d; 
		case 0xc0000201: return 0x41; 
		case 0xc0000202: return 0x572; 
		case 0xc0000203: return 0x3b; 
		case 0xc0000204: return 0x717; 
		case 0xc0000205: return 0x46a; 
		case 0xc0000206: return 0x6f8; 
		case 0xc0000207: return 0x4be; 
		case 0xc0000208: return 0x4be; 
		case 0xc0000209: return 0x44; 
		case 0xc000020a: return 0x34; 
		case 0xc000020b: return 0x40; 
		case 0xc000020c: return 0x40; 
		case 0xc000020d: return 0x40; 
		case 0xc000020e: return 0x44; 
		case 0xc000020f: return 0x3b; 
		case 0xc0000210: return 0x3b; 
		case 0xc0000211: return 0x3b; 
		case 0xc0000212: return 0x3b; 
		case 0xc0000213: return 0x3b; 
		case 0xc0000214: return 0x3b; 
		case 0xc0000215: return 0x3b; 
		case 0xc0000216: return 0x32; 
		case 0xc0000217: return 0x32; 
		case 0xc0000218: return 0x24d; 
		case 0xc0000219: return 0x24e; 
		case 0xc000021a: return 0x24f; 
		case 0xc000021b: return 0x250; 
		case 0xc000021c: return 0x17e6; 
		case 0xc000021d: return 0x251; 
		case 0xc000021e: return 0x252; 
		case 0xc000021f: return 0x253; 
		case 0xc0000220: return 0x46c; 
		case 0xc0000221: return 0xc1; 
		case 0xc0000222: return 0x254; 
		case 0xc0000223: return 0x255; 
		case 0xc0000224: return 0x773; 
		case 0xc0000225: return 0x490; 
		case 0xc0000226: return 0x256; 
		case 0xc0000227: return 0x4ff; 
		case 0xc0000228: return 0x257; 
		case 0xc0000229: return 0x57; 
		case 0xc000022a: return 0x1392; 
		case 0xc000022b: return 0x1392; 
		case 0xc000022c: return 0x258; 
		case 0xc000022d: return 0x4d5; 
		case 0xc000022e: return 0x259; 
		case 0xc000022f: return 0x25a; 
		case 0xc0000230: return 0x492; 
		case 0xc0000231: return 0x25b; 
		case 0xc0000232: return 0x25c; 
		case 0xc0000233: return 0x774; 
		case 0xc0000234: return 0x775; 
		case 0xc0000235: return 0x6; 
		case 0xc0000236: return 0x4c9; 
		case 0xc0000237: return 0x4ca; 
		case 0xc0000238: return 0x4cb; 
		case 0xc0000239: return 0x4cc; 
		case 0xc000023a: return 0x4cd; 
		case 0xc000023b: return 0x4ce; 
		case 0xc000023c: return 0x4cf; 
		case 0xc000023d: return 0x4d0; 
		case 0xc000023e: return 0x4d1; 
		case 0xc000023f: return 0x4d2; 
		case 0xc0000240: return 0x4d3; 
		case 0xc0000241: return 0x4d4; 
		case 0xc0000242: return 0x25d; 
		case 0xc0000243: return 0x4c8; 
		case 0xc0000244: return 0x25e; 
		case 0xc0000245: return 0x25f; 
		case 0xc0000246: return 0x4d6; 
		case 0xc0000247: return 0x4d7; 
		case 0xc0000248: return 0x4d8; 
		case 0xc0000249: return 0xc1; 
		case 0xc0000250: return 0x260; 
		case 0xc0000251: return 0x261; 
		case 0xc0000252: return 0x262; 
		case 0xc0000253: return 0x4d4; 
		case 0xc0000254: return 0x263; 
		case 0xc0000255: return 0x264; 
		case 0xc0000256: return 0x265; 
		case 0xc0000257: return 0x4d0; 
		case 0xc0000258: return 0x266; 
		case 0xc0000259: return 0x573; 
		case 0xc000025a: return 0x267; 
		case 0xc000025b: return 0x268; 
		case 0xc000025c: return 0x269; 
		case 0xc000025e: return 0x422; 
		case 0xc000025f: return 0x26a; 
		case 0xc0000260: return 0x26b; 
		case 0xc0000261: return 0x26c; 
		case 0xc0000262: return 0xb6; 
		case 0xc0000263: return 0x7f; 
		case 0xc0000264: return 0x120; 
		case 0xc0000265: return 0x476; 
		case 0xc0000266: return 0x26d; 
		case 0xc0000267: return 0x10fe; 
		case 0xc0000268: return 0x26e; 
		case 0xc0000269: return 0x26f; 
		case 0xc000026a: return 0x1b8e; 
		case 0xc000026b: return 0x270; 
		case 0xc000026c: return 0x7d1; 
		case 0xc000026d: return 0x4b1; 
		case 0xc000026e: return 0x15; 
		case 0xc000026f: return 0x21c; 
		case 0xc0000270: return 0x21c; 
		case 0xc0000271: return 0x271; 
		case 0xc0000272: return 0x491; 
		case 0xc0000273: return 0x272; 
		case 0xc0000275: return 0x1126; 
		case 0xc0000276: return 0x1129; 
		case 0xc0000277: return 0x112a; 
		case 0xc0000278: return 0x1128; 
		case 0xc0000279: return 0x780; 
		case 0xc000027a: return 0x291; 
		case 0xc000027b: return 0x54f; 
		case 0xc000027c: return 0x54f; 
		case 0xc0000280: return 0x781; 
		case 0xc0000281: return 0xa1; 
		case 0xc0000282: return 0x273; 
		case 0xc0000283: return 0x488; 
		case 0xc0000284: return 0x489; 
		case 0xc0000285: return 0x48a; 
		case 0xc0000286: return 0x48b; 
		case 0xc0000287: return 0x48c; 
		case 0xc000028a: return 0x5; 
		case 0xc000028b: return 0x5; 
		case 0xc000028c: return 0x284; 
		case 0xc000028d: return 0x5; 
		case 0xc000028e: return 0x5; 
		case 0xc000028f: return 0x5; 
		case 0xc0000290: return 0x5; 
		case 0xc0000291: return 0x1777; 
		case 0xc0000292: return 0x1778; 
		case 0xc0000293: return 0x1772; 
		case 0xc0000295: return 0x1068; 
		case 0xc0000296: return 0x1069; 
		case 0xc0000297: return 0x106a; 
		case 0xc0000298: return 0x106b; 
		case 0xc0000299: return 0x201a; 
		case 0xc000029a: return 0x201b; 
		case 0xc000029b: return 0x201c; 
		case 0xc000029c: return 0x1; 
		case 0xc000029d: return 0x10ff; 
		case 0xc000029e: return 0x1100; 
		case 0xc000029f: return 0x494; 
		case 0xc00002a0: return 0x274; 
		case 0xc00002a1: return 0x200a; 
		case 0xc00002a2: return 0x200b; 
		case 0xc00002a3: return 0x200c; 
		case 0xc00002a4: return 0x200d; 
		case 0xc00002a5: return 0x200e; 
		case 0xc00002a6: return 0x200f; 
		case 0xc00002a7: return 0x2010; 
		case 0xc00002a8: return 0x2011; 
		case 0xc00002a9: return 0x2012; 
		case 0xc00002aa: return 0x2013; 
		case 0xc00002ab: return 0x2014; 
		case 0xc00002ac: return 0x2015; 
		case 0xc00002ad: return 0x2016; 
		case 0xc00002ae: return 0x2017; 
		case 0xc00002af: return 0x2018; 
		case 0xc00002b0: return 0x2019; 
		case 0xc00002b1: return 0x211e; 
		case 0xc00002b2: return 0x1127; 
		case 0xc00002b3: return 0x275; 
		case 0xc00002b4: return 0x276; 
		case 0xc00002b5: return 0x277; 
		case 0xc00002b6: return 0x651; 
		case 0xc00002b7: return 0x49a; 
		case 0xc00002b8: return 0x49b; 
		case 0xc00002b9: return 0x278; 
		case 0xc00002ba: return 0x2047; 
		case 0xc00002c1: return 0x2024; 
		case 0xc00002c2: return 0x279; 
		case 0xc00002c3: return 0x575; 
		case 0xc00002c4: return 0x27a; 
		case 0xc00002c5: return 0x3e6; 
		case 0xc00002c6: return 0x1075; 
		case 0xc00002c7: return 0x1076; 
		case 0xc00002c8: return 0x27b; 
		case 0xc00002c9: return 0x4ed; 
		case 0xc00002ca: return 0x10e8; 
		case 0xc00002cb: return 0x2138; 
		case 0xc00002cc: return 0x4e3; 
		case 0xc00002cd: return 0x2139; 
		case 0xc00002ce: return 0x27c; 
		case 0xc00002cf: return 0x49d; 
		case 0xc00002d0: return 0x213a; 
		case 0xc00002d1: return 0x27d; 
		case 0xc00002d2: return 0x27e; 
		case 0xc00002d3: return 0x15; 
		case 0xc00002d4: return 0x2141; 
		case 0xc00002d5: return 0x2142; 
		case 0xc00002d6: return 0x2143; 
		case 0xc00002d7: return 0x2144; 
		case 0xc00002d8: return 0x2145; 
		case 0xc00002d9: return 0x2146; 
		case 0xc00002da: return 0x2147; 
		case 0xc00002db: return 0x2148; 
		case 0xc00002dc: return 0x2149; 
		case 0xc00002dd: return 0x32; 
		case 0xc00002de: return 0x27f; 
		case 0xc00002df: return 0x2151; 
		case 0xc00002e0: return 0x2152; 
		case 0xc00002e1: return 0x2153; 
		case 0xc00002e2: return 0x2154; 
		case 0xc00002e3: return 0x215d; 
		case 0xc00002e4: return 0x2163; 
		case 0xc00002e5: return 0x2164; 
		case 0xc00002e6: return 0x2165; 
		case 0xc00002e7: return 0x216d; 
		case 0xc00002e8: return 0x280; 
		case 0xc00002e9: return 0x577; 
		case 0xc00002ea: return 0x52; 
		case 0xc00002eb: return 0x281; 
		case 0xc00002ec: return 0x2171; 
		case 0xc00002ed: return 0x2172; 
		case 0xc00002f0: return 0x2; 
		case 0xc00002fe: return 0x45b; 
		case 0xc00002ff: return 0x4e7; 
		case 0xc0000300: return 0x4e6; 
		case 0xc0000301: return 0x106f; 
		case 0xc0000302: return 0x1074; 
		case 0xc0000303: return 0x106e; 
		case 0xc0000304: return 0x12e; 
		case 0xc000030c: return 0x792; 
		case 0xc000030d: return 0x793; 
		case 0xc0000320: return 0x4ef; 
		case 0xc0000321: return 0x4f0; 
		case 0xc0000350: return 0x4e8; 
		case 0xc0000352: return 0x177d; 
		case 0xc0000353: return 0x282; 
		case 0xc0000354: return 0x504; 
		case 0xc0000355: return 0x283; 
		case 0xc0000357: return 0x217c; 
		case 0xc0000358: return 0x2182; 
		case 0xc0000359: return 0xc1; 
		case 0xc000035a: return 0xc1; 
		case 0xc000035c: return 0x572; 
		case 0xc000035d: return 0x4eb; 
		case 0xc000035f: return 0x286; 
		case 0xc0000361: return 0x4ec; 
		case 0xc0000362: return 0x4ec; 
		case 0xc0000363: return 0x4ec; 
		case 0xc0000364: return 0x4ec; 
		case 0xc0000365: return 0x287; 
		case 0xc0000366: return 0x288; 
		case 0xc0000368: return 0x289; 
		case 0xc0000369: return 0x28a; 
		case 0xc000036a: return 0x28b; 
		case 0xc000036b: return 0x4fb; 
		case 0xc000036c: return 0x4fb; 
		case 0xc000036d: return 0x28c; 
		case 0xc000036e: return 0x28d; 
		case 0xc000036f: return 0x4fc; 
		case 0xc0000371: return 0x21ac; 
		case 0xc0000372: return 0x312; 
		case 0xc0000373: return 0x8; 
		case 0xc0000374: return 0x54f; 
		case 0xc0000388: return 0x4f1; 
		case 0xc000038e: return 0x28e; 
		case 0xc0000401: return 0x78c; 
		case 0xc0000402: return 0x78d; 
		case 0xc0000403: return 0x78e; 
		case 0xc0000404: return 0x217b; 
		case 0xc0000405: return 0x219d; 
		case 0xc0000406: return 0x219f; 
		case 0xc0000407: return 0x28f; 
		case 0xc0000408: return 0x52e; 
		case 0xc0000409: return 0x502; 
		case 0xc0000410: return 0x503; 
		case 0xc0000411: return 0x290; 
		case 0xc0000412: return 0x505; 
		case 0xc0000413: return 0x78f; 
		case 0xc0000414: return 0x506; 
		case 0xc0000416: return 0x8; 
		case 0xc0000417: return 0x508; 
		case 0xc0000418: return 0x791; 
		case 0xc0000419: return 0x215b; 
		case 0xc000041a: return 0x21ba; 
		case 0xc000041b: return 0x21bb; 
		case 0xc000041c: return 0x21bc; 
		case 0xc000041d: return 0x2c9; 
		case 0xc0000420: return 0x29c; 
		case 0xc0000421: return 0x219; 
		case 0xc0000423: return 0x300; 
		case 0xc0000424: return 0x4fb; 
		case 0xc0000425: return 0x3fa; 
		case 0xc0000426: return 0x301; 
		case 0xc0000427: return 0x299; 
		case 0xc0000428: return 0x241; 
		case 0xc0000429: return 0x307; 
		case 0xc000042a: return 0x308; 
		case 0xc000042b: return 0x50c; 
		case 0xc000042c: return 0x2e4; 
		case 0xc0000432: return 0x509; 
		case 0xc0000433: return 0xaa; 
		case 0xc0000434: return 0xaa; 
		case 0xc0000435: return 0x4c8; 
		case 0xc0000441: return 0x1781; 
		case 0xc0000442: return 0x1782; 
		case 0xc0000443: return 0x1783; 
		case 0xc0000444: return 0x1784; 
		case 0xc0000445: return 0x1785; 
		case 0xc0000446: return 0x513; 
		case 0xc0000450: return 0x50b; 
		case 0xc0000451: return 0x3b92; 
		case 0xc0000452: return 0x3bc3; 
		case 0xc0000453: return 0x5bb; 
		case 0xc0000454: return 0x5be; 
		case 0xc0000455: return 0x6; 
		case 0xc0000456: return 0x57; 
		case 0xc0000457: return 0x57; 
		case 0xc0000458: return 0x57; 
		case 0xc0000459: return 0xbea; 
		case 0xc0000460: return 0x138; 
		case 0xc0000461: return 0x13a; 
		case 0xc0000462: return 0x3cfc; 
		case 0xc0000463: return 0x13c; 
		case 0xc0000464: return 0x141; 
		case 0xc0000465: return 0x13b; 
		case 0xc0000466: return 0x40; 
		case 0xc0000467: return 0x20; 
		case 0xc0000468: return 0x142; 
		case 0xc0000469: return 0x3d00; 
		case 0xc000046a: return 0x151; 
		case 0xc000046b: return 0x152; 
		case 0xc000046c: return 0x153; 
		case 0xc000046d: return 0x156; 
		case 0xc000046e: return 0x157; 
		case 0xc000046f: return 0x158; 
		case 0xc0000470: return 0x143; 
		case 0xc0000471: return 0x144; 
		case 0xc0000472: return 0x146; 
		case 0xc0000473: return 0x14b; 
		case 0xc0000474: return 0x147; 
		case 0xc0000475: return 0x148; 
		case 0xc0000476: return 0x149; 
		case 0xc0000477: return 0x14a; 
		case 0xc0000478: return 0x14c; 
		case 0xc0000479: return 0x14d; 
		case 0xc000047a: return 0x14e; 
		case 0xc000047b: return 0x14f; 
		case 0xc000047c: return 0x150; 
		case 0xc000047d: return 0x5b4; 
		case 0xc000047e: return 0x3d07; 
		case 0xc000047f: return 0x3d08; 
		case 0xc0000480: return 0x40; 
		case 0xc0000481: return 0x7e; 
		case 0xc0000482: return 0x7e; 
		case 0xc0000483: return 0x1e3; 
		case 0xc0000486: return 0x159; 
		case 0xc0000487: return 0x1f; 
		case 0xc0000488: return 0x15a; 
		case 0xc0000489: return 0x3d0f; 
		case 0xc000048a: return 0x32a; 
		case 0xc000048b: return 0x32c; 
		case 0xc000048c: return 0x15b; 
		case 0xc000048d: return 0x15c; 
		case 0xc000048e: return 0x162; 
		case 0xc000048f: return 0x15d; 
		case 0xc0000490: return 0x491; 
		case 0xc0000491: return 0x2; 
		case 0xc0000492: return 0x490; 
		case 0xc0000493: return 0x492; 
		case 0xc0000494: return 0x307; 
		case 0xc0000495: return 0x15; 
		case 0xc0000496: return 0x163; 
		case 0xc0000497: return 0x3d5a; 
		case 0xc0000499: return 0x167; 
		case 0xc000049a: return 0x168; 
		case 0xc000049b: return 0x12e; 
		case 0xc000049c: return 0x169; 
		case 0xc000049d: return 0x16f; 
		case 0xc000049e: return 0x170; 
		case 0xc000049f: return 0x49f; 
		case 0xc00004a0: return 0x4a0; 
		case 0xc00004a1: return 0x18f; 
		case 0xc0000500: return 0x60e; 
		case 0xc0000501: return 0x60f; 
		case 0xc0000502: return 0x610; 
		case 0xc0000503: return 0x15; 
		case 0xc0000504: return 0x13f; 
		case 0xc0000505: return 0x140; 
		case 0xc0000506: return 0x5bf; 
		case 0xc0000507: return 0xaa; 
		case 0xc0000508: return 0x5e0; 
		case 0xc0000509: return 0x5e1; 
		case 0xc000050b: return 0x112b; 
		case 0xc000050e: return 0x115c; 
		case 0xc000050f: return 0x10d3; 
		case 0xc0000510: return 0x4df; 
		case 0xc0000511: return 0x32e; 
		case 0xc0000512: return 0x5; 
		case 0xc0000513: return 0x180; 
		case 0xc0000514: return 0x115d; 
		case 0xc0000602: return 0x675; 
		case 0xc0000604: return 0x677; 
		case 0xc0000606: return 0x679; 
		case 0xc000060a: return 0x67c; 
		case 0xc000060b: return 0x67d; 
		case 0xc0000700: return 0x54f; 
		case 0xc0000701: return 0x54f; 
		case 0xc0000702: return 0x57; 
		case 0xc0000703: return 0x54f; 
		case 0xc0000704: return 0x32; 
		case 0xc0000705: return 0x57; 
		case 0xc0000706: return 0x57; 
		case 0xc0000707: return 0x32; 
		case 0xc0000708: return 0x54f; 
		case 0xc0000709: return 0x30b; 
		case 0xc000070a: return 0x6; 
		case 0xc000070b: return 0x6; 
		case 0xc000070c: return 0x6; 
		case 0xc000070d: return 0x6; 
		case 0xc000070e: return 0x6; 
		case 0xc000070f: return 0x6; 
		case 0xc0000710: return 0x1; 
		case 0xc0000711: return 0x1; 
		case 0xc0000712: return 0x50d; 
		case 0xc0000713: return 0x310; 
		case 0xc0000714: return 0x52e; 
		case 0xc0000715: return 0x5b7; 
		case 0xc0000716: return 0x7b; 
		case 0xc0000717: return 0x459; 
		case 0xc0000718: return 0x54f; 
		case 0xc0000719: return 0x54f; 
		case 0xc000071a: return 0x54f; 
		case 0xc000071b: return 0x1; 
		case 0xc000071c: return 0x57; 
		case 0xc000071d: return 0x1; 
		case 0xc000071e: return 0x1; 
		case 0xc000071f: return 0x1; 
		case 0xc0000720: return 0x1; 
		case 0xc0000721: return 0x1; 
		case 0xc0000722: return 0x72b; 
		case 0xc0000723: return 0x1f; 
		case 0xc0000724: return 0x1f; 
		case 0xc0000725: return 0x1f; 
		case 0xc0000726: return 0x1f; 
		case 0xc0000800: return 0x30c; 
		case 0xc0000801: return 0x21a4; 
		case 0xc0000802: return 0x50f; 
		case 0xc0000804: return 0x510; 
		case 0xc0000805: return 0x1ac1; 
		case 0xc0000806: return 0x1ac3; 
		case 0xc0000808: return 0x319; 
		case 0xc0000809: return 0x31a; 
		case 0xc000080a: return 0x31b; 
		case 0xc000080b: return 0x31c; 
		case 0xc000080c: return 0x31d; 
		case 0xc000080d: return 0x31e; 
		case 0xc000080e: return 0x31f; 
		case 0xc000080f: return 0x4d5; 
		case 0xc0000810: return 0x328; 
		case 0xc0000811: return 0x54f; 
		case 0xc0000901: return 0xdc; 
		case 0xc0000902: return 0xdd; 
		case 0xc0000903: return 0xde; 
		case 0xc0000904: return 0xdf; 
		case 0xc0000905: return 0xe0; 
		case 0xc0000906: return 0xe1; 
		case 0xc0000907: return 0xe2; 
		case 0xc0000908: return 0x317; 
		case 0xc0000909: return 0x322; 
		case 0xc0000910: return 0x326; 
		case 0xc0009898: return 0x29e; 
		case 0xc000a002: return 0x17; 
		case 0xc000a003: return 0x139f; 
		case 0xc000a004: return 0x154; 
		case 0xc000a005: return 0x155; 
		case 0xc000a006: return 0x32b; 
		case 0xc000a007: return 0x32; 
		case 0xc000a010: return 0xea; 
		case 0xc000a011: return 0xea; 
		case 0xc000a012: return 0x4d0; 
		case 0xc000a013: return 0x32; 
		case 0xc000a014: return 0x4d1; 
		case 0xc000a080: return 0x314; 
		case 0xc000a081: return 0x315; 
		case 0xc000a082: return 0x316; 
		case 0xc000a083: return 0x5b9; 
		case 0xc000a084: return 0x5ba; 
		case 0xc000a085: return 0x5bc; 
		case 0xc000a086: return 0x5bd; 
		case 0xc000a087: return 0x21bd; 
		case 0xc000a088: return 0x21be; 
		case 0xc000a089: return 0x21c6; 
		case 0xc000a100: return 0x3bc4; 
		case 0xc000a101: return 0x3bc5; 
		case 0xc000a121: return 0x3bd9; 
		case 0xc000a122: return 0x3bda; 
		case 0xc000a123: return 0x3bdb; 
		case 0xc000a124: return 0x3bdc; 
		case 0xc000a125: return 0x3bdd; 
		case 0xc000a126: return 0x3bde; 
		case 0xc000a141: return 0x3c28; 
		case 0xc000a142: return 0x3c29; 
		case 0xc000a143: return 0x3c2a; 
		case 0xc000a145: return 0x3c2b; 
		case 0xc000a146: return 0x3c2c; 
		case 0xc000a200: return 0x109a; 
		case 0xc000a201: return 0x109c; 
		case 0xc000a202: return 0x109d; 
		case 0xc000a203: return 0x5; 
		case 0xc000a281: return 0x1130; 
		case 0xc000a282: return 0x1131; 
		case 0xc000a283: return 0x1132; 
		case 0xc000a284: return 0x1133; 
		case 0xc000a285: return 0x1134; 
		case 0xc000a2a1: return 0x1158; 
		case 0xc000a2a2: return 0x1159; 
		case 0xc000a2a3: return 0x115a; 
		case 0xc000a2a4: return 0x115b; 
		case 0xc000ce01: return 0x171; 
		case 0xc000ce02: return 0x172; 
		case 0xc000ce03: return 0x173; 
		case 0xc000ce04: return 0x174; 
		case 0xc000ce05: return 0x181; 
		case 0xc000cf00: return 0x166; 
		case 0xc000cf01: return 0x16a; 
		case 0xc000cf02: return 0x16b; 
		case 0xc000cf03: return 0x16c; 
		case 0xc000cf06: return 0x177; 
		case 0xc000cf07: return 0x178; 
		case 0xc000cf08: return 0x179; 
		case 0xc000cf09: return 0x17a; 
		case 0xc000cf0a: return 0x17b; 
		case 0xc000cf0b: return 0x17c; 
		case 0xc000cf0c: return 0x17d; 
		case 0xc000cf0d: return 0x17e; 
		case 0xc000cf0e: return 0x17f; 
		case 0xc000cf0f: return 0x182; 
		case 0xc000cf10: return 0x183; 
		case 0xc000cf11: return 0x184; 
		case 0xc000cf12: return 0x185; 
		case 0xc000cf13: return 0x186; 
		case 0xc000cf14: return 0x187; 
		case 0xc000cf15: return 0x188; 
		case 0xc000cf16: return 0x189; 
		case 0xc000cf17: return 0x18a; 
		case 0xc000cf18: return 0x18b; 
		case 0xc000cf19: return 0x18c; 
		case 0xc000cf1a: return 0x18d; 
		case 0xc000cf1b: return 0x18e; 
		} 
		return static_cast<win32::DWORD>(-1); 
	} 
 
public: 
	//! The value type of the NT code, which is a `win32::NTSTATUS` 
	using value_type = win32::NTSTATUS; 
	//! Thread safe reference to a message string fetched by `FormatMessage()` 
	class string_ref : public _base::string_ref 
	{ 
	public: 
		explicit string_ref(const _base::string_ref& o) 
			: _base::string_ref(o) 
		{ 
		} 
		explicit string_ref(_base::string_ref&& o) 
			: _base::string_ref(std::move(o)) 
		{ 
		} 
		constexpr string_ref() 
			: _base::string_ref(_base::string_ref::_refcounted_string_thunk) 
		{ 
		} 
		constexpr explicit string_ref(const char* str) 
			: _base::string_ref(str, _base::string_ref::_refcounted_string_thunk) 
		{ 
		} 
		string_ref(const string_ref&) = default; 
		string_ref(string_ref&&) = default; 
		string_ref& operator=(const string_ref&) = default; 
		string_ref& operator=(string_ref&&) = default; 
		~string_ref() = default; 
		//! Construct from a NT error code 
		explicit string_ref(win32::NTSTATUS c) 
			: _base::string_ref(_base::string_ref::_refcounted_string_thunk) 
		{ 
			wchar_t buffer[32768]; 
			static win32::HMODULE ntdll = win32::GetModuleHandleW(L"NTDLL.DLL"); 
			win32::DWORD wlen = win32::FormatMessageW(0x00000800 /*FORMAT_MESSAGE_FROM_HMODULE*/ | 0x00001000 /*FORMAT_MESSAGE_FROM_SYSTEM*/ | 0x00000200 /*FORMAT_MESSAGE_IGNORE_INSERTS*/, ntdll, c, (1 << 10) /*MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/, buffer, 32768, nullptr); 
			size_t allocation = wlen + (wlen >> 1); 
			win32::DWORD bytes; 
			if (wlen == 0) 
			{ 
				goto failure; 
			} 
			for (;;) 
			{ 
				auto* p = static_cast<char*>(malloc(allocation)); // NOLINT 
				if (p == nullptr) 
				{ 
					goto failure; 
				} 
				bytes = win32::WideCharToMultiByte(65001 /*CP_UTF8*/, 0, buffer, wlen + 1, p, allocation, nullptr, nullptr); 
				if (bytes != 0) 
				{ 
					this->_begin = p; 
					this->_end = strchr(p, 0); 
					while (this->_end[-1] == 10 || this->_end[-1] == 13) 
					{ 
						--this->_end; 
					} 
					*const_cast<char*>(this->_end) = 0; // NOLINT 
					break; 
				} 
				free(p); // NOLINT 
				if (win32::GetLastError() == 0x7a /*ERROR_INSUFFICIENT_BUFFER*/) 
				{ 
					allocation += allocation >> 2; 
					continue; 
				} 
				goto failure; 
			} 
			_msg() = static_cast<_allocated_msg*>(calloc(1, sizeof(_allocated_msg))); // NOLINT 
			if (_msg() == nullptr) 
			{ 
				free((void*)this->_begin); // NOLINT 
				goto failure; 
			} 
			++_msg()->count; 
			return; 
		failure: 
			_msg() = nullptr; // disabled 
			this->_begin = "failed to get message from system"; 
			this->_end = strchr(this->_begin, 0); 
		} 
	}; 
 
public: 
	//! Default constructor 
	constexpr _nt_code_domain() noexcept : _base(0x93f3b4487e4af25b) {} 
	_nt_code_domain(const _nt_code_domain&) = default; 
	_nt_code_domain(_nt_code_domain&&) = default; 
	_nt_code_domain& operator=(const _nt_code_domain&) = default; 
	_nt_code_domain& operator=(_nt_code_domain&&) = default; 
	~_nt_code_domain() = default; 
 
	//! Constexpr singleton getter. Returns the address of the constexpr nt_code_domain variable. 
	static inline constexpr const _nt_code_domain* get(); 
 
	virtual _base::string_ref name() const noexcept override final { return _base::string_ref("NT domain"); } // NOLINT 
protected: 
	virtual bool _failure(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		return static_cast<const nt_code&>(code).value() < 0; // NOLINT 
	} 
	virtual bool _equivalent(const status_code<void>& code1, const status_code<void>& code2) const noexcept override final // NOLINT 
	{ 
		assert(code1.domain() == *this); 
		const auto& c1 = static_cast<const nt_code&>(code1); // NOLINT 
		if (code2.domain() == *this) 
		{ 
			const auto& c2 = static_cast<const nt_code&>(code2); // NOLINT 
			return c1.value() == c2.value(); 
		} 
		if (code2.domain() == generic_code_domain) 
		{ 
			const auto& c2 = static_cast<const generic_code&>(code2); // NOLINT 
			if (static_cast<int>(c2.value()) == _nt_code_to_errno(c1.value())) 
			{ 
				return true; 
			} 
		} 
		if (code2.domain() == win32_code_domain) 
		{ 
			const auto& c2 = static_cast<const win32_code&>(code2); // NOLINT 
			if (c2.value() == _nt_code_to_win32_code(c1.value())) 
			{ 
				return true; 
			} 
		} 
		return false; 
	} 
	virtual generic_code _generic_code(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const nt_code&>(code); // NOLINT 
		return generic_code(static_cast<errc>(_nt_code_to_errno(c.value()))); 
	} 
	virtual _base::string_ref _message(const status_code<void>& code) const noexcept override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const nt_code&>(code); // NOLINT 
		return string_ref(c.value()); 
	} 
	virtual void _throw_exception(const status_code<void>& code) const override final // NOLINT 
	{ 
		assert(code.domain() == *this); 
		const auto& c = static_cast<const nt_code&>(code); // NOLINT 
		throw status_error<_nt_code_domain>(c); 
	} 
}; 
//! (Windows only) A constexpr source variable for the NT code domain, which is that of NT kernel functions. Returned by `_nt_code_domain::get()`. 
constexpr _nt_code_domain nt_code_domain; 
inline constexpr const _nt_code_domain* _nt_code_domain::get() 
{ 
	return &nt_code_domain; 
} 
 
} 
 
#endif 
namespace system_error2 {  
/*! An erased-mutable status code suitably large for all the system codes 
which can be returned on this system. 
 
For Windows, these might be: 
 
	- `com_code` (`HRESULT`)  [you need to include "com_code.hpp" explicitly for this] 
	- `nt_code` (`LONG`) 
	- `win32_code` (`DWORD`) 
 
So the erased type is `intptr_t` on Windows, as that can represent all of 
the above. 
 
For POSIX, `posix_code` (`int`) is possible, so this erased type is `int` 
i.e. this is the type alias on POSIX, not the above: 
 
`using system_code = status_code<erased<int>>` 
*/ 
using system_code = status_code<erased<intptr_t>>; 
} 
 
#else 
 
namespace system_error2 {  
	using system_code = status_code<erased<int>>; 
} 
 
#endif 
 
#endif 
#endif 
