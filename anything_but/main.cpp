/*
    Type Handle to Avoid Implicit Conversions in standard C++

  Copyright 2019 Dušan B. Jovanović (dbj@dbj.org)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <exception>
#include <cstdio>


	void test_different_types();
	void test_try_to_trick();
	void test_assignments();
	void test_compatibility();

int main()
{
	using namespace std;
	try {
		test_different_types();
		test_try_to_trick();
		test_assignments();
		test_compatibility();
	}
	catch (exception & x)
	{
		fprintf( stderr, "\n\nstd::exception\n\t%s\n", x.what());

	}
	catch ( ... )
	{
		fprintf(stderr, "\n\nunknown exception\n");
	}

	return 0;
}
