// triangles_RT.h: geometry (as in RayTracing) related types, etc
//		BASED ON RHS unless otherwise stated
//		- Triangles: based std::valarray<vector_RT>
//		
//

#ifndef _DEFS_TRIANGLES_RT_GEOMETRY
#define _DEFS_TRIANGLES_RT_GEOMETRY

#include <iostream>
using std::cout ;
using std::endl ;

#include <type_traits>
#include <cmath>
#include <assert.h>

#include <valarray>

// #include "coords_RT.h"
#include "vectors_RT.h"


template <typename T>	// ??? measure for performance against Tri = (T x, T y, T z) and std::vector<>
class triangle_RT {		// defined through std::valarray<>
	static_assert(std::is_arithmetic<T>::value, "triangle_RT<T>: T must be arithmetic") ;
	public:
		using value_type = T ;
		using pointer_type = T* ;
		using reference_type = T& ;

	private:
		using VectArr = std::valarray<vector_RT<T>> ;

		VectArr _base{} ;
		// ??? vector_RT			_normal ; of the triangle; Not normalized

	private:
									// for internal operations, only: No checks, hence
		triangle_RT(VectArr&& b) : _base{std::move(b)} { cout << "\n___ triangle_RT(_base)..." ; }

	public:
		triangle_RT() : _base{{0,0,0}, {0,0,0}, {0,0,0}} {}
		triangle_RT(std::initializer_list<vector_RT<T>> il) : _base{il} {
			if (il.size() != 3)		throw std::range_error("triangle_RT<T>: Wrong initilizer_list size") ;
		}
		// ~triangle_RT() = default :                                 skip to enforce default MOVEs
		// triangle_RT(VectArr&& v) : _base{std::move(v._base)} {}    ...             defaults

		// Access
		void extract(vector_RT<T>& a, vector_RT<T>& b, vector_RT<T>& c) const 
								 { a = _base[0], b = _base[1], c = _base[2] ; }
		const vector_RT<T>& operator [](int i) const & { return(_base[i]) ; }
		vector_RT<T>&       operator [](int i) & { return(_base[i]) ; }
		vector_RT<T>        operator [](int i) && { return(std::move(_base[i])) ; }

		// Normal: NOT normalized
		vector_RT<T> normal() const {
			return (crossRHS(std::move(_base[1] - _base[0]), std::move(_base[2] - _base[0]))) ; 
		}
		friend vector_RT<T> normal(const triangle_RT& v) { return (v.normal()) ; }

		// Misc
		const T area() const & { return(length(normal()) * static_cast<T>(0.5)) ; }
		friend const T area(const triangle_RT& t) { return(t.area) ; }

		template <class T> friend std::ostream& operator<<(std::ostream& os, const triangle_RT<T>& t) ;
}; // class vector_RT

																		// triangle_RT: templates & inlines
template <class T> std::ostream&
operator<<(std::ostream& os, const triangle_RT<T>& t)
{
	os << "-- t{"/* << typeid(T).name()*/ << t[0] << ", " << t[1] << ", " << t[2] << "}"
		<< "->  normal: " << t.normal() << "; area: " << t.area() ;
	return(os) ;
} // class triangle_RT operator << 
																		// eoc triangle_RT

#endif

// eof triangles_RT.h