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
//

#ifndef _DEFS_RAYS_RT_RAYS
#define _DEFS_RAYS_RT_RAYS

#include <iostream>
#include <cmath>

#include <functional>

#include "geometry_RT.h"
#include "color_RT.h"
#include "grids_RT.h"


using											ray_RT = vector_RT<float> ;
using											slcRays = mGrid2_slice ;
using											slcImage = mGrid2_slice ;
using											refRays = mGrid2_ref<ray_RT> ;
using											refImage = mGrid2_ref<mColor> ;
template <unsigned int W, unsigned int H> using gridImage = mGrid2<mColor, W, H> ;
template <unsigned int W, unsigned int H> using gridRays = mGrid2<ray_RT, W, H> ;


template <unsigned int W, unsigned int H>
class render_RT  {
	private:
		vector<ray_RT>		_origins ;		// holds different origins. [0] is (0,0,0)
		gridRays<W, H>		_rays ;			// ... 

	public:
		explicit render_RT() ; 
		~render_RT() = default ;

		explicit render_RT(const render_RT&) = default ;
		render_RT& operator =(const render_RT&) = default ;

		render_RT(render_RT&& r) : _origins{std::move(r._origins)}, _rays{std::move(r._rays)} {}
		render_RT& operator =(render_RT&& r) { 
			_origins = std::move(r._origins), _rays = std::move(r._rays) ;
			return(*this) ;
		}
		
		template <typename SCENE> gridImage<W,H>& render_image(unsigned int o, 
						 								  const slcRays& s, 
														  gridImage<W,H>& image,
														  SCENE i3D) ;					
		template <typename SCENE> gridImage<W,H>  render_image(unsigned int o, 
														  const slcRays& s,
														  SCENE i3D) ;
		template <typename F> void render_image(F f, 
												unsigned int o, 
												const slcRays& s, gridImage<W, H>& image) ;
		template <typename F> void render_2Dimage(F f, const slcRays& s, gridImage<W, H>& image) ;
		
		template <unsigned int W, unsigned int H> 
		friend std::ostream& operator <<(std::ostream& os, render_RT<W,H>& r) ;
}; // class render_RT <>

/* Chaos: Raster to World Space, normalized rays
* At each pixel
*	Find its center, based on the raster coordinates: 	x += 0.5; y += 0.5
*	Convert raster coordinates to NDC space[0.0, 1.0]:	x /= width; y /= height*
*	Convert NDC coordinates to Screen space[-1.0, 1.0]:	x = (2.0 * x) - 1.0
*														y = 1.0 - (2.0 * y)
*	Consider the aspect ratio:							x *= width / height
*	Ray direction = :									(x, y, -1.0)
*	Normalize ray direction vector
*	Store the ray with the calculated direction and origin
*/

template <unsigned int W, unsigned int H>
render_RT<W, H>::render_RT() : _origins{}, _rays{} // _origins(1, {}), _image{}
{
	_origins.push_back(ray_RT(0.,0.,0.)) ;

	slcRays		all(coord_RASTER(0, 0), H, W, W) ;
	refRays		rays_ref(_rays(all)) ;

	constexpr float x_coefNDC = (1 / static_cast<float>(W)) ;
	constexpr float y_coefNDC = (1 / static_cast<float>(H)) ;
	constexpr float aspect_ratio = (static_cast<float>(W) / static_cast<float>(H)) ;

	ray_RT  r_temp ;
	float   x_temp , y_temp ;
	for (unsigned int x = 0 ; x < W ; x++) {
		for (unsigned int y = 0 ; y < H ; y++) {
			x_temp = x + 0.5f , y_temp = y + 0.5f ;					// Raster space
			x_temp *= x_coefNDC , y_temp *= y_coefNDC ;				// NDC space
			x_temp = (2 * x_temp) - 1, y_temp = 1 - (2 * y_temp) ;	// Screen space
			x_temp *= aspect_ratio ;
								
			r_temp = ray_RT(x_temp, y_temp, -1.) ;
			rays_ref(y, x) = r_temp.normalize() ;  // normalize() ;
		}
	}
} // render_RT ()

template <unsigned int W, unsigned int H>
template <typename F> inline void
render_RT<W, H>::render_2Dimage(F f, const slcRays& s, gridImage<W, H>& image)
// std::function<mColor(mColor&, ray_RT&)> f,
{
	apply_bin_value(image(s), _rays(s), f, 0) ;
} // render_RT render_image(image, function)

template <unsigned int W, unsigned int H>
template <typename F> inline void
render_RT<W, H>::render_image(F f, unsigned int o, const slcRays& s, gridImage<W, H>& image)
{
	apply_unary(image(s), _rays(s), f) ;
} // render_RT render_image(image, function)

template <unsigned int W, unsigned int H> 
template <typename SCENE> gridImage<W,H>&
render_RT<W, H>::render_image(unsigned int o,
							  const slcRays& s, 
							  gridImage<W,H>& image,
							  SCENE i3D)
{
	
	cout << endl << endl << "---- rendering: update image for i3D" ;
	
	return(image) ;
} // render_RT render_image(image)

template <unsigned int W, unsigned int H>
template <typename SCENE> gridImage<W, H>
render_RT<W, H>::render_image(unsigned int o, 
							  const slcRays& s, SCENE i3D)
{
	cout << endl << endl << "---- rendering: creating empty image for i3D" ;
	gridImage<W,H>	temp ;
	render_image(0, s, temp, i3D) ;
	return(std::move(temp)) ;
} // render_RT render_image()


#include <fstream>

template <unsigned int W, unsigned int H> void
grid_to_ppm(const gridImage<W, H>& gr, const char * fname)
{
	constexpr int maxColorComponent = 255;
	try {
		cout << endl << endl << "> Generating <" << fname << "> ..." ;
		// output .ppm file header
		std::ofstream ppmFileStream(fname, std::ios::out | std::ios::binary);
		ppmFileStream << "P3\n";
		ppmFileStream << W << " " << H << "\n";
		ppmFileStream << maxColorComponent << "\n";

		size_t count = 1 ;
		size_t rows = 0 ;
		for (auto pix : gr._base) {
			ppmFileStream << pix << '\t' ;
			if (count++ % W == 0) { ppmFileStream << "\n" ; rows++ ; }
			if (rows % 100 == 0)   cout << endl << ">>> processing row " << ++rows << " of " << H ;
		} // # of calculations ??????::: But, same time interval as without them

		/* --- takes the same time interval as the above
			auto pix = (gr._base).data() ;
			for (int rowIdx = 0; rowIdx < H ; ++rowIdx) {
				for (int colIdx = 0; colIdx < W ; ++colIdx, pix++) {
					ppmFileStream << *pix << '\t' ;
				}
				ppmFileStream << "\n";
			}
		*/
		ppmFileStream.close();
	} catch (...) {
		throw ;
	}
	cout << endl << ">> Done." ;
} // image to ppm file


template <unsigned int W, unsigned int H> std::ostream&
operator <<(std::ostream& os, render_RT<W,H>& r)
{
	os << endl << endl
		<< "- render_RT -> resolution (" << W << ", " << H << ") " << endl
		<< "-- # of origins: " << r._origins.size() << "-> 1st: " << r._origins[0] << endl
		<< "-- rays are: " << r._rays << endl ;
	
	mGrid2_slice		all(coord_RASTER(0, 0), H, W, W) ;
	mGrid2_ref<ray_RT>	rays_ref(r._rays(all)) ;

	// show the pixels at the four corners
	cout << "--- the 4 corners are:" << endl ;
	cout << "   " << rays_ref(0, 0) ; cout << "   " << rays_ref(0, W - 1) << endl ;
	cout << "   " << rays_ref(H -1, 0) ; cout << "   " << rays_ref(H - 1, W - 1) ;

	return(os) ;
} // render_RT operator <<
																		// eotemplates for render_RT<>

#endif

// eof ray_RT.h