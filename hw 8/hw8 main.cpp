// hw8 main.cpp: to generate .ppm files from *.crtscene
//		NB: - all scene files looked for as in data/scene?.crtscene" ;
//			- all .ppm files generated in ppm/scene?/" ;
//		- uses: - ../tools/parse/parse_RT.* and ../tools/parse/iterators_RT.h
//				- scne_RT.h, etc
//		- scene_RT.*:
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
const char *  SCENE1_name = "data/scene1.crtscene" ;
const char *  SCENE2_name = "data/scene2.crtscene" ;
const char *  SCENE3_name = "data/scene3.crtscene" ;


string extract_fname(const string& str) ;							// remove extension and path
string ppm_fname(string&& str, int cam_id, string batch_id = {}) ;	// .ppm file name: ppm/task/task_camID.ppm

void		get_sceneWH(cl_SceneDescr& scene, unsigned int& w, unsigned int& h) ;
camera_RT	get_sceneCAM(cl_SceneDescr& scene) ;

void render_scene(const char* scene_name) ;

// size_t		atomChecks{0} ;
// size_t		boxMisses{0} ;
// size_t		boxChecks{0} ;


int
main()
{
	cout << endl << endl ;
	cout << "> id of main() is: " << std::this_thread::get_id() ;
	std::cout << endl << "> " << std::thread::hardware_concurrency()
		<< " concurrent threads are supported;" << endl ;
	

	try {
		unsigned char   ch ;

		cout << endl << "> render data/scene0 (y/n): " ; std::cin >> ch ;
		if (std::toupper(ch) == 'Y')	render_scene(SCENE0_name) ;

		cout << endl << endl << "> render data/scene1 (y/n): " ; std::cin >> ch ;
		if (std::toupper(ch) == 'Y')	render_scene(SCENE1_name) ;

		cout << endl << endl << "> render data/scene2 (y/n): " ; std::cin >> ch ;
		if (std::toupper(ch) == 'Y')	render_scene(SCENE2_name) ;

		cout << endl << endl << "> render data/scene3 (y/n): " ; std::cin >> ch ;
		if (std::toupper(ch) == 'Y')	render_scene(SCENE3_name) ;
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << "--- That's it ..." ;
	return 0 ;

} // main()

void render_scene(const char* scene_name)
{
	unsigned int   grWidth = 0, grHeight = 0 ;

	cl_SceneDescr	scene_descr{scene_name} ;
	scene_descr.activate() ;						// cout << endl << "- ", scene_descr.show_sections(cout) ;

	mObjects_RT		scene_objects{&scene_descr} ;
	cl_seqLightsRT	scene_lights(&scene_descr) ;
	camera_RT		camWS{get_sceneCAM(scene_descr)} ;

	mScene_RT		scene_rt{&scene_descr, &scene_objects, &scene_lights} ;
	cout << "\n: camera #0: " << camWS ;

	get_sceneWH(scene_descr, grWidth, grHeight) ; 
	cout << "\n: resolution: " << grWidth << " x " << grHeight ;
	cout << endl << "\n--- <" << scene_descr.name() << "> ready: "<<std::boolalpha<<scene_descr.activate() ;

	scene_rt.set_concurrency(&scene_descr, grWidth, grHeight) ;

	render_RT	test(grWidth, grHeight, camWS) ;	// the Rays: camera #0: camWS
	gridImage	gIm(grWidth, grHeight) ;			// image plane
												// throw std::range_error("------ partial completion ------") ;

	unsigned int cNumb = test.rwd_cameras_list() ;	// start from the camera at WS
	for (unsigned int i = 0 ; i < cNumb ; i++) {
		render_slicesV3(std::move(clF_renderSlices<mScene_RT>
								 (std::move(&test),
								  std::move(&gIm), std::move(&scene_rt))),
					   scene_rt.get_concurrency()) ;
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
		return camera_RT(std::move(vector_from_string<RT_default_type>(settings[0])),
						 std::move(m)) ;
		
	} catch (...) { throw ; }
	return camera_RT() ;
}


#include <sstream>
#include <iomanip>

const char *			ppm_Dir = "ppm/" ;
const char *			ppm_Form = "crt_output.ppm" ;

string ppm_fname(string&& str, int cam_id, string batch_id)	// prepare  .ppm file name for sub-folder ppm_Dir
{
	string	camera_id{} ;

	if (cam_id >= 0) {
		std::stringstream sss ; sss << std::setw(2) << std::setfill('0') << std::to_string(cam_id) ;

		camera_id = "_(" + batch_id + "_camera #" + sss.str() + ")" ;
	}
	return ppm_Dir + str + '/' + str + camera_id + ppm_Form ;
} // ppm_fname()


string extract_fname(const string& str)
{
	size_t	e_pos{str.rfind('.')} ;
	size_t	s_pos{str.find_last_of("/\\")} ;

	s_pos = (s_pos == std::string::npos) ? 0 : s_pos + 1 ;
	if (e_pos == std::string::npos || e_pos <= s_pos) throw std::runtime_error("___ extract: wrong file name") ;

	return str.substr(s_pos, e_pos - s_pos) ;
}


// eof hw8 main.cpp