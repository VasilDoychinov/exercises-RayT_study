// progress counter.h: just a simple progress counter
//		- launch_counter()
//		- stop_counter()
//

#ifndef _DEFS_PR_COUNTER
#define _DEFS_PR_COUNTER

#include <iomanip>
#include <iostream>
using std::cout ;
using std::endl ;

#include <utility>
#include <thread>
#include <chrono>
#include <future>

#include "../miscellaneous.h"
// #include "miscellaneous.h"

class cl_KERNEL {
	private:
		std::promise<void>		_pro{} ;
		std::thread				_thr{} ;

	public:
		explicit cl_KERNEL(std::function<void(std::future<void>, size_t)>, size_t l) ;
		virtual  ~cl_KERNEL() ;

		cl_KERNEL(const cl_KERNEL&) = delete ;
		cl_KERNEL& operator = (const cl_KERNEL&) = delete ;

		cl_KERNEL(cl_KERNEL&& k) noexcept : _thr{std::move(k._thr)}, _pro{std::move(k._pro)} {}
		cl_KERNEL& operator = (cl_KERNEL&& k) noexcept {
														// cout << "\n___ localTh = &&" ;
			_thr = std::move(k._thr), _pro = std::move(k._pro) ;
			return(*this) ;
		}

		std::thread::id get_id() const { return(_thr.get_id()) ; }
		bool			joinable() const { return(_thr.joinable()) ; }
		void			stop() { _pro.set_value() ; }
		void			join() { _thr.join() ; }
}; // cl_KERNEL


void		th_counter(std::future<void> fo, size_t l) ;

bool		is_task_ready(const std::future<void>* result) ;

#endif
// eof progress counter.h