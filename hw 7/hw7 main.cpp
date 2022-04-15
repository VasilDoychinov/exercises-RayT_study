// hw7 main.cpp: to generate .ppm files from *.crtscene
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
const char *  SCENE1_name = "data/scene1.crtscene" ;
const char *  SCENE2_name = "data/scene2.crtscene" ;
const char *  SCENE3_name = "data/scene3.crtscene" ;
const char *  SCENE4_name = "data/scene4.crtscene" ;

string extract_fname(const string& str) ;						 // remove extension and path
string ppm_fname(string&& str, int cam_id, string batch_id = {}) ; // .ppm file name as: ppm/task/task_camID.ppm

void		get_sceneWH(cl_SceneDescr& scene, unsigned int& w, unsigned int& h) ;
camera_RT	get_sceneCAM(cl_SceneDescr& scene) ;

void render_scene(const char* scene_name) ;

int
main()
{
	cout << endl << endl ;
	cout << "> id of main() is: " << std::this_thread::get_id() ;
	std::cout << endl << "> " << std::thread::hardware_concurrency()
		<< " concurrent threads are supported;" ;
	cout << endl ;

	srand(static_cast<unsigned int>(time(0))) ; // used for color generation

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
		cout << endl << endl << "> render data/scene4 (y/n): " ; std::cin >> ch ;
		if (std::toupper(ch) == 'Y')	render_scene(SCENE4_name) ;

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
	mObjects_RT		scene_objects{&scene_descr} ;

	mScene_RT		scene_rt{&scene_descr, &scene_objects} ;
	cout << endl << endl << " --- <" << scene_descr.name()
		<< "> ready: " << std::boolalpha << scene_descr.activate() ;
	
	get_sceneWH(scene_descr, grWidth, grHeight) ;
	cout << endl << "--- resolution: " << grWidth << " x " << grHeight ;
	camera_RT  camWS{get_sceneCAM(scene_descr)} ; cout << endl << "--- camera: " << camWS ;

	render_RT	test(grWidth, grHeight, camWS) ;	// the Rays: camera at camWS
	gridImage	gIm(grWidth, grHeight) ;			// use for an image
	slcRays		all(coord_RASTER(0, 0), grHeight, grWidth, grWidth) ;

	unsigned int cNumb = test.rwd_cameras_list() ;	// start from the camera at WS
	auto iSlices = slices_for_threads(grWidth, grHeight) ; // 
	for (unsigned int i = 0 ; i < cNumb ; i++) {
		render_imSlices(std::move(clF_RenderSlice<mScene_RT>
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
		std::vector<string>   settings{extract_data(scene_rt, "\"settings\"", "\"image_settings\"", 1)} ;
		w = h = 0 ;
		size_t   pos = 0 ;
		for (auto s : settings) {
			if ((pos = s.find("\"width\"")) != std::string::npos 
				&& (pos = s.rfind(':')) != std::string::npos) {
				w = static_cast<unsigned int>(std::stoi(s.substr(pos + 1))) ;
			}
			else if ((pos = s.find("\"height\"")) != std::string::npos 
					 && (pos = s.rfind(':')) != std::string::npos) {
				h = static_cast<unsigned int>(std::stoi(s.substr(pos + 1))) ;
			}
																				// cout << endl << "   " << s ;
		}
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
	return(ppm_Dir + str + camera_id + ppm_Form) ;
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


// eof hw7 main.cpp