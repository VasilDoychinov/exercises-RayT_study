// ray_RT.cpp: 
//		- _calculate[_src]_rays(): fill in render_RT::_rays  ->  used with std::thread
//		  takes grid reference, camera and grid's W x H
//		- camera(s): add, next, etc
//		- image to .ppm file
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
			r_temp = camera.camera_ray(r_temp) ;

			rays_ref(y, x) = unitV(r_temp) ; // ray_RT(x_temp, y_temp, -1.)) ;  // normalize() ;
		}
	}
	return ;
} // render_RT friend _calculate_slc_rays()


void
render_RT::_calculate_rays(unsigned int camId)
{
	for (auto slc = begin(_r_slices) ; slc != end(_r_slices) ; ++slc) {

		slcRays s = slc->first ;			// just in case
		camera_RT cam{_origins[camId]} ;	// ...

		slc->second = std::thread(_calculate_slc_rays, _rays(s), cam, _rays.W(), _rays.H()) ;
	}
	for (auto slc = begin(_r_slices) ; slc != end(_r_slices) ; ++slc) {
		if (slc->second.joinable())   slc->second.join() ;
	}

	return ;
} // render_RT:: _calculate_rays()


render_RT::render_RT(unsigned int W, unsigned int H, const camera_RT& camWS) 
					: _origins{}, _rays(W, H), _currentCamera{0}, _r_slices{}
{
	_origins.push_back(camWS) ;

	std::vector<slcRays>	slc{slices_for_threads(W,H)} ;
																		// for (const auto& s : slc)   
																		//     cout << endl << "--- " << s ;
	for (auto t : slc) {
		_r_slices.push_back(std::make_pair<slcRays, std::thread>(std::move(t), {}));
	}
	_r_slices.shrink_to_fit() ;

	_calculate_rays(_currentCamera) ;
} // render_RT ()


camera_RT
render_RT::add_Cameras(std::function<camera_RT(RT_default_type, camera_RT)> funct,
					   RT_default_type step, unsigned int stepsNum,
					   camera_RT first_cam)
{
	camera_RT	cam{first_cam} ;
	for (; stepsNum > 0 ; stepsNum--) {
		this->add_camera(cam = funct(step, cam)) ;
	}
	return cam ;
} // render_RT add_Cameras()


void
render_RT::del_camera(unsigned int ind)
{
	if (_origins.size() <= 1)		return ;		// the WS is an invariant

	if (ind >= _origins.size())		ind = 0 ;		// indicate pop_back() 
	if (ind == 0)					_origins.pop_back() ;
	else							_origins.erase(begin(_origins) + ind) ;

	return ;
} // render_RT del_camera ()

unsigned int
render_RT::_transform_rays(unsigned int cameraId) // match _ray to cameraId(simplified for now)		
{
	assert((cameraId) < _origins.size()) ;

	// NB: the iteration allowed is ONE STEP FORWARD ONLY
	// transform _rays to cameraId
	_calculate_rays(cameraId) ;

	return cameraId ;
} // render_RT:: _transform_rays()

unsigned int
render_RT::next_camera()	// returns the ID: leaves it ready for rendering, etc 
{
	if (_currentCamera + 1 >= _origins.size())		return _origins.size() ;
	_currentCamera++ ;

	cout << endl << "> switching to camera #" << _currentCamera << " of " << _origins.size() ;
	auto t0 = mClock::now() ;

	// NB: the rays in _rays must match _currentCamera. 
	_transform_rays(_currentCamera) ;

	cout << "> ready after: " << std::setprecision(3)
		<< (mDuration_sec(mClock::now() - t0)).count() << " s";
	return _currentCamera ;
} // render_RT next_camera()


void
grid_to_ppm(const gridImage& gr, const std::string& fname)
{
	constexpr int maxColorComponent = 255;
	try {
		cout << endl << "  to <" << fname ; // << endl ;
		cl_KERNEL	ker(th_counter, 700) ;

		// output .ppm file header
		std::ofstream ppmFileStream(fname.c_str(), std::ios::out | std::ios::binary);
		ppmFileStream << "P3\n";
		ppmFileStream << gr.W() << " " << gr.H() << "\n";
		ppmFileStream << maxColorComponent << "\n";

		size_t count = 1 ;
		size_t rows = 0 ;
		for (auto pix : gr.data()) {
			ppmFileStream << pix << '\t' ;
			/*
			if (count++ % gr.W() == 0) {
				if (rows++ % 100 == 0) cout << "\r> to .ppm-> row: " << rows ; // << "\r" ;
			}*/
		}
		ppmFileStream.close();
		// cl_KERNEL used: cout << " -> Done." << endl ;
	} catch (...) { throw ; }
} // image to ppm file

std::ostream&
show_cameras(std::ostream& os, render_RT& r)
{
	os << endl << "--- cameras are:" ;

	unsigned int i{0} ;
	for (auto cam = cbegin(r._origins) ; cam != cend(r._origins) ; ++cam, i++) {
		os << endl << "> #" << i << ": " << *cam << (i == r._currentCamera ? ": current" : "") ;
	}

	os << endl << "--- eol cameras---" ;
	return os ;
} // render_RT friend show_cameras() 


std::ostream&
operator <<(std::ostream& os, render_RT& r)
{
	os << endl << endl
		<< "- render_RT -> resolution (" << r._rays.W() << ", " << r._rays.H() << ") " << endl
		<< "--- # of cameras: " << r._origins.size() << "-> 1st: " << r._origins[0].position() << endl
		<< "--- rays are: " << r._rays << endl ;

	mGrid2_slice		all(coord_RASTER(0, 0), r._rays.H(), r._rays.W(), r._rays.W()) ;
	mGrid2_ref<ray_RT>	rays_ref(r._rays(all)) ;

	// show the pixels at the four corners
	os << "--- the 4 corners are:" << endl ;
	os << "   " << rays_ref(0, 0) ; os << "   " << rays_ref(0, r._rays.W() - 1) << endl ;
	os << "   " << rays_ref(r._rays.H() - 1, 0) ; os << "   " << rays_ref(r._rays.H() - 1, r._rays.W() - 1) ;

	return os ;
} // render_RT friend operator <<


// eof rays_RT.cpp