// coords_RT.h: geometry (as in RayTracing) related types, etc
//		- coordinates
//

#ifndef _DEFS_COORDS_RT_GEOMETRY
#define _DEFS_COORDS_RT_GEOMETRY

#include <iostream>
using std::cout ;
using std::endl ;

#include <type_traits>
#include <cmath>
#include <assert.h>

#include <valarray>


template <typename T, unsigned int N> class mCoord {	// general template class mCoord
	static_assert(std::is_arithmetic<T>::value, "mCoord<T,N>: T must be arithmetic") ;
	private:
		std::valarray<T>	_base ;
	public:
		using value_type = typename T ;

		mCoord(std::initializer_list<T> il = {}) : _base{il} { assert(il.size() == N) ; }
		bool operator ==(const mCoord& c) { return (_base == c._base).min() ; }
}; // class mCoord<T,N>

template <typename T>
class mCoord<T, 2> {				// (i, j) specialization
	static_assert(std::is_arithmetic<T>::value, "mCoord<T,2>: T must be arithmetic") ;
	public:
		T	_x ;
		T	_y ;

	public:
		mCoord(T x = 0, T y = 0) : _x{x}, _y{y} {}
		mCoord(std::initializer_list<T> il = {}) : _x{il[0]}, _y{il[1]} { assert(il.size() == 2) ; }
		
		bool operator ==(const mCoord& c) { return _x == c._x && _y == c._y ; }
		template <typename T> friend std::ostream& operator <<(std::ostream & os, const mCoord<T, 2>& sl) ;
}; // mCoord<2>
template <typename T>
class mCoord<T, 3> {				// (i, j, k) specialization
	static_assert(std::is_arithmetic<T>::value, "mCoord<T,3>: T must be arithmetic") ;
	public:
	T	_x ;
	T	_y ;
	T	_z ;

	public:
	mCoord(T x = 0, T y = 0, T z = 0) : _x{x}, _y{y}, _z{z} {}
	mCoord(std::initializer_list<T> il = {}) : _x{il[0]}, _y{il[1]}, _z{il[2]} { assert(il.size() == 3) ; }

	bool operator ==(const mCoord& c) { return _x == c._x && _y == c._y && _z == c._z ; }
	template <typename T> friend	std::ostream& operator <<(std::ostream & os, const mCoord<T, 3>& sl) ;
}; // mCoord<3>

using						coord_RASTER = mCoord<unsigned int, 2> ;
template <typename T> using coord_3D = mCoord<T, 3> ;


																		// templates class mCoord<>
template <typename T>
std::ostream&
operator <<(std::ostream& os, const mCoord<T, 2>& sl)
{
	os << '(' << (sl._x) << ", " << (sl._y) << ")" ;
	return os ;
} // mCoord<2> operator <<

template <typename T>
std::ostream&
operator <<(std::ostream& os, const mCoord<T, 3>& sl)
{
	os << '(' << (sl._x) << ", " << (sl._y) << ", " << (sl._z) << ")" ;
	return os ;
} // mCoord<3> operator <<
																		// class mCoord<>

#endif

// eoc coords_RT.h