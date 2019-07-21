#pragma once
/*
DBJ ERRC error type
(c) 2019 by dbj@dbj.org
*/

#include <cstdint>
#include <cstdio>
#include <optional>
#include <cassert>
#include <string_view>
#include <array>
#include <memory>

#include "dbj_id_and_message.h"

namespace dbj::errc
{
	using namespace std;
	using namespace std::literals::string_view_literals;

	/*
	charbuf used inside dbj::errc
	we do not use string as char buffer
	as that is wastefull and slower
	*/
	template<size_t N>
	using charbuf_type = std::array<char, N>;

	using buffer_type = std::shared_ptr<char>;

	namespace inner {

		/*
		from string literal make compile time char buffer inside std::array

		constexpr auto buffer_1 = inner::char_buff("Hola Lola!");
		constexpr std::array<char, buffer_1.size() > buffer_2 = buffer_1;
		*/
		template<size_t N >
		constexpr auto char_buff(const char(&sl_)[N])
		{
			std::array<char, N + 1 > buffer_{ { 0 } };
			size_t k = 0;
			for (auto chr : sl_) { buffer_[k++] = chr; }
			return buffer_;
		}

		inline char const* to_string(buffer_type buf) { return buf.get(); }

		template<typename ... Args, size_t max_arguments = 255>
		inline auto
			format(char const* format_, Args ... args)
			noexcept
		{
			static_assert(sizeof...(args) < max_arguments, "\n\nmax 255 arguments allowed\n");
			assert(format_);
			// 1: what is the size required
			size_t size = 1 + std::snprintf(nullptr, 0, format_, args...);
			assert(size > 0);
			// 2: use it at runtime
			auto buf = buffer_type(new char[size + 1]);
			// 
			size = std::snprintf(buf.get(), size, format_, args...);
			assert(size > 0);

			return buf;
		}

		inline void buffer_test()
		{
			assert(format("Hello, %s !", "World").get());
		}
	} // inner


	struct location_type {
		using line_number_type = std::uint32_t;
		using file_name_type = buffer_type;

		line_number_type line{};
		file_name_type   file{};

		location_type(line_number_type line_, const char* file_)
			: line(line_),
			file(inner::format("%s", file_))
		{ }

		// note the optional return type
		static optional<location_type> opt(
			line_number_type& line_,
			const char* file_)
		{
			return location_type{ line_, file_ };
		}

		static buffer_type json(optional<location_type>& oploc_) {
			if (!oploc_) return {}; // on nullopt return nullptr to char
			location_type& loc_ = *oploc_;
			return inner::format(
				R"( "location" : { "line" : "%d", "file" : "%s"} )", loc_.line, loc_.file.get()
			);
		}
	};

	struct error_type final
	{
		using id_type = std::int32_t;
		using message_type = buffer_type;

		id_type			id;
		message_type	msg;
		optional<location_type> location = nullopt;

		error_type(
			id_type id_, message_type msg_, optional<location_type> location_
		) : id(id_),
			msg(msg_),
			location(location_)
		{}

		error_type(const error_type& other_) :
			id(other_.id),
			msg(other_.msg),
			location(other_.location)
		{		}

		error_type& operator=(const error_type& other) {
			id = other.id;
			msg = other.msg;
			location = other.location;
			return *this;
		}

		error_type(error_type&& arg) :
			id(std::move(arg.id)),
			msg(std::move(arg.msg)),
			location(std::move(arg.location))
		{}

		error_type& operator=(error_type&& other) {
			id = std::move(other.id);
			msg = std::move(other.msg);
			location = std::move(other.location);
			return *this;
		}

		template<typename ... A>
		static constexpr error_type make(id_type id_, char const* msg_fmt_, A ... args_)
		{
			error_type err{ id_, {inner::format(msg_fmt_, args_...)} , {nullopt} };
			return err;
		}

		/*
		by 'locate' we mean add location to the error
		*/
		static error_type& locate(
			error_type& error_,
			std::uint32_t line_num_,
			const char* file_path_name_
		)
		{
			assert(file_path_name_);
			// location already exist?
			assert(error_.location == nullopt);
			error_.location = location_type(line_num_, file_path_name_);
			return error_;
		}

		/*
		return the error in a json format
		*/
		static buffer_type json(error_type& error_) noexcept
		{
			buffer_type loc_json{ location_type::json(error_.location) };

			return inner::format
			(
				R"({ "id": "%d", "message" : "%s"%c %s })",
				error_.id,
				error_.msg.get(),
				(loc_json ? ',' : ' '),
				(loc_json ? loc_json.get() : "")
			);
		}
	};
} // dbj:errc