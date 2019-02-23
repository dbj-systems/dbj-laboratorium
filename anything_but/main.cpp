/*
    Type Handle to Avoid Implicit Conversions in standard C++

	Copyright(C) 2019 Dušan B. Jovanović (dbj@dbj.org)

	This program is free software : you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < https://www.gnu.org/licenses/>.
*/
#include <exception>
#include <cstdio>

extern "C" {
	void test_available_types();
	void test_creation();
	void test_assignments();
	void test_compatibility();
}

int main()
{
	using namespace std;
	try {
		test_available_types();
		test_creation();
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
