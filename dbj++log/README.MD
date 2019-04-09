
# dbj++log
### syslog standard C++ API 

[![syslogwatcher](./media/syslogwatcher.com.jpg)](https://syslogwatcher.com "syslogwatcher.com")
We recomend the "Syslogwatcher". Click on the image for the download.

> The project we started from is Copyright (c) 2008 Secure Endpoints Inc.


> Disclaimer: <br/>
> This release is Proof Of Concept (POC). <br/>
> Use at your own risk.


Currently [RFC3164](https://tools.ietf.org/html/rfc3164) is foollowed. Plan is to develop a support for the newer one: 
[RFC5424](https://tools.ietf.org/html/rfc5424).

### The API

You need a single header to be on the path:
```cpp
#include <dbj++log.h>
```
Everything is in the namespace
```cpp
using namespace ::dbj::log;
```
Currently the user needs to initialise the logging .

```cpp
// targeting the local syslog server, if one is found
	syslog_init();

// targeting the syslog server on the LAN
//	init_syslog("192.168.0.202");
```

Next step is optional. But this is how you control the logging.

```cpp
// careful! max len of the tag is 128
// some syslog servers will move the tag to the message if tag len > 128
// some will truncate it to len 128
syslog_open(module_basename_, syslog_open_options::log_perror );
``` 


Please make sure you have read once the RFC's 3164 and 5424 . (Links are at the top of the doc.)

The dbj++log behaviour is standard, as described on  numerous pages on the web.
`syslog.h` header in here is 99% standard and you are free to browse through it to understand all the options.

Also, dbj++log doesn't interfere if you try to redirect 
cout/cerr/clog 
to somewhere else. The decision is up to you, the library user. 
In case you are developing GUI app, make 
sure you've check out the [example code here](https://gist.github.com/kingseva/a918ec66079a9475f19642ec31276a21).

there is also a `dbj_log_test.h` where we have a simple testing functions, from where you might start, using the syslog(), with added C++ functionality.

#### Decouple the logging from your code

Loging and syslog are cool and best way to develop resilient software.

But be carefull not to introduce too much hard-wired dependancies on the dbj++log (syslog).

We recomend using the set of macros found at the botom of the dbj++log.h, in order to 
decouple a bit more.

```cpp
#define DBJ_LOG_ERR(...) 
#define DBJ_LOG_CRT(...) 
#define DBJ_LOG_LRT(...) 
#define DBJ_LOG_WRG(...) 
#define DBJ_LOG_MCY(...) 
#define DBJ_LOG_DBG(...) 
#define DBJ_LOG_INF(...) 
#define DBJ_LOG_NTC(...)
#define DBJ_LOG_STD_ERR(err,msg_if_err) 
```

So, please use these macros. They are active if DBJ_SYSLOG is defined before 
including dbj++log.h

Alternatively one can avoid the macros. dbj++log.h is quite simple to understand how to do that.

#### std::error_code 

Is logged by using that macro above. Example usage.

```cpp
std::error_code err = some_function() ;_
if (err)
   DBJ_LOG_STD_ERR(err,__FILE__) ;
```
Above logs an syslog error, the std::error_code message, and user defined prompt with it.
#### Async behaviour and Multi Threading

This is all asynchronous as much as windows sockets are asynchronous.
But all the impotrtant functions in here are capable of sequential working, 
in the presence of multiple threads.

#### WARNING: Messages are composed following the RFC3164

Make sure you syslog server can understand them. Which is the case with almost all of them. 
This is simply because there are milions of devices in use (nad will be for a long time to come) who are emitting 
syslog messages in the RFC3164 format. Device on the network, not PC's or laptops.

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

