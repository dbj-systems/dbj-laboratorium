#pragma once

#include <valstat>

namespace dbj::num {

    using fibo_type = int64_t;

    /*
    extern "C" constexpr fibo_type fibo(fibo_type n)
    {
        fibo_type c = 0, a = 1, b = 1;
        for (fibo_type i = 3; i <= n; i++) {
            c = a + b;
            a = b;
            b = c;
        }
        return b;
    }
    int64 can hold fibonacci(N) up to N = 64
    we out the output into the table 0 .. 93 long = 94 elements
    */

    constexpr auto fibo_table_size = 94;

    constexpr int64_t fibo_table_64[fibo_table_size]{
     1 ,   1 ,   1 ,   2 ,   3 ,   5 ,   8 ,   13 ,   21 ,   34 ,   55 ,   89 ,   144 ,   233 ,   377 ,   610 ,
     987 ,   1597 ,   2584 ,   4181 ,   6765 ,   10946 ,   17711 ,   28657 ,   46368 ,   75025 ,   121393 ,
     196418 ,   317811 ,   514229 ,   832040 ,   1346269 ,   2178309 ,   3524578 ,   5702887 ,   9227465 ,
     14930352 ,   24157817 ,   39088169 ,   63245986 ,   102334155 ,   165580141 ,   267914296 ,   433494437 ,
     701408733 ,   1134903170 ,   1836311903 ,   2971215073 ,   4807526976 ,   7778742049 ,   12586269025 ,
     20365011074 ,   32951280099 ,   53316291173 ,   86267571272 ,   139583862445 ,   225851433717 ,
     365435296162 ,   591286729879 ,   956722026041 ,   1548008755920 ,   2504730781961 ,   4052739537881 ,
     6557470319842 ,   10610209857723 ,   17167680177565 ,   27777890035288 ,   44945570212853 ,   72723460248141 ,
     117669030460994 ,   190392490709135 ,   308061521170129 ,   498454011879264 ,   806515533049393 ,
     1304969544928657 ,  2111485077978050 ,   3416454622906707 ,   5527939700884757 ,   8944394323791464 ,
     14472334024676221 ,  23416728348467685 ,   37889062373143906 ,   61305790721611591 ,   99194853094755497 ,
     160500643816367088 ,  259695496911122585 ,   420196140727489673 ,   679891637638612258 ,   1100087778366101931 ,
     1779979416004714189 ,  2880067194370816120 ,   4660046610375530309 ,   754011380474634642 };

    // declare the valstat type
    // status is POSIX error code
    using valstat_fibo = std::valstat< fibo_type, std::errc >;

    // the outcome is very safe and simple, compile time fibonacci
    constexpr inline valstat_fibo fibo(fibo_type n) noexcept
    {
        if (n < 0 || n > fibo_table_size)
            return { {} , std::errc::invalid_argument };

        return { fibo_table_64[n], {} }; // signal OK
    }

    // instant compile time test
    // structured binding can not be constexpr
    constexpr auto vstat_ = fibo(93);
    static_assert(vstat_.value, "OK: fibo(N), max N == 93 ");

    static_assert(fibo(93).value, "OK: fibo(N), max N == 93 ");

#ifdef DBJ_FIBONACCI_TESTING

    DBJ_TEST_UNIT(dbj_fibonacci_testing)
    {
        // run time usage
        if (auto [val, stat] = fibo(42); val)
        {
            // val is retuned, OK state is signaled
            ::printf("\nfibo(42) ==  %I64d ", *val);
        }
        else {
            // no val returned thus the error state is signalled
            ::printf("\nPOSIX errno: %d", *stat);
        }

        using dbj::posix::e_to_s;
        // let's try a sequence of valstat calls
        // show the benefits of new if syntax
        if (auto [val, stat] = fibo(41); stat) printf("error: %s", e_to_s(*stat));
        else // proceed
            if (auto [val, stat] = fibo(42); stat) printf("error: %s", e_to_s(*stat));
            else
                if (auto [val, stat] = fibo(43); stat) printf("error: %s", e_to_s(*stat));
                else
                    printf("And the final fibo(43) == %I64d", *val);
     };

#endif // DBJ_FIBONACCI_TESTING

/*
the "clever" factorial is just a lookup,
since for 32bit int overflow happens for 13!

fact values source: http://www.tsm-resources.com/alists/fact.html

reminder:
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
*/

    constexpr std::int32_t factorials32[]{ 1, 1, 2, 6, 24, 120, 720,
            5040, 40320, 362880, 3628800, 39916800, 479001600 };

    constexpr  
        inline std::valstat< int32_t, std::errc >
        fact32( std::int32_t i ) noexcept
    {
        if (i < 0U || i> 12U) {
            return { {}, std::errc::result_out_of_range  };
        }
        return { factorials32[i] , {} };
    }

    constexpr  std::int64_t factorials64[]
    { 1,
            1,
            2,
            6,
            24,
            120,
            720,
            5040, 40320, 362880, 3628800, 39916800, 479001600,
            6227020800, /*13!*/
            87178291200,
            1307674368000,
            20922789888000,
            355687428096000,
            6402373705728000,
            121645100408832000,
            2432902008176640000 /*20!*/
    };
    
    constexpr
        inline std::valstat< int64_t, std::errc >
        fact64(std::int64_t i) {

        if (i < 0U || i> 20U) {
            return { {}, std::errc::result_out_of_range };
        }
        return { factorials64[i], {} };
    }
} // nspace