#pragma once
/*
(c) 2019 by dbj.systems, author: dbj@dbj.org, Licence GPLv3

syslog + local file log 
-----------------------
+ stderr redirection to the same file

Both async and both following the syslog protocol
Currently RFC3164 is foollowed. Plas is to develop a support for the newer one: RFC5424.

syslog project started from is Copyright (c) 2008 Secure Endpoints Inc.

*/

#include "syslog/syslog.h"

#ifdef DBJ_LOG_TESTING
#include "test/dbj_log_test.h"
#endif

