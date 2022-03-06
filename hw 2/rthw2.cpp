// rthw2.cpp: member functions, etc. 
//		- includes rthw2.h
//		Member function for:
//		- class mGrid2: none, all templates in rthw2.h
//		- class mGrid2_ref:
//			- operator(): access a pixel. defined as inline in rthw2.h
//			- at(): ... but with a range check
//			- family paint() with and without a condition
//			- operator <<(): for testing
//		- class mGrid2_slice:
//			- topleft(): distance from the Origin
//			- operator <<(): for testing
//			- mCoord<> operator << (): ...
//		- class mPixel:
//			- operator <<(): output RGB (incl. to .ppm file)
//


#include "rthw2.h"

using std::cout ;
using std::endl ;

																		// class mGrid2
																		// eoc mGrid2

																		// class mGrid2_ref
mPixel&
mGrid2_ref::at(unsigned int i, unsigned int j)				// Range check
{
	if (i >= _descr._rnum || j >= _descr._cnum)     throw std::range_error("___ Grid reference range error") ;
	return(*(_pix + (i * (_descr._step)) + j)) ;
} // mGrid2_ref at()

void
mGrid2_ref::paint(const mPixel& p)
{
	// Here the "slice" is expected: compliant
	// Use iterators with traits static check or, just a plain pointer
	// Might check for types compliance, etc., as well

	auto rowLim = (_descr._rnum) ;
	for (auto pix = _pix ; rowLim > 0 ; --rowLim, pix += (_descr._step - _descr._cnum)) {
		for (auto colLim = (_descr._cnum) ; colLim > 0 ; --colLim, pix++)      *pix = p ;
	}
} // mGrid2_ref paint()

void
mGrid2_ref::paint(const mPixel& p, std::function<bool(unsigned int, unsigned int)> pred)
{
	auto rowLim = (_descr._rnum) ;
	auto colLim = (_descr._cnum) ;

	for (typename decltype(rowLim)  i = 0 ; i < rowLim ; i++) {
		for (typename decltype(rowLim) j = 0 ; j < colLim ; j++) {
			if (pred(i, j))		(this->at)(i, j) = p ;
		}
	}
} // mGrid2_ref paint(pred)

std::ostream& 
operator <<(std::ostream& os, const mGrid2_ref& gr)
{
	os << endl << endl 
		<< "- reference to a grid " << gr._descr
	    << endl << "--- data starts at (" << gr._pix << ")" << endl ;

	return(os) ;
} // mGrid2_ref operator <<
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

//template <unsigned int N> 
std::ostream&
operator <<(std::ostream& os, const mCoord<2>& sl)
{
	os << "(" << sl._x << ", " << sl._y << ")" ;

	return(os) ;
} // mCoord<2> operator <<
																		// eoc mGrid2_slice

																		// class mPixel
std::ostream&
operator <<(std::ostream& os, const mPixel& p)
{
	os << static_cast<short>(p._cR) << "+" 
		<< static_cast<short>(p._cG) << "+" 
		<< static_cast<short>(p._cB) ;

	return(os) ;
} // mPixel operator <<
																		// eoc mPixel
// eof rthw2.cpp