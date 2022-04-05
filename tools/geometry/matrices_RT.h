// matrices_RT.h: defines a very narrow specialization of Matrix<Type, Order> like
//				  square (3 x 3) matrix<T, order = 2> to be used for rotation and scaling;
//				  translations are to be provided with a separate vector_RT<>
//		- based on T[3 * 3] ; NOT in Heap -> possible update: move it to Heap and compare performance
//		- using RAW-MAJOR approach
//		- access to an element:
//			- operator(i,j) const : no range check, returns T
//			- .at(i,j)	    : range check, return T&    ???
//


#ifndef _DEFS_MATRIX_RT_GEOMETRY
#define _DEFS_MATRIX_RT_GEOMETRY


#include <iostream>
using std::cout ;
using std::endl ;

#include <utility>
#include <vector>


#include <functional>
#include <type_traits>
template <bool B, typename T> using Enable_if = typename std::enable_if<B, T>::type ;

#include "../miscellaneous.h"
// #include "miscellaneous.h"

#include "coords_RT.h"
#include "vectors_RT.h"


template <typename T>
class mMatrix {
	static_assert(std::is_arithmetic<T>::value, "vector_RT<T>: T must be arithmetic") ;

	public:
		using value_type = T ;

		static constexpr unsigned int	_rowsN = 3 ;
		static constexpr unsigned int	_colsN = 3 ;

	private:	
		T		_base[_rowsN * _colsN] ;
		
		Enable_if<std::is_pod<decltype((_base + 0))>::value, void> _copy_base(const mMatrix<T>& src) { 
			memcpy(_base, src._base, sizeof(_base)) ; 
		}

	public:
		mMatrix() : _base{} {}
		mMatrix(std::initializer_list<T> tx, std::initializer_list<T> ty, std::initializer_list<T> tz) ;
		mMatrix(const vector_RT<T>& r0, const vector_RT<T>& r1, const vector_RT<T>& r2) {
				r0.extract(_base[0], _base[1], _base[2]),
				r1.extract(_base[3], _base[4], _base[5]),
				r2.extract(_base[6], _base[7], _base[8]) ;
		}
		// ~mGrid2() = default ; :: use defaults for all special members
				
		// Access
		T operator ()(unsigned int i, unsigned int j) const { return(_base[i * _colsN + j]) ; }
		T at(unsigned int i, unsigned int j) const 			{ return(_base[i * _colsN + j]) ; }
		T& at(unsigned int i, unsigned int j)				{ return(_base[i * _colsN + j]) ; }

		vector_RT<T> row(unsigned int i) const {	assert(i < _rowsN) ;
			i *= _colsN ;
			return(vector_RT<T>{_base[i], _base[i + 1], _base[i + 2]}) ;
		}
		vector_RT<T> col(unsigned int j) const {	assert(j < _colsN) ;
			return(vector_RT<T>{_base[j], _base[j + _colsN], _base[j + _colsN + _colsN]}) ;
		}
		
		// Operations
		mMatrix& multiplyByMatrix(const mMatrix& m) ;						// changes (this)
		friend mMatrix operator *(const mMatrix& m1, const mMatrix& m2) {
			mMatrix<T> temp{m1} ; return(temp.multiplyByMatrix(m2)) ;
		}

		// Friends
		friend bool operator ==(const mMatrix& m1, const mMatrix& m2) { 
			for (int i = 0 ; i < _rowsN * _colsN ; i++) if ((m1._base)[i] != (m2._base)[i]) return(false) ;
			return(true) ;
		}
		
		// class vector_RT<T> ;
		template <typename T> friend std::ostream& operator << (std::ostream& os, const mMatrix<T>& m) ;
}; // class mGrid2

template <typename T> inline
mMatrix<T>::mMatrix(std::initializer_list<T> tx,
					std::initializer_list<T> ty,
					std::initializer_list<T> tz)
{
	assert(tx.size() == 3 && ty.size() == 3 && tx.size() == 3) ;

	_base[0] = *(begin(tx)), _base[1] = *(begin(tx) + 1), _base[2] = *(begin(tx) + 2),
	_base[3] = *(begin(ty)), _base[4] = *(begin(ty) + 1), _base[5] = *(begin(ty) + 2),
	_base[6] = *(begin(tz)), _base[7] = *(begin(tz) + 1), _base[8] = *(begin(tz) + 2) ;
} // mMatrix (initilizer_lists) 

template <typename T> inline mMatrix<T>&
mMatrix<T>::multiplyByMatrix(const mMatrix& m)
{
	mMatrix<T>		temp ;
	int   k{0} ;
	for (int i = 0 ; i < _rowsN ; i++, k += _colsN)   {
		for (int j = 0 ; j < _colsN ; j++) {
			temp._base[k + j] = dotPR(this->row(i), m.col(j)) ;
		}
	}
	_copy_base(temp) ;			// private: if POD or NOT POD
	
	return(*this) ;
} // mMatrix multiplyByMatrix()

template <typename T> std::ostream& 
operator << (std::ostream& os, const mMatrix<T>& m)
{
	os << "{r0" << m.row(0) << ", r1" << m.row(1) << ", r2" << m.row(2) << '}' << " - ";
	// os << "{c0" << m.col(0) << ", c1" << m.col(1) << ", c2" << m.col(2) << '}' ;
				
	return(os) ;
} // mMatrix friend operator <<


// definition of Rotational Matrices
inline mMatrix<float>
X_rotation(float degs)
{
	float	rads{static_cast<float>(degs_to_rads(degs))} ;
	float	c_cos{std::cosf(rads)} ;
	float	c_sin{std::sinf(rads)} ;

	return(mMatrix<float>	{ 
							{1.f, 0.f, 0.f},
							{0.f, c_cos, -c_sin}, //{c_cos, 0.f, -c_sin},
							{0.f, c_sin, c_cos}   //{c_sin, 0.f, c_cos}
							}
	) ;
} // rotate_around_X

inline mMatrix<float>
Y_rotation(float degs)
{
	float	rads{static_cast<float>(degs_to_rads(degs))} ;
	float	c_cos{std::cosf(rads)} ;
	float	c_sin{std::sinf(rads)} ;

	return(mMatrix<float>	{
							{c_cos, 0.f, -c_sin},
							{0.f, 1.f, 0.f},
							{c_sin, 0.f, c_cos}
							}
	) ;
} // rotate_around_Y

inline mMatrix<float>
Z_rotation(float degs)
{
	float	rads{static_cast<float>(degs_to_rads(degs))} ;
	float	c_cos{std::cosf(rads)} ;
	float	c_sin{std::sinf(rads)} ;

	return(mMatrix<float>	{
							{c_cos, -c_sin, 0.f},	// {c_cos, 0.f, -c_sin},
							{c_sin, c_cos, 0.f},	// {c_sin, 0.f, c_cos},
							{0.f, 0.f, 1.f}
							}
	) ;
} // rotate_around_Z


#endif
// eof matrices_RT.h