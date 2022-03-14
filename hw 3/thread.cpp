// thread.cpp
//

#include <time.h>
#include "thread.h"


																		// class cl_KERNEL

cl_KERNEL::cl_KERNEL(std::function<void(std::future<void> p, size_t)> service, size_t l)
					: _rus{}, _thr{}
{
	std::future<void>	futureSig = _rus.get_future() ;
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
	while (stopSignaled(std::ref(fo), lapse) == false) {
		cout << "> " ;
	}
	cout << endl << "> Done..." ;
} // th_kernel()

/*
cl_KERNEL
launch_th_counter()
{
	cl_KERNEL			ker(th_counter) ;
	std::promise<void>	stop ;
	std::future<void>	futureSig = stop.get_future() ;

	std::thread		lth(th_counter, std::move(futureSig)) ;

	return(ker) ;   // the created
} // launch_th_counter()

void stop_th_counter(std::promise<void> s) { s.set_value() ; }
*/

// eof thread.cpp