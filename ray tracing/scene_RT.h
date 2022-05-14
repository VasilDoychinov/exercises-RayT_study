// scene_RT.H: to define a Scene
//		- Types
//			- mAtom_RT:	defines the atom of an Object: based on triangle_RT
//					- flat shading
//					- smooth shading: bary-centric co-ords
//			- Lights: in lights_RT.h
//			- Textures: in materials_RT.h 
//			- cl_objectRT: ... an object, seq Atmos; box to accelerate; link to a Material
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
//				- shadow colours, etc: in_sahdow(), shadow_color()
//				- moveing object and lights between sequential cameras
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

#include <atomic>

#include "../tools/color_RT.h"

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/triangles_RT.h"
#include "../tools/geometry/shapes_RT.h"
#include "../tools/geometry/grids_RT.h"
#include "../tools/parser/parser_RT.h"

/*
#include "color_RT.h"

#include "vectors_RT.h"
#include "triangles_RT.h"
#include "shapes_RT.h"
#include "grids_RT.h"
#include "parser_RT.h"
*/

#include "rays_RT.h"
#include "lights_RT.h"
#include "materials_RT.h"


class mAtom_RT   {
	public:
		using value_type = triangle_RT<RT_default_type> ;

	private:
		value_type					_shape{} ;
		vector_RT<unsigned int>		_iovs{} ;
		// mColor			_col{} ;
		// std::function<mColor(const ray_RT& o, const ray_RT& ray)>  _fcol{} ;

	public:
		mAtom_RT() : _shape{}, _iovs{} {}	
		mAtom_RT(_data_iter& tri, _data_iter& vert_beg) ;
        mAtom_RT(_data_iter& tri, std::vector<long>& vertices) ;
        
		// mAtom_RT(const value_type& tri, const mColor& c) : _shape{tri}, _col{c} {}
		// mAtom_RT(value_type&& tri, const mColor& c) : _shape{std::move(tri)}, _col{c} {}

		// Visibility
		bool _ray_hit(const ray_RT& orig, const ray_RT& ray, float& dist, ray_HP& hP) const {
			// atomChecks++ ;  -> atomic<>
			return _shape._ray_hit(orig, ray, dist, hP) ;
		}

		mAlbedo bc_color(ray_HP& hP) const { // return albedo coef. based on bary-centric co-ords
			_shape.barycentric(hP) ;			
			return mAlbedo(hP._u_bc, hP._v_bc, 1 - hP._u_bc - hP._v_bc) ;
		}
		
		// Access (CONST)
		const value_type& shape() const & { return _shape ; }
		const vector_RT<unsigned int>&  iovs() const { return _iovs ; }

		// operational: prepare for the camera to start ...
		void move_atom_by(ray_RT::value_type mx, ray_RT::value_type my, ray_RT::value_type mz) {
			_shape = _shape + ray_RT{mx, my, mz} ;
		}
		
}; // class mAtom_RT


class cl_objectRT {
	public:
		using value_type	 = typename mAtom_RT ;
		using iterator		 = std::vector<value_type>::iterator ;
		using const_iterator = std::vector<value_type>::const_iterator ;
		
	private:
		std::vector<mAtom_RT>	_base{} ;		// all the Atoms
		size_t					_tex_ind{} ;	// index of the texture (in Scene)
	
		obox_RT					_box{} ;		// the box built

		std::vector<ray_RT>		_vertice_normals{} ; // holding normals of all vertices: smooth shading

	public:
		cl_objectRT() : _base{}, _tex_ind{}, _box{}, _vertice_normals{} {}
		cl_objectRT(std::vector<value_type>&& b, size_t ti, obox_RT&& ob, std::vector<ray_RT>&& vN) 
				: _base{std::move(b)}, _tex_ind{ti}, _box{std::move(ob)}, _vertice_normals{std::move(vN)} {}
		// all special members  = default

		size_t	size() { return _base.size() ; }

		const obox_RT&  box() const & { return _box ; }
		bool			box_hit(const ray_RT& orig, const ray_RT& ray) const ;

		// atom's hit normal based on barycentric co-ords
		ray_RT hitNormal(const mAtom_RT& tr, ray_HP& hP) const { // all structures assumed to comply
			tr.shape().barycentric(hP) ;

			return	unitV(_vertice_normals.at(tr.iovs()[1]) * hP._u_bc +
					_vertice_normals.at(tr.iovs()[2]) * hP._v_bc +
					_vertice_normals.at(tr.iovs()[0]) * (1 - hP._u_bc - hP._v_bc)) ;
		}

		size_t	t_ind() const { return _tex_ind ; }
		void	obj_fine_tune(bool fl) ;
		
		const_iterator obj_begin() const { return _base.cbegin() ; }
		const_iterator obj_end() const { return _base.cend() ; }

		// operational: mve for the camera to start ...
		void move_by(ray_RT::value_type mx, ray_RT::value_type my, ray_RT::value_type mz) {
			for (auto& dr : _base)		dr.move_atom_by(mx, my, mz) ;
		}
		
}; // classs cl_objectRT

class mObjects_RT {
    public:
        using value_type = typename mAtom_RT ;
        using value_reference = mAtom_RT& ;
        using const_reference = const mAtom_RT& ;
        using value_pointer = mAtom_RT * ;
        using iterator = std::vector<value_type>::iterator ;
        using const_iterator = std::vector<value_type>::const_iterator ;

    private:
		std::vector <cl_objectRT>	_base{} ; 

    public:
        mObjects_RT() : _base{} {}
        mObjects_RT(cl_SceneDescr* scene) ;

		std::vector <cl_objectRT>& data() { return _base ; }
}; // class mObjects_RT


class mScene_RT { // : public cl_SceneDescr {
	private:
		mObjects_RT *				_objects{nullptr} ;     // the objects loaded

        cl_seqLightsRT*             _lights{} ;
        mAlbedo						_clrb{} ;
		RT_default_type				_shadowBias ;

		struct _s_conc {	size_t		_num_of_threads ;
							std::vector<slcRays>	_slices ;		// slices prepared

							_s_conc() : _num_of_threads{}, _slices{} {}
		} _set_concurrency ;

		cl_seqTexturesRT*			_textures{} ;

	public:
		static constexpr int Scene_MAX_Depth = 5 ;

	public:
		mScene_RT(cl_SceneDescr * sd, mObjects_RT *objs, cl_seqLightsRT* lights, cl_seqTexturesRT* tex) ;
		// all special members = default

		void	set_concurrency(cl_SceneDescr * sd, unsigned int W, unsigned int H) ;
		const _s_conc& get_concurrency() const & { return _set_concurrency ; }

		mColor	operator ()(ray_RT ray, const ray_RT& camPos) ;

		std::pair<mObjects_RT::const_iterator, ray_RT::value_type>
			ray_hit(size_t id_iter, const ray_RT& orig, const ray_RT& ray, ray_HP& hP) ;

		bool	in_shadow(const mAtom_RT& tr, const ray_RT& hP, const ray_RT& ld, double l_dist) ; //, mAlbedo& shalb) ;
		mColor	shadow_color(const mAtom_RT& tr, ray_HP& hP, const cl_objectRT& ob) ;
		// mColor	shadow_color(const mAtom_RT& tr, ray_RT& hP, const mAlbedo& alb) ;

		// access to characteristics
		const texture_RT&	texture(const cl_objectRT& ob) const { return (*_textures)[ob.t_ind()] ; }
		mAlbedo	albedo(const cl_objectRT& ob) const { return texture(ob).color() ; }
		bool	smooth_shading(const cl_objectRT& ob) const { return texture(ob).smooth_shading() ; }
		bool	reflective(const cl_objectRT& ob) const { return texture(ob).reflective() ; }
		ray_RT  reflected_ray(const ray_RT& r, const cl_objectRT&ob, const mAtom_RT& tr) {
			return unitV(r - (tr.shape().normalN()) * dotPR(r, tr.shape().normalN()) * 2) ;
		}

		// fine tuning, eventual checking, etc
		void scene_fine_tune() ;

		// Operational: change position of a light/object in-between cameras switch
		void move_object_by(size_t oid, ray_RT::value_type mx, ray_RT::value_type my, ray_RT::value_type mz) {
			((_objects->data()).at(oid)).move_by(mx, my, mz) ;
		}
		
		void move_light_by(size_t lid, ray_RT::value_type mx, ray_RT::value_type my, ray_RT::value_type mz) {
			((_lights->data()).at(lid)).move_by(mx, my, mz) ;
		}
		void set_light_albedo(size_t lid, const mAlbedo&la) {
			((_lights->data()).at(lid)).set_albedo(la) ;
		}

		// Friends
		friend std::ostream& operator <<(std::ostream& os, const mScene_RT& sc) ;
}; // class mScene_RT

mColor randomCRT() ;


#endif
// eof scene_RT.h