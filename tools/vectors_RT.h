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

// #include "coords_RT.h"

template <typename T>	// ??? measure for performance against V = (T x, T y, T z) and std::vector<>
class vector_RT {		// defined through std::valarray<>
	static_assert(std::is_arithmetic<T>::value, "vector_RT<T>: T must be arithmetic") ;
	public: 
		using value_type = T ;
		using pointer_type = T* ;
		using reference_type = T& ;

	private:
		std::valarray<T> _base{} ;

	private:
										// for internal operations, only: No checks, hence
		vector_RT(std::valarray<T>&& b) : _base{std::move(b)} { /*cout << "\n___ vector_RT(_base)..." ;*/ }

	public:
		vector_RT(T x = 0, T y = 0, T z = 0) : _base{x, y, z} {}
		// vector_RT(mCoord<T, 3> p) : _base{p._base} {}
		vector_RT(std::initializer_list<T> il) : _base{il} {
			if (il.size() != 3)		throw std::range_error("vector_RT<T>: Wrong initilizer_list size") ;
		}
		// ~vector_RT() = default :                                 skip to enforce default MOVEs
		// vector_RT(vector_RT&& v) :                               ...             defaults		

		// Access
		void extract(T& x, T& y, T& z) const	{ x = _base[0], y = _base[1], z = _base[2] ; }
		T operator [](int i) &&					{ return(std::move(_base[i])) ; }
		const T& operator [](int i) const &		{ return(_base[i]) ; }

		// Characteristics
		T norm() const		{ return ((_base * _base).sum()) ; }
		T length() const	{ return(std::sqrt(norm())) ; }
		friend T norm(const vector_RT& v)   { return (v.norm()) ; }
		friend T length(const vector_RT& v) { return (v.length()) ; }

		// Normalizing
		void normalize()				    			{ if (_base.min() > 0) _base /= length() ;	}
		friend vector_RT& normalize(const vector_RT& v) { v.normalize() ; return(v) ; }
		friend vector_RT normalize(vector_RT&& vm) { auto v{std::move(vm._base)} ; v.normalize() ; return(v) ; }

		// V1 . V2
		T dotPR(const vector_RT& v) const { return ((_base * v._base).sum()) ; }
		T dotPR(vector_RT&& vm) && { auto v{std::move(vm._base)} ; return ((_base * v._base).sum()) ; }
		friend T dotPR(const vector_RT& v0, const vector_RT& v1) { return(v0.dotPR(v1)) ; }
		friend T dotPR(vector_RT&& v0, vector_RT&& v1) { return((std::move(v0)).dotPR(std::move(v1))) ; }

		// Operators (MOVE ???)
		vector_RT operator +(const vector_RT & v) const	{ return(_base + v._base) ; }
		vector_RT operator -(const vector_RT & v) const	{ return(_base - v._base) ; }
		vector_RT operator -() const					{ return(- _base) ; }

		vector_RT operator *(const T u) const			{ return(_base * u) ; }
		vector_RT operator *(const vector_RT & v) const { return(_base * v._base) ; }

		// V1 X V2
		vector_RT crossRHS(const vector_RT& v) const { 
			return(vector_RT {	_base[1] * (v._base)[2] - _base[2] * (v._base)[1],
								_base[2] * (v._base)[0] - _base[0] * (v._base)[2],
								_base[0] * (v._base)[1] - _base[1] * (v._base)[0]
							 }) ;
		}
		vector_RT crossRHS(vector_RT&& vP) && {							// cout << "<cross &&>" ;
			auto	vbase{std::move(vP._base)} ;
			return(vector_RT{_base[1] * (vbase)[2] - _base[2] * (vbase)[1],
							 _base[2] * (vbase)[0] - _base[0] * (vbase)[2],
							 _base[0] * (vbase)[1] - _base[1] * (vbase)[0]
				   }) ;
		}
		friend vector_RT crossRHS(const vector_RT& v0, const vector_RT& v1) { return(v0.crossRHS(v1)) ; }
		friend vector_RT crossRHS(vector_RT&& v0, vector_RT&& v1) {		// cout << "<cross &&,&&>" ;
									return((std::move(v0)).crossRHS(std::move(v1))) ; 
		}
		
		// Misc
		template <class T> friend std::ostream& operator<<(std::ostream& os, const vector_RT<T>& v) ;
}; // class vector_RT

																		// vector_RT: templates & inlines
template <class T> std::ostream& 
operator<<(std::ostream& os, const vector_RT<T>& v)
{
	os /*<< '{' << typeid(T).name()*/ << '(' << v[0] << ", " << v[1] << ", " << v[2] << ")" ; //}" ;
	return(os) ;
} // class vector_RT operator << 
																		// eoc vector_RT

#endif

// eof vectors_RT.h