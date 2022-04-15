// miscellaneous.h: miscellaneous tools and types like
// 
//			- type traits and predicates
//			- math constants, etc
//			- chrono
// 
//


#ifndef _DEFS_MISC_RT
#define _DEFS_MISC_RT

#include <iostream>
#include <chrono>
#include <vector>

// Types
#include <type_traits>
template <typename T1, typename T2> inline bool Is_same(T1, T2) { return(std::is_same<T1, T2>::value) ; }


// Math
#include <limits>
constexpr double dEps = std::numeric_limits<double>::min() * 100 ;
constexpr float  fEps = std::numeric_limits<float>::min() * 100 ;
constexpr double dINFINITY = std::numeric_limits<double>::max() ;   
constexpr double fINFINITY = std::numeric_limits<float>::max() ;
template <typename T> inline T nINFINITY() { return(std::numeric_limits<T>::max()) ; }

inline bool near_zero(float n) { return (abs(n) < fEps) ; }
inline bool near_zero(double n) { return (abs(n) < dEps) ; }

//   Trigonometric
const double const_PI = std::acos(-1.) ;      // constexpr auto coef_PI = 3.14159 ;
const double coef_DEG_PI = const_PI / 180. ;  // constexpr auto coef_DEG_PI = coef_PI / 180. ;
template <typename T> inline double degs_to_rads(T degs) { return (static_cast<double>(degs * coef_DEG_PI)) ; }
 
// Chrono, etc
using mTimeP = decltype(std::chrono::steady_clock::now()) ;
using mClock = typename std::chrono::steady_clock ;
using mLapse = std::chrono::nanoseconds ;
using mDuration_ns = std::chrono::duration<double, std::nano> ;
using mDuration_ms = std::chrono::duration<double, std::milli> ;
using mDuration_sec = std::chrono::duration<double> ;


class mTimer {
	private:
		mLapse			_elapsed{} ;	// total time running from when start pressed
		mTimeP			_start{} ;		// start the timer
		mTimeP			_stop{} ;		// timer stopped at

		mTimeP			_lap_start ;    // ... started at
		mTimeP			_lap ;			// current lap stopped at
		
		bool			_isRunning ;	// if the timer is running: started and not yet stopped

		std::vector<mLapse>	_wps ;		// lapses between way points(lap pressed)

	public:
		// ~mTimer() = default ;
		mTimer(size_t numP = 10) : _elapsed{}, _start{}, _stop{},
								   _lap_start{}, _lap{}, _isRunning{false}, _wps{numP} {}

		void start(bool fl) { 
			if (!_isRunning) { 
				if (fl)		std::cout << "_____ timer "
										<< (_elapsed > mLapse(0) ? "re" : "") << "started " ;

				_lap_start = _start = mClock::now(), _isRunning = true ; 
			}
		}
		void reset()   { if (!_isRunning)   _elapsed = {}, _wps.resize(0) ; }
		void stop() {
			if (_isRunning) { 
				_lap = _stop = (mClock::now()), 
					_elapsed += (_stop - _start),
					_wps.push_back(_lap - _lap_start),
				_isRunning = false ; 
			}
		}
		void lap() {
			if (_isRunning) { 
				_lap = mClock::now(), _wps.push_back(_lap - _lap_start) ;
				_lap_start = _lap ;
			}
		}
	
		bool isRunning() const { return(_isRunning) ; }
		mLapse elapsed() const { return(_elapsed) ; }

		friend std::ostream& operator <<(std::ostream& os, mTimer& mt) {
			mt.stop() ;
			os << "_____ break -> lapse(ms) since: start("
					<< (static_cast<mDuration_ms>(mt._elapsed)).count() << ')' ;
			os << ", last break("
					<< (static_cast<mDuration_ms>(mt._wps.back())).count() << ')' ;
			mt.start(false) ;
			return(os) ;
		}
}; // class mTimer


time_t show_lapse(const bool fl_start) ;

#endif
// eof miscellaneous.h
