// miscellaneous.h: miscellaneous tools and types like
//			- measuring intervals
//			- predicates
// 
//

#include <iostream>
using std::cout ;
using std::endl ;

#include <chrono>

#include "miscellaneous.h"


time_t
show_lapse(const bool fl_start)  // returns the lapse (in ...) since the start (fl_start)
{
	static time_t	start_lapse{0} ;
	static time_t	break_point{0} ;
	time_t			end_lapse{0} ;

	if (fl_start) {
		time(&start_lapse), break_point = start_lapse ;
		cout << "_____ start at: " << start_lapse ;
	} else if (start_lapse == 0) {   // has never been started
		time(&end_lapse) ;
		cout << endl << "___ " << end_lapse << ": the stopwatch is not running ..." << endl ;
	} else {
		time(&end_lapse) ;
		cout << "_____ break -> lapse since: start(" << (end_lapse - start_lapse) << ')' ;
		cout << ", last break(" << (end_lapse - break_point) << ')' ;
		break_point = end_lapse,
			end_lapse -= start_lapse ;
	}
	return(end_lapse) ;
} // show_lapse()



// eof miscellaneous.h
