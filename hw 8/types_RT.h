// types_RT.h: types, defaults, etc used
//


#ifndef _DEFS_TYPES_RT_RAYS
#define _DEFS_TYPES_RT_RAYS

#include <chrono>

#include "../tools/geometry/grids_RT.h"
#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/matrices_RT.h"
#include "../tools/geometry/shapes_RT.h"
#include "../tools/color_RT.h"

/*
#include "grids_RT.h"
#include "vectors_RT.h"
#include "matrices_RT.h"
#include "shapes_RT.h"
#include "color_RT.h"
*/

using RT_duration = std::chrono::nanoseconds ;

using	RT_default_type = float ;
using	ray_RT          = vector_RT<RT_default_type> ;
using   obox_RT         = box_RT<RT_default_type> ;
using   matrix_RT       = mMatrix<RT_default_type> ;
using	slcRays         = mGrid2_slice ;
using	slcImage        = mGrid2_slice ;
using	refRays         = mGrid2_ref<ray_RT> ;
using	refImage        = mGrid2_ref<mColor> ;
using   gridImage       = mGrid2<mColor> ;
using   gridRays        = mGrid2<ray_RT> ;


#endif
// eof types_RT.h