// scene_RT.cpp: members, etc of classes in scene_RT.h(included)
//		- mAtom_RT:
//		- mObject_RT:
//		- mScene_RT:
//

#include "scene_RT.h"
																		// class mAtom_RT
std::ostream&
operator <<(std::ostream& os, const mAtom_RT& w)
{
	os << w.shape() << "; color: " << w.color() ;
	return(os) ;
} // mAtom_RT friend operator <<
																		// eoc mAtom_RT
																		// class mAObject_RT
std::pair<size_t, ray_RT::value_type>
mObject_RT::ray_hit(const ray_RT& orig, const ray_RT& ray) const
{
	auto distance{std::numeric_limits<ray_RT::value_type>::max()} ;
	auto cdist{distance} ;

	size_t i{0} ;
	size_t ind{0} ;
	const_iterator wend = _base.end() ;
	for (auto wat = _base.begin() ; wat != wend ; ++wat, i++) {
		if (wat->_ray_hit(orig, ray, cdist)) {
			if (cdist < distance) { ind = i, distance = cdist ; }
		}
	}

	return(distance < std::numeric_limits<ray_RT::value_type>::max()
			? std::make_pair(ind, distance) 
			: std::make_pair(i, std::numeric_limits<ray_RT::value_type>::max())
		   ) ;
} // mObject_RT ray_hit()

mColor				// if the ray hits it, return color(), otherwise _clrb
mObject_RT::operator ()(vector_RT<RT_default_type> ray, vector_RT<RT_default_type>& camera)
{
	auto res = std::move(this->ray_hit(camera, ray)) ;

	if (res.first == _base.size()) return(_clrb) ;
	return((_base[res.first]).color()) ; // (ray, lll_mov_color)) ;
} // mObject_RT operator ()

std::ostream& 
operator <<(std::ostream& os, const mObject_RT& obj)
{
	os << endl << "--- " << obj.name() << " {objRT of size: " << obj._base.size() 
		<< ", capacity : " << obj._base.capacity() << "} holding:" ;
	for (auto t : obj._base) {
		os << endl << "----- " << t ;
	}
	return(os) ;
} // mObject_RT friend operator <<
																		// eoc mObject_RT
																		// class mScene_RT
mColor
mScene_RT::operator()(vector_RT<RT_default_type> ray, vector_RT<RT_default_type>& camera)
{
	auto	current = std::make_pair(std::numeric_limits<size_t>::max(), 
									 std::numeric_limits<ray_RT::value_type>::max()) ;
	auto	chosen{current} ;

	std::vector<mObject_RT>::const_iterator  chosen_object = cend(_base) ;
	for (auto it = cbegin(_base) ; it != cend(_base) ; ++it) {
		current = std::move(it->ray_hit(camera, ray)) ;
		if (current.second < std::numeric_limits<ray_RT::value_type>::max()) {  // hit
			if (current.second < chosen.second) { chosen = std::move(current), chosen_object = it ; }
		}
	}
	if (chosen_object != cend(_base)) {	// the object with the closest Atom that's been hit
		return(((*chosen_object)[chosen.first]).color()) ; //ray, lll_mov_color)) ;
	}
	return(_clrb) ;
} // mScene operator()


std::ostream&
operator <<(std::ostream& os, const mScene_RT& sc)
{
	os << endl << "-------- mScene of " << (sc._base).size() << " element(s):";

	for (auto t : sc._base) {
		os << endl << t ;
	}
	
	return(os) ;
} // mScene_RT friend operator <<
																		// eoc mScene_RT

// eof scene_RT.cpp