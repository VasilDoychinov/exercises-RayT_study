// lights_RT.h: define Lights of a scene as
//		- Types
//          - class light_RT:
//          - class cl_lightsRT: 
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


#ifndef _DEFS_LIGHTS_RT_RAYS
#define _DEFS_LIGHTS_RT_RAYS

#include <vector>
#include <utility>

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/matrices_RT.h"

/*
#include "vectors_RT.h"
#include "matrices_RT.h"
*/

#include "types_RT.h"


class light_RT {
    private:
        size_t      _intensity{} ;
        ray_RT      _position{} ;
        mAlbedo     _color{} ;

    public:
        light_RT() : _intensity{0}, _position{0.f, 0.f, 0.f}, _color{} {}
        light_RT(size_t i, const ray_RT& r, const mAlbedo& c) : _intensity{i}, _position{r}, _color{c} {}
        // all special members  = default

        const ray_RT& position() const& { return _position ; }
        size_t        intensity() const { return _intensity ; }
        mAlbedo       color() const { return _color ; }

        friend std::ostream& operator <<(std::ostream& os, const light_RT& li) {
            os << "light{intensity: " << li._intensity << ", position: " << li._position 
                << ", albedo: " << li._color << '}' ;
            return os ;
        }
}; // class light_RT


class cl_seqLightsRT {
    public:
        using iterator = std::vector<light_RT>::iterator ;
        using const_iterator = std::vector<light_RT>::const_iterator ;

    private:
        std::vector<light_RT>   _base{} ;

    public:
        cl_seqLightsRT() : _base{} {}
        cl_seqLightsRT(cl_SceneDescr* scene) ;
        // all special members  = default

        template <typename T> void add(T&& light) { _base.push_back(std::forward<T>(light)) ; }
        void remove(size_t ind) { _base.erase(cbegin(_base) + ind) ; }

        const_iterator begin_lights() { return _base.cbegin() ; }
        const_iterator end_lights() { return _base.cend() ; }

        friend std::ostream& operator <<(std::ostream& os, const cl_seqLightsRT& ls) {
            os << endl << ": # of lights: " << ls._base.size() ;
            for (const auto& li : ls._base)   cout << endl << "--- " << li ;
            return os ;
        }
}; // class cl_seqLightsRT 


#endif
// eof lights_RT.h