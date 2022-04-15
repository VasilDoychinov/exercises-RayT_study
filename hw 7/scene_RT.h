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
		bool _ray_hit(const ray_RT& orig, const ray_RT& ray, float& dist) const {
			return(_shape._ray_hit(orig, ray, dist)) ;
		}

		void set_color(mColor c) { _col = c ; }
		mColor color() const { return(_col) ; }
		mColor color(const ray_RT& pix, std::function<float(float)> f) const { 
			return(mColor(255, 0, 0)) ; // vect_to_CRT(pix, f)) ; 
		}

		// Access (CONST)
		const triangle_RT<float>& shape() const & { return(_shape) ; }
}; // class mAtom_RT

class mObjects_RT {
    public:
        using value_type = typename mAtom_RT ;
        using value_reference = mAtom_RT& ;
        using const_reference = const mAtom_RT& ;
        using value_pointer = mAtom_RT * ;
        using iterator = std::vector<value_type>::iterator ;
        using const_iterator = std::vector<value_type>::const_iterator ;

    public:
        std::vector<std::vector<mAtom_RT>>    _base{} ;

    public:
        mObjects_RT() : _base{} {}
        mObjects_RT(cl_SceneDescr* scene) ;

}; // class mObjects_RT


class mScene_RT { // : public cl_SceneDescr {
	private:
		mObjects_RT *				_objects{nullptr} ;     // what's been loaded
		size_t						_obj_num{} ;	        // the real number of objects

		vector_RT<RT_default_type>	_cam{} ;
		mColor						_clrb{} ;
	public:
		
	public:
		mScene_RT(cl_SceneDescr * sd, mObjects_RT *objs) ;
		// all special members = default
		
		mColor	operator ()(ray_RT ray, const ray_RT& camPos) ; // vector_RT<RT_default_type>& camera) ;

		std::pair<mColor/*size_t*/, ray_RT::value_type>
			ray_hit(size_t id_iter, const ray_RT& orig, const ray_RT& ray) ; // const ;

		// Friends
		friend std::ostream& operator <<(std::ostream& os, const mScene_RT& sc) ;
}; // class mScene_RT

mColor randomCRT() ;

// convertion functions
template <typename T> vector_RT<T>
vector_from_string(std::string str)  // there has to be two ',': defining three numbers 
{
    T       mark[3] = {{},} ;
    // cout << endl << endl 
    // << "___ in vector from_string(" 
    // << str << ")" ;
    try {
        size_t  i = 0 ;
        size_t  count = 0 ;
        for (size_t j = 0 ; j < str.size() && count < 2 ; j++) {
            if (str[j] == ',') {
                mark[count ++] = static_cast<T>(std::stod(str.substr(i, j - i))),
                    i = j + 1 ;
            }
        }
        if (count < 2)      throw std::runtime_error("___ bad vector descriptor") ;
        mark[2] = static_cast<T>(std::stod(str.substr(i))) ;
    } catch (...) { throw ; }

    return(vector_RT<T>(mark[0], mark[1], mark[2])) ;
} // vector_from_string()

#include <algorithm>

#ifdef NO_INDEXING
template <typename T> triangle_RT<T>
triangle_from_iterator(_data_iter& it_tr, _data_iter& begin_vertices)  // it: assumed to be (ui,ui,ui)
{
    // cout << endl << "--- in triangle_from_ITER (i(" << *it_tr << "), at v0{" << *begin_vertices << "})" ;
    vector_RT<unsigned int> temp{vector_from_string<unsigned int>(*it_tr)} ; // get the indexes

    std::pair<size_t, size_t>    vvv[3] = {std::pair<size_t,size_t>(0, temp[0]),
                                            std::pair<size_t,size_t>(1, temp[1]),
                                            std::pair<size_t,size_t>(2, temp[2])
    } ;
    std::sort(begin(vvv), end(vvv), [](std::pair<size_t, size_t> i, std::pair<size_t, size_t> j)->bool
                {return(i.second < j.second); }) ;
    // vvv holds the indexes in "vertices" in ascending order 

    // The vertice indexes are supposedly: sorted here.
    auto            iter = begin_vertices ; auto iter_end = iter.limit() ;
    size_t          ind_vertice = 0 ;
    vector_RT<T>    vertices[3] = {vector_RT<T>{}, vector_RT<T>{}, vector_RT<T>{}} ;

    for (int i = 0 ; i < _countof(vvv) ; i++) {
        // cout << endl << "--- loading vertice:" 
        // << vvv[i].second << 
       // " for triangle descr.: " << temp ;
        // fwd_iterator: position it to the current vertice index
        for (; ind_vertice < (vvv[i]).second && iter != iter_end ; ++iter, ind_vertice++) ;
        // load the vector      
        vertices[(vvv[i]).first] = std::move(vector_RT<T>(vector_from_string<T>(*iter))) ;
    }

    return(triangle_RT<T>{vertices[0], vertices[1], vertices[2]}) ;
} // triangle_from_iterator()
#endif

template <typename T> triangle_RT<T>
triangle_from_indexes(_data_iter& it_tr, std::vector<long>& indexes)  // it: assumed to be (ui,ui,ui)
{
    // cout << endl << "--- in triangle_from_ITER (i(" << *it_tr << "), at v0{" << *begin_vertices << "})" ;

    vector_RT<unsigned int> temp{vector_from_string<unsigned int>(*it_tr)} ; // get the indexes
    // temp holds the indexes of triangle's vertices in indexes

    vector_RT<T>    vertices[3] = {vector_RT<T>{}, vector_RT<T>{}, vector_RT<T>{}} ;

    for (int i = 0 ; i < 3 ; i++) {
        auto iter = std::move(it_tr + indexes[temp[i]]) ; // a bit of cheating but _iters are of same scope
        vertices[i] = std::move(vector_from_string<T>(*iter)) ;
    }

    return(triangle_RT<T>{vertices[0], vertices[1], vertices[2]}) ;
} // triangle_from_indexes()


#endif
// eof scene_RT.h