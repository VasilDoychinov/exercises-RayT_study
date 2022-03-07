// hw2_main.cpp:  launches
//		- rt_hw2_1(): generates file: crt_output_hw2_1.ppm for task 1 in .\ppm folder
//						NB: the colors generated will differ from launch to launch
//		- rt_hw2_2(): generates file: crt_output_hw2_2.ppm for task 2 ...
// 
//		Auxiliary functions contained in here:
//			- randomCRT(): generates random components 
//			- show_lapse(): shows the time lapse between calls
//		Auxiliary types contained in here:
//			- class mCircle: function objects to define a circle
//				- C&A
//				- operator(): passed as an argument
//


#include <exception>
#include <time.h>

time_t show_lapse(const bool fl_start) ;

#include "rthw2.h"

mPixel randomCRT(const bool fl_start) ;
void   rt_hw_2_1() ;
void   rt_hw_2_2() ;

constexpr unsigned int grWidth = 1920 ;				// Resolution
constexpr unsigned int grHeight = 1080 ;			// ...
const char *           ppmF1_Name = "ppm\\crt_output_hw2_1.ppm" ; // Output file names
const char *           ppmF2_Name = "ppm\\crt_output_hw2_2.ppm" ; // ...

int
main()
{
	cout << endl << endl, show_lapse(true) ;

	try {
		cout << endl << endl << "> using rt_hw_2_1() to generate <" << ppmF1_Name ;
		rt_hw_2_1() ;

		cout << endl << endl << "> using rt_hw_2_2() to generate <" << ppmF2_Name ;
		rt_hw_2_2() ;

		cout << endl << endl << "-- end of try block - ", show_lapse(false) ;
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << "--- That's it ..." << endl << endl ;
	return(0) ;
} // main()

void
rt_hw_2_1()
{
	constexpr unsigned int	RowUnits = 5 ;							// Horizontal blocks
	constexpr unsigned int	ColUnits = 5 ;							// Vertical
	constexpr unsigned int	RectHorSize = grWidth / RowUnits ;
	constexpr unsigned int	RectVerSize = grHeight / ColUnits ;

	mGrid2<grWidth, grHeight>   grid(randomCRT(true)) ;

	cout << endl << endl << ">> painting starts: ", show_lapse(false) ;
	for (int rowIdx = 0 ; rowIdx < grHeight ; rowIdx += RectVerSize) {
		for (int colIdx = 0; colIdx < grWidth; colIdx += RectHorSize) {
			mGrid2_slice gsl(mCoord<2>(rowIdx, colIdx), RectVerSize, RectHorSize, grWidth) ;
			mGrid2_ref   grf(grid(gsl)) ;

			grf.paint(randomCRT(false)) ; // for testing: [](unsigned int i, unsigned int j){ return(true) ;}) ;
			grf.paint(randomCRT(false), [&](const unsigned int i, const unsigned int j)
											{ return (i < RectVerSize / 4 && j < RectHorSize / 6) ; }
					 ) ;
		}
	}
	cout << endl << endl << ">>> Done painting: ", show_lapse(false) ;

	grid_to_ppm(grid, ppmF1_Name) ;		// generate the file
} // rt_hw_2_1()


class mCircle {
	private:
		mCoord<2>		_c ;
		unsigned int	_r ;
	public:
		mCircle(const mCoord<2>& p, const unsigned int r) : _c{p}, _r{r} {} ;
		bool operator() (unsigned int i, unsigned int j) ;
		mCircle(const mCircle& c) = default ;
		mCircle& operator =(const mCircle& c) = default ;
}; // class mCircle

bool
mCircle::operator ()(unsigned int i, unsigned int j)  // from (top, left) = (0, )0
{
	return(
		((i - _c._x) * (i - _c._x) + (j - _c._y) * (j - _c._y))
		<= (_r * _r)
		) ;
} // mCircle operator()

void
rt_hw_2_2()
{
	const mPixel greyP(160, 160, 160) ;
	const mPixel greenP(0, 102, 51) ;
	constexpr unsigned int   wTop = 290 ;
	constexpr unsigned int   wLeft = 710 ;
	constexpr unsigned int   wRows = 500 ;
	constexpr unsigned int   wCols = 500 ;


	cout << endl << endl << ">> painting starts: ", show_lapse(false) ;

	mGrid2<grWidth, grHeight>	grid(greyP) ;
	mGrid2_slice				gsl(mCoord<2>(wTop, wLeft), wRows, wCols, grWidth) ;
	mGrid2_ref					grf(grid(gsl)) ;
	
	grf.paint(greenP, mCircle(mCoord<2>(250, 250), 250)) ;
	grf.paint(greyP, mCircle(mCoord<2>(250, 250), 170)) ;

	cout << endl << endl << ">>> Done painting: ", show_lapse(false) ;

	grid_to_ppm(grid, ppmF2_Name) ;
} // rt_hw_2_2()


mPixel
randomCRT(const bool fl_start) // if NOT RANDOM enough -> a unique list could be built
{
	constexpr CompT maxColorComponent = 255 ;	// or, the max(CompT) could be used

	if (fl_start)	srand(static_cast<unsigned int>(time(0))) ;
	return(mPixel(rand() % maxColorComponent, 
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

// eof hw2 main.cpp