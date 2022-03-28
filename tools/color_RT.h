// color_RT.h: define a color = (R,G,B)


#ifndef _DEFS_COLORS_RT_COLORS
#define _DEFS_COLORS_RT_COLORS

#include <iostream>

using CompT = unsigned char ;				// for color components

class mColor {								// (R,G,B)
	private:
	CompT	_cR ;
	CompT	_cG ;
	CompT	_cB ;
	public:
	mColor(CompT r = 0, CompT g = 0, CompT b = 0) : _cR{r}, _cG{g}, _cB{b} {}
	mColor(const mColor& p) = default ;
	mColor& operator =(const mColor& p) = default ;

	friend std::ostream& 
		operator <<(std::ostream& os, const mColor& p) {
		os << static_cast<short>(p._cR) << "+"
			<< static_cast<short>(p._cG) << "+"
			<< static_cast<short>(p._cB) ;
		return(os) ;
	}
}; // class mColor


#endif
// eof color_RT.h
