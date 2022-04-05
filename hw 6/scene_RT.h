// scene_RT.H: to define a Scene
//		- Types
//			- mAtom_RT:	defines the atom of an Object: NOT parameterized for now - based on triangle_RT
//			- mObject_RT: ...   an Object as a seq mAtom_RT: FURTHER might be {Atom<T ... types>}
//				- all special member F: default 
//				- other constructors
//				- partial functionallity of std::vector<>
//				- ray_hit(): Visibility, returns pair<closest Atom, distance to plane>
//			- mScene_RT: ...    a Scene as a seq mObject_RT
//				- initiated as a sequence (std::vector)<mObject_RT>
//				- some of std::vector<> functionality to add, remove, etc elements
//				- operator(): does the job (ray tracing)
//


#ifndef _DEFS_SCENE_RT_RAYS
#define _DEFS_SCENE_RT_RAYS

#include <iostream>
#include <cmath>
#include <vector>

#include <utility>
#include <functional>

#include "../tools/color_RT.h"

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/triangles_RT.h"
#include "../tools/geometry/grids_RT.h"

/*
#include "../tools/color_RT.h"

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/triangles_RT.h"
#include "../tools/geometry/grids_RT.h"
*/

#include "rays_RT.h"


class mAtom_RT   {
	public:
		using value_type = triangle_RT<RT_default_type> ;

	private:
		triangle_RT<float>			_shape{} ;
		mColor						_col{} ;
		// std::function<mColor(const ray_RT& o, const ray_RT& ray)>  _fcol{} ;

	public:
		mAtom_RT(const value_type& tri, const mColor& c) : _shape{tri}, _col{c} {}

		// Visibility
		bool _ray_hit(const ray_RT& orig, const ray_RT& ray, float& dist) const {
			return(_shape._ray_hit(orig, ray, dist)) ;
		}
		mColor color() const { return(_col) ; }
		mColor color(const ray_RT& pix, std::function<float(float)> f) const { 
			return(mColor(255, 0, 0)) ; // vect_to_CRT(pix, f)) ; 
		}

		// Access (CONST)
		const triangle_RT<float>& shape() const & { return(_shape) ; }
}; // class mAtom_RT

class mObject_RT {
	public:
		using value_type = typename mAtom_RT ;
		using value_reference = mAtom_RT& ;
		using const_reference = const mAtom_RT& ;
		using value_pointer = mAtom_RT * ;
		using iterator = std::vector<value_type>::iterator ;
		using const_iterator = std::vector<value_type>::const_iterator ;

	private:
		std::vector<value_type>		_base{} ;		// provides the functionality of std::vector<>
		std::string					_id{} ;			// identifier
		mColor						_clrb{} ;

		// Other necessary characteristics of an Object

	public:
		explicit mObject_RT(std::string id = {}) : _base{}, _id{id}, _clrb{0,0,0} {}
		mObject_RT(std::string id, std::initializer_list<value_type> il, mColor b = {0,0,0})
					: _base{il}, _id{id}, _clrb{b}{}
		mObject_RT(std::string id, std::vector<value_type>&& il, mColor b = {0,0,0})
			: _base{std::move(il)}, _id{id}, _clrb{b}{}

		// Interface for the Visibility (how it is seen from a Origin(orig))
		std::pair<size_t, ray_RT::value_type> ray_hit(const ray_RT& orig, const ray_RT& ray) const ;
		mColor	operator ()(ray_RT ray, ray_RT& camPos) ;

		// Interface to create, modify, etc an Object
		//	Access
		const_reference operator [](size_t i) const & { return(_base[i]) ; }   // No range check
		iterator begin() { return(_base.begin()) ; }
		const_iterator cbegin() const { return(_base.cbegin()) ; }
		iterator end() { return(_base.end()) ; }
		const_iterator cend() const { return(_base.cend()) ; }
		const std::string& name() const & { return(_id) ; }

		//	Add new one
		template <typename T> void add(T atom) { _base.push_back(std::forward<T>(atom)) ; }
		// Delete
		void remove(size_t i)  { if (i < _base.size()) _base.erase(_base.begin() + i) ; }

		// Possible interface to use an Object

		// Friends
		friend std::ostream& operator <<(std::ostream& os, const mObject_RT& obj) ;
}; // class mObject_RT

class mScene_RT {
	private:
		std::vector<mObject_RT>		_base{};
		vector_RT<RT_default_type>	_cam{} ;
		mColor						_clrb{} ;
	public:
		explicit mScene_RT(const std::vector<mObject_RT>& v = {}) : _base{v}, _cam{}, _clrb{} {}
		mScene_RT(std::vector<mObject_RT>&& v) : _base{std::move(v)}, _cam{}, _clrb{} {}
		mScene_RT(std::initializer_list< mObject_RT> il) : _base{il}, _cam{}, _clrb{} {}

		mColor	operator ()(ray_RT ray, ray_RT& camPos) ; // vector_RT<RT_default_type>& camera) ;

		//	Add new one
		template <typename T> void add(T obj) { _base.push_back(std::forward<T>(obj)) ; }
		// Delete
		void remove(size_t i) { if (i < _base.size()) _base.erase(_base.begin() + i) ; }

		// Friends
		friend std::ostream& operator <<(std::ostream& os, const mScene_RT& sc) ;
}; // class mScene_RT


#endif
// eof scene_RT.h