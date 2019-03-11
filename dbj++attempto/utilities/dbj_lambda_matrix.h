#pragma once
#pragma region  LAMBDA MATRIX

// https://stackoverflow.com/a/52473261/10870835
// https://wandbox.org/permlink/wCzrLsPc9k0YErDA

template <typename T>
struct lambda_instance_handle final
{
	T  m_Data;
	lambda_instance_handle() { };
	lambda_instance_handle(T && pInput) : m_Data(std::move(pInput)) {}
	auto & operator()(size_t R, size_t C)
	{
		return m_Data(R, C);
	}
};

template <typename T>
lambda_instance_handle(T &&)->lambda_instance_handle<std::decay_t<T>>;

int lambada()
{
	// two INSTANCES for two storages
	auto sstorage = lambda_instance_handle(dbj::lambda_matrix::mtx<int>(3, 3));
	auto mmstorage = lambda_instance_handle(dbj::lambda_matrix::fast_mtx< int, 3, 3>());

	auto cv1 = mmstorage(0, 0) = 42;
	auto cv2 = sstorage(0, 0) = 42;

	noexcept(cv1, cv2);
	/*
	auto strange_lambda1 = lambda_instance_handle([]{ return "Hello\n"; });
	strange_lambda1();

	auto strange_lambda2 = lambda_instance_handle([]{ return "Goodbye\n"; });
	strange_lambda2();

	strange_lambda1();
	*/
	return 0;
}
/*
How to use lambda matrix as a storage in some matrix class type
note: the simple option
*/

template<typename T, size_t H, size_t W>
struct I_matrix_storage {

	// we do not need to waste cycles on checking on each call
	// inside the operator()'s bellow
	using row_range = ::dbj::core::util::insider 
		< size_t, 0, H, ::dbj::core::util::insider_exit_policy >;
	using col_range = ::dbj::core::util::insider 
		< size_t, 0, W, ::dbj::core::util::insider_exit_policy >;

	using type = I_matrix_storage;
	using value_type = T;

	constexpr static  size_t height { H };
	constexpr static  size_t width  { W };
	constexpr static  size_t weight { sizeof(T) * H * W };

	virtual T & operator () (row_range R, col_range C) const = 0;
};

template<typename T, size_t H, size_t W>
struct stack_storage final : I_matrix_storage<T, H, W> 
{
	using type = stack_storage;
	using base = I_matrix_storage<T, H, W>;

	T & operator () (base::row_range R, base::col_range C) const override
	{
		static auto storage = dbj::lambda_matrix::fast_mtx< T, H, W>();
		return storage(R, C);
	}
};

template<typename T, size_t H, size_t W>
struct heap_storage final : I_matrix_storage<T, H, W> 
{
	using type = heap_storage;
	using base = I_matrix_storage<T, H, W>;

	T & operator () (base::row_range R, base::col_range C) const override
	{
		static auto storage = dbj::lambda_matrix::mtx< T >(H, W);
		return storage(R, C);
	}
};


template<
	typename T, size_t HEIGHT, size_t WIDTH,
	template< typename T, size_t HEIGHT, size_t WIDTH> typename S
>
struct matrix_struct final
{
	// this is a lot of code, but we use from the
	// storage suite whatever we can
	using storage_type = S<T, HEIGHT, WIDTH>;
	using row_idx = typename storage_type::base::row_range;
	using col_idx = typename storage_type::base::col_range;

	using type = matrix_struct;
	using value_type = typename storage_type::base::value_type ;

	constexpr static size_t height{ storage_type::base::height };
	constexpr static size_t width { storage_type::base::width  };

	matrix_struct() {};

	T & operator () (row_idx R, col_idx C) {
		return storage(R, C);
	}
private:
	storage_type storage;
};

DBJ_TEST_UNIT(test_dbj_mtx)
{
	lambada();

	// API is comfortable yet fully functional
	auto mm_stck1 = matrix_struct<int, 3, 3, stack_storage >();
	auto mm_stck2 = matrix_struct<int, 3, 3, stack_storage >();
	auto mm_heap1 = matrix_struct<int, 3, 3, heap_storage  >();
	auto mm_heap2 = matrix_struct<int, 3, 3, heap_storage  >();

	mm_heap1(0, 0) = 100;  mm_stck1(0, 0) = 102;
	mm_heap2(0, 0) = 103;  mm_stck2(0, 0) = 104;

	//asking with an wrong index automagicaly fails
	// but at runtime, so ...
	//try {
	//	mm_heap1(9, 9);
	//}
	//catch (...) {
	//	//
	//}

	// we must have storage private for each instance
	assert( mm_heap1(0,0) != mm_stck1(0,0) != mm_heap2(0,0) != mm_stck2(0,0) ) ;

#ifdef DBJ_LAMBDA_MATRIX_TESTING
	(void)dbj_lambda_matrix_testing();
#endif
}

#pragma endregion  LAMBDA MATRIX
#pragma region  compile time insider
template<
	typename T, T L, T H
>
struct ct_insider
{
	using type = ct_insider;

	template< T X,
		std::enable_if_t< (X <= H) && (X >= L), int> = 0
	> 
		static ct_insider make() { return type{X}; }

	T value{};
};
/*
template< typename T, T X, T L, T H>
    using checked_t = std::conditional< (X <= H) && (X >= L), T, void>;

template<size_t X>
	using inside_10_to_24 = checked_t<size_t, X, 10, 24>;
	*/
template< typename T, T X, T L, T H>
    using inside_t = std::enable_if_t< (X <= H) && (X >= L), std::integral_constant<T, X> >;

template<size_t X>
	using ten_to_24 = inside_t<size_t, X, 10, 24>;

	template<size_t index_, typename I = ten_to_24<index_> >
	auto indexor()
	{
		constexpr I::value_type idx_ = I::value;
		return ten_to_24<idx_ + 1> () ;
	}

	template<typename C, unsigned K>	struct X; // final {};

	template<unsigned K>
	struct X<char, K> final 
	{
		using ascii_ordinal = inside_t<unsigned, K, 0, 127>;
		char value = char(ascii_ordinal::value);
	};


	X<char, 30> a;//here 300 is out of range and I would like to be able to detect that.

DBJ_TEST_UNIT(compile_time_insider)
{
	ten_to_24<11>::value_type eleven = ten_to_24<11>::value ;

	noexcept(eleven);
	// ten_to_24<9>  nine; // does not compile

	auto xxx = indexor<11>();

	using one_to_nine = ct_insider<size_t, 1, 9>;
	auto checker = [] ( one_to_nine ) {
	};

	using s1 = std::integral_constant<size_t, 1>;
	using s9 = std::integral_constant<size_t, 9>;

	using gt = std::bool_constant< (4 > 9) >;
	using lt = std::bool_constant< (4 < 9) >;

	// checker( one_to_nine::make<4>() ) ;
	// error -- checker( one_to_nine::make<14>() );





} // nspace
#pragma endregion  compile time insider


