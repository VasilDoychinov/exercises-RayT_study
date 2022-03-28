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

#include "../tools/miscellaneous.h"
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
		triangle_RT(VectArr&& b) : _base{std::move(b)} {/*cout << "\n___ triangle_RT(_base)..." ;*/}

	public:
		triangle_RT() : _base{{0,0,0}, {0,0,0}, {0,0,0}} {}
		triangle_RT(std::initializer_list<vector_RT<T>> il) : _base{il} { assert(il.size() == 3) ; }
		// ~triangle_RT() = default :                                 skip to enforce default MOVEs
		// triangle_RT(VectArr&& v) : _base{std::move(v._base)} {}    ...             defaults

		// Access
		void extract(vector_RT<T>& a, vector_RT<T>& b, vector_RT<T>& c) const 
								 { a = _base[0], b = _base[1], c = _base[2] ; }
		const vector_RT<T>& operator [](int i) const & { assert(i >=0 && i <=2) ; return(_base[i]) ; }
		vector_RT<T>&       operator [](int i) & { assert(i >= 0 && i <= 2) ; return(_base[i]) ; }
		// vector_RT<T>        operator [](int i) && { assert(i >=0 && i <=2) ; return(std::move(_base[i])) ; }
		// NB: edge(2) is -(V2 - V0) which is used to calcute the normal
		vector_RT<T>		edge(int i) const {return ((i == 0 ? std::move(_base[1] - _base[0])
														: (i == 1 ? std::move(_base[2] - _base[1])
														: std::move(_base[0] - _base[2])))) ; } // -(V2-V0)
		// Normal: NOT normalized
		vector_RT<T> normal() const {
			return (crossRHS(std::move(_base[1] - _base[0]), std::move(_base[2] - _base[0]))) ; 
		}
		friend vector_RT<T> normal(const triangle_RT& v) { return (v.normal()) ; }
		// Normal: NORMALIZED
		vector_RT<T> normalN() const {
			return (unitV(crossRHS(std::move(_base[1] - _base[0]), 
								   std::move(_base[2] - _base[0])))
					) ;
		}
		friend vector_RT<T> normalN(const triangle_RT& v) { return (v.normalN()) ; }

		// Ray intersection
		bool _ray_hit(const vector_RT<T>& o, const vector_RT<T>& r, T& dist) const ;
		// template <typename T>
		// friend bool ray_hit_triangle(const vector_RT<T>& origin, const vector_RT<T>& ray,
			//						 const triangle_RT<T>& tri, T& dist
				//					 ) ; //, Tout g) ;

		// Misc
		const T area() const { return(length(normal()) * static_cast<T>(0.5)) ; }
		friend const T area(const triangle_RT& t) { return(t.area) ; }

		// Moves
		triangle_RT operator +(const vector_RT<T>& mv) { 
			return (triangle_RT{std::move(_base + mv)}) ;
		}
		triangle_RT operator -(const vector_RT<T>& mv) {
			return (triangle_RT{std::move(_base - mv)}) ;
		}
		

		template <class T> friend std::ostream& operator<<(std::ostream& os, const triangle_RT<T>& t) ;
}; // class vector_RT

																		// triangle_RT: templates & inlines
template <typename T> bool
ray_hit_triangle(const vector_RT<T>& origin,	// origin
				 const vector_RT<T>& ray,		// ray (normalized)
				 const triangle_RT<T>& tri,		// triangle
				 T& dist						// OUTPUT: distance from origin(if returns (true))
				)
{
					// static ... CALCULATIONS ???
	// pointHit = Origin + coefHit * Ray
	// coefHit = -(dot(N, Origin) - dot(N,V0)) / dot(N,Ray)

	vector_RT<T>	normT{std::move(normalN(tri))} ;
	auto			dot_NR{dotPR(normT, ray)} ;

	if (near_zero(dot_NR))		return(false) ;		// ray || plane(triangle)

	auto			coefHit { - (dotPR(normT, origin) - dotPR(normT, tri[0])) / dot_NR} ;
																// cout << "\n_ coefHit= " << coefHit ;

	if (coefHit < 0)			return(false) ;		// triangle is in the back of Origin

	auto pointHit = origin + std::move((ray * coefHit)) ;
																// cout << "\n_ pointHit= " << pointHit ;

	// inside-outside test(s): test < 0 if pointHit is on the right side: 0, 1, 2
	if (normT.dotPR(crossRHS(std::move(tri.edge(0)), std::move(pointHit - tri[0]))) < 0)   return(false) ;
	if (normT.dotPR(crossRHS(std::move(tri.edge(1)), std::move(pointHit - tri[1]))) < 0)   return(false) ;
	if (normT.dotPR(crossRHS(std::move(tri.edge(2)), std::move(pointHit - tri[2]))) < 0)   return(false) ;

	dist = coefHit ;
	return(true) ;
} // class triangle_RT friend ray_hit_triangle

template <typename T> bool
triangle_RT<T>::_ray_hit(const vector_RT<T>& o, const vector_RT<T>& r, T& dist) const
{
	bool fl = ray_hit_triangle(o, r, *this, dist) ;
	return(fl) ;
} // triangle_RT _ray_hit()



template <class T> std::ostream&
operator<<(std::ostream& os, const triangle_RT<T>& t)
{
	os << "t{"/* << typeid(T).name()*/ << t[0] << ", " << t[1] << ", " << t[2] << "}"
		<< "-> normal: " << t.normal() ; // << "; area: " << t.area() ;
	return(os) ;
} // class triangle_RT operator << 
																		// eoc triangle_RT

#endif

// eof triangles_RT.h