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

template <unsigned int W, unsigned int H, typename SCENE> 
class clF_RenderSlice ;									// functor for std::thread -> defined just before eof


template <unsigned int W, unsigned int H>
class render_RT  {
	private:
		std::vector<camera_RT>		_origins{} ;		// seq Cameras. [0] is (0,0,0) WorldSpace
		gridRays<W, H>				_rays{} ;			// ... 
		unsigned int				_currentCamera{} ;	// the one corresponding to _rays

		std::vector<std::pair<slcRays, std::thread>>	_r_slices{} ;
		
	private:
		unsigned int _transform_rays(unsigned int cameraId) ;	// match _ray to cameraId(simplified for now)		
		void		 _calculate_rays(unsigned int camId) ;

	public:
		render_RT() ;
		// ~render_RT() = default ; all special members = default

		template <typename SCENE> void render_image(const slcRays& s,
													gridImage<W, H>& image,
													SCENE& i3D) ;
		template <typename SCENE> gridImage<W,H>  render_image(const slcRays& s, SCENE& i3D) ;

		// Cameras interface
		void add_camera(camera_RT&& cam)		{ _origins.push_back(std::move(cam)) ; return ;}
		void add_camera(const camera_RT& cam)	{ _origins.push_back(cam) ; }
		camera_RT add_Cameras(std::function<camera_RT(RT_default_type, camera_RT)> f,
							 RT_default_type step, unsigned int stepsNum = 1,
							 camera_RT first_cam = {}) ;
		void del_camera(unsigned int ind = 0) ;							// ... pop_back()
		unsigned int num_of_cameras() { return(_origins.size()) ; }

		unsigned int clear_cameras() { render_RT t{} ; std::swap(*this, t) ; return(_origins.size()) ; }
		unsigned int fix_cameras_list() { _origins.shrink_to_fit(); _currentCamera = 0; return(_origins.size());}
		unsigned int rwd_cameras_list() { render_RT t{} ; 
			std::swap(this->_rays, t._rays) ; _currentCamera = 0 ;

			return(_origins.size()) ; 
		}

		unsigned int next_camera() ; // returns # and, leaves rays ready for rendering, etc 
		
		// Friends
		template <unsigned int W, unsigned int H, typename SCENE> friend class clF_RenderSlice ;
		friend void	 _calculate_slc_rays(refRays r_ref, camera_RT cam, 
										 unsigned int w, unsigned int h) ;

		template <unsigned int W, unsigned int H> 
		friend std::ostream& operator <<(std::ostream& os, render_RT<W,H>& r) ;
		template <unsigned int W, unsigned int H>
		friend std::ostream& show_cameras(std::ostream& os, render_RT<W, H>& r) ;
}; // class render_RT <>

void _calculate_slc_rays(refRays r_ref, camera_RT cam, unsigned int w, unsigned int h) ;

template <unsigned int W, unsigned int H> void
render_RT<W, H>::_calculate_rays(unsigned int camId)
{	
	for (auto slc = begin(_r_slices) ; slc != end(_r_slices) ; ++slc) {

		slcRays s = slc->first ;			// just in case
		camera_RT cam{_origins[camId]} ;	// ...
		
		slc->second = std::thread(_calculate_slc_rays, _rays(s), cam, W, H) ;
	}
	for (auto slc = begin(_r_slices) ; slc != end(_r_slices) ; ++slc) {
		if (slc->second.joinable())   slc->second.join() ;
	}

	return ;
} // render_RT:: _calculate_rays()

template <unsigned int W, unsigned int H> inline
render_RT<W, H>::render_RT() : _origins{}, _rays{}, _currentCamera{0}, _r_slices{}
{
	_origins.push_back(camera_RT{}) ;

	std::vector<slcRays>	slc{slices_for_threads(W,H)} ;
											// for (const auto& s : slc)   cout << endl << "--- " << s ;
	for (auto t : slc) {
		_r_slices.push_back(std::make_pair<slcRays, std::thread>(std::move(t), {}));
	}
	_r_slices.shrink_to_fit() ;

	_calculate_rays(_currentCamera) ;
} // render_RT ()


template <unsigned int W, unsigned int H> camera_RT
render_RT<W, H>::add_Cameras(std::function<camera_RT(RT_default_type, camera_RT)> funct,
							 RT_default_type step, unsigned int stepsNum,
							 camera_RT first_cam)
{
	camera_RT	cam{first_cam} ;
	for ( ; stepsNum > 0 ; stepsNum--) {
		this->add_camera(cam = funct(step, cam)) ;
	}
	return(cam) ;
} // render_RT add_Cameras()


template <unsigned int W, unsigned int H> void
render_RT<W, H>::del_camera(unsigned int ind)
{
	if (_origins.size() <= 1)		return ;		// the WS is an invariant

	if (ind >= _origins.size())		ind = 0 ;		// indicate pop_back() 
	if (ind == 0)					_origins.pop_back() ;
	else							_origins.erase(begin(_origins) + ind) ;

	return ;
} // render_RT del_camera ()

template <unsigned int W, unsigned int H> unsigned int
render_RT<W, H>::_transform_rays(unsigned int cameraId) // match _ray to cameraId(simplified for now)		
{
	assert((cameraId) < _origins.size()) ; 

	// NB: the iteration allowed is ONE STEP FORWARD ONLY
	// transform _rays to cameraId
	_calculate_rays(cameraId) ;

	return(cameraId) ;
} // render_RT:: _transform_rays()

template <unsigned int W, unsigned int H> unsigned int 
render_RT<W, H>::next_camera()	// returns the ID: leaves it ready for rendering, etc 
{
	if (_currentCamera + 1 >= _origins.size())		return(_origins.size()) ;
	_currentCamera++ ;

	cout << endl << "> switching to camera #" << _currentCamera << " of " << _origins.size() ;
	auto t0 = mClock::now() ;

	// NB: the rays in _rays must match _currentCamera. 
	_transform_rays(_currentCamera) ;
	
	cout << "> ready after: " << std::setprecision(3) 
		<< (mDuration_sec(mClock::now() - t0)).count() << " s";
	return(_currentCamera) ;
} // render_RT next_camera()


template <typename SCENE> void
show_idScene(SCENE& i3D)
{
	cout << endl << "- rendering image(s) of a slice for i3D{" << typeid(i3D).name() << "}";
	cout << " defined as " << endl << i3D << endl ;
}

template <unsigned int W, unsigned int H> 
template <typename SCENE> void
render_RT<W, H>::render_image(const slcRays& s, 
							  gridImage<W,H>& image,
							  SCENE& i3D)
{
	cl_KERNEL	ker(th_counter, 700) ;									// show_idScene(i3D) ;
	
	ray_RT ray{(_origins[_currentCamera]).position()} ;
	cout << endl << "> camera #" << _currentCamera ;

	apply_value(image(s), _rays(s), i3D, ray) ; // (_origins[o]).position()) ;
	return ;
} // render_RT render_image(image)

template <unsigned int W, unsigned int H>
template <typename SCENE> gridImage<W, H>
render_RT<W, H>::render_image(const slcRays& s, SCENE& i3D)
{
	gridImage<W,H>	temp ;
	render_image(s, temp, i3D) ;
	return(temp) ;
} // render_RT render_image()


#include <fstream>

template <unsigned int W, unsigned int H> void
grid_to_ppm(const gridImage<W, H>& gr, const std::string& fname)
{
	constexpr int maxColorComponent = 255;
	try {
		cout << endl << "  to <" << fname ;
		cl_KERNEL	ker(th_counter, 700) ;
		
		// output .ppm file header
		std::ofstream ppmFileStream(fname.c_str(), std::ios::out | std::ios::binary);
		ppmFileStream << "P3\n";
		ppmFileStream << W << " " << H << "\n";
		ppmFileStream << maxColorComponent << "\n";

		size_t count = 1 ;
		size_t rows = 0 ;
		for (auto pix : gr.data()) {
			ppmFileStream << pix << '\t' ;
		}
		ppmFileStream.close();
		// end of 'ker' scope:
	} catch (...) {	throw ;	}																	
} // image to ppm file

template <unsigned int W, unsigned int H> std::ostream&
show_cameras(std::ostream& os, render_RT<W, H>& r)
{
	os << endl << "--- cameras are:" ;

	unsigned int i{0} ;
	for (auto cam = cbegin(r._origins) ; cam != cend(r._origins) ; ++cam, i++) {
		os << endl << "> #" << i << ": " << *cam << (i == r._currentCamera ? ": current" : "") ;
	}

	os << endl << "--- eol cameras---" ;
	return(os) ;
} // render_RT friend show_cameras() 


template <unsigned int W, unsigned int H> std::ostream&
operator <<(std::ostream& os, render_RT<W,H>& r)
{
	os << endl << endl
		<< "- render_RT -> resolution (" << W << ", " << H << ") " << endl
		<< "--- # of cameras: " << r._origins.size() << "-> 1st: " << r._origins[0].position() << endl
		<< "--- rays are: " << r._rays << endl ;
	
	mGrid2_slice		all(coord_RASTER(0, 0), H, W, W) ;
	mGrid2_ref<ray_RT>	rays_ref(r._rays(all)) ;

	// show the pixels at the four corners
	os << "--- the 4 corners are:" << endl ;
	os << "   " << rays_ref(0, 0) ; os << "   " << rays_ref(0, W - 1) << endl ;
	os << "   " << rays_ref(H -1, 0) ; os << "   " << rays_ref(H - 1, W - 1) ;

	return(os) ;
} // render_RT friend operator <<
																		// eotemplates for render_RT<>

template <unsigned int W, unsigned int H, typename SCENE>
class clF_RenderSlice {					// functor for std::thread -> runs apply_value() for _currentCamera
	public:
		render_RT<W, H>*	test ;		// Rays and Camera
		gridImage<W, H>*	image ;		// to produce the image in

		const SCENE	*		obj ;		// to render using test._currentCamera

		clF_RenderSlice(render_RT<W, H>* t,
						gridImage<W, H>* im,
						SCENE* xobj) : test{std::move(t)}, 
												  image{std::move(im)},
									    	      obj{std::move(xobj)} {}
		unsigned int camera_id() { return(test->_currentCamera) ; }

	void operator () (slcRays s) {

		apply_value((*image)(s), (test->_rays)(s), *obj, 
					((test->_origins)[test->_currentCamera]).position()) ;
	}
}; // functor clF_RenderSlice


template <typename Funct> void
render_imSlices(Funct func, const std::vector<slcRays>& iSlices)
{
	cout << endl << "> camera #" << func.camera_id() << "; " << iSlices.size() << " threads";
	cl_KERNEL	ker(th_counter, 700) ;

	std::vector<std::thread>	ths{} ;
	for (auto slc = begin(iSlices) ; slc != end(iSlices) ; ++slc) {
		std::thread th((func), *slc) ;
		ths.push_back(std::move(th)) ;
	}
	for (auto th = begin(ths) ; th != end(ths) ; ++th) { if (th->joinable())     th->join() ; }

	return ;
} // render_imSlices(): use simple threading, ie num_of_Threads = num_of_Slices


using pair_PackedTask = std::pair <std::thread, std::future<void >> ;
using vect_PackedTasks = std::vector < pair_PackedTask> ;
using packed_slcRays = std::packaged_task<void(slcRays)> ;

template <typename Funct> pair_PackedTask
_assign_task(Funct func, const slcRays& slc, bool fl_detach = false) // assign a thread to a slice
{
	pair_PackedTask		w_pair ;
	packed_slcRays		do_task(std::move(func)) ;

	w_pair.second = std::move(do_task.get_future()) ;

	std::thread th((func), slc) ; // a slice is assigned to ; 
	w_pair.first = std::move(th) ;
	if (fl_detach)	(w_pair.first).detach() ;
	return(w_pair) ;
} // _assign_task(): assign a thread to a slice

template <typename Funct> void
render_imSlcV2(Funct func, const std::vector<slcRays>& iSlices)
{
	unsigned int nTh{std::thread::hardware_concurrency()} ;
	if (nTh > 1)  nTh-- ;   // 'Max concurrent supported' - 1
	assert(nTh < iSlices.size()) ;  // otherwise, just run render_iSlices()

	cout << endl << "> camera #" << func.camera_id() << ": " << iSlices.size() 
		<< " slices; threads " << nTh ;

	cl_KERNEL	ker(th_counter, 700) ;		// in the place of a future(eventual) Kernel that handles messages

	static vect_PackedTasks		seqPThs(nTh) ;
	unsigned int				slcAssigned = 0 ;

	// Initial assignments
	for (auto t = begin(seqPThs) ; t != end(seqPThs) ; ++t)   {
		*t = std::move(_assign_task(std::move(func), iSlices[slcAssigned++], true)) ;
	}

	// move threads to a next slice (if any)
	while (slcAssigned < iSlices.size())  {// until stopped for slcAssigned reaches iSLices.size(). ??? time-out
		for (auto t = begin(seqPThs) ; t != end(seqPThs) ; ++t) {
			if (is_task_ready(&(t->second))) {	// the thread has finished its job
				if ((t->first).joinable())   (t->first).join() ;
				*t = std::move(_assign_task(std::move(func), iSlices[slcAssigned++])) ;
				if (slcAssigned >= iSlices.size())	break ; // the closer - the better
			}
		}
	}

	// Joining threads (and cleaning)
	for (auto t = begin(seqPThs) ; t != end(seqPThs) ; ++t) { 
		if ((t->first).joinable()) (t->first).join() ; 
	}
	return ;
} // render_imSlcV2(): use more sophisticated threading, ie  num_of_Threads <= num_of_Slices


#endif
// eof rays_RT.h