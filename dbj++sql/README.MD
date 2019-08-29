
# dbj++sql 

**Version 2.0.0**

#### YAPI (Yet Another API)
C++17 SQLITE(3) for the rest of us

(2019Q1, standard C++ is C++17)

This is independent lib, but logicaly a part of the [**`dbj-laboratorium`** solution](https://github.com/dbj-systems/dbj-laboratorium).

What we think is minimal functionality is implemented. For more we hope you can use this lib as the base.

Currently ( 2019 AUG ) this is MSVC code. "Compiler agnostic", that is CLANG and GNU version, will come out next.

> Disclaimer: <br/>
> Immediatly usable in your projects. <br/>
> This is Proof Of Concept (POC). <br/>
> Use at your own risk.<br/>
> I do.

More documentation is always in the works, if not all published. For immediate professional help please do mail me at: 
[dbj@dbj.org](mailto:dbj@dbj.org)

#### Testing
Is in the separate project called `dbjsqluser`, also part of `dbj-laboratorium` Visual Studio solution.

#### Dependancies
All the code in here is the `dbj::sql` namespace. This is done, so that it differs from the `sqlite` namespace we have made in here.

This is a static library. It depends on SQLITE 3, C library.
It is is linked in, by the virtue of Visual Studio project.  But it is in the separate namespace, as done in `sqlite++.h`
```cpp
#ifdef __cplusplus
namespace sqlite{
#endif
/*
here we use the relative path
and sqlite3 static lib

in case you hotly disagree and wish to use the dll
please download and install some sqlite3 built as dll, 
and include its header bellow
*/
#include "../sqlite3_lib/sqlite3.h"

#ifdef __cplusplus
}
#endif
/*
assumption is you know how to use pragma comment lib
in case you might preffer it.
*/
```

Beside C++ std lib no other external code is required.

### Exception safety

Writing such a code is [not trivial](https://stackoverflow.com/questions/1853243/do-you-really-write-exception-safe-code/1853769#1853769). We are hoping we might help you if you 
need or want to write such a code. 

C++ STD lib code is throwing exceptions. Code we have in here is never throwing exceptions. To signal the status to the callers
we use  `dbj_db_status_type`, please see the `dbj_sql_error_concept.h`

In case of terminating error, we write it to the stderr and call the `exit()`. 

---

Built with Microsoft Visual Studio 2019 and C/C++ Optimizing Compiler Version 19.15.26732.1 or better.


---

```
Copyright 2017,2018, 2019 by dbj.systems uk ltd.

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
```
---------------------------------------------------------------------  

[![dbj();](http://dbj.org/wp-content/uploads/2015/12/cropped-dbj-icon-e1486129719897.jpg)](http://www.dbj.org "dbj")  

