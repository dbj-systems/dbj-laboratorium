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
*/
template<typename T, size_t H, size_t W>
struct Istorage {
	virtual T & operator () (size_t R, size_t C) const = 0;
};

template<typename T, size_t H, size_t W>
struct stack_storage final : Istorage<T, H, W> {
	T & operator () (size_t R, size_t C) const override
	{
		static auto storage = dbj::lambda_matrix::fast_mtx< T, H, W>();
		return storage(R, C);
	}
};

template<typename T, size_t H, size_t W>
struct heap_storage final : Istorage<T, H, W> {
	T & operator () (size_t R, size_t C) const override
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
	using type = matrix_struct;
	using value_type = T;

	const  size_t H{ HEIGHT };
	const  size_t W{ WIDTH };

	matrix_struct() {};

	T & operator () (size_t R, size_t C) {
		return storage(R, C);
	}
private:
	S<T, HEIGHT, WIDTH> storage;
};

DBJ_TEST_UNIT(test_dbj_mtx)
{
	lambada();

	auto mm_stck = matrix_struct<int, 3, 3, stack_storage >();
	auto mm_heap = matrix_struct<int, 3, 3, heap_storage  >();

	mm_heap(0, 0) = mm_stck(0, 0) = 42;

#ifdef DBJ_LAMBDA_MATRIX_TESTING
	(void)dbj_lambda_matrix_testing();
#endif
}

#pragma endregion  LAMBDA MATRIX