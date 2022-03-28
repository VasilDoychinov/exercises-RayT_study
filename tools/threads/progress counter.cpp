// progress counter.cpp
//

// #include <time.h>
#include "progress counter.h"


																		// class cl_KERNEL
cl_KERNEL::cl_KERNEL(std::function<void(std::future<void> p, size_t)> service, size_t l)
					: _pro{}, _thr{}
{
	std::future<void>	futureSig = _pro.get_future() ;
	_thr =	std::thread(th_counter, std::move(futureSig), l) ;
} // cl_KERNEL ()

cl_KERNEL::~cl_KERNEL()
{
	if (joinable())		{ stop() , join() ; }
} // cl_KERNEL ~
																		// eoc cl_KERNEL
bool inline
stopSignaled(const std::future<void>& fo, size_t lapse)
{
	return(fo.wait_for(std::chrono::milliseconds(lapse)) == std::future_status::timeout ? false : true) ;
} // stopSignaled()

void
th_counter(std::future<void> fo, size_t lapse)
{
	auto t0 = mClock::now() ;

	while (stopSignaled(std::ref(fo), lapse) == false) {
		cout << "> " ;
	}
	cout << "> Done: " << std::setprecision(3) << (mDuration_sec(mClock::now() - t0)).count() << " s";
} // th_kernel()

// eof progress counter.cpp