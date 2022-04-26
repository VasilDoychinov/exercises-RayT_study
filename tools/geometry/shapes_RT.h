// shapes_RT.h: some 3D shapes - definititons, etc
//      - box_RT: a box cintaining a scene object: to accelerate
//			- _box_hit() check if a ray hits the box: courtesy of "scratchapixel.com: render simple shapes"
//													  cosmetic changes applied 
//

#ifndef _DEFS_SHAPES_RT_GEOMETRY
#define _DEFS_SHAPES_RT_GEOMETRY

#include <limits>

#include "vectors_RT.h"

template <typename T>
class box_RT {
	public:
		using value_type = T ;
		using pointer_type = T* ;
		using reference_type = T& ;

	private:
		static constexpr auto T_MIN = (-std::numeric_limits<T>::max()) ;
		static constexpr auto T_MAX = std::numeric_limits<T>::max() ;

		vector_RT<T>	_min ;
		vector_RT<T>	_max ;

	private:
		void _set_min_max(const T& id, vector_RT<T>& vmin, vector_RT<T>& vmax) const ;

	public:
		box_RT() : _min{T_MAX, T_MAX, T_MAX}, _max{T_MIN, T_MIN, T_MIN} {}
		// all specials  = default

		template <typename Sh> void min_max(const Sh& s) {	vector_RT<T>	t{s.min_shape()} ;
			for (int i = 0 ; i < 3 ; i++)		if (_min[i] > t[i]) { _min[i] = t[i] ; }
			t = s.max_shape() ;
			for (int i = 0 ; i < 3 ; i++)		if (_max[i] < t[i]) { _max[i] = t[i] ; }
		}

		bool operator != (const box_RT& b) const { return _min != b._min || _max != b._max ; }

		bool _box_hit(const vector_RT<T>& orig, const vector_RT<T>& dir, vector_RT<T>& pHit) const ;

		friend std::ostream& operator <<(std::ostream& os, const box_RT& b) {
			os << "box[" ;
			if (b != box_RT{})	os << b._min << ", " << b._max ;
			else				os << "{}, {}" ;
			os << "]" ;
			return os ;
		}
}; // class box_RT

template <typename T> void
box_RT<T>::_set_min_max(const T& id, vector_RT<T>& vmin, vector_RT<T>& vmax) const
{
	if (id < 0)		vmin = _max, vmax = _min ; 
	else			vmin = _min, vmax = _max ;
} // set_min_max()

template <typename T> bool
box_RT<T>::_box_hit(const vector_RT<T>& orig, const vector_RT<T>& dir, vector_RT<T>& pHit) const
{
	T tx_min, tx_max, ty_min, ty_max, tz_min, tz_max, t ;

	vector_RT<T>	inv_dir{1 / dir} ;
	vector_RT<T>	v_min, v_max ;

	T	ox, oy, oz ;	orig.extract(ox, oy, oz) ;
	T	ix, iy, iz ;	inv_dir.extract(ix, iy, iz) ;

	_set_min_max(ix, v_min, v_max) ;
	tx_min = (v_min[0] - ox) * ix ;
	tx_max = (v_max[0] - ox) * ix;

	_set_min_max(iy, v_min, v_max) ;
	ty_min = (v_min[1] - oy) * iy ;
	ty_max = (v_max[1] - oy) * iy ;
	if ((tx_min > ty_max) || (ty_min > tx_max))		return false ;

	if (ty_min > tx_min)		tx_min = ty_min ;
	if (ty_max < tx_max)		tx_max = ty_max;

	_set_min_max(iz, v_min, v_max) ;
	tz_min = (v_min[2] - oz) * iz ;
	tz_max = (v_max[2] - oz) * iz ;
	if ((tx_min > tz_max) || (tz_min > tx_max))		return false;

	if (tz_min > tx_min)		tx_min = tz_min;
	if (tz_max < tx_max)		tx_max = tz_max;

	t = tx_min;
	if (t < 0) { if ((t = tx_max) < 0) return false ; }

	pHit = orig + dir * t ;
	return true;
} // box_RT _box_hit() 

#endif
// eof shapes_RT.h