// thread.h: progress counter
//		- launch_counter()
//		- stop_counter()
//

#ifndef _DEFS_PR_COUNTER
#define _DEFS_PR_COUNTER

#include <iostream>
using std::cout ;
using std::endl ;

#include <utility>
#include <thread>
#include <chrono>
#include <future>

class cl_KERNEL {
	private:
		std::promise<void>		_rus{} ;
		std::thread				_thr{} ;

	public:
		explicit cl_KERNEL(std::function<void(std::future<void>, size_t)>, size_t l) ;
		virtual  ~cl_KERNEL() ; // { cout << "\n___ ~ localTh" ; /*if (joinable()) stop(), join() ; */ cout << "\n_ end ~ localTh" ; }

		cl_KERNEL(const cl_KERNEL&) = delete ;
		cl_KERNEL& operator = (const cl_KERNEL&) = delete ;

		cl_KERNEL(cl_KERNEL&& k) noexcept : _thr{std::move(k._thr)}, _rus{std::move(k._rus)} {}
		cl_KERNEL& operator = (cl_KERNEL&& k) noexcept {
														// cout << "\n___ localTh = &&" ;
			_thr = std::move(k._thr), _rus = std::move(k._rus) ;
			return(*this) ;
		}

		std::thread::id get_id() const { return(_thr.get_id()) ; }
		bool			joinable() const { return(_thr.joinable()) ; }
		void			stop() { _rus.set_value() ; }
		void			join() { _thr.join() ; }
}; // cl_KERNEL


void		th_counter(std::future<void> fo, size_t l) ;

#endif

// eoc thread.h