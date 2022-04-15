// scene_RT.cpp: members, etc of classes in scene_RT.h(included)
//		- mAtom_RT:
//		- mObject_RT:
//		- mScene_RT:
//

#include "scene_RT.h"

																		// class mAtom_RT
#ifdef NO_INDEXING
mAtom_RT::mAtom_RT(_data_iter& tria, _data_iter& vert_beg) 
{
	_shape = triangle_from_iterator<RT_default_type>(tria, vert_beg) ;
	_col = randomCRT() ;
} // mAtom_RT (_data_iter, _data_iter)
#endif

mAtom_RT::mAtom_RT(_data_iter& tria, std::vector<long>& vertices)
{
	_shape = triangle_from_indexes<RT_default_type>(tria, vertices) ;
	_col = randomCRT() ;
} // mAtom_RT (data_iter, index for _data_iter)


std::ostream&
operator <<(std::ostream& os, const mAtom_RT& w)
{
	os << w.shape() << "; color: " << w.color() ;
	return(os) ;
} // mAtom_RT friend operator <<
																		// eoc mAtom_RT
												
																		// class mObjects_RT
mObjects_RT::mObjects_RT(cl_SceneDescr* sd) : _base{}
{
	if (sd->activate())   cout << endl << "> the scene is active ..." ;
	else				  throw std::runtime_error("___ scene: cannot activate the descriptor") ;

	// preset a scope for "objects"
	std::vector<mAtom_RT>		atoms{} ;
	mAtom_RT	colorAt ; // for the simulation of color
	cout << endl << "--- preset objects:"  ;
	_data_iter	tri_i{}, tri_ei ;	// for triangles
	// _data_iter	ver_i{} ;			// begin(vertices)-> instead, use the indexes to speed up
	std::vector<long> 	ver_i{} ;		// indexing "vertices"
	auto	obj_iter = sd->begin_scope("\"objects\"") ; auto oiter_end = obj_iter.limit() ;

	for ( ; obj_iter != oiter_end ; ++obj_iter) {
		cout << endl << "--- loading data for: " << obj_iter->_name ;
		tri_i = sd->begin_data(sd->data(obj_iter, "\"triangles\""), 3), tri_ei = tri_i.limit() ;
		ver_i = sd->index_data(sd->begin_data(sd->data(obj_iter, "\"vertices\""), 3)) ;
		for ( ; tri_i != tri_ei ; ++ tri_i) {   // for all triangles
			atoms.push_back(std::move(mAtom_RT(tri_i, ver_i))) ;
		}
		atoms.shrink_to_fit(), _base.push_back(std::move(atoms)), atoms.clear() ;
	}
} // class mObjects_RT
																		// eoc mObjects_RT
																		
																		// class mScene_RT
mScene_RT::mScene_RT(cl_SceneDescr * sd, mObjects_RT *objs) : _objects{objs}, _cam{}, _clrb{}
{
	if (!(sd->activate()))   throw std::runtime_error("___ scene: cannot activate the descriptor") ;
	
	std::string color = *(sd->begin_data(sd->data("\"settings\"", "\"background_color\""), 3)) ;
																		// cout << endl << endl << 
																		// ">>> background color: " << color ;		
	cout << endl << ": # of objects: " << _objects->_base.size() ; 
	int i = 0 ;
	for (auto dr : _objects->_base) {
		cout << endl << "--- # of triangles in object #" << i++ << ": " << dr.size() ;
	}
} // mScene_RT ()

using VectF = vector_RT<RT_default_type> ;
using TriaF = triangle_RT<RT_default_type> ;

std::pair<mColor, ray_RT::value_type>
mScene_RT::ray_hit(size_t ind_iter, const ray_RT& orig, const ray_RT& ray)
{
	auto		distance{std::numeric_limits<ray_RT::value_type>::max()} ;	// the infinity
	auto		cdist{distance} ;											// for the closest
	mColor		ind{} ;
												
	auto		tr_next { (_objects->_base)[ind_iter].cbegin() };
	auto		tr_end  { (_objects->_base)[ind_iter].cend() };
	
	for ( ; tr_next != tr_end ; ++tr_next)   {
		if (tr_next->_ray_hit(orig, ray, cdist)) {   // checks for a hit
			if (cdist < distance) { ind = tr_next->color(), distance = cdist ; }
		}
	}
	
	return(distance < std::numeric_limits<ray_RT::value_type>::max()
		   ? std::move(std::make_pair(ind, distance))
		   : std::move(std::make_pair(_clrb, std::numeric_limits<ray_RT::value_type>::max()))
		   ) ;
} // mScene_RT ray_hit()


mColor
mScene_RT::operator()(ray_RT ray, const ray_RT& camPos)
{
	static auto limit	{std::make_pair<mColor, ray_RT::value_type>(
									   mColor{},
									   std::numeric_limits<ray_RT::value_type>::max())
						};
	auto	current{limit} ;
	auto	chosen{current} ;

	size_t	nfound{(_objects->_base).size()} ; size_t  chosen_object{nfound} ;
	for (size_t count = 0 ; count < nfound ; count++)   {
		current = ray_hit(count, camPos, ray) ;
		if (current.second < std::numeric_limits<ray_RT::value_type>::max()) {  // hit
			if (current.second < chosen.second) { chosen = std::move(current), chosen_object = count ; }
		}
	}

	return(chosen_object < nfound ? chosen.first : _clrb) ;
} // mScene operator()

mColor
randomCRT()		// assumes srand() has been already called 	
{
	constexpr CompT maxColorComponent = 255 ;
	return(mColor(rand() % maxColorComponent,
				  rand() % maxColorComponent,
				  rand() % maxColorComponent)
		   ) ;
} // randomCRT


std::ostream&
operator <<(std::ostream& os, const mScene_RT& sc)
{
	os << endl << endl << "-------- mScene: " ; // << (sc._base).size() << " element(s):";

	return(os) ;
} // mScene_RT friend operator <<
																		// eoc mScene_RT
// eof scene_RT.cpp