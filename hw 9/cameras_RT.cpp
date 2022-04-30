// camers_RT.cpp:
//


#include "cameras_RT.h"

/* as inlines
void
camera_RT::_translate(const ray_RT& movement)
{
    _position = _position + (movement * _mTransform) ;
} // camera_RT _translate ()

void
camera_RT::_transform(const matrix_RT& trans)
{
    _mTransform.multiplyByMatrix(trans) ;
} // camera_RT _translate ()
*/

std::ostream& 
operator <<(std::ostream& os, const camera_RT& cam)
{
    os << "cam{pos" << cam._position << ", transf" << cam._mTransform ;
    return os ;
} // cameras_RT friend operator <<

// eof cameras_RT.cpp