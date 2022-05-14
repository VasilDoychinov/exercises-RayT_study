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
//				 - render_*() family
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
#include <atomic>

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

extern std::atomic<size_t>		slcAssigned ;	// used in clf_ProcessSlices & render_slicesV3()
extern std::atomic<size_t>		slcProcessed ;	// used in clf_ProcessSlices & render_slicesV3()

// extern size_t atomChecks ;		// statistics
// extern size_t boxMisses ;		// ...
// extern size_t boxChecks ;		// ...

// template <typename SCENE> class clF_RenderSlice ;	// functor for std::thread -> defined just before eof
template <typename SCENE> class clF_renderSlices ;	// functor for std::thread: based on atomic<slice ind>


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

		// Cameras interface
		void add_camera(camera_RT&& cam)		{ _origins.push_back(std::move(cam)) ; return ;}
		void add_camera(const camera_RT& cam)	{ _origins.push_back(cam) ; }
		camera_RT add_Cameras(std::function<camera_RT(RT_default_type, camera_RT)> f,
							 RT_default_type step, unsigned int stepsNum = 1,
							 camera_RT first_cam = {}) ;
		void del_camera(unsigned int ind = 0) ;							// ... pop_back()
		unsigned int num_of_cameras() { return _origins.size() ; }

		/*
		unsigned int clear_cameras() { render_RT t(_rays.W(), _rays.H(), camera_RT{}) ;	std::swap(*this, t) ; return _origins.size() ; 	}
		*/
		unsigned int fix_cameras_list() { _origins.shrink_to_fit(); _currentCamera=0; return _origins.size();}
		unsigned int rwd_cameras_list() {
			render_RT t(_rays.W(), _rays.H(), _origins[0]) ; 
			std::swap(this->_rays, t._rays) ; _currentCamera = 0 ;

			return _origins.size() ; 
		}

		unsigned int next_camera() ; // returns # and, leaves rays ready for rendering, etc 
		camera_RT	 camera(size_t ind) const { return _origins.at(ind) ; }

		gridImage	image_plane() { return gridImage(_rays.W(), _rays.H()) ; }

		// Friends
		// template <typename SCENE> friend class clF_RenderSlice ;
		template <typename SCENE> friend class clF_renderSlices ;

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
																		// eotemplates for render_RT<>
																		// templates for concurrency

using pair_PackedTask = std::pair <std::thread, std::future<void>> ;
using vect_PackedTasks = std::vector <pair_PackedTask> ;
using packed_slcRays = std::packaged_task<void()> ;

template <typename Funct> std::thread
_assign_task(Funct func, std::future<void>& result)
{
	packed_slcRays		do_task(std::move(func)) ;
	result = do_task.get_future() ;

	std::thread th(std::move(do_task)) ;

	return th ;
} // _assign_task(): assign a thread to a slice


template <typename SCENE>
class clF_renderSlices {	// functor for std::thread -> runs apply_value() till slices are there
	public:
	render_RT*	test ;		// Rays and Camera
	gridImage*	image ;		// to produce the image in

	const SCENE	*		obj ;		// to render using test._currentCamera

	clF_renderSlices(render_RT* t,
					gridImage* im,
					const SCENE* xobj) : test{std::move(t)},
		image{std::move(im)},
		obj{std::move(xobj)} {}
	unsigned int camera_id() { return test->_currentCamera ; }
	const camera_RT& camera() { return test->_origins[test->_currentCamera] ; }

	void operator () () {
		auto conc_set = obj->get_concurrency() ;
		auto number_of_slices = conc_set._slices.size() ;
		
		size_t  slc_ind ;
		while (true) {
			slc_ind = slcAssigned.fetch_add(1) ;
			if (slc_ind >= number_of_slices)   break ;
			
			apply_value((*image)(conc_set._slices[slc_ind]), 
						(test->_rays)(conc_set._slices[slc_ind]),
						*obj,
						((test->_origins)[test->_currentCamera]).position()) ;
			slcProcessed.fetch_add(1) ;
		}

		return ;
	}
}; // functor clF_renderSlices


template <typename Funct, typename Conc> void
render_slicesV3(Funct func, Conc co) // , const std::vector<slcRays>& iSlices)
{
	size_t	nTh{co._num_of_threads} ; // std::thread::hardware_concurrency() } ;

	{
		// auto	n_slc = co._slices.size() ;
		cout << endl << endl << "> #" << func.camera_id() << ": " << func.camera() << endl ;
			// << ": " << n_slc << " slices; threads used: " << nTh << endl ;
		assert(nTh <= co._slices.size()) ;
	}

	slcAssigned.store(0) ;		// from the beginning
	slcProcessed.store(0) ;

	auto	t0 = mClock::now() ;

	static vect_PackedTasks		seqPThs(nTh) ;
	static const auto			ths_begin = begin(seqPThs) ;
	static const auto			ths_end = end(seqPThs) ;

	// Launch tasks: they should move to a next slice on their own
	for (auto t = ths_begin ; t != ths_end ; ++t) {
		t->first = _assign_task(func, t->second) ;
		cout << "\r--- slice #" << slcAssigned.load() << " assigned to    \r" ;
	}

	// Waiting, joining threads (and cleaning)
	int i = seqPThs.size() ;
	for (auto t = ths_begin ; i > 0 ; ) {
		if (t != ths_end) {
			cout << "\r--- # slices processed: " << slcProcessed.load() << "      \r" ;
			if ((t->first).joinable() && is_task_ready(t->second)) {
				(t->first).join(), i-- ;
				// cout << "\r--- tasks still working: " << i << "     " ;
			}
			++t ;
		} else t = ths_begin ;
	}

	cout << endl << "> all done: " << std::setprecision(3)
		<< (mDuration_sec(mClock::now() - t0)).count() << " s" ;
	/*
	cout << endl << endl << ">>> atom checks: " << atomChecks << " -> " << atomChecks / (1920 * 1080)
		<< " p/p; boxChecks: " << boxChecks << " -> " << boxChecks / (1920 * 1080) 
		<< " p/p; boxMisses: " << boxMisses ;
	*/
	return ;
} // render_slicesV3(): use threading based on std::atomic

																		// eot concurrency
#endif
// eof rays_RT.h