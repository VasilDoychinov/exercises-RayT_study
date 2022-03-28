// grids_RT.h: types, etc. Templates placed in here, other (member) functions in grids_RT.cpp
//			- an element(pixel) is the "atom" (of type value_type)
//			- a grid = seq Ts = or, seq (seq Pixel) :: (width, height)
//			- grid slice: describes a sub-grid
//			- grid reference: to references elements of a (sub-)grid
//				* to facilitate concurrency
//			- (top, left) is (0, 0) for grids and slices
//		= mGrid2					: define a grid of certain (parameterize) resolution
//			friends (types): MGrid2_ref, mGrid2_slice
//			friends(functs): grid_to_file(), 
//			- C&A, operator <<
//			- operator(): mGrid2_ref
//			- template grid
// 		= mGrid2_slice				: define a slice of a grid. relative to (top, left)
//			- C&A, operator <<
//			- topleft(): offset to grid's (0,0)
//		= mGrid2_ref				: the only access to a grid. NB: NO COPY operations
//			- MOVE C&A only: might be UNDEFINED (FOR NOW) after moved-from
//			- C&A, operator <<.
//			- at(), operator(): access a pixel
//			- paint(): the whole of it 
//			- paint(pred): the ones that fulfill pred()
//			- apply_*() family as friends
//


#ifndef _DEFS_RT_GRIDS
#define _DEFS_RT_GRIDS

#include <iostream>
using std::cout ;
using std::endl ;

#include <utility>
#include <vector>
using std::vector ;

#include <functional>

#include "../miscellaneous.h"
// #include "miscellaneous.h"

#include "coords_RT.h"

template <typename T> class mGrid2_ref ;
struct mGrid2_slice ;

template <typename T, unsigned int W, unsigned int H>
class mGrid2 {
	public:
		using value_type = T ;

	private:
		vector<T>	_base ;

	public:
		static constexpr unsigned int	_width = W ;
		static constexpr unsigned int	_height = H ;

	public:
		explicit mGrid2() : _base(W * H) {} ;	// the default
		explicit mGrid2(const T& p) : _base(W * H, p) {} ;
		~mGrid2() = default ;

		explicit mGrid2(const mGrid2& g) : _base{g._base} {} ;					// Copy & Move constructor
		mGrid2(mGrid2&& g) noexcept : _base{std::move(g._base)} { g._base.resize(0) ; } ;

		mGrid2& operator =(const mGrid2& g) ;									// Copy assignment
		mGrid2& operator =(mGrid2&& g) noexcept ;								// Move ...

		mGrid2& operator =(value_type value) ; 

		mGrid2_ref<T> operator ()(const mGrid2_slice& slc) ;

		const auto & data() const & { return(_base) ; }
		// template <unsigned int W, unsigned int H>
		// friend void grid_to_file(const mGrid2<T,W,H>& gr, std::function<bool(const char *)> f) ;

		template <typename T, unsigned int W, unsigned int H>
			friend std::ostream& operator << (std::ostream& os, const mGrid2<T,W,H>& gr) ;

		template <typename T> friend class mGrid2_ref ;
}; // class mGrid2

struct mGrid2_slice {		// might outlive a Grid
	mCoord<unsigned int, 2>		_start ;
	unsigned int	_rnum ;
	unsigned int	_cnum ;
	unsigned int	_step ;

	explicit mGrid2_slice() : _start(0,0), _rnum { 0 }, _cnum{0}, _step{0} {}
	mGrid2_slice(const mCoord<unsigned int, 2>& rc, 
						  unsigned int rn, unsigned int cn, unsigned int step)
						: _start{rc}, _rnum{rn}, _cnum{cn}, _step{step} {}
	mGrid2_slice(const mGrid2_slice& msl) = default ;
	mGrid2_slice& operator =(const mGrid2_slice& msl) = default ;

	size_t topleft() const 	{ return(_start._x * _step + _start._y) ; } // mGrid2_slice offset()
	bool operator ==(const mGrid2_slice& s) {
		return(_start == s._start && _rnum == s._rnum && _cnum == s._cnum && _step == s._step) ;
	}
	friend std::ostream& operator <<(std::ostream&, const mGrid2_slice&) ;
}; // struct mGrid2_slice 

template <typename T>
class mGrid2_ref {	// Bound to mGrid2/seq T: use it ONLY in the scope of those
	public:
		using value_type = T ;
		using pointer_type = T* ;
		using reference_type = T& ;

	private:
		mGrid2_slice	_descr ;
		T *				_pix{} ;

	public:
		mGrid2_ref(const mGrid2_slice& s, T *p) : _descr{s}, _pix{p} { assert(p != nullptr) ; }
		~mGrid2_ref() = default ;

		mGrid2_ref(const mGrid2_ref&) = delete ;				// COPY C & A
		mGrid2_ref& operator =(const mGrid2_ref&) = delete ;	// 
																// MOVE C & A
		mGrid2_ref(mGrid2_ref&& r) noexcept : _descr{std::move(r._descr)}, _pix{std::move(r._pix)} 
											{ r._pix = nullptr ;}
		mGrid2_ref& operator =(mGrid2_ref&& ref) noexcept
					{ _descr = std::move(ref._descr), _pix = std::move(ref._pix), ref._pix = nullptr ; }

		template <typename R> void paint(R p) ;
		template <typename R> void paint(R p, std::function<bool(unsigned int, unsigned int)> pred) ;

		T& operator ()(unsigned int i, unsigned int j) ;	// acces a pixel, No range check
		T& at(unsigned int i, unsigned int j) ;				// acces a pixel, range check

		template <typename T, typename R, typename F> 
			friend void apply_binary(mGrid2_ref<T> t, mGrid2_ref<R> s, F f) ;
		template <typename T, typename R, typename F>
			friend void apply_unary(mGrid2_ref<T> t, mGrid2_ref<R> s, F f) ;
		template <typename T, typename R, typename F, typename V>
			friend void apply_value(mGrid2_ref<T> t, mGrid2_ref<R> s, F f, V v) ;
		template <typename T, typename R, typename F, typename V> void
			friend apply_bin_value(mGrid2_ref<T> rt, mGrid2_ref<R> rs, F f, V v) ;

		template <typename T>
		friend std::ostream& operator <<(std::ostream& os, const mGrid2_ref<T>& gr) ;
}; // class mGrid2_ref

																		// templates for mGrid2<> follow
template <typename T, unsigned int W, unsigned int H> mGrid2_ref<T>
mGrid2<T, W, H>::operator ()(const mGrid2_slice& slc)
{										// Speed ??? check for complying slice or not
#ifndef _DEF_THROWS
	assert(!(slc._start._x >= _height || slc._start._y >= _width)) ;
	assert(!(slc._cnum > _width - slc._start._y)) ;
	assert(!(slc._rnum > _height - slc._start._x)) ;
	assert(!(slc._step != _width)) ;
	assert(!(_base.size() != (W * H))) ;
#else
	if (slc._start._base[0] >= _height || slc._start._base[1] >= _width)	throw std::range_error("mGrid2: Wrong base") ;
	if (slc._cnum > _width - slc._start._base[1])						throw std::range_error("mGrid2: Wrong width") ;
	if (slc._rnum > _height - slc._start._base[0])					throw std::range_error("mGrid2: Wrong height") ;
	if (slc._step != _width)									throw std::range_error("mGrid2: Wrong step") ;
	if (_base.size() != (W * H))			throw std::range_error("mGrid2: Wrong SIZE") ;
#endif

	return(mGrid2_ref<T>(slc, _base.data() + slc.topleft())) ;
} // mGrid2 operator (slice)

template <typename T, unsigned int W, unsigned int H> mGrid2<T, W, H>&
mGrid2<T, W, H>::operator =(const mGrid2& gr)
{
	_base = gr._base ;
	return(*this) ;
} // mGrid2 operator=(&)

template <typename T, unsigned int W, unsigned int H> mGrid2<T, W, H>&
mGrid2<T, W, H>::operator =(mGrid2&& gr) noexcept
{
	_base = std::move(gr._base), gr._base.resize(0) ;
	return(*this) ;
} // mGrid2 operator=(&&)

template <typename T, unsigned int W, unsigned int H> mGrid2<T, W, H>&
mGrid2<T, W, H>::operator =(T val)	// allows the re-use of "moved from"
{
	assert(_base.size() == 0) ;
	vector<T>   temp(_width, _height, val) ;
	_base = std::move(temp) ;
	 
	return(*this) ;
} // mGrid2 operator =(value_type): for {})


template <typename T, unsigned int W, unsigned int H> std::ostream&
operator <<(std::ostream& os, const mGrid2<T, W, H>& gr)
{
	cout << "-- grid {" << gr._width << ", " << gr._height << "}" ;
	cout << "- holding " << (gr._base).size() << " elements at (" << (gr._base).data() << ")" ;
	// cout << endl << "--- capacity " << (gr._base).capacity() << ")" << endl ;

	return(os) ;
} // mGrid2 operator <<
			 
																		// class mGrid2_ref
template <typename T> inline T&
mGrid2_ref<T>::operator ()(unsigned int i, unsigned int j)				// No range check.
{
	return(*(_pix + (i * (_descr._step)) + j)) ;
} // mGrid2_ref operator ()

template <typename T> T&
mGrid2_ref<T>::at(unsigned int i, unsigned int j)				// Range check
{
#ifdef _DEF_THROWS
	if (i >= _descr._rnum || j >= _descr._cnum)     throw std::range_error("___ Grid reference range error") ;
#else
	assert(!(i >= _descr._rnum || j >= _descr._cnum)) ;
#endif
	return(*(_pix + (i * (_descr._step)) + j)) ;
} // mGrid2_ref at()

template <typename T> 
template <typename R> void
mGrid2_ref<T>::paint(R p)
{
	static_assert(std::is_same<std::remove_cv_t<T>, std::remove_cv_t<R>>::value,
				  "--- mGrid::paint(): incompatible types") ;
	// Here the "slice" is expected: compliant
	// ??? Use iterators with traits static check or, just a plain pointer
	// ??? Might check for types compliance, etc., as well

	auto rowLim = (_descr._rnum) ;
	auto jump = (_descr._step - _descr._cnum) ;
	for (auto pix = _pix ; rowLim > 0 ; --rowLim, pix += jump) {
		for (auto colLim = (_descr._cnum) ; colLim > 0 ; --colLim, pix++)      *pix = p ;
	}
} // mGrid2_ref paint()

template <typename T>
template <typename R> void
mGrid2_ref<T>::paint(R p, std::function<bool(unsigned int, unsigned int)> pred)
{
	static_assert(std::is_same<std::remove_cv_t<T>, std::remove_cv_t<R>>::value,
				  "--- mGrid::paint(): incompatible types") ;
	auto rowLim = (_descr._rnum) ;
	auto colLim = (_descr._cnum) ;

	for (decltype(rowLim) i = 0 ; i < rowLim ; i++) {
		for (decltype(colLim) j = 0 ; j < colLim ; j++) {
			if (pred(i, j))		(this->at)(i, j) = p ;
		}
	}
} // mGrid2_ref paint(pred)

template <typename T, typename R, typename F> void 
apply_binary(mGrid2_ref<T> rt, mGrid2_ref<R> rs, F f)
{
	assert(rt._descr == rs._descr) ;
	
	mGrid2_slice des(rt._descr) ;
	auto rowLim = (des._rnum) ;
	auto jump = (des._step - des._cnum) ;
	auto pix_src = rs._pix ;
	auto pix_t	= rt._pix ;
	for ( ; rowLim > 0 ; --rowLim, pix_src += jump, pix_t += jump) {
		for (auto colLim = des._cnum ; colLim > 0 ; --colLim, pix_src++, pix_t++) {
			*pix_t = (f(*pix_t, *pix_src)) ;
		}
	}
} // mGrid2_ref friend apply_binary()

template <typename T, typename R, typename F> void
apply_unary(mGrid2_ref<T> rt, mGrid2_ref<R> rs, F f)
{
	assert(rt._descr == rs._descr) ;

	mGrid2_slice des(rt._descr) ;
	auto rowLim = (des._rnum) ;
	auto jump = (des._step - des._cnum) ;
	auto pix_src = rs._pix ;
	auto pix_t = rt._pix ;
	for (; rowLim > 0 ; --rowLim, pix_src += jump, pix_t += jump) {
		for (auto colLim = des._cnum ; colLim > 0 ; --colLim, pix_src++, pix_t++) {
			*pix_t = (f(*pix_src)) ;
		}
	}
} // mGrid2_ref friend apply_unary()

template <typename T, typename R, typename F, typename V> void
apply_value(mGrid2_ref<T> rt, mGrid2_ref<R> rs, F f, V v)
{
	assert(rt._descr == rs._descr) ;
															// cout << endl << "___ in apply: " << rt._descr ;
	mGrid2_slice des(rt._descr) ;
	auto rowLim = (des._rnum) ;
	auto jump = (des._step - des._cnum) ;
	auto pix_src = rs._pix ;
	auto pix_t = rt._pix ;
	for (; rowLim > 0 ; --rowLim, pix_src += jump, pix_t += jump) {
		for (auto colLim = des._cnum ; colLim > 0 ; --colLim, pix_src++, pix_t++) {
			*pix_t = (f(*pix_src, v)) ;
		}
	}
} // mGrid2_ref friend apply_value()

template <typename T, typename R, typename F, typename V> void
apply_bin_value(mGrid2_ref<T> rt, mGrid2_ref<R> rs, F f, V v)
{
	assert(rt._descr == rs._descr) ;

	mGrid2_slice des(rt._descr) ;
	auto rowLim = (des._rnum) ;
	auto colLim = des._cnum ;
	
	for (size_t i = 0 ; i < rowLim ; i++) {
		for (size_t j = 0 ; j < colLim ; j++) {
			rt.at(i, j) = (f(rt(i, j), rs(i, j), v, i, j)) ;
		}
	}
} // mGrid2_ref friend apply_bin_value()

template <typename T> std::ostream&
operator <<(std::ostream& os, const mGrid2_ref<T>& gr)
{
	os << endl << endl
		<< "- reference to a grid of " << typeid(mGrid2_ref<T>::value_type).name() 
		<< ": " << gr._descr
		<< endl << "--- data starts at (" << gr._pix << ")" << endl ;

	return(os) ;
} // mGrid2_ref operator <<
																		// eoc mGrid2_ref

#endif

// eof grids_RT.h