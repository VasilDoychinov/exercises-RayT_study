// geometry_RT.h: geometry related types, etc
//		- coordinates
//		- points
//		- vectors
//

#ifndef _DEFS_GEOMETRY_RT_GEOMETRY
#define _DEFS_GEOMETRY_RT_GEOMETRY

#include <iostream>
using std::cout ;
using std::endl ;
#include <type_traits>
#include <cmath>
#include <assert.h>


template <typename T, unsigned int N> class mCoord {	// general template class mCoord
	static_assert(std::is_arithmetic<T>::value, "mCoord<T,N>: T must be arithmetic") ;
	public:
		using value_type = typename T ;
		mCoord(const mCoord& xy) = default ;
		mCoord& operator =(const mCoord & xy) = default ;
}; // class mCoord<T,N>

template <typename T> 
class mCoord<T, 2> {				// (i, j) specialization
	static_assert(std::is_arithmetic<T>::value, "mCoord<T,2>: T must be arithmetic") ;
	public:
		T _x ;	// in height
		T _y ;	// in width 

	public:
		explicit mCoord(T r, T c) : _x{r}, _y{c} {} ;
		bool operator ==(const mCoord& c) { return(_x == c._x && _y == c._y) ; }
		template <typename T> friend std::ostream& operator <<(std::ostream& os, const mCoord<T,2>& co) ;
}; // mCoord<2>
template <typename T> 
class mCoord<T, 3> {				// (i, j, k) specialization
	static_assert(std::is_arithmetic<T>::value, "mCoord<T,3>: T must be arithmetic") ;
	public:
		T _x ;
		T _y ;
		T _z ;

		bool operator ==(const mCoord& c) { return(_x == c._x && _y == c._y && _z == c._z) ; }
		template <typename T> friend std::ostream& operator <<(std::ostream& os, const mCoord<T,3>& co) ;
}; // mCoord<3>

using coord_RASTER = mCoord<unsigned int, 2> ;
template <typename T> using coord_3D = mCoord<T, 3> ;

template <typename T> 
class vector_RT {
	static_assert(std::is_arithmetic<T>::value, "vector<T>: T must be arithmetic") ;
	public: 
		using value_type = T ;
		using pointer_type = T* ;
		using reference_type = T& ;

	private:
		T	_x ;
		T	_y ;
		T	_z ;
	public:
		explicit vector_RT(T x = 0, T y = 0, T z = 0) : _x{x}, _y{y}, _z{z} {}
		vector_RT(const mCoord<T, 3> p) : _x{p._x}, _y{p._y}, _z{p._z} {}
		~vector_RT() = default ;

		vector_RT(const vector_RT& v) = default ;			// COPY C&A
		vector_RT& operator =(const vector_RT& v) = default ;

		T lengthP() const { return (_x*_x + _y*_y + _z*_z) ; }
		T length() const { return(std::sqrt(lengthP())) ; }
		void extract(T& x, T& y, T& z) const { x = _x, y = _y, z = _z ; }

		vector_RT& normalize() ; 
		friend vector_RT& normalize(vector_RT& v) { return(v.normalize()) ; }
		
		template <class T> friend std::ostream& operator<<(std::ostream& os, const vector_RT<T>& v) ;
}; // class vector_RT

																		// vector_RT: templates & inlines
template <typename T> inline vector_RT<T>&
vector_RT<T>::normalize()
{
	if (_x != 0 || _y != 0 || _z != 0) {
		auto norm = length() ; assert(norm > 0) ;
		norm = (1 / norm), _x *= norm, _y *= norm, _z *= norm ;
	}
	return(*this) ;
} // vector_RT normalize()

template <class T> std::ostream& 
operator<<(std::ostream& os, const vector_RT<T>& v)
{
	os << '{' << typeid(T).name() << '(' << v._x << ',' << v._y << ',' << v._z << ")}" ;
	return(os) ;
} // class vector_RT operator << 
																		// eoc vector_RT
																		
																		// templates class mCoord<>
template <typename T>
std::ostream&
operator <<(std::ostream& os, const mCoord<T, 2>& sl)
{
	os << "(" << sl._x << ", " << sl._y << ")" ;

	return(os) ;
} // mCoord<2> operator <<

template <typename T>
std::ostream&
operator <<(std::ostream& os, const mCoord<T, 3>& sl)
{
	os << "(" << sl._x << ", " << sl._y << ", " << sl._z << ")" ;

	return(os) ;
} // mCoord<3> operator <<
																		// class mCoord<>

#endif

// eof geometry_RT.h