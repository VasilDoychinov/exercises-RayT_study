// ray_RT.cpp: 
//		- _calculate_src_rays(): fill in render_RT::_rays  ->  used with std::thread
//		  takes grid reference, camera and grid's W x H
//

#include "rays_RT.h"


/* Chaos: Raster to World Space, normalized rays
* At each pixel (x,y)
*	Find its center, based on the raster coordinates: 	x += 0.5; y += 0.5
*	Convert raster coordinates to NDC space[0.0, 1.0]:	x /= width; y /= height*
*	Convert NDC coordinates to Screen space[-1.0, 1.0]:	x = (2.0 * x) - 1.0
*														y = 1.0 - (2.0 * y)
*	Consider the aspect ratio:							x *= width / height
*	Ray Origin = Camera Position
*	Ray direction = :									(x, y, -1.0)
*	Normalize ray direction vector
*	Apply Camera Rotation Matrix: rayDir = rayDirection * CRM
*	Store the ray with the calculated direction and origin
*/


void
_calculate_slc_rays(refRays rays_ref, camera_RT camera, unsigned int W, unsigned int H)
{
	static const float x_coefNDC = (1 / static_cast<float>(W)) ;
	static const float y_coefNDC = (1 / static_cast<float>(H)) ;
	static const float aspect_ratio = (static_cast<float>(W) / static_cast<float>(H)) ;
	
	slcRays		slc{rays_ref.descr()} ;
	ray_RT		r_temp ;
	float		x_temp, y_temp ;
															// cout << endl << "___ _calculate_slc: " << slc ;
	for (unsigned int x = 0 ; x < slc._cnum ; x++) {
		for (unsigned int y = 0 ; y < slc._rnum ; y++) {	// x: column, y: row

			x_temp = (x + slc._start._y) + 0.5f ;	// Raster space
			y_temp = (y + slc._start._x) + 0.5f ;

			x_temp *= x_coefNDC, y_temp *= y_coefNDC ;				// NDC space
			x_temp = (2 * x_temp) - 1, y_temp = 1 - (2 * y_temp) ;	// Screen space
			x_temp *= aspect_ratio ;
			// Ray Origin is _origins[_currentCamera]._position ;
			r_temp = ray_RT(x_temp, y_temp, -1.) ;
			// Apply the Camera Rotation Matrix (_origins[_currentCamera]._mTransform);  I	for WS
			// r_temp.multiplyByMatrix((_origins[camera]).crm()) ;
			camera.camera_ray(r_temp) ;

			rays_ref(y, x) = unitV(r_temp) ; // ray_RT(x_temp, y_temp, -1.)) ;  // normalize() ;
		}
	}
	return ;
} // render_RT friend _calculate_slc_rays()


// eof rays_RT.cpp