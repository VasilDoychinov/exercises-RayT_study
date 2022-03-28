// grids_RT.cpp: member functions, etc. 
//		- includes grids_RT.h
//		Member function for:
//		- class mGrid2:     none, all templates in rthw2.h
//		- class mGrid2_ref: none, all templates in rthw2.h
//		- class mGrid2_slice:
//			- operator <<(): for testing
//


#include "grids_RT.h"

using std::cout ;
using std::endl ;
																		// class mGrid2
																		// eoc mGrid2

																		// class mGrid2_ref
																		// eoc mGrid2_ref

																		// class mGrid2_slice
std::ostream&
operator <<(std::ostream& os, const mGrid2_slice& sl)
{
	os << "slice { " << sl._start
		<< " + " << sl._rnum << " + " << sl._cnum
		<< ", step: " << sl._step << "}" ;

	return(os) ;
} // mGrid2_slice operator <<
																		// eoc mGrid2_slice

// eof grids_RT.cpp