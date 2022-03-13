// grids_RT.cpp: member functions, etc. 
//		- includes grids_RT.h
//		Member function for:
//		- class mGrid2:     none, all templates in rthw2.h
//		- class mGrid2_ref: none, all templates in rthw2.h
//		- class mGrid2_slice:
//			- topleft(): distance from the Origin
//			- operator <<(): for testing
//			- mCoord<> operator << (): ...
//


#include "grids_RT.h"

using std::cout ;
using std::endl ;


																		// class mGrid2
																		// eoc mGrid2

																		// class mGrid2_ref
																		// eoc mGrid2_ref

																		// class mGrid2_slice
size_t
mGrid2_slice::topleft() const
{
	return(_start._x * _step + _start._y) ;
} // mGrid2_slice offset()

std::ostream&
operator <<(std::ostream& os, const mGrid2_slice& sl)
{
	os << "slice { " << sl._start
		<< " + " << sl._rnum << " + " << sl._cnum
		<< ", step: " << sl._step << "}" ;

	return(os) ;
} // mGrid2_slice operator <<

// eof grids_RT.cpp