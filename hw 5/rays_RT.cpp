// ray_RT.cpp: 
//

#include "rays_RT.h"


mColor
vect_to_CRT(const ray_RT& vrt, std::function<float(float)> f)
{
	constexpr CompT maxCC = 255 ;	// or, the max(CompT) could be used
	typename ray_RT::value_type x, y, z ;
	vrt.extract(x, y, z) ;
	return(mColor(static_cast<CompT>(f(x) * maxCC),
				  static_cast<CompT>(f(y) * maxCC),
				  static_cast<CompT>(f(z) * maxCC))) ;
} // vect_to_CRT



// eof rays_RT.cpp