#include "../adblock.c"

/*

TODO: add tests:

||example.com/banner.gif will block all these addresses

	http://example.com/banner.gif
	https://example.com/banner.gif
	http://www.example.com/banner.gif

while not blocking:

	http://badexample.com/banner.gif
	http://gooddomain.example/analyze?http://example.com/banner.gif

*/

int
main(void)
{
	int status;

	init();

	status = allowrequest("https://tweakers.net/", "https://tweakers.net/adtracker/a");
	printf("%d\n", status);

	status = allowrequest("http://tweakers.net/", "http://tweakers.net/adtracker/a");
	printf("%d\n", status);

	status = allowrequest("https://tweakers.net/", "https://tweakers.net/adtracker.");
	printf("%d\n", status);

	status = allowrequest("https://tweakers.net/", "https://tweakers.net/index.html");
	printf("%d\n", status);

	status = allowrequest("https://360ads.com/", "https://360ads.com/index.html");
	printf("%d\n", status);

	status = allowrequest("https://www.360ads.com/", "https://www.360ads.com/index.html");
	printf("%d\n", status);

	status = allowrequest("http://www.360ads.com/", "http://360ads.com/index.html");
	printf("%d\n", status);

	status = allowrequest("https://360ads.com:8000/", "https://360ads.com/index.html");
	printf("%d\n", status);

	status = allowrequest("https://360ads.com/", "https://360ads.com:8000/index.html");
	printf("%d\n", status);

	status = allowrequest("https://360ads.com:8000/", "https://360ads.com:8000/index.html");
	printf("%d\n", status);

	status = allowrequest("https://google.com/", "https://google.com/index.html");
	printf("%d\n", status);

	/*http://statics.360ads.com/statics/images/2016/home/t3.png*/

	cleanup();

	return 0;
}
