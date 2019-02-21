#include <boost/timer/timer.hpp>

void
spin(int usecs)
{
	struct timeval start, end, diff;
	int ret;

	ret = gettimeofday(&start, NULL);
	assert(!ret);
	while (1) {
		ret = gettimeofday(&end, NULL);
		timersub(&end, &start, &diff);

		if ((diff.tv_sec * 1000000 + diff.tv_usec) >= usecs) {
			break;
		}
	}
}
