#pragma once

#include <array>
#include <tuple>
#include <algorithm>

namespace constexpr_format {

    //Utility data structures and functions
    namespace util {

        //Wrapper for std::array<char,N> to not overload operator+ on std::array for anyone using this namespace.
        template<std::size_t N>
        struct static_string {
            std::array<char,N> string;

            constexpr decltype(auto) operator[](std::size_t n) {return string[n];}
            constexpr decltype(auto) operator[](std::size_t n) const {return string[n];}

            constexpr decltype(auto) data() {return string.data();}
            constexpr decltype(auto) data() const {return string.data();}

            constexpr auto size() const {return N;}

            constexpr auto begin() {return string.begin();}
            constexpr auto begin() const {return string.begin();}

            constexpr auto end() {return string.end();}
            constexpr auto end() const {return string.end();}

            constexpr std::array<char,N+1> getNullTerminatedString() {
                return (*this + static_string<1>{{'\0'}}).string;
            }
        };

        template<std::size_t N, std::size_t M>
        constexpr auto operator+(static_string<N> a, static_string<M> b) {
            return std::apply([&](const auto&... as) {
                return std::apply([&](const auto&... bs) {
                    return static_string<N+M>{{as...,bs...}};
                }, b.string);
            }, a.string);
        }

        //Simple constexpr-enabled string_view for views on char arrays
        class string_view {
            const char* data;
            std::size_t n;
        public:
            template<int N>
            constexpr string_view(const char (&init)[N]) : data(init),n(init[N-1]=='\0'?N-1:N) {};

            template<std::size_t N>
            constexpr string_view(const static_string<N>& array) : data(array.string.data()), n(array[N-1]=='\0'?N-1:N) {};

            constexpr string_view(const char* init, std::size_t len) : data(init),n(len) {};

            constexpr string_view(const string_view&) = default;

            constexpr std::size_t size() const {return n;};
            constexpr const char& operator[](int i) const {
                return data[i];
            }
            constexpr auto* begin() const {
                return data;
            }
            constexpr auto* end() const {
                return data+n;
            }

            constexpr std::size_t find(char c) const {
                for(std::size_t i = 0; i < n; ++i) {
                    if (data[i] == c) return i;
                }
                return n;
            }

            constexpr string_view prefix(std::size_t len) const {
                if(len >= n) return *this;
                return {data,len};
            }

            constexpr string_view remove_prefix(std::size_t len) const {
                if(len >= n) return {data,0};
                return {data+len,n-len};
            }
        };

        constexpr bool operator==(const string_view& a, const string_view& other) {
            if(a.size() != other.size()) {
                return false;
            }
            auto start = a.begin();
            for(auto c : other) {
                if (*start++ != c) return false;
            }
            return true;
        }

        template<std::size_t... I>
        constexpr auto view_to_static_impl(string_view s, std::index_sequence<I...>) {
            return static_string<sizeof...(I)>{{s[I]...}};
        }

        template<int N>
        constexpr auto view_to_static(string_view s) {
            return view_to_static_impl(s,std::make_index_sequence<N>{});
        }

        template<typename StringViewF>
        constexpr auto view_to_static(StringViewF s) {
            return view_to_static_impl(s(),std::make_index_sequence<s().size()>{});
        }

        namespace detail {
            template <class F, class TupleF, std::size_t... I>
            constexpr decltype(auto) constexpr_apply_impl(F&& f, TupleF t, std::index_sequence<I...>)
            {
                return std::forward<F>(f)([t]{return std::get<I>(t());}...);
            }
        }

        //std::apply using constexpr lambda idiom
        template <class F, class TupleF>
        constexpr decltype(auto) constexpr_apply(F&& f, TupleF t)
        {
            return detail::constexpr_apply_impl(
                std::forward<F>(f), t,
                std::make_index_sequence<std::tuple_size_v<std::remove_cv_t<decltype(t())>>>{});
        }

        template<typename T, std::size_t N>
        constexpr auto prepend(T t, std::array<T,N> a) {
            return std::apply([&](const auto&... as) {
                return std::array{t,as...};
            },a);
        }

    } // util

    namespace string_udl {
        constexpr auto operator""_sv (const char* c, std::size_t n) {
            return util::string_view(c,n);
        }
    }

}
