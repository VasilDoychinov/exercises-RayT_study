// color_RT.h: define a color = (R,G,B)


#ifndef _DEFS_COLORS_RT_COLORS
#define _DEFS_COLORS_RT_COLORS

#include <iostream>
#include <valarray>
#include <assert.h>


class mColor ;

class mAlbedo {
	public:
		using value_type = float ;

	private:
		value_type	_aR ;  // 0 <= . <= 1
		value_type	_aG ;
		value_type	_aB ;

	private:
		value_type _clamp_0_1(value_type c) { return std::min(std::max(0.f, c), 1.f) ; }
	public: 
		mAlbedo() : _aR{1.f}, _aG{1.f}, _aB{1.f} {}
		mAlbedo(value_type r, value_type g, value_type b) 
			: _aR{_clamp_0_1(r)}, _aG{_clamp_0_1(g)}, _aB{_clamp_0_1(b)} {}
		// all special members  = default 
		
		mAlbedo operator+(const mAlbedo& a) { mAlbedo t{_aR + a._aR, _aG + a._aG, _aB + a._aB} ; return t ; }
		mAlbedo& operator +=(const mAlbedo& a) {
			_aR = _clamp_0_1(_aR + a._aR), _aG = _clamp_0_1(_aG + a._aG), _aB = _clamp_0_1(_aB + a._aB) ;
			return *this ;
		}
		mAlbedo operator *(value_type c) const { mAlbedo t{_aR * c, _aG * c, _aB * c} ; return t ; }
		mAlbedo operator *(const mAlbedo& c) const {
			mAlbedo t{_aR * c._aR, _aG * c._aG, _aB * c._aB} ;
			return t ;
		}
		bool operator ==(const mAlbedo& a) { return _aR == a._aR && _aG == a._aG && _aB == a._aB ; }

		friend mAlbedo operator*(value_type c, const mAlbedo& alb) { return alb * c ; }
		friend class mColor ;

		friend std::ostream&  operator <<(std::ostream& os, const mAlbedo& p) {
			os << '(' << (p._aR) << "+"	<< (p._aG) << "+" << (p._aB) << ')' ;
			return os ;
		}
}; // struct mAlbedo


using CompT = unsigned char ;				// for color components

class mColor {								// (R,G,B)
	private:
		// std::valarray<CompT>	_base ;
		CompT	_cR ; 
		CompT	_cG ; 
		CompT	_cB ;

	public:
		
	public:
		mColor(CompT r = 0, CompT g = 0, CompT b = 0) : _cR{r}, _cG{g}, _cB{b} {}
		mColor(const mAlbedo& alb) :_cR{static_cast<CompT>(255 * alb._aR)}, 
									_cG{static_cast<CompT>(255 * alb._aG)}, 
									_cB{static_cast<CompT>(255 * alb._aB)} {}
		// all special members	= default

		friend std::ostream&  operator <<(std::ostream& os, const mColor& p) {
			os << static_cast<short>(p._cR) << "+"
				<< static_cast<short>(p._cG) << "+"
				<< static_cast<short>(p._cB) ;
			return os ;
		}
}; // class mColor

#endif
// eof color_RT.h