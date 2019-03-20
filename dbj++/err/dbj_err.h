#pragma once
/*
single include for dbj+err

https://bit.ly/2S36RMp
https://goo.gl/Jm9Lgk

*/
#include "dbj_error_code.h"
#include "dbj_status_code.h"
#include "dbj_p1095.h"

namespace dbj::err {

#if 0
	// compile time
	template<typename T> struct is_dbj_cat	: false_type	{	};

	template<>	struct is_dbj_cat<common_category>	: true_type	{		};
	template<>	struct is_dbj_cat<inner::dbj_status_category>	: true_type	{		};

	template<class T>
	inline constexpr bool is_dbj_cat_v = is_dbj_cat<T>::value;
#endif
	/*
	find out at runtime if code or condition or category belong to dbj::err
	*/
	inline bool is_dbj_err(std::error_category const & cat_)
	{
		if (cat_ == get_common_category() || (cat_ == make_dbj_status_category()))
		{
			return true;
		}
		return false;
	}
	// stop shenanigans with temporaries
	bool is_dbj_err(std::error_category && ) = delete;

	inline bool is_dbj_err( std::error_code const & ec_ )
	{
		return is_dbj_err( ec_.category());
	}
	// stop shenanigans with temporaries
	bool is_dbj_err(std::error_code &&) = delete;

	inline bool is_dbj_err( std::error_condition const & ecn_ )
	{
		return is_dbj_err( ecn_.category());
	}
	// stop shenanigans with temporaries
	bool is_dbj_err(std::error_condition &&) = delete;
}

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
