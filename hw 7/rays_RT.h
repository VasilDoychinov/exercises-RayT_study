// rays_RT.H: Ray Tracing - rays, cameras, ...
//		- Types
//			- ray_RT: for rays
//			- slcRays: slice of a grid of Rays
//			- refRays: reference to a grid of Rays
//			- gridRays: grid of Rays
//			- slcImage: slice of an Image (grid of Pixels/colors)
//			- refImage: reference to an Image
//			- gridImage: grid of Pixels/colors
// 
//			- class render_RT: defines a scene, calculates, etc.
//				- related functions:
//				 - the default Constructor calculates and sets cameras and Rays
//				 - C&A
//				 - render_image() family
//				 - render_2D(): mostly to test grids, calculations, etc
//				 - cameras interface: _origins[0] is always the WS
//					- add_camera(cam, ind = 0): inserts cam at [ind] or if ind == 0: push_back
//					- del_camera(ind = 0):      removes ...                          pop_back
//					- fix_cameras_list(): = shrink_to_fit(container); returns the number of cameras
//					- iterations through the camera list: ONLY ONE step forward, ie
//						- each camera described relative to the previous one, NOT to WS
//						- next_camera() ; [render_image() for that camera]
//

#ifndef _DEFS_RAYS_RT_RAYS
#define _DEFS_RAYS_RT_RAYS

#include <iostream>
#include <cmath>
#include <chrono>

#include <functional>

#include <fstream>

#include "../tools/color_RT.h"

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/grids_RT.h"
#include "../tools/geometry/matrices_RT.h"

#include "../tools/threads/progress counter.h"

/*
#include "color_RT.h"

#include "vectors_RT.h"
#include "grids_RT.h"
#include "matrices_RT.h"

#include "progress counter.h"
*/

#include "types_RT.h"
#include "cameras_RT.h"


template <typename SCENE> class clF_RenderSlice ;	// functor for std::thread -> defined just before eof


class render_RT  {
	private:
		std::vector<camera_RT>		_origins{} ;		// seq Cameras. [0] is (0,0,0) WorldSpace
		gridRays					_rays ;			// ... 
		unsigned int				_currentCamera{} ;	// the one corresponding to _rays

		std::vector<std::pair<slcRays, std::thread>>	_r_slices{} ;
		
	private:
		unsigned int _transform_rays(unsigned int cameraId) ;	// match _ray to cameraId(simplified for now)		
		void		 _calculate_rays(unsigned int camId) ;

	public:
		render_RT(unsigned int W, unsigned int H, const camera_RT& cam) ;
		// all special members = default

		template <typename SCENE> void render_image(const slcRays& s, gridImage& image, SCENE& i3D) ;

		// Cameras interface
		void add_camera(camera_RT&& cam)		{ _origins.push_back(std::move(cam)) ; return ;}
		void add_camera(const camera_RT& cam)	{ _origins.push_back(cam) ; }
		camera_RT add_Cameras(std::function<camera_RT(RT_default_type, camera_RT)> f,
							 RT_default_type step, unsigned int stepsNum = 1,
							 camera_RT first_cam = {}) ;
		void del_camera(unsigned int ind = 0) ;							// ... pop_back()
		unsigned int num_of_cameras() { return(_origins.size()) ; }

		unsigned int clear_cameras() {
						render_RT t(_rays.W(), _rays.H(), camera_RT{}) ;
						std::swap(*this, t) ; return(_origins.size()) ; 
		}
		unsigned int fix_cameras_list() { _origins.shrink_to_fit(); _currentCamera = 0; return(_origins.size());}
		unsigned int rwd_cameras_list() {
			render_RT t(_rays.W(), _rays.H(), _origins[0]) ; 
			std::swap(this->_rays, t._rays) ; _currentCamera = 0 ;

			return(_origins.size()) ; 
		}

		unsigned int next_camera() ; // returns # and, leaves rays ready for rendering, etc 
		
		// Friends
		template <typename SCENE> friend class clF_RenderSlice ;
		friend void	 _calculate_slc_rays(refRays r_ref, camera_RT cam, 
										 unsigned int w, unsigned int h) ;

		friend std::ostream& operator <<(std::ostream& os, render_RT& r) ;
		friend std::ostream& show_cameras(std::ostream& os, render_RT& r) ;
}; // class render_RT <>

void _calculate_slc_rays(refRays r_ref, camera_RT cam, unsigned int w, unsigned int h) ;
void grid_to_ppm(const gridImage& gr, const std::string& fname) ;

template <typename SCENE> void
show_idScene(SCENE& i3D)
{
	cout << endl << "- rendering image(s) of a slice for i3D{" << typeid(i3D).name() << "}";
	cout << " defined as " << endl << i3D << endl ;
}

template <typename SCENE> void
render_RT::render_image(const slcRays& s, gridImage& image, SCENE& i3D)
{
	cl_KERNEL	ker(th_counter, 700) ;									// show_idScene(i3D) ;
	
	ray_RT ray{(_origins[_currentCamera]).position()} ;
	cout << endl << "> camera #" << _currentCamera ;

	apply_value(image(s), _rays(s), i3D, ray) ; // (_origins[o]).position()) ;
	return ;
} // render_RT render_image(image)
																		// eotemplates for render_RT<>
																		// templates for concurrency
template <typename SCENE>
class clF_RenderSlice {					// functor for std::thread -> runs apply_value() for _currentCamera
	public:
		render_RT*	test ;		// Rays and Camera
		gridImage*	image ;		// to produce the image in

		const SCENE	*		obj ;		// to render using test._currentCamera

		clF_RenderSlice(render_RT* t,
						gridImage* im,
						const SCENE* xobj) : test{std::move(t)}, 
												  image{std::move(im)},
									    	      obj{std::move(xobj)} {}
		unsigned int camera_id() { return(test->_currentCamera) ; }

	void operator () (slcRays s) {

		apply_value((*image)(s), (test->_rays)(s), *obj, 
					((test->_origins)[test->_currentCamera]).position()) ;
		return ;
	}
}; // functor clF_RenderSlice

template <typename Funct> void
render_imSlices(Funct&& func, const std::vector<slcRays>& iSlices)
{
	cout << endl << "> camera #" << func.camera_id() << "; " << iSlices.size() << " threads";
	cl_KERNEL	ker(th_counter, 700) ;

	std::vector<std::thread>	ths{} ;
	for (auto slc = begin(iSlices) ; slc != end(iSlices) ; ++slc) {
		std::thread th(std::move(func), *slc) ;
		ths.push_back(std::move(th)) ;
	}
	for (auto th = begin(ths) ; th != end(ths) ; ++th) { if (th->joinable())     th->join() ; }

	return ;
} // render_imSlices(): use simple threading, ie num_of_Threads = num_of_Slices


#endif
// eof rays_RT.h