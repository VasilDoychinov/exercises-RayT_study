// scene_RT.H: to define a Scene
//		- Types
//			- mAtom_RT:	defines the atom of an Object: NOT parameterized for now - based on triangle_RT
//			- mObjects_RT: ...   an Object is a seq mAtom_RT: FURTHER might be {Atom<T ... types>}
//                        provides link to the scene descriptor, iterators, etc
//				- all special members: default 
//				- other constructors
//				- ray_hit(): Visibility, returns pair<closest Atom, distance to plane>
//              - descriptor: might be used with indexing - see triangle_from_indexes() or, without
//                            see triangle_from_iterator()
//			- mScene_RT: ...    a Scene, linked to mObjetcs_RT->scene descriptor
//              - includes: camera(s), etc
//				- operator(): does the job (ray tracing)
//


#ifndef _DEFS_SCENE_RT_RAYS
#define _DEFS_SCENE_RT_RAYS

#include <iostream>
#include <cmath>
#include <string>

#include <vector>


#include <utility>
#include <functional>
#include <algorithm>

#include "../tools/color_RT.h"

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/triangles_RT.h"
#include "../tools/geometry/grids_RT.h"
#include "../tools/parser/parser_RT.h"

/*
#include "color_RT.h"

#include "vectors_RT.h"
#include "triangles_RT.h"
#include "grids_RT.h"
#include "parser_RT.h"
*/

#include "rays_RT.h"
#include "lights_RT.h"


class mAtom_RT   {
	public:
		using value_type = triangle_RT<RT_default_type> ;

	private:
		triangle_RT<float>			_shape{} ;
		mColor						_col{} ;
		// std::function<mColor(const ray_RT& o, const ray_RT& ray)>  _fcol{} ;

	public:
		mAtom_RT() : _shape{}, _col{} {}
		mAtom_RT(_data_iter& tri, _data_iter& vert_beg) ;
        mAtom_RT(_data_iter& tri, std::vector<long>& vertices) ;
        
		// mAtom_RT(const value_type& tri, const mColor& c) : _shape{tri}, _col{c} {}
		// mAtom_RT(value_type&& tri, const mColor& c) : _shape{std::move(tri)}, _col{c} {}

		// Visibility
		bool _ray_hit(const ray_RT& orig, const ray_RT& ray, float& dist, ray_RT& hP) const {
			return(_shape._ray_hit(orig, ray, dist, hP)) ;
		}

		void set_color(mColor c) { _col = c ; }
		mColor color() const { return(_col) ; }
		mColor color(const ray_RT& pix, std::function<float(float)> f) const { 
			return(mColor(255, 0, 0)) ; // vect_to_CRT(pix, f)) ; 
		}

		// Access (CONST)
		const value_type& shape() const & { return(_shape) ; }
}; // class mAtom_RT

class cl_objectRT {
	public:
		using value_type	 = typename mAtom_RT ;
		using iterator		 = std::vector<value_type>::iterator ;
		using const_iterator = std::vector<value_type>::const_iterator ;
		
	private:
		std::vector<mAtom_RT>	_base{} ;
		mAlbedo					_alb ;

	public:
		cl_objectRT() : _base{}, _alb{} {}
		cl_objectRT(std::vector<value_type>&& b, mAlbedo&& a) : _base{std::move(b)}, _alb{std::move(a)} {}
		// all special members  = default

		size_t	size() { return(_base.size()) ; }
		mAlbedo albedo() const { return(_alb) ; }

		const_iterator obj_begin() const { return(_base.cbegin()) ; }
		const_iterator obj_end() const { return(_base.cend()) ; }
}; // classs cl_objectRT

class mObjects_RT {
    public:
        using value_type = typename mAtom_RT ;
        using value_reference = mAtom_RT& ;
        using const_reference = const mAtom_RT& ;
        using value_pointer = mAtom_RT * ;
        using iterator = std::vector<value_type>::iterator ;
        using const_iterator = std::vector<value_type>::const_iterator ;

    public:
	std::vector <cl_objectRT>	_base{} ;  // mAtom_RT >> _base{} ;

    public:
        mObjects_RT() : _base{} {}
        mObjects_RT(cl_SceneDescr* scene) ;

}; // class mObjects_RT


class mScene_RT { // : public cl_SceneDescr {
	private:
		mObjects_RT *				_objects{nullptr} ;     // what's been loaded
		// size_t						_obj_num{} ;	        // the real number of objects

		//vector_RT<RT_default_type>	_cam{} ;
        cl_seqLightsRT*             _lights{} ;
        mAlbedo						_clrb{} ;
		RT_default_type				_shadowBias ;

	public:
		
	public:
		mScene_RT(cl_SceneDescr * sd, mObjects_RT *objs, cl_seqLightsRT* lights) ;
		// all special members = default
		
		mColor	operator ()(ray_RT ray, const ray_RT& camPos) ; // vector_RT<RT_default_type>& camera) ;

		std::pair<mObjects_RT::const_iterator, ray_RT::value_type>
			ray_hit(size_t id_iter, const ray_RT& orig, const ray_RT& ray, ray_RT& hP) ; // const ;

		bool	in_shadow(const mAtom_RT& tr, const ray_RT& hP, const ray_RT& ld, double l_dist) ;
		mColor	shadow_color(const mAtom_RT& tr, ray_RT& hP, const mAlbedo& alb) ;

		// Friends
		friend std::ostream& operator <<(std::ostream& os, const mScene_RT& sc) ;
}; // class mScene_RT

mColor randomCRT() ;


#endif
// eof scene_RT.h