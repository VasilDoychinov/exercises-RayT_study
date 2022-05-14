// materials_RT.h: defines textures objects are described with as
//		- Types
//          - class texture_RT:
//          - class cl_seqTexturesRT: 
//


#ifndef _DEFS_TEXTURES_RT_RAYS
#define _DEFS_TEXTURES_RT_RAYS

#include <vector>
#include <utility>

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/matrices_RT.h"

/*
#include "vectors_RT.h"
#include "matrices_RT.h"
*/

#include "types_RT.h"


class texture_RT {
    public:
        enum class e_Texture_type { Diffuse, Reflective } ;

    private:
        e_Texture_type  _type ;
        mAlbedo         _color{} ;
        bool            _smooth{false} ;

    public:
        texture_RT() : _type{e_Texture_type::Diffuse}, _color{}, _smooth{false} {}
        texture_RT(e_Texture_type ty, const mAlbedo& a, bool fl) : _type{ty}, _color{a}, _smooth{fl} {}
        // all special members  = default
    
        mAlbedo     color() const { return _color ; }
        bool        smooth_shading() const { return _smooth ; }
        bool        reflective() const { return _type == e_Texture_type::Reflective ; }

        friend std::ostream& operator <<(std::ostream& os, const texture_RT& li) {
            os << "texture{type: " 
                << (li._type == e_Texture_type::Reflective ? "reflective" : "diffuse")
                << ", albedo: " << li._color
                << "; smooth: " << std::boolalpha << li._smooth << '}' ;
            return os ;
        }
}; // class texture_RT


class cl_seqTexturesRT {
    public:
    using iterator = std::vector<texture_RT>::iterator ;
    using const_iterator = std::vector<texture_RT>::const_iterator ;

    private:
    std::vector<texture_RT>   _base{} ;

    public:
        cl_seqTexturesRT() : _base{} {}
        cl_seqTexturesRT(cl_SceneDescr* scene) ;
        // all special members  = default

    template <typename T> void add(T&& tex) { _base.push_back(std::forward<T>(tex)) ; }
    void remove(size_t ind) { _base.erase(cbegin(_base) + ind) ; }

    const texture_RT& operator[](size_t i) const { return _base[i] ; }
    const_iterator begin_textures() { return _base.cbegin() ; }
    const_iterator end_textures() { return _base.cend() ; }

    friend std::ostream& operator <<(std::ostream& os, const cl_seqTexturesRT& ls) {
        os << endl << ": # of textures: " << ls._base.size() ;
        for (const auto& li : ls._base)   cout << endl << "--- " << li ;
        return os ;
    }
}; // class cl_seqTexturesRT 


#endif
// eof materials_RT.h