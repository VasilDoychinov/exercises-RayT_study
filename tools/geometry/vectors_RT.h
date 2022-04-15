// vectors_RT.h: geometry (as in RayTracing) related types, etc
//		BASED ON RHS unless otherwise stated
//		- vector_RT: based on std::valarray<>: for calculations, MOVEs, etc.
//

#ifndef _DEFS_VECTORS_RT_GEOMETRY
#define _DEFS_VECTORS_RT_GEOMETRY

#include <iostream>
using std::cout ;
using std::endl ;

#include <type_traits>
#include <cmath>
#include <assert.h>

#include <valarray>


template <typename T> class mMatrix ;

template <typename T>	
class vector_RT {				// V = (T x, T y, T z) performs faster than V{std::valarray<>
	static_assert(std::is_arithmetic<T>::value, "vector_RT<T>: T must be arithmetic") ;
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
		vector_RT(std::initializer_list<T> il) : 
				_x{*(begin(il))}, _y{*(begin(il) + 1)}, _z{*(begin(il) +2)} { assert(il.size() == 3) ; }
		// all special members  = default

		// Access
		void extract(T& x, T& y, T& z) const	{ x = _x, y = _y, z = _z ; }
		T& operator [](int i) { return((i == 0 ? _x : (i == 1 ? _y : _z))) ; }
		const T& operator [](int i) const { return((i == 0 ? _x : (i == 1 ? _y : _z))) ; }

		// Characteristics
		T norm() const		{ return (_x * _x + _y * _y + _z * _z) ; }
		T length() const	{ return(std::sqrt(norm())) ; }
		friend T norm(const vector_RT& v)   { return (v.norm()) ; }
		friend T length(const vector_RT& v) { return (v.length()) ; }

		// Normalizing
		vector_RT& normalize() { 
			/*if (_x != 0 || _y != 0 || _z != 0) { */
			auto norm = length() ; assert(norm > 0) ;
			norm = (1 / norm), _x *= norm, _y *= norm, _z *= norm ;
			/*}*/ 
			return(*this) ;
		}
		friend vector_RT& normalize(vector_RT& v)	{ return(v.normalize()) ; }
		friend vector_RT  unitV(const vector_RT& v) { vector_RT t{v} ; t.normalize() ; return(t) ; }
		friend vector_RT  unitV(vector_RT&& vm)		{ vm.normalize() ; return(std::move(vm)) ; }

		// V1 . V2
		T dotPR(const vector_RT& v) const { return(_x * v._x + _y * v._y + _z * v._z) ; }
		friend T dotPR(const vector_RT& v0, const vector_RT& v1) { return(v0.dotPR(v1)) ; }

		// Operators (MOVE ???)
		vector_RT operator +(const vector_RT & v) const	{ return(vector_RT{_x + v._x, _y + v._y, _z + v._z}) ; }
		vector_RT operator -(const vector_RT & v) const	{ return(vector_RT{_x - v._x, _y - v._y, _z - v._z}) ; }
		vector_RT operator -() const					{ return(vector_RT{-_x, -_y, -_z}) ; }

		vector_RT operator *(const T u) const			{ return(vector_RT{_x * u, _y * u, _z * u}) ; }

		void multiplyByMatrix(const mMatrix<T>& m)		{ T x, y, z ;
			x = dotPR(m.col(0)), y = dotPR(m.col(1)), z = dotPR(m.col(2)) ;
			_x = x, _y = y, _z = z ;
		}
		friend vector_RT operator *(const vector_RT<T>& v, const mMatrix<T>& m) { 
			vector_RT<T> temp{v} ; temp.multiplyByMatrix(m) ; return(temp) ;
		}
		/*
		friend vector_RT operator *(vector_RT<T>&& v, const mMatrix<T>& m) {
			return((std::move(v)).multiplyByMatrix(m)) ;
		}
		friend vector_RT multiplyByMatrix(vector_RT<T>& v, const mMatrix<T>& m) {
			return(v.multiplyByMatrix(m)) ;
		}
		*/
		// V1 X V2
		vector_RT crossRHS(const vector_RT& v) const { 
			return(vector_RT {	_y * (v._z) - _z * (v._y),
								_z * (v._x) - _x * (v._z),
								_x * (v._y) - _y * (v._x)
							 }) ;
		}
		friend vector_RT crossRHS(const vector_RT& v0, const vector_RT& v1) { return(v0.crossRHS(v1)) ; }
				
		// Misc
		template <class T> friend std::ostream& operator<<(std::ostream& os, const vector_RT<T>& v) ;

		// Friends
		// template <typename T> friend class mMatrix ;
}; // class vector_RT

																		// vector_RT: templates & inlines
template <class T> std::ostream& 
operator<<(std::ostream& os, const vector_RT<T>& v)
{
	os /*<< '{' << typeid(T).name()*/ << '(' << v[0] << "," << v[1] << "," << v[2] << ")" ; //}" ;
	return(os) ;
} // class vector_RT operator << 
																		// eoc vector_RT

#endif
// eof vectors_RT.h