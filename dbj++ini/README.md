# dbj++ini
#### simple ini file parser
Inspired by **inih (INI Not Invented Here)** 
A simple [.INI file](http://en.wikipedia.org/wiki/INI_file) parser written in C. 

**inih** is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

Also please visit that project page to understand all the other various features avalilbale in the C part of this project.

## Standard C++

The purpose of this project is modern C++ interface. It is resilient and fast.
`std::string` use is minimal, and will be completetely removed in the next release.

Future release will be also built with C++exceptions mechanism removed.

### The API

```cpp
#include <dbj++ini.h>
#include <cstdio>

int main()
{
	using namespace std;
	using namespace inih;

	ini_reader const & reader = ini_reader_instance("../examples/test.ini");

    if (reader.parse_error() < 0) {
        fprintf( stderr, "Can't load 'test.ini'\n");
        return 1;
    }
	printf("\n\nConfig loaded from 'test.ini': \nversion=%3d\nname=%16s\nemail=%16s\npi=%3.3f\nactive=%6s"
		, reader.get_integer("protocol", "version", -1) 
		, reader.get("user", "name", "UNKNOWN").get()
		, reader.get("user", "email", "UNKNOWN").get()
		, reader.get_real("user", "pi", -1) 
		, (reader.get_bool("user", "active", true) ? "true" : "false")
	);
```
Return type of the `get()` and `get_string()` methods is 
```cpp
using smart_buffer = std::shared_ptr<char>;
```
To avoid `std::string` we use `std::shared_ptr<char>`. 
`std::string` is not to be used as char buffer, rather it is to be used for 
large(r) text processing in the context of the C++ code.
We also deliberately do not introduce some elaborate wrapper 
to figure on the API, so that users are aware what is used and are already familliar with the `std::shared_ptr`.

We also handle the "ini file lists". As the original project did.

```cpp
	printf("\n\nList in the ini file:\n"
		"\n[list]"
		"\nkey = A"
		"\nkey = B"
		"\nkey = C"
		"\n\nParsed as:\n%s",
		reader.get("list","key", "null").get()
	);

    return 0;
}
```

The output of the above is: `"A\nB\nC"`.

For some other C++ ini file projects please see:

  * https://github.com/Blandinium/inih
  * https://github.com/OSSystems/inih


## Platform-specific notes ##

At the time of this writting, dbj++ini does not include `wchar_t` or Unicode handling.

> This is a static library. 

> Built with Microsoft (R) C/C++ Optimizing Compiler Version 19.15.26732.1 or better.

> (In 2019Q1, standard C++ was C++17)


-------------------------------------

<pre>
Copyright 2017,2018, 2019 by dbj@dbj.org, dbj.org, dbj.systems ltd.

Licensed under the GNU GPL License, Version 3.0 (the "License");
you may not use any of files in this project except in compliance with the License.
You may obtain a copy of the License in the file LICENSE enclosed in
this project.

https://www.gnu.org/licenses/gpl-3.0.html

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
</pre>
---------------------------------------------------------------------  

[![dbj();](http://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  

