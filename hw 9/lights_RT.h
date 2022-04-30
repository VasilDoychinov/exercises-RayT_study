// lights_RT.h: define Lights of a scene as
//		- Types
//          - class light_RT: defines a light
//          - class cl_lightsRT: ... sequence of ...
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
        size_t  num_of_lights() const { return _base.size() ; }

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