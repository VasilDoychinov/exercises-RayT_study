// scene_RT.cpp: members, etc of classes in scene_RT.h(included)
//		- mAtom_RT:
//		- mObject_RT:
//		- mScene_RT:
//

#include "scene_RT.h"

std::atomic<size_t>		slcAssigned{0} ;  // index of the next slice to assign
std::atomic<size_t>		slcProcessed{0} ; // index of the next slice to assign

																		// class mAtom_RT
#ifdef NO_INDEXING
mAtom_RT::mAtom_RT(_data_iter& tria, _data_iter& vert_beg) 
{
	_shape = triangle_from_iterator<RT_default_type>(tria, vert_beg) ;
	// _col = randomCRT() ;
} // mAtom_RT (_data_iter, _data_iter)
#endif

mAtom_RT::mAtom_RT(_data_iter& tria, std::vector<long>& vertices)
{
	_shape = triangle_from_indexes<RT_default_type>(tria, vertices) ;
	// _col = randomCRT() ;
} // mAtom_RT (data_iter, index for _data_iter)


std::ostream&
operator <<(std::ostream& os, const mAtom_RT& w)
{
	os << w.shape() ; // << "; color: " << w.color() ;
	return os ;
} // mAtom_RT friend operator <<
																		// eoc mAtom_RT
												
																		// class mObjects_RT
mAlbedo get_obj_color(cl_SceneDescr* sd, cl_SceneDescr::iterator obj_i)
{
	mAlbedo		alb{} ;
	std::vector<std::string>   settings{} ;

	settings = extract_data(*sd, obj_i->_name, "\"colour\"", 3) ;
	if (settings.size() == 1)	alb = color_from_string<RT_default_type, mAlbedo>(settings[0]) ;
	
	return alb ;
} // get_obj_albedo()

bool get_obj_fl_box(cl_SceneDescr* sd, cl_SceneDescr::iterator obj_i)
{
	bool		fl_box{false} ;
	std::vector<std::string>   settings{} ;

	settings = extract_data(*sd, obj_i->_name, "\"use_box\"", 1) ;
	if (settings.size() == 1)    fl_box = (settings[0].find("yes") != std::string::npos) ;

	return fl_box ;
} // get_obj_fl_box()


mObjects_RT::mObjects_RT(cl_SceneDescr* sd) : _base{}
{
	if (sd->activate())   cout << endl << "> the scene is active: set objects" ;
	else				  throw std::runtime_error("___ scene: cannot activate the descriptor") ;

	// preset a scope for "objects"
	std::vector<mAtom_RT>	atoms{} ;
	obox_RT					o_box{} ;
	bool					fl_box{false} ;
	
	_data_iter	tri_i{}, tri_ei ;	// for the triangles
	// _data_iter	ver_i{} ;			// begin(vertices)-> instead, use the indexes to speed up
	std::vector<long> 	ver_i{} ;		// indexing "vertices"
	auto	obj_iter = sd->begin_scope("\"objects\"") ; auto oiter_end = obj_iter.limit() ;

	for ( ; obj_iter != oiter_end ; ++obj_iter) {
		// cout << endl << "--- loading data for: " << obj_iter->_name ;
		fl_box = get_obj_fl_box(sd, obj_iter), o_box = obox_RT{} ;
		tri_i = sd->begin_data(sd->data(obj_iter, "\"triangles\""), 3), tri_ei = tri_i.limit() ;
		ver_i = sd->index_data(sd->begin_data(sd->data(obj_iter, "\"vertices\""), 3)) ;
		for (size_t i = 0 ; tri_i != tri_ei ; i++, ++ tri_i) {   // for all triangles
			atoms.push_back(std::move(mAtom_RT(tri_i, ver_i))) ;
			if (fl_box)		o_box.min_max(atoms[i].shape()) ;
		}
																	// cout << " -> the box is: " << o_box ;
		atoms.shrink_to_fit(),
		_base.push_back(std::move(
									cl_objectRT(std::move(atoms), 
												get_obj_color(sd, obj_iter),
												std::move(o_box)
										       )
								 )
		), 
		atoms.clear() ;
	}
} // mObjects_RT ()

bool
cl_objectRT::box_hit(const ray_RT& orig, const ray_RT& ray) const 
{
	ray_RT	pHit{} ;
	bool	fl{true} ;	// use it if no box's been defined

	if (_box != obox_RT{}) {	// a box is defined: check for a hit
		// boxChecks++ ;
		fl = _box._box_hit(orig, ray, pHit) ;
		// if (!(fl = _box._box_hit(orig, ray, pHit)))	boxMisses++ ;
	}

	return fl ; // true: indicates go through the shapes sequence
} // cl_objectRT box_hit()
																		// eoc mObjects_RT															
																		// classes for lights
cl_seqLightsRT::cl_seqLightsRT(cl_SceneDescr* sd)
{
	if (sd->activate())   cout << " & lights" ;
	else				  throw std::runtime_error("___ scene: cannot activate the descriptor") ;

	size_t	intensity{0} ;
	mAlbedo	color{} ;

	std::vector<std::string>   settings ;
	auto	li_iter = sd->begin_scope("\"lights\"") ; auto liter_end = li_iter.limit() ;

	for ( ; li_iter != liter_end ; ++li_iter) {
		// cout << endl << "--- loading data for: " << li_iter->_name ;
		settings = extract_data(*sd, li_iter->_name, "\"intensity\"", 1) ;
		if (settings.size() != 1)			throw std::runtime_error("___ lights: intensity") ;
		intensity = number_from_string<size_t>(settings[0]) ;

		settings = extract_data(*sd, li_iter->_name, "\"albedo\"", 3) ;
		if (settings.size() == 1)	color = color_from_string<RT_default_type, mAlbedo>(settings[0]) ;
		else						color = mAlbedo{1.,1.,1.} ;

		settings = extract_data(*sd, li_iter->_name, "\"position\"", 3) ;
		if (settings.size() != 1)			throw std::runtime_error("___ lights: position") ;
		_base.push_back(light_RT(intensity, 
								 std::move(vector_from_string<RT_default_type>(settings[0])),
								 color)) ;
	}
} // cl_seqLightsRT
																		// eoc lights
																		// class mScene_RT
mScene_RT::mScene_RT(cl_SceneDescr * sd, mObjects_RT *objs, cl_seqLightsRT *lights) 
	: _objects{objs}, _lights{lights}, _clrb{}, _shadowBias{}, _set_concurrency{}
{
	if (!(sd->activate()))   throw std::runtime_error("___ scene: cannot activate the descriptor") ;
	
	_clrb = color_from_string<RT_default_type, mAlbedo>
							 (*(sd->begin_data(sd->data("\"settings\"", "\"background_color\""), 3))) ;
															
	_shadowBias = number_from_string<RT_default_type>     // 0 if data not found
					(*(sd->begin_data(sd->data("\"settings\"", "\"shadow_bias\""), 3))) ;

	cout << endl << endl << ": # of objects: " << _objects->_base.size() ; 
	int i = 0 ;
	for (auto dr : _objects->_base) {
		cout << endl << "--- # of triangles in object #" << i++ << ": " << dr.size() 
			<< "; colour: " << dr.albedo() ; // << "; " << dr.box() ;
	}
	cout << *_lights ;
	cout << endl << ": background color: " << _clrb << "; shadow bias: " << _shadowBias ;
} // mScene_RT ()


void
mScene_RT::set_concurrency(cl_SceneDescr * sd, unsigned int W, unsigned int H)
{
	// set parameters of concurrency: number of threads
	_set_concurrency._num_of_threads = number_from_string<size_t>
					(*(sd->begin_data(sd->data("\"concurrency\"", "\"#_of_threads\""), 1))) ;
	if (_set_concurrency._num_of_threads == 0) {
		_set_concurrency._num_of_threads = std::thread::hardware_concurrency() ;
	}

	{ // prepare the slices
		size_t   rn{number_from_string<size_t>
				(*(sd->begin_data(sd->data("\"concurrency\"", "\"#_of_horizontal_cuts\""), 1)))
		} ;
		if (rn == 0)	rn = _set_concurrency._num_of_threads ; // # of horizontal cuts

		size_t   cn{number_from_string<size_t>
				(*(sd->begin_data(sd->data("\"concurrency\"", "\"#_of_vertical_cuts\""), 1)))
		} ;
		if (cn == 0)	cn = _set_concurrency._num_of_threads ; // # of vertical cuts

		_set_concurrency._slices = slices_to_render(W, H, rn, cn) ;
		slcAssigned.store(0) ;	// the next slice to assign for processing
	}

	if (_set_concurrency._num_of_threads > _set_concurrency._slices.size()) {
		_set_concurrency._num_of_threads = _set_concurrency._slices.size() ;
	}

	return ;
} // mScene_RT set_concurrency()


std::pair<mObjects_RT::const_iterator, ray_RT::value_type>	// locate the closest triangle of an Object(ind)
mScene_RT::ray_hit(size_t ind_iter, const ray_RT& orig, const ray_RT& ray, ray_RT& hP)
{
	auto		distance{std::numeric_limits<ray_RT::value_type>::max()} ;	// the infinity
	auto		cdist{distance} ;											// for the closest
	ray_RT		w_hP{} ;
	
	auto		tr_next { (_objects->_base)[ind_iter].obj_begin() };
	auto		tr_end  { (_objects->_base)[ind_iter].obj_end() };
	decltype(tr_next)	ind ;

	if ((_objects->_base)[ind_iter].box_hit(orig, ray)) {
		for ( ; tr_next != tr_end ; ++tr_next)   {
			if (tr_next->_ray_hit(orig, ray, cdist, w_hP)) {   // checks for a hit
				if (cdist < distance) {	ind = tr_next, distance = cdist, hP = std::move(w_hP) ; }
			}
		}
		// get out the iterator to the triangle ....
	}

	return distance < std::numeric_limits<ray_RT::value_type>::max()
		   ? std::make_pair(ind, distance)
		   : std::make_pair(tr_end, std::numeric_limits<ray_RT::value_type>::max()) ;
} // mScene_RT ray_hit()


bool
mScene_RT::in_shadow(const mAtom_RT& tr, const ray_RT& hP, const ray_RT& ld, double l_dist)
{
	ray_RT::value_type cdist{} ;	// compare against distance to light l_dist
	ray_RT			   whit ;		// ...

	for (auto obj = cbegin(_objects->_base), obj_e = cend(_objects->_base) ; obj != obj_e ; ++obj) {
		for (auto tr = obj->obj_begin(), tr_e = obj->obj_end() ; tr != tr_e ; ++tr) {
			if (tr->_ray_hit(hP, ld, cdist, whit)) if (cdist < l_dist) {   return true ; }
		}
	}
	return false ;
} // mScene_RT in_shadow()

mColor
mScene_RT::shadow_color(const mAtom_RT& tr, ray_RT& hP, const mAlbedo& alb)
{
	float   dist{0.f} ; // use it as a parameter to _ray_hit(), only

	auto	final_color{mAlbedo(0.,0.,0.)} ;

	ray_RT	lightDir{} ;
	RT_default_type   sr{0.} ;
	RT_default_type   cos_law{0.} ;

	for (auto li = _lights->begin_lights(), li_end = _lights->end_lights() ; li != li_end ; ++li) {
		lightDir = li->position() - hP, sr = lightDir.norm(), lightDir.normalize() ;
		cos_law = std::max(static_cast<RT_default_type>(0.), dotPR(lightDir, tr.shape().normalN())) ;
		if (cos_law == 0.) continue ; // || sr == 0.)	continue ;

		hP = (hP + (tr.shape().normalN()) * _shadowBias) ,
		lightDir = li->position() -  hP ;
	
		if (!(in_shadow(tr, hP, lightDir, sr)))	{

			final_color += ((alb * li->color()) * cos_law * 
							(li->intensity() / static_cast<float>(4.f * const_PI * sr))) ; // tr.color() ;
		}
	}

	return final_color ;
} // mScene_RT shadow_color()

mColor
mScene_RT::operator()(ray_RT ray, const ray_RT& camPos)
{
	static auto limit	{std::make_pair<mObjects_RT::const_iterator, ray_RT::value_type>(
									   {},
									   std::numeric_limits<ray_RT::value_type>::max())
						};
	ray_RT	hP{}, w_hP{} ;
	auto	current{limit} ;
	auto	chosen{current} ;

	size_t	nfound{(_objects->_base).size()} ; size_t  chosen_object{nfound} ;
	for (size_t count = 0 ; count < nfound ; count++)   {
		current = ray_hit(count, camPos, ray, w_hP) ;
		if (current.second < std::numeric_limits<ray_RT::value_type>::max()) {  // hit
			if (current.second < chosen.second) { 
				chosen = std::move(current), hP = std::move(w_hP),
				chosen_object = count ; 
			}
		}
	}
	if (chosen_object >= nfound)		return _clrb ;   // no hit

	// of all triangles of all the objects the closest has been chosen: 
	// check if in shadow and define its color
	// chosen.first points to the triangle chosen:

	return shadow_color(*(chosen.first), hP, ((_objects->_base)[chosen_object]).albedo()) ;
} // mScene operator()

std::ostream&
operator <<(std::ostream& os, const mScene_RT& sc)
{
	os << endl << endl << "-------- mScene: " ; // << (sc._base).size() << " element(s):";

	return os ;
} // mScene_RT friend operator <<
																		// eoc mScene_RT
// eof scene_RT.cpp