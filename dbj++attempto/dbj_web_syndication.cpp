#ifdef DBJ_CPP_WINRT_TESTING

#include "pch.h"
/*
https://moderncpp.com/2017/11/01/cppwinrt-in-the-windows-sdk/
https://kennykerr.ca/2018/03/09/cppwinrt-producing-async-objects/
*/
#pragma comment(lib, "windowsapp")
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Syndication.h>

/// <summary>
/// dbj namespace for all things related to WIN RT C++ API
/// <remarks>
/// 1. Be very careful not to have any namespace named "winrt"
/// 2. Be extremely carefull not to #define anything 
/// to equal any cppWINRT name
/// </remarks>
/// </summary>
namespace dbj_samples::wrt {

	constexpr const wchar_t * feed_uri[]{
		L"https://kennykerr.ca/feed",
		L"https://dbj.org/feed"
	} ;

	using namespace winrt;
	using namespace Windows::Foundation;
	using namespace Windows::Web::Syndication;

	/// <summary>
	/// Main Async method to read the feed 
	/// </summary>
	/// <returns>Async object conforming to IAsyncAction interface</returns>
	inline IAsyncAction MainAsync()
	{
		Uri uri( feed_uri[1] );
		SyndicationClient client;
		SyndicationFeed feed = co_await client.RetrieveFeedAsync(uri);

		for (auto&& item : feed.Items())
		{
			hstring title	= item.Title().Text();
			hstring summary = item.Summary().Text();

			WINRT_TRACE("\nTitle:\t", title.data(), "\nSummary:\t", summary.data());
		}
	}

	DBJ_TEST_UNIT(dbj_winrt_web_syndication)
	{
		// hum? 
		winrt::init_apartment();
		//
		MainAsync().get();
	}

}

#endif // DBJ_CPP_WINRT_TESTING