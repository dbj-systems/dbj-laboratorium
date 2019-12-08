#pragma once
#ifndef _DBJ_CONSOLE_OPS
#define _DBJ_CONSOLE_OPS

// #define DBJ_TYPE_INSTRUMENTS

/// <summary>
/// out overloads for outputing to IConsole instance
/// all fundamental types
/// all compound types
/// all ranges that have begin() and end() methods
/// </summary>
namespace dbj::console {

#pragma region forward declarations

	template<typename T1, typename ... T2  >
	void print(T1 const& first_param, T2 ... params);

	/*
	the cacth all, function declaration
	whatever is not implemented here goes into this function
	*/
	template <typename T> inline void out(T);

	/*
	this is the special out that does not use the console output class
	but painter commander
	Thus we achieved a decoupling of console and painter
	*/
	inline void out(dbj::console::painter_command cmd_)
	{
		dbj::console::painter_commander_instance.execute(cmd_);
	}


	// stings and string literals are different by design ***********************************************
	template<> inline void out< char*>(char* str);
	template<> inline void out< wchar_t*>(wchar_t* str);
	template<> inline void out< char16_t*>(char16_t* str);
	template<> inline void out< char32_t*>(char32_t* str);

	template<> inline void out<const char*>(const char* str);
	template<> inline void out<const wchar_t*>(const wchar_t* str);
	template<> inline void out<const char16_t*>(const char16_t* str);
	template<> inline void out<const char32_t*>(const char32_t* str);
	// std strings
	template<> inline void out< std::string >(std::string  str);
	template<> inline void out< std::wstring >(std::wstring  str);
	template<> inline void out< std::u16string >(std::u16string  str);
	template<> inline void out< std::u32string >(std::u32string  str);

	// std string views
	template<> inline void out< std::string_view >(std::string_view  str);
	template<> inline void out< std::wstring_view >(std::wstring_view  str);
	template<> inline void out< std::u16string_view >(std::u16string_view  str);

	// DBJ: char32_t is a problem for the latest CL as of 2019-07-22
#ifndef _MSC_VER
	template<> inline void out< std::u32string_view >(std::u32string_view  str);
#endif

	// fundamental types ********************************************************************************
	template<> inline void out<nullptr_t>(nullptr_t);
	// fundamental - floating point types
	template<> inline void out<float>(float fv);
	template<> inline void out<double>(double fv);
	template<> inline void out<long double>(long double fv);
	// fundamental - integral types
	template<> inline void out<bool>(bool bv);
	// char types are integral types too
	template<> inline void out<char>(char val);
	template<> inline void out<signed char>(signed char val);
	template<> inline void out<unsigned char>(unsigned char val);
	template<> inline void out<wchar_t>(wchar_t val);
	template<> inline void out<char16_t>(char16_t val);
	template<> inline void out<char32_t>(char32_t val);

	/// <summary>
	/// signed integer types (short int, int, long int, long long int);
	/// </summary>
	template<> inline void out<int>(int val);
	template<> inline void out<short int>(short int val);
	template<> inline void out<long int>(long int val);
	template<> inline void out<long long int>(long long int val);

	/// <summary>
	/// unsigned integer types (unsigned short int, unsigned int, unsigned long int, unsigned long long int);
	/// </summary>
	template<> inline void out<unsigned short int>(unsigned short int val);
	template<> inline void out<unsigned int>(unsigned int val);
	template<> inline void out<unsigned long int>(unsigned long int val);
	template<> inline void out<unsigned long long int>(unsigned long long int val);

	/*
	for buffering we use vector<char_type> for runtime and array <char_type, N> for compile time
	*/

	/*template<>*/ inline void out/*<::std::vector<char> const & >*/(::std::vector<char> const&);
	/*template<>*/ inline void out/*< ::std::vector<wchar_t> const & >*/(::std::vector<wchar_t> const&);

	// template <typename T> inline void out(T);

	template<size_t N> inline void out/*<::std::array<char,N> const& >*/(::std::array<char, N> const&);
	template<size_t N> inline void out/*< ::std::array<wchar_t, N> const& >*/(::std::array<wchar_t, N> const&);

#pragma endregion

	constexpr inline auto MAX_ARGUMENTS = BUFSIZ * 2; // 1024
	// for compound types
	constexpr inline auto MAX_ELEMENTS = BUFSIZ * 2 * 64; // 65535

	 // inline Printer PRN{ &console_ };
	inline const auto& PRN = printer_instance();

	namespace inner {

		template<typename T, typename U, typename = std::void_t<>>
		struct comparable_with_less
			: std::false_type
		{};

		template<typename T, typename U>
		struct comparable_with_less<T, U, std::void_t<decltype((std::declval<T>() < std::declval<U>()))>>
			: std::true_type
		{};

		/*
		print anything between two iterators
		note: of the same sequence
		*/
		// inline auto print_between = [](auto left_, auto right_) -> void
		template<
			typename TL_, typename TR_
			, std::enable_if_t<
			std::is_same_v< TL_, TR_ >
			, bool
			> = true
		>
			inline void print_between(TL_ left_, TR_ right_)
		{

			static_assert(comparable_with_less<  decltype(left_), decltype(right_)  >::value,
				"\n\n" __FILE__ "\n function: " __FUNCSIG__ "\n\n left_ < right_ operator cound not be found?");

			std::size_t argsize =
				static_cast<std::size_t>(std::distance(left_, right_));

			_ASSERTE(argsize);
			_ASSERTE(argsize < MAX_ELEMENTS);

			std::size_t arg_count{ 0 };
			//auto delimited_out = [&](auto && val_) {
			   // // try to find the required out()
			   // out(val_);
			   // if ((arg_count++) < (argsize - 1)) PRN.wchar_to_console(wdelim_str);
			//};

			PRN.wchar_to_console(wprefix_str); PRN.wchar_to_console(wspace_str);

			do {
				out(*left_);
				std::advance(left_, 1);
				if ((arg_count++) < (argsize - 1)) PRN.wchar_to_console(wdelim_str);
			} while (std::distance(left_, right_));

			PRN.wchar_to_console(wspace_str); PRN.wchar_to_console(wsuffix_str);
		};
		/*
		anything that has begin and end
		NOTE: that includes references to native arrays
		*/
		template<typename RG_>
		inline void print_range(RG_ const& range) {

			print_between(std::begin(range), std::end(range));
#if 0
			// not requiring  range.size();
			// thus can do native arrays
			std::size_t argsize =
				static_cast<std::size_t>(
					std::distance(
						std::begin(range), std::end(range)
					)
					);

			if (argsize < 1) return;

			std::size_t arg_count{ 0 };

			auto delimited_out = [&](auto&& val_) {
				// try to find the required out()
				out(val_);
				if ((arg_count++) < (argsize - 1)) PRN.wchar_to_console(wdelim_str);
			};

			PRN.wchar_to_console(wprefix_str); PRN.wchar_to_console(wspace_str);
			for (auto item : range) {
				delimited_out(item);
			}
			PRN.wchar_to_console(wspace_str); PRN.wchar_to_console(wsuffix_str);
#endif
		};

		/* also called from void out(...) functions for compound types. e.g. void out(tuple&) */
		// template<typename... Args >
		inline	auto print_varargs = [](
			auto&& first,
			auto&& ... args
			)
		{
			constexpr std::size_t pack_size = sizeof...(args);

			std::size_t arg_count = 0;

			auto delimited_out = [&](auto&& val_)
			{
				out(val_);
				if (arg_count < pack_size) PRN.wchar_to_console(wdelim_str);
				arg_count += 1;
			};

			PRN.wchar_to_console(wprefix_str); PRN.wchar_to_console(wspace_str);

			delimited_out(first);

			if constexpr (pack_size > 0) {
				(delimited_out(args), ...);
			}

			PRN.wchar_to_console(wspace_str); PRN.wchar_to_console(wsuffix_str);
		};

	} // inner nspace

#ifdef DBJ_TYPE_INSTRUMENTS
#define DBJ_TYPE_REPORT_FUNCSIG	dbj::console::PRN.printf("\n\n%-20s : %s\n%-20s :-> ", "Function", __FUNCSIG__, " ")
	template<typename T>
	auto  dbj_type_report = []() { using namespace dbj::tt; PRN.printf("\n%s", T::to_string<T>().c_str()); };
#else
#define DBJ_TYPE_REPORT_FUNCSIG __noop
	template<typename T>
	auto  dbj_type_report = []() { __noop;   };
#endif

#define DBJ_UNHANDLED_(P)\
	 dbj::console::PRN.printf(\
		 __FUNCSIG__ "\nunhandled " P " argument type: %s\nbase type: %s\n",\
		 DBJ_TYPENAME(T), DBJ_TYPENAME(actual::base) )

	/*
	dbj::console::out does not have ref or pointer argument
	thus it can not be sent pointer or ref to some ABC
	this is not a bug, this is by design
	explanations: circle and triangle are both shapes
	but triangle has no radius or center point
	triangle and circle need to output different string
	thus out(shape *) will do what with the pointer to the ABC?
	it can call some mwthod that offsprings have implemented
	if such a method exists. If not, it can not be done and the
	out for the offspiring value has to be created

	It is not logical to force the method ono the ABC for the
	implementation convenince ... that is yet another bad
	consequence of using inhertiance at all.

	Example: it seems very logical to have method "rotate()"
	on the shape ABC. But then what is rotate() implementation
	for the circle ?

	This is the logic and this is the design.

	Users can not creates specializations of top level out< T > ( T )
	with reference or pointer type argument.
	*/

	/// <summary>
	/// in here we essentially mimic the standard C++ 
	/// type hierarchy as described in here
	/// https://en.cppreference.com/w/cpp/language/type
	/// 
	/// the one and only, only the lonely ...etc ...
	/// </summary>
	template <typename T>
	inline void out([[maybe_unused]] T specimen)
	{
		using actual = dbj::tt::actual_type<T>;

#ifdef DBJ_TYPE_INSTRUMENTS
		using argument_instrument = dbj::tt::instrument<T>;
		PRN.printf("\n%-20s", "Master dispatcher");
		DBJ_TYPE_REPORT_FUNCSIG;
#endif
		// dbj_type_report<argument_type>( &console_ );

		if constexpr (std::is_fundamental_v< dbj::tt::actual_type<T>::unqualified >) {

			if constexpr (std::is_arithmetic_v< actual::unqualified_type >) {
				if constexpr (std::is_floating_point_v< actual::unqualified >) {
					DBJ_UNHANDLED_("floating point");
				}
				else if constexpr (std::is_integral_v< actual::unqualified >) {
					DBJ_UNHANDLED_("integral");
				}
			}
			else {
				DBJ_UNHANDLED_("fundamental");
			}
		}
		else if constexpr (std::is_compound_v< actual::unqualified >) {

			if constexpr (std::is_pointer_v< actual::unqualified >) {

				if constexpr (dbj::is_std_char_v<actual::not_ptr>)
				{
					DBJ_UNHANDLED_("string literal");
				}
				else {
					// just a pointer, so print it's address
					PRN.printf("%p", specimen);
				}
			}
			else if constexpr (std::is_member_pointer_v< actual::unqualified >) {
				// std::is_member_object_pointer
				// std::is_member_function_pointer
				PRN.printf("%p", specimen);
			}
			else if constexpr (std::is_array_v< actual::unqualified >) {
				DBJ_UNHANDLED_("array");
			}
			else if constexpr (std::is_function_v< actual::unqualified >) {
				DBJ_UNHANDLED_("function");
			}
			else if constexpr (std::is_enum_v< actual::unqualified >) {
				DBJ_UNHANDLED_("enum");
			}
			else if constexpr (std::is_class_v< actual::unqualified >) {
				DBJ_UNHANDLED_("class");
			}
			else if constexpr (std::is_union_v< actual::unqualified >) {
				DBJ_UNHANDLED_("union");
			}
			else {
				DBJ_UNHANDLED_("compound");
			}
		}
		else {
			DBJ_UNHANDLED_(" ");
		}
	}

	// the one out for the native array references
	template <typename T, size_t N>
	void out(T(&native_array)[N])
	{
		DBJ_TYPE_REPORT_FUNCSIG;

		using base_type = dbj::tt::to_base_t<T>;

		if constexpr (
			std::is_same_v< base_type, char >
			) {
			PRN.char_to_console((const char*)native_array);
		}
		else
			if constexpr (
				std::is_same_v< base_type, wchar_t >
				)
			{
				PRN.wchar_to_console((const wchar_t*)native_array);
			}
			else
			{
				// try to output as native 
				// array of any other type
				PRN.wchar_to_console(wprefix_str);
				for (auto&& elem : native_array) {
					PRN.wchar_to_console(wspace_str);
					out(elem);
				}
				PRN.wchar_to_console(wsuffix_str);
			}
	}

	// pointer to native array?
	template <typename T, size_t N>
	inline void out(T(*native_array)[N])
	{
		DBJ_TYPE_REPORT_FUNCSIG;
		out<T, N>(*native_array);
	}

	// 2019-02-03 DBJ
	// if someone says print(pp) and pp is a pointer-pointer
	// we assume address display is required
	template <typename T>
	inline void out(T** specimen)
	{
		DBJ_TYPE_REPORT_FUNCSIG;
		_ASSERTE(specimen);
		_ASSERTE(*specimen);
		PRN.printf("%p, %p", specimen, *specimen);
	}

	// stings and string literals are different by design ***********************************************
	template<> inline void out< char*>(char* str) { DBJ_TYPE_REPORT_FUNCSIG;  _ASSERTE(str != nullptr); PRN.char_to_console(str); }
	template<> inline void out< wchar_t*>(wchar_t* str) { DBJ_TYPE_REPORT_FUNCSIG; _ASSERTE(str != nullptr); PRN.wchar_to_console(str); }
	template<> inline void out< char16_t*>(char16_t* str) { DBJ_TYPE_REPORT_FUNCSIG; _ASSERTE(str != nullptr); PRN.wchar_to_console(dbj::range_to_wstring(str).c_str()); }
	template<> inline void out< char32_t*>(char32_t* str) { DBJ_TYPE_REPORT_FUNCSIG; _ASSERTE(str != nullptr); PRN.wchar_to_console(dbj::range_to_wstring(str).c_str()); }

	template<> inline void out<const char*>(const char* str) { DBJ_TYPE_REPORT_FUNCSIG;  _ASSERTE(str != nullptr); PRN.char_to_console(str); }
	template<> inline void out<const wchar_t*>(const wchar_t* str) { DBJ_TYPE_REPORT_FUNCSIG; _ASSERTE(str != nullptr); PRN.wchar_to_console(str); }
	template<> inline void out<const char16_t*>(const char16_t* str) { DBJ_TYPE_REPORT_FUNCSIG; _ASSERTE(str != nullptr); PRN.wchar_to_console(dbj::range_to_wstring(str).c_str()); }
	template<> inline void out<const char32_t*>(const char32_t* str) { DBJ_TYPE_REPORT_FUNCSIG; _ASSERTE(str != nullptr); PRN.wchar_to_console(dbj::range_to_wstring(str).c_str()); }
	// std strings
	template<> inline void out< std::string >(std::string  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.char_to_console(str.c_str());
	}
	template<> inline void out< std::wstring >(std::wstring  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.cons().out(str.data(), str.data() + str.size());
	}
	template<> inline void out< std::u16string >(std::u16string  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.wchar_to_console(dbj::range_to_wstring(str).c_str());
	}
	template<> inline void out< std::u32string >(std::u32string  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.wchar_to_console(dbj::range_to_wstring(str).c_str());
	}

	// std string views
	template<> inline void out< std::string_view >(std::string_view  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.char_to_console(str.data());
	}
	template<> inline void out< std::wstring_view >(std::wstring_view  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.cons().out(str.data(), str.data() + str.size());
	}
	template<> inline void out< std::u16string_view >(std::u16string_view  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.wchar_to_console(dbj::range_to_wstring(str).c_str());
	}

	// DBJ: char32_t is a problem for the latest CL as of 2019-07-22
#ifndef _MSC_VER
	template<> inline void out< std::u32string_view >(std::u32string_view  str)
	{
		DBJ_TYPE_REPORT_FUNCSIG; if (!str.empty()) PRN.wchar_to_console(dbj::range_to_wstring(str).c_str());
	}
#endif

	// fundamental types ********************************************************************************
	template<> inline void out<nullptr_t>(nullptr_t) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("null"); }
	// fundamental - floating point types
	template<> inline void out<float>(float fv) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%f", fv); }
	template<> inline void out<double>(double fv) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%f", fv); }
	template<> inline void out<long double>(long double fv) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%f", fv); }
	// fundamental - integral types
	template<> inline void out<bool>(bool bv) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%s", (bv ? "true" : "false")); }
	// char types are integral types too
	template<> inline void out<char>(char val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%c", val); }
	template<> inline void out<signed char>(signed char val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%c", val); }
	template<> inline void out<unsigned char>(unsigned char val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%c", val); }
	template<> inline void out<wchar_t>(wchar_t val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%c", val); }
	template<> inline void out<char16_t>(char16_t val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%c", val); }
	template<> inline void out<char32_t>(char32_t val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%c", val); }

	/// <summary>
	/// signed integer types (short int, int, long int, long long int);
	/// </summary>
	template<> inline void out<int>(int val) { DBJ_TYPE_REPORT_FUNCSIG;	PRN.printf("%d", val); }
	template<> inline void out<short int>(short int val) { DBJ_TYPE_REPORT_FUNCSIG;	PRN.printf("%d", val); }
	template<> inline void out<long int>(long int val) { DBJ_TYPE_REPORT_FUNCSIG;	PRN.printf("%I32d", val); }
	template<> inline void out<long long int>(long long int val) { DBJ_TYPE_REPORT_FUNCSIG;	PRN.printf("%I64d", val); }

	/// <summary>
	/// unsigned integer types (unsigned short int, unsigned int, unsigned long int, unsigned long long int);
	/// </summary>
	template<> inline void out<unsigned short int>(unsigned short int val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%hu", val); }
	template<> inline void out<unsigned int>(unsigned int val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%hu", val); }
	template<> inline void out<unsigned long int>(unsigned long int val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%I32d", val); }
	template<> inline void out<unsigned long long int>(unsigned long long int val) { DBJ_TYPE_REPORT_FUNCSIG; PRN.printf("%I64d", val); }

	///////////////////////////////////////////////////////////////////////////
	// std classes
	///////////////////////////////////////////////////////////////////////////

	template<> inline void out< std::exception >(std::exception x_) {
		DBJ_TYPE_REPORT_FUNCSIG;
		out(painter_command::bright_red);
		PRN.char_to_console(x_.what());
		paint(painter_command::text_color_reset);
	}

	template<> inline void out< std::runtime_error >(std::runtime_error x_) {
		DBJ_TYPE_REPORT_FUNCSIG;
		out(painter_command::bright_red);
		PRN.char_to_console(x_.what());
		paint(painter_command::text_color_reset);
	}

	template<typename T, typename A	>
	inline void out(const std::vector<T, A>& v_) {
		DBJ_TYPE_REPORT_FUNCSIG;
		if (v_.empty()) return;
		inner::print_range(v_);
	}

	template<typename K, typename V	>
	inline void out(const std::map<K, V>& map_) {
		DBJ_TYPE_REPORT_FUNCSIG;
		if (map_.empty()) return;
		inner::print_range(map_);
	}

	template<typename T, size_t N>
	inline void out(const std::array<T, N>& arr)
	{
		DBJ_TYPE_REPORT_FUNCSIG;
		if (arr.empty()) return;
		inner::print_range(arr);
	}

	template<typename T>
	inline void out(const std::variant<T>& x_) {
		DBJ_TYPE_REPORT_FUNCSIG;
		out(std::get<0>(x_));
	}

	template <class... Args>
	inline void out(const std::tuple<Args...>& tple) {
		DBJ_TYPE_REPORT_FUNCSIG;
		if constexpr (std::tuple_size< std::tuple<Args...> >::value > 0) {
			std::apply([](auto&& ... xs) {	inner::print_varargs(xs...); }, tple);
		}
	}

	template <typename T1, typename T2>
	inline void out(const std::pair<T1, T2>& pair_) {
		DBJ_TYPE_REPORT_FUNCSIG;

		PRN.wchar_to_console(L"{ ");
		out(pair_.first);
		PRN.wchar_to_console(L" , ");
		out(pair_.second);
		PRN.wchar_to_console(L" }");
		// std::apply(	[](auto&&... xs) { inner::print_varargs(xs...);	},	pair_);
	}

	/* output the { ... } aka std::initializer_list<T> */
	template <typename T>
	inline void out(std::initializer_list<T> il_)
	{
		DBJ_TYPE_REPORT_FUNCSIG;
		if (il_.size() < 1) return;
		inner::print_range(il_);
	}

	template< typename T, size_t N >
	inline void out(const std::reference_wrapper< T[N] >& wrp)
	{
		DBJ_TYPE_REPORT_FUNCSIG;
		static_assert(N > 1);
		if (wrp.get() == nullptr) {
			throw std::runtime_error(__FUNCSIG__ " -- reference to dangling pointer");
		}
		else {
			using nativarref = T(&)[N];
			inner::print_range((nativarref)wrp.get());
		}
	}

	template< typename T>
	inline void out(std::optional<T> opt_)
	{
		if (opt_)
			::dbj::console::print("{ value: ", *opt_, " }");
		else
			::dbj::console::print("{ value: empty }");
	}

#pragma region smart pointers 

	/*
	to print the smart pointer of array we need to know the array count
	_countof(array) will not work since smar ptr arr is made on heap
	*/
	template<typename T>
	inline void out
	(::std::pair< size_t, ::std::unique_ptr<T[]> >const& smart_pair_)
	{
		DBJ_TYPE_REPORT_FUNCSIG;
		auto sz_ = smart_pair_.first;
		auto const& sp_ = smart_pair_.second;
		if (!sp_) return;
		inner::print_between(sp_.get(), sp_.get() + sz_);
	}

	// unique_ptr copy is forbiden so it can not
	// act as pass by value argument
	inline void out
	(::std::unique_ptr<char[]> const& smart_charr_)
	{
		::dbj::console::PRN.printf(L"%S", smart_charr_.get());
	}

	inline void out
	(::std::unique_ptr<wchar_t[]> const& smart_wcharr_)
	{
		::dbj::console::PRN.printf(L"%s", smart_wcharr_.get());
	}

	inline void out
	(::std::shared_ptr<char[]> const& smart_charr_)
	{
		::dbj::console::PRN.printf(L"%S", smart_charr_.get());
	}

	inline void out
	(::std::shared_ptr<wchar_t[]> const& smart_wcharr_)
	{
		::dbj::console::PRN.printf(L"%s", smart_wcharr_.get());
	}

#pragma endregion 

#pragma region filesystem

	inline void out
	(::std::filesystem::path const& path_)
	{
		// MSVC STL filesystem uses wchar_t by default
		::dbj::console::PRN.printf(L"%s", path_.c_str());
	}

#pragma endregion 

#pragma region dbj buffers

	/*template<>*/ inline void out/*<::std::vector<char> const & >*/(::std::vector<char> const& vector_charr_buffer_)
	{
		::dbj::console::PRN.printf(L"%S", vector_charr_buffer_.data());
	}

	/*template<>*/ inline void out/*< ::std::vector<wchar_t> const & >*/(::std::vector<wchar_t> const& vector_charr_buffer_)
	{
		::dbj::console::PRN.printf(L"%s", vector_charr_buffer_.data());
	}

	template<size_t N> inline void out/*<::std::array<char, N> const& >*/(::std::array<char, N> const& charr_)
	{
		::dbj::console::PRN.printf(L"%S", charr_.data());

	}

	template<size_t N> inline void out/*< ::std::array<wchar_t, N> const& >*/(::std::array<wchar_t, N> const& charr_)
	{
		::dbj::console::PRN.printf(L"%s", charr_.data());
	}

#ifdef DBJ_DEPRECATED_BUFFERS
	inline void out(typename ::dbj::unique_ptr_buffer_type <char> const& x_) {
		PRN.printf("%s", x_.buffer().get());
	}

	inline void out(typename ::dbj::unique_ptr_buffer_type <wchar_t> const& x_) {
		PRN.printf(L"%s", x_.buffer().get());
	}
#endif // DBJ_DEPRECATED_BUFFERS
#pragma endregion 

#pragma region error codes and options
	// we can not place a friend inside std::error_code, so...
	// using namespace dbj::console;
	inline void out
	(class std::error_code ec_)
	{
		::dbj::console::PRN.printf(
			"Error code { category:'%s', value:%d, message:'%s' }",
			ec_.category().name(),
			ec_.value(),
			ec_.message().c_str()
		);
	}

	inline void out
	(class std::error_condition ecn_)
	{
		::dbj::console::PRN.printf(
			"Error condition { category:'%s', value:%d, message:'%s' }",
			ecn_.category().name(),
			ecn_.value(),
			ecn_.message().c_str()
		);
	}

#pragma endregion
	/*
	template <unsigned Size, char filler = ' '>
	void out__(const dbj::c_line<Size, filler> & cline_) {
		console_.out__(cline_.data());
	}
	*/

	template<typename T1, typename ... T2  >
	inline void print (T1 const& first_param,  T2 ... params)
//	inline auto print = [](auto const& first_param, auto const& ... params)
//		constexpr
	{
#if DBJ_CONSOLE_INIT_REQUIRED
		_ASSERTE(::dbj::console_is_initialized());
#endif
		out(first_param);

		// if there are  more params
		if constexpr (sizeof...(params) > 0) {
			// recurse
			print(params...);
		}
		// return print;
	};

	/*
	CAUTION!
	on using wrong format specifier bellow
	MSVC will *not* be able to warn as it does with printf family
	UCRT dll will simply crash if wrong format specifier is passed
	NOTE!
	As ever on WIN32 this is faster if called with wide format string
	*/
	template <typename T, typename ... Args>
	inline void prinf(T const* format_, Args ... args) noexcept
	{
#if DBJ_CONSOLE_INIT_REQUIRED
		_ASSERTE(::dbj::console_is_initialized());
#endif
		_ASSERTE(format_);
		PRN.printf(format_, args...);
	}

} // dbj::console


/*
to use dbj::console::print on UDT's
you add the required out method here in this file
thus the UDT has to be visible at this point

   namespace dbj::console  {
   // be sure X is not const but the reference is const
	   inline void out ( X const & x_ ) {
			 out(x.name) ;
		  // or the shortcut solution
		  PRN.printf("%s",x.name)
	   }
   };

   ::dbj::console::print(X{}) ; // works

*/
#endif // !_DBJ_CONSOLE_OPS
