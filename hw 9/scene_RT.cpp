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
	std::pair<triangle_RT<RT_default_type>, vector_RT<unsigned int>> 
					wp{triangle_from_indexes<RT_default_type>(tria, vertices)} ;
	_shape = std::move(wp.first), 
	_iovs = std::move(wp.second) ;
	// _col = randomCRT() ;
} // mAtom_RT (data_iter, index for _data_iter)


std::ostream&
operator <<(std::ostream& os, const mAtom_RT& w)
{
	os << "atom{" << w.shape() ; // << "; color: " << w.color() ;
	os << "; VI: " << w.iovs() << "}" ;
	return os ;
} // mAtom_RT friend operator <<
																		// eoc mAtom_RT
												
																		// class mObjects_RT
size_t get_obj_texture(cl_SceneDescr* sd, cl_SceneDescr::iterator obj_i)
{
	size_t 		ti{0} ;
	std::vector<std::string>   settings{} ;

	settings = extract_data(*sd, obj_i->_name, "\"material_index\"", 1) ;
	if (settings.size() == 1)	ti = number_from_string<size_t>(settings[0]) ;
	
	return ti ;
} // get_obj_texture()

void atom_to_vertice_normals(const mAtom_RT& am, std::vector<ray_RT>& ver_normals)
{
	// all structures assumed to comply: otherwise the vectors should throw ;
	const vector_RT<unsigned int>&  iovs{am.iovs()} ;
	for (size_t i = 0 ; i < 3 ; i++) { // for each vertice add triangles's normal 
		ver_normals.at(iovs[i]) = ver_normals.at(iovs[i]) + (am.shape()).normalN() ;
	}

	return ;
}

void	
cl_objectRT::obj_fine_tune(bool fl)
{
	// cout << endl << "--- fine tuning object: " << (fl?"normalizing":"releasing") << " vertice normals" ;
	if (fl)			{ for (auto dr : _vertice_normals) dr.normalize() ; }
	else			_vertice_normals.clear() ;

	return ;
} // cl_objectRT obj_fine_tune


mObjects_RT::mObjects_RT(cl_SceneDescr* sd) : _base{}
{
	if (sd->activate())   cout << endl << "> the scene is active: set objects" ;
	else				  throw std::runtime_error("___ scene: cannot activate the descriptor") ;

	// preset a scope for "objects"
	std::vector<mAtom_RT>	atoms{} ;
	obox_RT					o_box{} ;
	
	_data_iter	tri_i{}, tri_ei ;	// access the triangles
	// _data_iter	ver_i{} ;			// begin(vertices)->: instead, use the indexes to speed up
	std::vector<long> 	ver_i{} ;		// indexing "vertices"
	std::vector<ray_RT> ver_normals{} ; // to place vertice normals in

	auto	obj_iter = sd->begin_scope("\"objects\"") ; auto oiter_end = obj_iter.limit() ;
	for ( ; obj_iter != oiter_end ; ++obj_iter) {
											// cout << endl << "--- loading data for: " << obj_iter->_name ;
		o_box = obox_RT{} ;
		tri_i = sd->begin_data(sd->data(obj_iter, "\"triangles\""), 3), tri_ei = tri_i.limit() ;
		ver_i = sd->index_data(sd->begin_data(sd->data(obj_iter, "\"vertices\""), 3)) ;
		
		ver_normals.resize(ver_i.size(), ray_RT{0,0,0}) ;	// holder of vertice normals
															// cout << " - # of vertices: " << ver_i.size() ;
		for (size_t i = 0 ; tri_i != tri_ei ; i++, ++ tri_i) {   // for all triangles
			atoms.push_back(std::move(mAtom_RT(tri_i, ver_i))) ;
			atom_to_vertice_normals(atoms[i], ver_normals) ;
			o_box.min_max(atoms[i].shape()) ;
		}
																	// cout << " -> the box is: " << o_box ;
		atoms.shrink_to_fit(),
		_base.push_back(std::move(
									cl_objectRT(std::move(atoms), 
												get_obj_texture(sd, obj_iter),
												std::move(o_box), 
												std::move(ver_normals)
										       )
								 )
		), 
		atoms.clear() ;
		ver_normals.clear() ;
	}
	_base.shrink_to_fit() ;
} // mObjects_RT ()

bool
cl_objectRT::box_hit(const ray_RT& orig, const ray_RT& ray) const 
{
	ray_RT	pHit{} ;
	bool	fl{true} ;	// use it if no box's been defined
	if (!(_box != obox_RT{}))		throw std::runtime_error("_object: invalid Box") ;

	fl = _box._box_hit(orig, ray, pHit) ;
	// if (!(fl = _box._box_hit(orig, ray, pHit)))	boxMisses++ ;

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
	_base.shrink_to_fit() ;
} // cl_seqLightsRT
																		// eoc lights
																		// classes for lights
cl_seqTexturesRT::cl_seqTexturesRT(cl_SceneDescr* sd)
{
	if (sd->activate())   cout << " & textures" ;
	else				  throw std::runtime_error("___ scene: cannot activate the descriptor") ;

	texture_RT::e_Texture_type	t_type{texture_RT::e_Texture_type::Diffuse} ;
	mAlbedo						color{} ;
	bool						fl_smooth{false} ;

	std::vector<std::string>   settings ;
	auto	li_iter = sd->begin_scope("\"materials\"") ; auto liter_end = li_iter.limit() ;

	for (; li_iter != liter_end ; ++li_iter) {
		// cout << endl << "--- loading data for: " << li_iter->_name ;
		// Type of texture
		settings = extract_data(*sd, li_iter->_name, "\"type\"", 1) ;
		if (settings.size() == 1) {
			t_type = (settings[0].find("reflective") != std::string::npos) 
					? texture_RT::e_Texture_type::Reflective
					: texture_RT::e_Texture_type::Diffuse ;
		} else t_type = texture_RT::e_Texture_type::Diffuse ;

		// Albedo
		settings = extract_data(*sd, li_iter->_name, "\"albedo\"", 3) ;
		if (settings.size() == 1)	color = color_from_string<RT_default_type, mAlbedo>(settings[0]) ;
		else						color = mAlbedo{1.,1.,1.} ;

		// Smooth flag
		settings = extract_data(*sd, li_iter->_name, "\"smooth_shading\"", 3) ;
		fl_smooth = false ;
		if (settings.size() == 1)    fl_smooth = (settings[0].find("true") != std::string::npos) ;
		
		_base.push_back(texture_RT(t_type, color, fl_smooth)) ;
	}
	if (_base.empty())	throw std::runtime_error("___ materials missing") ;

	_base.shrink_to_fit() ;
} // cl_seqtexturesRT
																		// eoc textures
																		// class mScene_RT
mScene_RT::mScene_RT(cl_SceneDescr * sd, mObjects_RT *objs, cl_seqLightsRT *lights, cl_seqTexturesRT *tex) 
	: _objects{objs}, _lights{lights}, _clrb{}, _shadowBias{}, _set_concurrency{}, _textures{tex}
{
	if (!(sd->activate()))   throw std::runtime_error("___ scene: cannot activate the descriptor") ;
	
	_clrb = color_from_string<RT_default_type, mAlbedo>
							 (*(sd->begin_data(sd->data("\"settings\"", "\"background_color\""), 3))) ;
															
	_shadowBias = number_from_string<RT_default_type>     // 0 if data not found
					(*(sd->begin_data(sd->data("\"settings\"", "\"shadow_bias\""), 3))) ;
	if (_shadowBias == 0)  _shadowBias = static_cast<RT_default_type>(0.002) ;

	scene_fine_tune() ;		// the Scene is set - fine tune it: like veritices normals(, etc)

	cout << endl << endl << ": # of objects: " << _objects->_base.size() ;
	int i = 0 ;
	for (auto dr : _objects->_base) {
		cout << endl << "--- atoms in object #" << i++ << ": " << dr.size() 
			<< "; " << texture(dr) ; // << "; " << dr.box() ;
		/*for (auto at = dr.obj_begin() ; at != dr.obj_end() ; ++at) {
			cout << endl << "------ " << *at ;
		}*/
	}
	cout << *_lights ;
	cout << endl << ": background color: " << _clrb << "; shadow bias: " << _shadowBias ;
} // mScene_RT ()

void
mScene_RT::scene_fine_tune()
{
	for (auto dr : _objects->_base) { dr.obj_fine_tune(smooth_shading(dr)) ; }

	return ;
} // mScene_RT scene_fine_tune()

void
mScene_RT::set_concurrency(cl_SceneDescr * sd, unsigned int W, unsigned int H)
{
	// set parameters of concurrency: number of threads
	_set_concurrency._num_of_threads = number_from_string<size_t>
					(*(sd->begin_data(sd->data("\"concurrency\"", "\"#_of_threads\""), 1))) ;
	if (_set_concurrency._num_of_threads == 0) {
		_set_concurrency._num_of_threads = (2 * std::thread::hardware_concurrency()) ;
	}

	{ // prepare the slices
		size_t   rn{number_from_string<size_t>
				(*(sd->begin_data(sd->data("\"concurrency\"", "\"#_of_horizontal_cuts\""), 1)))
		} ;
		if (rn == 0)	rn = (_set_concurrency._num_of_threads) ; // # of horizontal cuts

		size_t   cn{number_from_string<size_t>
				(*(sd->begin_data(sd->data("\"concurrency\"", "\"#_of_vertical_cuts\""), 1)))
		} ;
		if (cn == 0)	cn = (2 * _set_concurrency._num_of_threads) ; // # of vertical cuts

		_set_concurrency._slices = slices_to_render(W, H, rn, cn) ;
		slcAssigned.store(0) ;	// the next slice to assign for processing
	}

	if (_set_concurrency._num_of_threads > _set_concurrency._slices.size()) {
		_set_concurrency._num_of_threads = _set_concurrency._slices.size() ;
	}

	return ;
} // mScene_RT set_concurrency()


std::pair<mObjects_RT::const_iterator, ray_RT::value_type>	// locate the closest triangle of an Object(ind)
mScene_RT::ray_hit(size_t ind_iter, const ray_RT& orig, const ray_RT& ray, ray_HP& hP)
{
	auto		distance{std::numeric_limits<ray_RT::value_type>::max()} ;	// the infinity
	auto		cdist{distance} ;											// for the closest
	ray_HP		w_hP{} ;
	
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
	ray_HP			   whit ;		// ...

	for (auto obj = cbegin(_objects->_base), obj_e = cend(_objects->_base) ; obj != obj_e ; ++obj) {
		for (auto tr = obj->obj_begin(), tr_e = obj->obj_end() ; tr != tr_e ; ++tr) {
			if (tr->_ray_hit(hP, ld, cdist, whit)) if (cdist < l_dist) {   return true ; }
		}
	}
	return false ;
} // mScene_RT in_shadow()

mColor
mScene_RT::shadow_color(const mAtom_RT& tr, ray_HP& hP, const cl_objectRT& ob)
{
	float   dist{0.f} ; // use it as a parameter to _ray_hit(), only

	auto	final_color{mAlbedo(0.,0.,0.)} ;

	ray_RT	lightDir{} ;
	ray_RT	tri_normal{} ;
	RT_default_type   sr{0.} ;
	RT_default_type   cos_law{0.} ;

	for (auto li = _lights->begin_lights(), li_end = _lights->end_lights() ; li != li_end ; ++li) {
		lightDir = li->position() - hP._hp, sr = lightDir.norm(), lightDir.normalize() ;
		tri_normal = (smooth_shading(ob)) ? ob.hitNormal(tr, hP) : tr.shape().normalN() ;
		cos_law = dotPR(lightDir, tri_normal) ;
		// cos_law = std::max(static_cast<RT_default_type>(0.), dotPR(lightDir, tr.shape().normalN())) ;
		if (near_zero(cos_law) || cos_law < 0.)  continue ; // || sr == 0.)	continue ;

		// hP._hp = (hP._hp + (tr.shape().normalN()) * _shadowBias),
		hP._hp = (hP._hp + tri_normal * _shadowBias) ,
		lightDir = li->position() -  hP._hp ;
	
		if (!(in_shadow(tr, hP._hp, lightDir, sr)))	{
			final_color += 	((((albedo(ob)) * li->color()) * cos_law * 
							(li->intensity() / static_cast<float>(4.f * const_PI * sr)))) ; // tr.color() ;
		}
	}
	if (_lights->num_of_lights() == 0)   final_color = tr.bc_color(hP) ;  // as, there are no lights
	return final_color ;
} // mScene_RT shadow_color()

mColor
mScene_RT::operator()(ray_RT ray, const ray_RT& camPos)
{
	ray_RT							orig{camPos} ;	// for the stack
	size_t							obj_first{0} ;	// ...
	ray_HP							hP_first{} ;	// ...
	mObjects_RT::const_iterator		atom_first{} ;	// ...

	for (int depth = 0 ; depth < Scene_MAX_Depth ; depth++) { // recursion for other than diffuse textures
		static auto limit{std::make_pair<mObjects_RT::const_iterator, ray_RT::value_type>(
										   {},
										   std::numeric_limits<ray_RT::value_type>::max())
		};
		ray_HP	hP{}, w_hP{} ;
		auto	current{limit} ;
		auto	chosen{current} ;

		size_t	nfound{(_objects->_base).size()} ; size_t  chosen_object{nfound} ;
		for (size_t count = 0 ; count < nfound ; count++) {
			// current = ray_hit(count, camPos, ray, w_hP) ;
			current = ray_hit(count, orig, ray, w_hP) ;
			
			if (current.second < chosen.second) {
				chosen = std::move(current), hP = std::move(w_hP),
					chosen_object = count ;
			}
		}
		if (chosen_object >= nfound) {
			if (depth > 0)		break   ;
			return _clrb ;		// no hit
		}
		if (reflective((_objects->_base)[chosen_object])) { 
			if (depth == 0) {   // save the stack for shadow_color()
				atom_first = chosen.first ;		// the atom/triangle
				hP_first = hP ;					// the hit point
				obj_first = chosen_object ;		// ...
			}
			ray = reflected_ray(ray, (_objects->_base)[chosen_object], *(chosen.first)) ;
			orig = hP._hp  + (((chosen.first)->shape()).normalN()) * _shadowBias ;
		} else {
			// of all NOT reflective triangles of all the objects the closest has been chosen: 
			// check if in shadow and define its color
			// chosen.first points to the triangle chosen:
			return shadow_color(*(chosen.first), hP, ((_objects->_base)[chosen_object])) ;
		}
	}
	return shadow_color(*atom_first, hP_first, (_objects->_base)[obj_first]) ;
	// return(mColor{}) ;
} // mScene operator()

std::ostream&
operator <<(std::ostream& os, const mScene_RT& sc)
{
	os << endl << endl << "-------- mScene: " ; // << (sc._base).size() << " element(s):";

	return os ;
} // mScene_RT friend operator <<
																		// eoc mScene_RT
// eof scene_RT.cpp