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
#include <memory>

namespace dbj::errc 
{
	using namespace std;
	using namespace std::literals::string_view_literals;

	namespace inner {

		/*
		charbuf used inside dbj::errc
		we do not use string as char buffer
		as that is wastefull and slower
		*/
		using buffer_type = std::shared_ptr<char>;

		inline char const* to_string(buffer_type buf) { return buf.get();  }

		template<typename ... Args, size_t max_arguments = 255>
		inline auto
			format(char const *  format_, Args ... args)
			noexcept
		{
			static_assert(sizeof...(args) < max_arguments, "\n\nmax 255 arguments allowed\n");
			assert( format_ );
			// 1: what is the size required
			size_t size = 1 + std::snprintf(nullptr, 0, format_, args...);
			assert(size > 0);
			// 2: use it at runtime
			auto buf = buffer_type( new char[size + 1]);
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
		using line_number_type = std::uint_fast32_t ;
		using file_name_type = typename inner::buffer_type ;

		line_number_type line{};
		file_name_type   file{};

		// note the optional return type
		static optional<location_type> opt(
			line_number_type & line_,
			file_name_type   & file_)
		{
			return location_type{ line_, file_ };
		}

		static inner::buffer_type json( optional<location_type> & oploc_ ) {
			if (!oploc_) return {}; // on nullopt return nullptr to char
				location_type & loc_ = *oploc_;
			return inner::format(
				R"( "location" : { "line" : "%d", "file" : "%s"} )", loc_.line, loc_.file.get() 
			);
		}
	};

	struct error_type final
	{
		using id_type = std::int32_t ;
		using message_type = inner::buffer_type;

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
			id (other_.id),
			msg (other_.msg),
			location (other_.location)
			{
		}

		// Simple move assignment operator
		error_type& operator=(const error_type & other) {
			id = other.id ;
			msg = other.msg ;
			location = other.location ;
			return *this;
		}

		// Simple move constructor
		error_type(error_type&& arg) : 
			id(std::move(arg.id)),
			msg(std::move(arg.msg)),
			location(std::move(arg.location))
		{}

		// Simple move assignment operator
		error_type& operator=(error_type&& other) {
			id = std::move(other.id);
			msg = std::move(other.msg);
			location = std::move(other.location);
			return *this;
		}

	

		template<typename ... A>
		static error_type make( id_type id_, char const * msg_fmt_, A ... args_ ) 
		{
			error_type err{ id_, {inner::format(msg_fmt_, args_...)} , {nullopt} };
			return err;
		}

		/*
		by 'locate' we mean add location to it
		*/
		template<typename ... A>
		static error_type& locate(error_type & error_, 
			location_type::line_number_type line_num_ ,
			char const * file_path_name_
		) 
		{
			assert(file_path_name_);
			// location already exist?
			assert( error_.location == nullopt );
			error_.location = location_type{ line_num_, inner::format(file_path_name_) };
			return error_;
		}

/*
return the error in a json format
*/
		static inner::buffer_type json(error_type & error_) noexcept
		{
			inner::buffer_type loc_json{ location_type::json(error_.location)  };
			return inner::format
			(
				R"({ "id": "%d", "message" : "%s"%c %s })",
				error_.id ,
				error_.msg.get(),
				(loc_json ? ',' : ' ' ),
				(loc_json ? loc_json.get() : "" )
			);
		}
	};
} // dbj:errc