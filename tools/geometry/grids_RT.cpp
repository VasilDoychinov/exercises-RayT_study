// grids_RT.cpp: member functions, etc. 
//		- includes grids_RT.h
//		Member function for:
//		- class mGrid2:     none, all templates in rthw2.h
//		- class mGrid2_ref: none, all templates in rthw2.h
//		- class mGrid2_slice:
//			- operator <<(): for testing
//

#include <thread>
#include <assert.h>
#include "grids_RT.h"

using std::cout ;
using std::endl ;
																		// class mGrid2
																		// eoc mGrid2

																		// class mGrid2_ref
																		// eoc mGrid2_ref

																		// class mGrid2_slice
std::vector<mGrid2_slice>
slices_for_threads(unsigned int w, unsigned int h)
{
	unsigned int nTh = std::thread::hardware_concurrency() ;

	std::vector<mGrid2_slice>	iSlices{} ;

	int		slc_num = (nTh < 3 ? 1 : nTh - 1) ;   // 'Max concurrent supported' - 1
	int		row_num{static_cast<int>(h / slc_num)} ;

	int row = 0 ;
	for (int i = 0 ; i < slc_num - 1 ; i++, row += row_num) {	// store slices
		iSlices.push_back(mGrid2_slice(coord_RASTER(row, 0), row_num, w, w)) ; // ??????? h)) ;
	}
	// store the last one with the remaining rows
	iSlices.push_back(mGrid2_slice(coord_RASTER(row, 0), h - row, w, w)) ;

	iSlices.shrink_to_fit() ;
	// for (const auto& s : iSlices)   cout << endl << "-ini-- " << s ;

	return(iSlices) ;
} // slices_for_threads()

std::vector<mGrid2_slice>		
slices_to_render(unsigned int w, unsigned int h, unsigned int rn, unsigned int cn)  // 
{
	assert(w % cn == 0 && h % rn == 0) ;
	rn = h / rn, cn = w / cn ;

	std::vector<mGrid2_slice>	iSlices{} ;

	unsigned int row{0} ;
	unsigned int col{0} ;
	
	for ( ; row < h ; row += rn) {
		for (col = 0 ; col < w ; col += cn) {
			iSlices.push_back(mGrid2_slice(coord_RASTER(row, col), rn, cn, w)) ;
		}
	}
	iSlices.shrink_to_fit() ;
	// for (const auto& s : iSlices)   cout << endl << "--- slices_to_render: " << s ;

	return(iSlices) ;
} // slices_for_threads()


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