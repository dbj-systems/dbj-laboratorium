#pragma once

DBJ_TEST_SPACE_OPEN(app_env_)


DBJ_TEST_UNIT(_application_environment) {

	using dbj::console::print;

	auto application_rt_environment
		= DBJ_TEST_ATOM(dbj::application_environment_data);

	DBJ_TEST_ATOM(application_rt_environment.cli_data);
	print("\nEnv vars found");
	int counter = 0;
	for (auto [ key,val] : application_rt_environment.env_vars)
	{
		::dbj::console::prinf(
			L"\n[%3d] Key: %-12s\t = \tValue: %-22s", counter++, key, val
     	);
	}
}

DBJ_TEST_SPACE_CLOSE

