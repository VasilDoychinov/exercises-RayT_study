// rthw2.h: types, etc. Templates place in here, other (member) functions in rthw2.cpp
//			- a pixel (mPixel) = {Red;Green;Blue}
//			- a grid = seq PixelRows = seq (seq Pixel)
//			- grid slice: describes a sub-grid
//			- grid reference: to references elements of a (sub-)grid
//				* to facilitate concurrency
//			- (top, left) is (0, 0) for grids and slices
//		= mCoord<N>					: co-ordinates from (top, left) = (0, 0)
//			- C&A, operator <<
//		= mPixel					: (R,G,B)
//			- C&A, operator <<
//		= mGrid2					: define a grid of certain (parameterize) resolution
//			friends: MGrid2_ref, mGrid2_slice
//			- C&A, operator << 
//			- operator(): mGrid2_ref
//			- grid_to_ppm()
// 		= mGrid2_slice				: define a slice of a grid. relative to (top, left)
//			- C&A, operator <<
//			- topleft(): offset to grid's (0,0)
//		= mGrid2_ref				: the only access to a grid. NB: NO COPY operations
//			- C&A, operator <<
//			- at(), operator(): access a pixel
//			- paint(): the whole of it 
//			- paint(pred): the ones that fulfill pred()
//

#ifndef _RTHW2_TYPES
#define _RTHW2_TYPES

#include <iostream>
using std::cout ;
using std::endl ;

#include <utility>
#include <vector>
using std::vector ;

#include <functional>

using CompT = unsigned char ;				// for color components

template <unsigned int N> class mCoord {};	// general template class mCoord
template <> class mCoord<2> {				// (i, j) specialization
	public: 
		unsigned int _x ;	// in height
		unsigned int _y ;	// in width 

	public:
		explicit mCoord(unsigned int r, unsigned int c) : _x{r}, _y{c} {} ;
		mCoord(const mCoord& xy) = default ;
		mCoord& operator =(const mCoord& xy) = default ;

		friend std::ostream& operator <<(std::ostream& os, const mCoord& co) ;
}; // mCoord<2>
template <> class mCoord<3> {				// (i, j, k) specialization
	public:
		unsigned int _x ;
		unsigned int _y ;	
		unsigned int _z ;
}; // mCoord<3>

class mPixel {								// (R,G,B)
	private:
		CompT	_cR ;
		CompT	_cG ;
		CompT	_cB ;
	public:
		mPixel(CompT r = 0, CompT g = 0, CompT b = 0) : _cR{r}, _cG{g}, _cB{b} {}
		mPixel(const mPixel& p) = default ;
		mPixel& operator =(const mPixel& p) = default ;

	friend std::ostream& operator <<(std::ostream& os, const mPixel& p) ;
}; // class mPixel

class mGrid2_ref ;
struct mGrid2_slice ;

template <unsigned int W, unsigned int H>
class mGrid2 {
	private:
		vector<mPixel>	_base ;

	public:
		static constexpr unsigned int	_width = W ;
		static constexpr unsigned int	_height = H ;

	public:
		explicit mGrid2() : _base(_width * _height) {} ;
		explicit mGrid2(const mPixel& p) : _base(_width * _height, p) {} ;
		~mGrid2() = default ;

		explicit mGrid2(const mGrid2& g) : _base{g._base} {} ;					// Copy constructor
		explicit mGrid2(mGrid2&& g) noexcept : _base{std::move(g._base)} {} ;	// Move ...

		mGrid2& operator =(const mGrid2& g) ;									// Copy assignment
		mGrid2& operator =(mGrid2&& g) noexcept ;								// Move ...

		mGrid2_ref operator ()(const mGrid2_slice& slc) ;
		// mPixel* base() { return(_base.data()) ; }

		template <unsigned int W, unsigned int H>
		friend void grid_to_ppm(const mGrid2<W,H>& gr, const char * fname) ;

		template <unsigned int W, unsigned int H>
		friend std::ostream& operator << (std::ostream& os, const mGrid2<W,H>& gr) ;

		friend class mGrid2_ref ;
}; // class mGrid2

struct mGrid2_slice {
		mCoord<2>		_start ;
		unsigned int	_rnum ;
		unsigned int	_cnum ;
		unsigned int	_step ;

		explicit mGrid2_slice(const mCoord<2>& rc, unsigned int rn, unsigned int cn, unsigned int step) 
								: _start{rc}, _rnum{rn}, _cnum{cn}, _step{step} {} 
		explicit mGrid2_slice(const mGrid2_slice& msl) = default ;
		mGrid2_slice& operator =(const mGrid2_slice& msl) = default ;

		size_t topleft() const ;
		friend std::ostream& operator <<(std::ostream&, const mGrid2_slice&) ;
}; // struct mGrid2_slice 

class mGrid2_ref {	// Bound to mGrid2: an object not to be outside the scope of mGrid2 Object(or seq mPixel)
	private:
		mGrid2_slice	_descr ;
		mPixel *		_pix ;
		
	public:
		mGrid2_ref(const mGrid2_slice& s, mPixel *p) : _descr{s}, _pix{p} {}
		~mGrid2_ref() = default ;

		mGrid2_ref(const mGrid2_ref&) = delete ;				// Constructors Copy & Move
		mGrid2_ref(mGrid2_ref&& r) noexcept : _descr{std::move(r._descr)}, _pix{std::move(r._pix)} {}

		mGrid2_ref& operator =(const mGrid2_ref&) = delete ;	// Assignments Copy & Move
		mGrid2_ref& operator =(mGrid2_ref&& ref) noexcept 
								{ _descr = std::move(ref._descr), _pix = std::move(ref._pix) ; }

		void paint(const mPixel& p) ;
		void paint(const mPixel& p, std::function<bool(unsigned int, unsigned int)> pred) ;

 		mPixel& operator ()(unsigned int i, unsigned int j) ;	// acces a pixel, No range check
		mPixel& at(unsigned int i, unsigned int j) ;			// acces a pixel, range check

		friend std::ostream& operator <<(std::ostream& os, const mGrid2_ref& gr) ;
}; // class mGrid2_ref


																		// operator(s) << 
std::ostream& operator <<(std::ostream& os, const mCoord<2>& co) ;
std::ostream& operator <<(std::ostream& os, const mGrid2_ref& gr) ;

																		// templates for mGrid2<> follow
template <unsigned int W, unsigned int H> mGrid2_ref
mGrid2<W, H>::operator ()(const mGrid2_slice& slc)
{										// Speed ??? check for complying slice or not
	if (slc._start._x >= _height || slc._start._y >= _width)	throw std::range_error("mGrid2: Wrong base") ;
	if (slc._cnum > _width - slc._start._y)						throw std::range_error("mGrid2: Wrong width") ;
	if (slc._rnum > _height - slc._start._x)					throw std::range_error("mGrid2: Wrong height") ;
	if (slc._step != _width)									throw std::range_error("mGrid2: Wrong step") ;

	return(mGrid2_ref(slc, _base.data() + slc.topleft())) ;
} // mGrid2 operator (slice)

template <unsigned int W, unsigned int H> mGrid2<W, H>&
mGrid2<W, H>::operator =(const mGrid2& gr)
{
	_base = gr._base ;
	return(*this) ;
} // mGrid2 operator=(&)

template <unsigned int W, unsigned int H> mGrid2<W, H>&
mGrid2<W, H>::operator =(mGrid2&& gr) noexcept
{
	_base = std::move(gr._base) ;
	return(*this) ;
} // mGrid2 operator=(&&)

template <unsigned int W, unsigned int H> std::ostream&
operator <<(std::ostream& os, const mGrid2<W, H>& gr)
{
	cout << endl ;
	cout << endl << "- grid {" << gr._width << ", " << gr._height << "}" ;
	cout << endl << "--- holding " << (gr._base).size() << " pixels at (" << (gr._base).data() << ")" ;
	cout << endl << "--- capacity " << (gr._base).capacity() << ")" << endl ;

	return(os) ;
} // mGrid2 operator <<

#include <fstream>
template <unsigned int W, unsigned int H> void
grid_to_ppm(const mGrid2<W, H>& gr, const char * fname)
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
			if (count++ % W == 0)  { ppmFileStream << "\n" ; rows++ ; }
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
} // mGrid2 to ppm file

// eotemplates for mGrid2<>

																		// class mGrid2_ref
inline mPixel&
mGrid2_ref::operator ()(unsigned int i, unsigned int j)		// No range check.
{
	return(*(_pix + (i * (_descr._step)) + j)) ;
} // mGrid2_ref operator ()
																		// eoc mGrid2_ref

#endif
// eof rthw2.h