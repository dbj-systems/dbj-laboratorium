#pragma once

#if 1

#include "dbj_swapable_engines.h"
#include "dbj_polymorph.h"

namespace dbj_lambada_mixer {

	enum class engine_tags : char { legacy = 'L', contemporary = 'C', hybrid = 'H' };

	enum class wheel_tags : char { alloy = 'A', chrome_vanadium = 'C', steel = 'S' };

	/*
	car_base_facade -- not good using the term "car_base" it implies there might be some hiddent inheritance somewhere
	car_interface -- even worse ;)
	*/
	struct car_front final {
		//the 'start()' message
		template<typename T>
		constexpr bool start(const T & engine_) const
		{
			return engine_.start();
		}

		template<typename W>
		constexpr int knobs_required(const W & wheel_) const
		{
			return wheel_.fix_points();
		}

		template<typename W, typename E>
		constexpr const char * type_tag(const W & wheel_, const E & engine_) const
		{
			static char car_tag[]{ (char)engine_.uid(), '-', (char)wheel_.uid(), (char)0 };
			return car_tag;
		}

	};

	namespace wheels {
		// wheels
		struct alloy final {
			constexpr wheel_tags uid() const { return wheel_tags::alloy; }
			constexpr int fix_points() const { return 3; }
		};

		struct chrome_vanadium final {
			constexpr wheel_tags uid() const { return wheel_tags::chrome_vanadium; }
			constexpr int fix_points() const { return 6; }
		};

		struct steel final {
			constexpr wheel_tags uid() const { return wheel_tags::steel; }
			constexpr int fix_points() const { return 4; }
		};

	} // wheels

	namespace engines {
		// engines
		struct diesel final {
			constexpr engine_tags uid() const { return engine_tags::legacy; }
			constexpr bool start() const { return true; }
		};

		struct petrol final {
			constexpr engine_tags uid() const { return engine_tags::contemporary; }
			constexpr bool start() const { return true; }
		};

		struct hybrid final {
			constexpr engine_tags uid() const { return engine_tags::hybrid; }
			constexpr bool start() const { return true; }
		};
	} // engines

	inline auto car_assembly_line = [](auto engine_, auto wheels_)
	{
		// assemble and use the car perhaps this way
		/*
		car_facade base{ };

		auto starter   =   [=]() { return base.start(engine_);  };
		auto identty  =    [=]() { return base.type_tag(wheels_,engine_);  };
		auto knobs    =    [=]() { return 4 * base.knobs_required(wheels_);  };

		auto tuple_of_methods = std::make_tuple( starter, identty,  knobs );

		auto rv1 = std::get< 0 >(tuple_of_methods)();
		auto rv2 = std::get< 1 >(tuple_of_methods)();
		auto rv3 = std::get< 2 >(tuple_of_methods)();

		or this way ...
		*/
		using ET = decltype (engine_);
		using WT = decltype (wheels_);

		struct finished_car_ final {

			ET engine{};
			WT wheels{};

			car_front front{};

			auto start() { return front.start(engine); }
			auto tag() { return front.type_tag(wheels, engine); }
			auto knobs() { return 4 * front.knobs_required(wheels); };

			operator std::string() { return this->tag(); }
		};

		return finished_car_{};
	};

	DBJ_TEST_UNIT(_polymorph_but_not_inheritor)
	{
		// obtain the car
		auto diesel_car = (
			car_assembly_line(engines::diesel{}, wheels::alloy{})
			);
		auto petrol_car = (
			car_assembly_line(engines::petrol{}, wheels::chrome_vanadium{})
			);

		DBJ_TEST_ATOM(diesel_car.start());
		DBJ_TEST_ATOM(diesel_car.tag());
		DBJ_TEST_ATOM(diesel_car.knobs());

		DBJ_TEST_ATOM(petrol_car.start());
		DBJ_TEST_ATOM(petrol_car.tag());
		DBJ_TEST_ATOM(petrol_car.knobs());

		auto ht1 = DBJ_TEST_ATOM(typeid(diesel_car).hash_code());
		auto ht2 = DBJ_TEST_ATOM(typeid(petrol_car).hash_code());

		auto DBJ_MAYBE(same) = DBJ_TEST_ATOM(ht1 == ht2);

		dbj::console::print("\ndiesel and petrol car are ");

		if (!std::is_same_v< decltype(diesel_car), decltype(petrol_car) >) {
			dbj::console::print(" NOT ");
		}

		dbj::console::print(" the same type...");

		// problem?
	}

	DBJ_TEST_UNIT(_dbj_swappable_engines_) {

		car_factory::Automobile car_1
			= DBJ_TEST_ATOM(car_factory::assembly_line(car_factory::engine_tag::old));
		car_1.start();

		car_factory::Automobile car_2
			= DBJ_TEST_ATOM(car_factory::assembly_line(car_factory::engine_tag::next));
		car_2.start();

		// should be barred
		// and it is: car_1 = car_2 ;
	}

} // namespace dbj_lambada_mixer 
#endif // 0