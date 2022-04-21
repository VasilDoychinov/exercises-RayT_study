// hw8 main.cpp: to generate .ppm files from *.crtscene
//		NB: - all scene files looked for as in data/scene?.crtscene" ;
//			- all .ppm files generated in ppm/scene?/" ;
//		- uses: - ../tools/parse/parse_RT.* and ../tools/parse/iterators_RT.h
//				- scne_RT.h, etc
//

#include <iostream>
using std::cout ;
using std::endl ;
#include <string>
using std::string ;
#include <exception>

#include <thread>

#include "../tools/miscellaneous.h"
#include "../tools/parser/parser_RT.h"

/*
#include "miscellaneous.h"
#include "parser_RT.h"
*/

#include "scene_RT.h"


const char *  SCENE0_name = "data/scene0.crtscene" ;
const char *  SCENE1_name = "data/scene1.crtscene" ; // 206 s
const char *  SCENE2_name = "data/scene2.crtscene" ; // 
const char *  SCENE3_name = "data/scene3.crtscene" ; // 2.93e+3
const char *  SCENE4_name = "data/scene4.crtscene" ;

string extract_fname(const string& str) ;						 // remove extension and path
string ppm_fname(string&& str, int cam_id, string batch_id = {}) ; // .ppm file name as: ppm/task/task_camID.ppm

void		get_sceneWH(cl_SceneDescr& scene, unsigned int& w, unsigned int& h) ;
camera_RT	get_sceneCAM(cl_SceneDescr& scene) ;

void render_scene(const char* scene_name) ;

void funct_wait(const slcRays& x) {
	// cout << endl << "--- slc: " << x << endl ;
	std::this_thread::sleep_for(std::chrono::seconds(2)) ;
}

/*
float l_max = 0.f, sr_min ;
size_t li_max = 0 ;
mAlbedo alb_max ;
*/

int
main()
{
	cout << endl << endl ;
	cout << "> id of main() is: " << std::this_thread::get_id() ;
	std::cout << endl << "> " << std::thread::hardware_concurrency()
		<< " concurrent threads are supported;" << endl ;
	srand(static_cast<unsigned int>(time(0))) ; // used for color generation


	try {
		// unsigned char   ch ;

		if (true) {
			render_scene(SCENE3_name) ;
			//cout << endl << endl << "--- max: " << l_max << "-> lint: " << li_max 
				//<< "; r_min: " << sr_min << "; alb: " << alb_max ;
		}
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << "--- That's it ..." ;
	return(0) ;

} // main()

void render_scene(const char* scene_name)
{
	unsigned int   grWidth = 0, grHeight = 0 ;

	cl_SceneDescr	scene_descr{scene_name} ;
	scene_descr.activate() ;						// cout << endl << "- ", scene_descr.show_sections(cout) ;
	mObjects_RT		scene_objects{&scene_descr} ;
	cl_seqLightsRT	scene_lights(&scene_descr) ;

	mScene_RT		scene_rt{&scene_descr, &scene_objects, &scene_lights} ;
	cout << endl << endl << " --- <" << scene_descr.name()
		<< "> ready: " << std::boolalpha << scene_descr.activate() ;
	
	get_sceneWH(scene_descr, grWidth, grHeight) ;
	cout << endl << "--- resolution: " << grWidth << " x " << grHeight ;
	camera_RT  camWS{get_sceneCAM(scene_descr)} ; 
	cout << endl << "--- camera: " << camWS << " --- calculating rays" ;

	render_RT	test(grWidth, grHeight, camWS) ;	// the Rays: camera at camWS
	gridImage	gIm(grWidth, grHeight) ;			// use for an image
									// slcRays		all(coord_RASTER(0, 0), grHeight, grWidth, grWidth) ;
	// throw std::range_error("------ partial completion ------") ;

	unsigned int cNumb = test.rwd_cameras_list() ;	// start from the camera at WS
	auto iSlices = slices_to_render(grWidth, grHeight, 20, 30) ; // for_threads(grWidth, grHeight) ; // 
	for (unsigned int i = 0 ; i < cNumb ; i++) {
		// render_imSlcV2(std::move(funct_wait), iSlices);
		render_imSlcV2(std::move(clF_RenderSlice<mScene_RT>
								 (std::move(&test),
								  std::move(&gIm), std::move(&scene_rt))),
					   iSlices);
		grid_to_ppm(gIm, ppm_fname(extract_fname(scene_name), i)) ;

		test.next_camera() ;
	}
} // render_scene()


void get_sceneWH(cl_SceneDescr& scene_rt, unsigned int& w, unsigned int& h)
{
	try {
		w = h = 0 ;
		std::vector<string>   settings{extract_data(scene_rt, "\"image_settings\"", "\"width\"", 1)} ;
		if (settings.size() > 1)			throw std::runtime_error("___ image_settings: width") ;
		w = number_from_string<unsigned int>(settings[0]) ;

		settings =extract_data(scene_rt, "\"image_settings\"", "\"height\"", 1) ;
		if (settings.size() > 1)			throw std::runtime_error("___ image_settings: height") ;
		h = number_from_string<unsigned int>(settings[0]) ;
	} catch (...) { throw ; }

	if (w == 0 || h == 0)				throw std::runtime_error("___ settings: width/height") ;
}


camera_RT get_sceneCAM(cl_SceneDescr& scene_rt)
{
	try {

		std::vector<string>   settings{extract_data(scene_rt, "\"camera\"", "\"matrix\"", 3)} ;
				
		if (settings.size() != 3)			throw std::runtime_error("___ camera: matrix") ;
		matrix_RT m{vector_from_string<RT_default_type>(settings[0]),
					vector_from_string<RT_default_type>(settings[1]),
					vector_from_string<RT_default_type>(settings[2])
		} ;

		settings = std::move(extract_data(scene_rt, "\"camera\"", "\"position\"", 3)) ;
		
		if (settings.size() != 1)			throw std::runtime_error("___ camera: position") ;
		return(camera_RT(std::move(vector_from_string<RT_default_type>(settings[0])),
						 std::move(m))) ;
		
	} catch (...) { throw ; }
	return(camera_RT()) ;
}


#include <sstream>
#include <iomanip>

const char *			ppm_Dir = "ppm/" ;
const char *			ppm_Form = "crt_output.ppm" ;

string ppm_fname(string&& str, int cam_id, string batch_id)		// to generate a .ppm file name in sub-folder ppm_Dir
{
	string	camera_id{} ;
	// cout << endl << "--- to ppm_fname: <" << str << ">" ;
	if (cam_id >= 0) {
		std::stringstream sss ; sss << std::setw(2) << std::setfill('0') << std::to_string(cam_id) ;

		camera_id = "_(" + batch_id + "_camera #" + sss.str() + ")" ;
	}
	return(ppm_Dir + str + '/' + str + camera_id + ppm_Form) ;
} // ppm_fname()


string extract_fname(const string& str)
{
	// cout << endl << endl << endl << "--- to extract_fname: <" << str << ">" ;

	size_t	e_pos{str.rfind('.')} ;
	size_t	s_pos{str.find_last_of("/\\")} ;

	s_pos = (s_pos == std::string::npos) ? 0 : s_pos + 1 ;
	if (e_pos == std::string::npos || e_pos <= s_pos) throw std::runtime_error("___ extract: wrong file name") ;
	// e_pos-- ;
	// cout << endl << "--- in extract_fname: (" << s_pos << ", " << e_pos << ")" ;
	return(str.substr(s_pos, e_pos - s_pos)) ;
}

/* THANK YOU !!!!!!!!
* 
* 
using pair_PackedTask = std::pair <std::thread, std::future<void >> ;
using vect_PackedTasks = std::vector < pair_PackedTask> ;
using packed_Rays = std::packaged_task<void()> ;

void funct_wait(const slcRays& x) { 
	// cout << endl << "--- slc: " << x << endl ;
	std::this_thread::sleep_for(std::chrono::seconds(2)) ; 
}


		pair_PackedTask w_pair{} ;

		std::future<void>	result ;
		std::thread			th ;
		constexpr std::chrono::seconds  l{4} ;

		cout << endl << "> assign_task() about to start" << endl ;
		// th = assign_task([&](int x)->void {
			//								std::this_thread::sleep_for(std::chrono::seconds(l)) ; },
		// 100, result) ;

		w_pair.first = _assign_task(funct_wait, slcRays(), w_pair.second) ;
		for (int i = 0 ; !is_task_ready(w_pair.second) ; i++) {
			cout << "> outside assign_tast() iteration #" << i
				<< "-> task ready: " << std::boolalpha << is_task_ready(w_pair.second) << "\r" ;
		}
		cout << endl << "--- task ready: " << std::boolalpha << is_task_ready(w_pair.second) << " -> joining " ;
		if (w_pair.first.joinable())	w_pair.first.join() ;



template <typename Funct> std::thread
_assign_task(Funct func, const slcRays& slc, std::future<void>& res)
{
	packed_slcRays		do_task(std::move(func)) ;
	std::future<void>	result = do_task.get_future() ;

	std::thread th(std::move(do_task), slc) ;
	// if (fl_detach)	(w_pair.first).detach() ;	

	res = std::move(result) ;
	return(th) ;
}
*/

// eof hw8 main.cpp