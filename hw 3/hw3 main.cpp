// hw3_main.cpp:  launches
//		- rt_hw_3(): 
//				{ creates 2 files following different approached to negative components
//					- crt_output_hw3_1.ppm in .\ppm folder
//					- crt_output_hw3_1_1.ppm in .\ppm folder
//				}
//				{
//				  combine the two approaches
//				}

//		Auxiliary functions contained in here:
//			- randomCRT(): generates random components 
//			vect_to_CRT()
//			- show_lapse(): shows the time lapse between calls
//		Auxiliary types contained in here:
//			- class mCircle: function objects to define a circle of different colours
//				- C&A
//				- operator(): passed as an argument
//


#include <exception>
#include <time.h>
#include <math.h>

#include <functional>

time_t show_lapse(const bool fl_start) ;

#include "color_RT.h"
#include "geometry_RT.h"
#include "grids_RT.h"
#include "rays_RT.h"

mColor randomCRT(const bool fl_start) ;
mColor vect_to_CRT(const ray_RT& vrt, std::function<float(float)>) ;

void   rt_hw_3() ;


constexpr unsigned int grWidth = 1920 ;				// Resolution
constexpr unsigned int grHeight = 1080 ;			// ...
const char *           ppmF1_Name = "ppm\\crt_output_hw3_1.ppm" ;   // Output file names
const char *           ppmF1_Nam1 = "ppm\\crt_output_hw3_1_1.ppm" ; // ...
const char *           ppmF2_Name = "ppm\\crt_output_hw3_2.ppm" ;   // ...


int
main()
{
	cout << endl << endl, show_lapse(true) ;

	try {
		cout << endl << endl << "> using rt_hw_3_1() to create:" 
			<< endl << ">>> <" << ppmF1_Name << "> via abs(negative)"
			<< endl << ">>> <" << ppmF1_Nam1 << "> via (1 + negative)" ;
		rt_hw_3() ;

		cout << endl << endl << "-- end of try block - ", show_lapse(false) ;
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << "--- That's it ..." << endl << endl ;
	return(0) ;
} // main()


auto lll_abs = [](float x)->float {return(abs(x)) ; } ;  // use abs(negative) for color Components
auto lll_less = [](float x)->float { return((x < 0) ? 1 + x : x) ; } ; // [-1,0] -> [0,1]

class mCircle {		// used as a function object in render_2Dimage() -> apply_bin_value()
	private:
	coord_RASTER	_c ;
	unsigned int	_r ;
	public:
	mCircle(const coord_RASTER& p, const unsigned int r) : _c{p}, _r{r} {} ;

	template <typename T, typename R, typename V>
	mColor operator() (T& t, R& s, V& v, size_t i, size_t j) ;
	mCircle(const mCircle& c) = default ;
	mCircle& operator =(const mCircle& c) = default ;
}; // class mCircle

template <typename T, typename R, typename V> mColor
mCircle::operator ()(T& t, R& s, V& v, size_t i, size_t j)  // from (top, left) = (0, 0)
{
	size_t	temp = ((i - _c._x) * (i - _c._x) + (j - _c._y) * (j - _c._y)) ;
	size_t	r2 = _r * _r ;
	if ((temp * 40) <= r2) 			return(vect_to_CRT(s, lll_less)) ;
	else if ((temp * 20) <= r2)		return(vect_to_CRT(s, lll_abs)) ;
	else if (temp * 5 <= r2)		return(vect_to_CRT(s, lll_less)) ;
	else if (temp * 2 <= r2)		return(vect_to_CRT(s, lll_abs)) ;

	return(vect_to_CRT(s, lll_less)) ;
} // mCircle operator()


void
rt_hw_3()
{

	render_RT<grWidth, grHeight>   test ;
	cout << endl << "> grid of rays initiated: ", show_lapse(false) ;
	// cout << test ;        // show parameters

	gridImage<grWidth, grHeight>	gIm ;

	{ // create two files with different approach to negative rays components
		slcRays							all(coord_RASTER(0, 0), grHeight, grWidth, grWidth) ;
		{
			// move negative component [-1,0] -> [0,1]
			auto setc = [&](typename gridRays<grWidth, grHeight>::value_type& j)->mColor
			{ return(vect_to_CRT(j, lll_less)) ; } ;
			cout << endl << endl << ">> painting starts (1 + [-1, 0)): ", show_lapse(false) ;
			test.render_image(setc, 0, all, gIm) ;
			cout << endl << endl << ">>> Done painting: ", show_lapse(false) ;

			grid_to_ppm(gIm, ppmF1_Nam1) ;		// generate the file
		}
		{
			// use abs(negative) for color Components
			auto setc = [&](typename gridRays<grWidth, grHeight>::value_type& j)->mColor
			{ return(vect_to_CRT(j, lll_abs)) ; } ;
			cout << endl << endl << ">> painting starts (abs([-1, 0]): ", show_lapse(false) ;
			test.render_image(setc, 0, all, gIm) ;
			cout << endl << endl << ">>> Done painting: ", show_lapse(false) ;

			grid_to_ppm(gIm, ppmF1_Name) ;		// generate the file
		}
	}
	{ // combine both approaches to negative
		constexpr unsigned int   wTop = 250 ; // 290 ;
		constexpr unsigned int   wLeft = 660 ; // 710 ;
		constexpr unsigned int   wRows = 600 ; // 500 ;
		constexpr unsigned int   wCols = 600 ; // 500 ;

		slcRays					gsl(coord_RASTER(wTop, wLeft), wRows, wCols, grWidth) ;

		cout << endl << endl << ">> painting of circles starts: ", show_lapse(false) ;

		test.render_2Dimage(mCircle(coord_RASTER(250, 250), 250), gsl, gIm) ;
		cout << endl << endl << ">>> Done painting: ", show_lapse(false) ;

		grid_to_ppm(gIm, ppmF2_Name) ;
	}
} // rt_hw_3()

mColor
vect_to_CRT(const ray_RT& vrt, std::function<float(float)> f)
{
	constexpr CompT maxCC = 255 ;	// or, the max(CompT) could be used
	typename ray_RT::value_type x, y, z ;
	vrt.extract(x, y, z) ;
	return(mColor(static_cast<CompT>(f(x) * maxCC), 
				  static_cast<CompT>(f(y) * maxCC), 
				  static_cast<CompT>(f(z) * maxCC))) ;
} // vect_to_CRT

mColor
randomCRT(const bool fl_start) // if NOT RANDOM enough -> a unique list could be built
{
	constexpr CompT maxColorComponent = 255 ;	// or, the max(CompT) could be used

	if (fl_start)	srand(static_cast<unsigned int>(time(0))) ;
	return(mColor(rand() % maxColorComponent,
				  rand() % maxColorComponent,
				  rand() % maxColorComponent)
		   ) ;
} // randomCRT

time_t
show_lapse(const bool fl_start)  // returns the lapse (in ...) since the start (fl_start)
{
	static time_t	start_lapse{0} ;
	static time_t	break_point{0} ;
	time_t			end_lapse{0} ;

	if (fl_start) {
		time(&start_lapse), break_point = start_lapse ;
		cout << "_____ start at: " << start_lapse ;
	} else if (start_lapse == 0) {   // has never been started
		time(&end_lapse) ;
		cout << endl << "___ " << end_lapse << ": the stopwatch is not running ..." << endl ;
	} else {
		time(&end_lapse) ;
		cout << "_____ break -> lapse since: start(" << (end_lapse - start_lapse) << ')' ;
		cout << ", last break(" << (end_lapse - break_point) << ')' ;
		break_point = end_lapse,
			end_lapse -= start_lapse ;
	}
	return(end_lapse) ;
} // show_lapse()


// eof hw3 main.cpp