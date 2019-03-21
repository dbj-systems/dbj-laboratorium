#include "dbj_util.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

/*
old fashioned timestamp as required by RFC3164
*/
void dbj_timestamp_rfc3164( char (*timestamp_rfc3164)[0xFF], int require_milli_secs ) 
{
	_ASSERTE(timestamp_rfc3164);

	static char const * month[] 
		= { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
						"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	SYSTEMTIME stm;
	GetLocalTime(&stm);

	unsigned len = 0;
	/* THIS IS https://tools.ietf.org/html/rfc3164 FORMAT */
	if (require_milli_secs == 0) {
		len = sprintf_s(*timestamp_rfc3164, sizeof(*timestamp_rfc3164),
			"%s %2d %02d:%02d:%02d",
			month[stm.wMonth - 1], stm.wDay, stm.wHour, stm.wMinute, stm.wSecond);
	}
	else {
		len = sprintf_s(*timestamp_rfc3164, sizeof(*timestamp_rfc3164),
			"%s %2d %02d:%02d:%02d.%03d",
			month[stm.wMonth - 1], stm.wDay, stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
	}
	_ASSERTE(len);
}
