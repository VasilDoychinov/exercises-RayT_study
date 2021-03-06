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

#include "../miscellaneous.h"
// #include "miscellaneous.h"

#include "vectors_RT.h"

template <typename T>
struct s_hitPoint {			// auxiliary: presenting a hit point with barycentric coordinates as well
	vector_RT<T>	_hp ;
	T				_u_bc ;
	T				_v_bc ;
	s_hitPoint() : _hp{}, _u_bc{0}, _v_bc{0} {}
}; // struct s_hitPoint

template <typename T>	// ??? measure for performance against Tri = (T x, T y, T z) and std::vector<>
class triangle_RT {		// defined through std::valarray<>
	static_assert(std::is_arithmetic<T>::value, "triangle_RT<T>: T must be arithmetic") ;
	public:
		using value_type = T ;
		using pointer_type = T* ;
		using reference_type = T& ;

	private:
		using VectArr = std::valarray<vector_RT<T>> ;

		VectArr			_base{} ;
		vector_RT<T>	_normalN ;			// normal of the triangle;

	private:
									// for internal operations, only: No checks, hence
		triangle_RT(VectArr&& b) : _base{std::move(b)},
								   _normalN{unitV(crossRHS((_base[1] - _base[0]),
														   (_base[2] - _base[0])))
										   }	{/*cout << "\n___ triangle_RT(_base)..." ;*/}

	public:
		triangle_RT() : _base{{0,0,0}, {0,0,0}, {0,0,0}}, _normalN{0,0,0} {}
		triangle_RT(std::initializer_list<vector_RT<T>> il) 
						: _base{il}, _normalN{unitV(crossRHS((_base[1] - _base[0]),
															 (_base[2] - _base[0])))
											 }	{ assert(il.size() == 3) ; }
		// all special members  = default

		// Access
		void extract(vector_RT<T>& a, vector_RT<T>& b, vector_RT<T>& c) const 
								 { a = _base[0], b = _base[1], c = _base[2] ; }
		const vector_RT<T>& operator [](int i) const & { assert(i >=0 && i <=2) ; return _base[i] ; }
		vector_RT<T>&       operator [](int i) & { assert(i >= 0 && i <= 2) ; return _base[i] ; }
		
		// NB: edge(2) is -(V2 - V0) which is used to calcute the normal
		vector_RT<T>		edge(int i) const {return (i == 0 ? (_base[1] - _base[0])
														: (i == 1 ? (_base[2] - _base[1])
														: (_base[0] - _base[2]))) ; } // -(V2-V0)
		// Normal (NORMALIZED)
		vector_RT<T> normalN() const { return _normalN ; }
		friend vector_RT<T> normalN(const triangle_RT& v) { return v._normalN ; }

		// Ray intersection
		bool _ray_hit(const vector_RT<T>& o, const vector_RT<T>& r, T& dist, s_hitPoint<T>& hP) const ;

		// barycentric co-ords: fine tuning
		void barycentric(s_hitPoint<T>& hitP) const ; // finally adjust barycentric coords: use carefully

		// Misc
		T areaP() const { return length(crossRHS((_base[1] - _base[0]), (_base[2] - _base[0]))) ; }

		// Moves
		triangle_RT operator +(const vector_RT<T>& mv) { 
			return triangle_RT{(_base + mv)} ;
		}
		triangle_RT operator -(const vector_RT<T>& mv) {
			return triangle_RT{(_base - mv)} ;
		}
		
		// Min - Max :: bottom left - top - right
		vector_RT<T>	min_shape() const ;
		vector_RT<T>	max_shape() const ;

		template <class T> friend std::ostream& operator<<(std::ostream& os, const triangle_RT<T>& t) ;
}; // class triangle_RT

																		// triangle_RT: templates & inlines
template <typename T> bool
ray_hit_triangle(const vector_RT<T>& origin,	// origin
				 const vector_RT<T>& ray,		// ray (normalized)
				 const triangle_RT<T>& tri,		// triangle
				 T& dist,						// OUTPUT: distance from origin(if returns (true))
				 s_hitPoint<T>& pointHit			// OUTPUT: the hip point
				)
{

	// pointHit = Origin + coefHit * Ray
	// coefHit = -(dot(N, Origin) - dot(N,V0)) / dot(N,Ray)

	vector_RT<T>	normT{normalN(tri)} ;
	auto			dot_NR{dotPR(normT, ray)} ;

	if (near_zero(dot_NR))		return false ;		// ray || plane(triangle)

	auto			coefHit { - (dotPR(normT, origin) - dotPR(normT, tri[0])) / dot_NR} ;
																// cout << "\n_ coefHit= " << coefHit ;

	if (coefHit < 0)			return false ;		// triangle is in the back of Origin

	pointHit._hp = origin + (ray * coefHit) ;			// auto pointHit = origin + (ray * coefHit) ;
																// cout << "\n_ pointHit= " << pointHit ;

	// inside-outside test(s): test < 0 if pointHit is on the right side: 0, 1, 2
	
	if (normT.dotPR(crossRHS(tri.edge(0), pointHit._hp - tri[0])) < 0)	return false ;
	if (normT.dotPR(crossRHS(tri.edge(1), pointHit._hp - tri[1])) < 0)	return false ;
	if (normT.dotPR(crossRHS(tri.edge(2), pointHit._hp - tri[2])) < 0)	return false ;
	// pointHit. _u_bc and _v_bc: just get them out 

	dist = coefHit ;
	return true ;
} // class triangle_RT friend ray_hit_triangle

template <typename T> T area_para(const triangle_RT<T>& t) { return t.areaP() ; }

template <typename T> void
triangle_RT<T>::barycentric(s_hitPoint<T>& hitP) const	// use carefully - proper timing applied
{
	T	a_ar = this->areaP() ;

	hitP._u_bc = area_para(triangle_RT<T>{_base[0], hitP._hp, _base[2]}) / a_ar ;
	hitP._v_bc = area_para(triangle_RT<T>{_base[0], _base[1], hitP._hp}) / a_ar ;

	return ;
} // triangle_RT barycentric 

template <typename T> bool
triangle_RT<T>::_ray_hit(const vector_RT<T>& o, const vector_RT<T>& r, T& dist, s_hitPoint<T>& hP) const
{
	bool fl = ray_hit_triangle(o, r, *this, dist, hP) ;
	return fl ;
} // triangle_RT _ray_hit()


template <typename T> vector_RT<T>
triangle_RT<T>::min_shape() const
{
	T x{std::numeric_limits<T>::max()}, y{std::numeric_limits<T>::max()}, z{std::numeric_limits<T>::max()} ;
	T u{0}, v{0}, w{0} ;

	for (int i = 0 ; i < 3 ; i++) {
		_base[i].extract(u, v, w) ;
		x = (x <= u) ? x : u ;
		y = (y <= v) ? y : v ;
		z = (z <= w) ? z : w ;
	}
	return vector_RT<T>{x, y, z} ;
} // traingle_RT max_shape()

template <typename T> vector_RT<T>
triangle_RT<T>::max_shape() const
{
	T x{-std::numeric_limits<T>::max()}, y{-std::numeric_limits<T>::max()}, z{-std::numeric_limits<T>::max()};
	T u{0}, v{0}, w{0} ;

	for (int i = 0 ; i < 3 ; i++) {
		_base[i].extract(u, v, w) ;
		x = (x >= u) ? x : u ;
		y = (y >= v) ? y : v ;
		z = (z >= w) ? z : w ;
	}
	return vector_RT<T>{x, y, z} ;
} // traingle_RT min_shape()


template <class T> std::ostream&
operator<<(std::ostream& os, const triangle_RT<T>& t)
{
	os << "t{"/* << typeid(T).name()*/ << t[0] << ", " << t[1] << ", " << t[2] << "}"
		<< "-> normal: " << t.normalN() ; // << "; area: " << t.area() ;
	return os ;
} // class triangle_RT operator << 
																		// eoc triangle_RT

#endif
// eof triangles_RT.h