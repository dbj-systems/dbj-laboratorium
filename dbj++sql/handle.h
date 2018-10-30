#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <crtdbg.h>

namespace dbj::sqlite {

	using namespace std;

	struct sql_exception
	{
		using message_arg_type = char const *;
		int code;
		string message;

		sql_exception(int const result, message_arg_type text) :
			code{ result },
			message{ text }
		{}
	};


}

#ifndef DBJ_STR
#define DBJ_STR(x) #x
#endif

#ifndef DBJ_VERIFY_
#define DBJ_VERIFY_(R,X) if ( R != X ) \
   throw dbj::sqlite::sql_exception( int(R), __FILE__ "(" DBJ_STR(__LINE__) ")\n" DBJ_STR(R) " != " DBJ_STR(X))
#endif

namespace KennyKerr
{
    template <typename Traits>
    class unique_handle
    {
        using pointer = typename Traits::pointer;

        pointer m_value;

        auto close() noexcept -> void
        {
            if (*this)
            {
                Traits::close(m_value);
            }
        }

    public:

        unique_handle(unique_handle const &) = delete;
        auto operator=(unique_handle const &) -> unique_handle & = delete;

        explicit unique_handle(pointer value = Traits::invalid()) noexcept :
            m_value { value }
        {
        }

        unique_handle(unique_handle && other) noexcept :
            m_value { other.release() }
        {
        }

        auto operator=(unique_handle && other) noexcept -> unique_handle &
        {
            if (this != &other)
            {
                reset(other.release());
            }

            return *this;
        }

        ~unique_handle() noexcept
        {
            close();
        }

        explicit operator bool() const noexcept
        {
            return m_value != Traits::invalid();
        }

        auto get() const noexcept -> pointer
        {
            return m_value;
        }

        auto get_address_of() noexcept -> pointer *
        {
            _ASSERTE(!*this);
            return &m_value;
        }

        auto release() noexcept -> pointer
        {
            auto value = m_value;
            m_value = Traits::invalid();
            return value;
        }

        auto reset(pointer value = Traits::invalid()) noexcept -> bool
        {
            if (m_value != value)
            {
                close();
                m_value = value;
            }

            return static_cast<bool>(*this);
        }

        auto swap(unique_handle<Traits> & other) noexcept -> void
        {
            std::swap(m_value, other.m_value);
        }
    };

    template <typename Traits>
    auto swap(unique_handle<Traits> & left,
              unique_handle<Traits> & right) noexcept -> void
    {
        left.swap(right);
    }

    template <typename Traits>
    auto operator==(unique_handle<Traits> const & left,
                    unique_handle<Traits> const & right) noexcept -> bool
    {
        return left.get() == right.get();
    }

    template <typename Traits>
    auto operator!=(unique_handle<Traits> const & left,
                    unique_handle<Traits> const & right) noexcept -> bool
    {
        return left.get() != right.get();
    }

    template <typename Traits>
    auto operator<(unique_handle<Traits> const & left,
                   unique_handle<Traits> const & right) noexcept -> bool
    {
        return left.get() < right.get();
    }

    template <typename Traits>
    auto operator>=(unique_handle<Traits> const & left,
                    unique_handle<Traits> const & right) noexcept -> bool
    {
        return left.get() >= right.get();
    }

    template <typename Traits>
    auto operator>(unique_handle<Traits> const & left,
                   unique_handle<Traits> const & right) noexcept -> bool
    {
        return left.get() > right.get();
    }

    template <typename Traits>
    auto operator<=(unique_handle<Traits> const & left,
                    unique_handle<Traits> const & right) noexcept -> bool
    {
        return left.get() <= right.get();
    }

    struct null_handle_traits
    {
        using pointer = HANDLE;

        static auto invalid() noexcept -> pointer
        {
            return nullptr;
        }

        static auto close(pointer value) -> void
        {
            DBJ_VERIFY_(TRUE, CloseHandle(value));
        }
    };

    struct invalid_handle_traits
    {
        using pointer = HANDLE;

        static auto invalid() noexcept -> pointer
        {
            return INVALID_HANDLE_VALUE;
        }

        static auto close(pointer value) -> void
        {
            DBJ_VERIFY_(TRUE, CloseHandle(value));
        }
    };

    using null_handle = unique_handle<null_handle_traits>;
    using invalid_handle = unique_handle<invalid_handle_traits>;
}
