// cameras_RT.h: defines the Camera, functionality and operations
//             - interface (render):
//                  - rotations
//                  - translations
//                  - position: camera_to_position() in WS
//             - transfrom a camera ray
//


#ifndef _DEFS_CAMERAS_RT_RAYS
#define _DEFS_CAMERAS_RT_RAYS

#include <utility>

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/matrices_RT.h"

/*
#include "vectors_RT.h"
#include "matrices_RT.h"
*/

#include "types_RT.h"


class camera_RT {
    private:
        ray_RT      _position{} ;
        matrix_RT   _mTransform{} ;

    private:

    public:
        camera_RT() : _position{0.f,0.f,0.f}, _mTransform{{1.f,0.f,0.f},{0.f,1.f,0.f},{0.f,0.f,1.f}} {}
        // all special members = default

        // Movements: translation 
        void _translate(const ray_RT& movement) { _position = _position + (movement * _mTransform) ; }
        void dolly(RT_default_type units) { _translate(ray_RT{0.f, 0.f, units}) ; }
        void boom(RT_default_type units) { _translate(ray_RT{0.f, units, 0.f}) ; }
        void truck(RT_default_type units) { _translate(ray_RT{units, 0.f, 0.f}) ; }
        // Movements: linear transformation -- ??? check for passing rvalues
        void _transform(const matrix_RT& trans) { _mTransform.multiplyByMatrix(trans) ; }
        void pan(RT_default_type degs) { _transform(Y_rotation(degs)) ; }
        void tilt(RT_default_type degs) { _transform(X_rotation(degs)) ; }
        void roll(RT_default_type degs) { _transform(Z_rotation(degs)) ; }
        
        // a camera in WS
        camera_RT camera_to_position(const ray_RT& pos) { _translate(pos) ; return(*this) ; }
        
        // Adjust a 'primary' ray to a camera: the origin is camera._position
        void camera_ray(ray_RT& ray) { ray.multiplyByMatrix(_mTransform) ; }
        friend ray_RT  camera_ray(ray_RT&& ray, const camera_RT& c) { return(std::move(ray) * c._mTransform) ; }

        // Position 
        const ray_RT& position() const & { return(_position) ; }
        // Matrix
        const matrix_RT& crm() const & { return(_mTransform) ; }

        friend std::ostream& operator <<(std::ostream& os, const camera_RT& cam) ;
}; // class camera_RT

// Movements: translation -- ??? check for passing rvalues
inline camera_RT camera_to_position(const ray_RT& ray, camera_RT camera = {}) { 
    camera._translate(ray) ;
    return(camera) ;
}
inline camera_RT dolly(RT_default_type units, camera_RT camera = {}) { 
    camera._translate(ray_RT{0, 0, units}) ; 
    return(camera) ;
} // dolly()

inline camera_RT boom(RT_default_type units, camera_RT camera = {}) {
    camera._translate(ray_RT{0, units, 0}) ; 
    return(camera) ;
} // boom()

inline camera_RT truck(RT_default_type units, camera_RT camera = {}) { 
    camera._translate(ray_RT{units, 0, 0}) ;
    return(camera) ;
} // truck()

// Movements: linear transformation -- ??? check for passing rvalues
inline camera_RT pan(RT_default_type degs, camera_RT camera = {}) { 
    camera._transform(Y_rotation(degs)) ;
    return(camera) ;
} // pan()

inline camera_RT tilt(RT_default_type degs, camera_RT camera = {}) { 
    camera._transform(X_rotation(degs)) ;
    return(camera) ;
} // tilt()

inline camera_RT roll(RT_default_type degs, camera_RT camera = {}) {
    camera._transform(Z_rotation(degs)) ;
    return(camera) ;
} // roll()


std::ostream& operator <<(std::ostream& os, const camera_RT& cam) ;


class batchesCameras {
    public:
    std::vector<std::pair<std::pair<int, int>, std::string>>	_batches ;

    batchesCameras(std::initializer_list<std::pair<std::pair<int, int>, std::string>> il) : _batches{il}  {}

    std::string operator [](int id) {
        for (auto p : _batches) {
            if (id >= (p.first).first && id <= (p.first).second)      return(p.second) ;
        }
        return(std::string{}) ;
    }
    unsigned int start(unsigned int i) { return((i>=0 && i<_batches.size()) ? (_batches[i]).first.first :0);}
    unsigned int end(unsigned int i) { return((i>=0 && i<_batches.size()) ? (_batches[i]).first.second :0);}

}; // struct batchesCameras


#endif
// eof cameras_RT.h