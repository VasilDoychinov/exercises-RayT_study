// hw8 main.cpp: to generate .ppm files from *.crtscene
//		NB: - all scene files looked for as in data/scene?.crtscene" ;
//			- all .ppm files generated in ppm/scene?/" ;
//		- uses: - ../tools/parse/parse_RT.* and ../tools/parse/iterators_RT.h
//				- scne_RT.h, etc
//		
//		- major function: render_scene()
//		- functionality for adding cameras through its parameter std::function<> set_cameras
//		- functionality for adding moves between two sequential cameras ... std::function<> scn_moves
//		- some manula work like switching descriptor files .crtscene still required yet
//		
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


string extract_fname(const string& str) ;							// remove extension and path
string ppm_fname(string&& str, int cam_id, string batch_id = {}) ;	// .ppm file name: ppm/task/task_camID.ppm

void		get_sceneWH(cl_SceneDescr& scene, unsigned int& w, unsigned int& h) ;
camera_RT	get_sceneCAM(cl_SceneDescr& scene) ;

// Additional cameras
void batch0_cameras(mScene_RT&, render_RT&) ;
void batch1_cameras(mScene_RT&, render_RT&) ;
void batch1_2_cameras(mScene_RT&, render_RT&) ;
void batch2_cameras(mScene_RT&, render_RT&) ;
void dragon_cameras(mScene_RT&, render_RT&) ;
// Additional moves
void batch0_moves(mScene_RT&, size_t) ;
void batch1_moves(mScene_RT&, size_t) ;
void batch1_2_moves(mScene_RT&, size_t) ;
void batch2_moves(mScene_RT&, size_t) ;
void dragon_moves(mScene_RT&, size_t) ;
void render_scene(const char* scene_name, std::function<void(mScene_RT&, render_RT&)> cameras, std::function<void(mScene_RT&, size_t)> moves) ;

int
main()
{
	cout << endl << endl ;
	cout << "> id of main() is: " << std::this_thread::get_id() ;
	std::cout << endl << "> " << std::thread::hardware_concurrency()
		<< " concurrent threads are supported;" << endl ;
	

	try {
		unsigned char   ch ;

		//cout << endl << endl << "> render batch 0 (y/n): " ; std::cin >> ch ;
		//if (std::toupper(ch) == 'Y')	render_scene(SCENE0_name, batch0_cameras, batch0_moves) ;
		
		// cout << endl << endl << "> render batch 1 (y/n): " ; std::cin >> ch ;
		// if (std::toupper(ch) == 'Y')	render_scene(SCENE1_name, batch1_cameras, batch1_moves) ;

		// cout << endl << endl << "> render batch 1_2 (y/n): " ; std::cin >> ch ;
		// if (std::toupper(ch) == 'Y')	render_scene(SCENE1_name, batch1_2_cameras, batch1_2_moves) ;
		// cout << endl << endl << "> render batch 2 (y/n): " ; std::cin >> ch ;
		// if (std::toupper(ch) == 'Y')	render_scene(SCENE1_name, batch2_cameras, batch2_moves) ;

		cout << endl << endl << "> render dragon scene (y/n): " ; std::cin >> ch ;
		if (std::toupper(ch) == 'Y')	render_scene(SCENE2_name, dragon_cameras, dragon_moves) ;

		//cout << endl << endl << "> render batch 3 (y/n): " ; std::cin >> ch ;
		//if (std::toupper(ch) == 'Y')	render_scene(SCENE1_name, batch3_cameras, batch3_moves) ;

	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << "--- That's it ..." ;
	return 0 ;

} // main()

void batch0_cameras(mScene_RT& scene_rt, render_RT& test)
{
	auto camWS{test.camera(0)} ;

	scene_rt.move_object_by(1, 0.f, +1.5f, -1.5f) ;
	test.add_camera(camWS), test.add_camera(camWS), test.add_camera(camWS),
	test.add_camera(camWS), test.add_camera(camWS), test.add_camera(camWS), 
	test.add_camera(camWS) ;
	
	return ;
} // batch0_cameras()

void batch0_moves(mScene_RT& scene_rt, size_t cind)
{
	scene_rt.move_object_by(1, 0.f, -0.25f, 0.25f) ;    // 4 x: (0, -0.25, +0.25) to land it
	
	return ;
} // batch0_moves()

void batch1_cameras(mScene_RT& scene_rt, render_RT& test)
{
	auto camWS{test.camera(0)} ;

	scene_rt.move_object_by(2, 0.f, 3.f, -16.f) ;	cout << endl << "--- move(2) #0 by:" << "0, 3, 16" ;
	test.add_camera(camWS), test.add_camera(camWS), test.add_camera(camWS),	
	test.add_camera(camWS), test.add_camera(camWS), test.add_camera(camWS), 
	test.add_camera(camWS), test.add_camera(camWS), test.add_camera(camWS) ;

	return ;
} // batch1_cameras()

void batch1_moves(mScene_RT& scene_rt, size_t cind)
{
	RT_default_type		y_move{-0.5f}, z_move{2.f} ;
	// if (cind == 2)		{ y_move = -0.5f ; }
	if (cind == 3)		{ y_move = -1.2f, z_move = 4.0f ; }
	else if (cind == 4)		{ y_move = -0.6f ; z_move = 3.0f ; }
	else if (cind == 5) { y_move = -0.2f ; z_move = 3.0f ; }
	else if (cind == 6) { y_move = 0.f ; z_move = 2.0f ; }
	else if (cind == 7) { y_move = -0.05f ; z_move = 0.2f ; }
	else if (cind == 8) { y_move = -0.1f ; z_move = -0.1f ; }
	else if (cind == 9) { y_move = -0.1f ; z_move = 0.f ; }
	cout << endl << "--- move(2) #" << cind << " by:" << "0, " << y_move << ", " << z_move ;
	scene_rt.move_object_by(2, 0.f, y_move, z_move) ;
		
	return ;
} // batch1_moves()

void batch1_2_cameras(mScene_RT& scene_rt, render_RT& test)
{
	auto camWS{test.camera(0)} ;
	
	test.add_camera(camWS) ; 

	return ;
} // batch1_2_cameras()

void batch1_2_moves(mScene_RT& scene_rt, size_t cind)
{
	scene_rt.move_light_by(0, 0, 0, -1) ;	//cout << endl << "--- move(2) #0 by:" << "0, 3, 16" ;
	scene_rt.move_light_by(1, 0, 0, -1) ;
	scene_rt.set_light_albedo(1, mAlbedo{1.f, 0.6f, 0.2f}) ;
	
	return ;
} // batch1_2_moves()


void batch2_cameras(mScene_RT& scene_rt, render_RT& test)
{
	constexpr auto	rot_angle = 15.f ;
	constexpr auto	rot_radius = 3.f ;
	auto cam{test.camera(0)} ;
	static const float truck_m{static_cast<RT_default_type>(rot_radius * sin(degs_to_rads(rot_angle)))} ;
	static const float dolly_m{static_cast<RT_default_type>(rot_radius - rot_radius * cos(degs_to_rads(rot_angle)))} ;
	
	for (int i = 0 ; i < 24 ; i++)	test.add_camera(cam = dolly(dolly_m, truck(truck_m, pan(15.f, cam)))) ;
	
	return ;
} // batch3_cameras()

void batch2_moves(mScene_RT& scene_rt, size_t cind)
{
	return ;
} // batch3_moves()


void dragon_cameras(mScene_RT& scene_rt, render_RT& test)
{
	constexpr auto	rot_angle = 45.f ;
	constexpr auto	rot_radius = 30.f ; // 30.f ;  // 40
	// auto cam{test.camera(0)} ;
	camera_RT cam{camera_to_position(ray_RT{0.f, 2.f, 30.f})} ;

	static const RT_default_type truck_m{static_cast<RT_default_type>(rot_radius * sin(degs_to_rads(rot_angle)))} ;
	static const RT_default_type dolly_m{static_cast<RT_default_type>(rot_radius - rot_radius * cos(degs_to_rads(rot_angle)))} ;

	test.add_camera(cam) ;
	for (int i = 0 ; i < 6 ; i++)   test.add_camera(cam = dolly(dolly_m, truck(-truck_m, pan(-rot_angle, cam)))) ;
	test.add_camera(dolly(-15, cam)) ;	// dragon at 90: forward
	cam = dolly(dolly_m, truck(-truck_m, pan(-rot_angle, cam))) ;
	test.add_camera(truck(20, boom(-15, cam))) ;
	test.add_camera(truck(10, boom(-7, cam))) ;
	// for camera 11, switch descriptor file: MANUALLY place scene2_1.crtscene as scene2.crtscene
	test.add_camera(cam = test.camera(0)) ;
	// for camera 12, switch descriptor file: MANUALLY place scene2_2.crtscene as scene2.crtscene
	test.add_camera(cam) ;

	return ;
} // dragon_cameras()

void dragon_moves(mScene_RT& scene_rt, size_t cind)
{
	return ;
} // dragon_moves()



void render_scene(const char* scene_name, 
				  std::function<void(mScene_RT&, render_RT&)> set_cameras,
				  std::function<void(mScene_RT&, size_t)> scn_moves)
{
	unsigned int   grWidth = 0, grHeight = 0 ;

	cl_SceneDescr	scene_descr{scene_name} ;
	scene_descr.activate() ;						// cout << endl << "- ", scene_descr.show_sections(cout) ;

	mObjects_RT			scene_objects{&scene_descr} ;
	cl_seqLightsRT		scene_lights(&scene_descr) ;
	cl_seqTexturesRT	scene_textures(&scene_descr) ;
	// cout << endl << " - " << scene_textures ;
	camera_RT			camWS{get_sceneCAM(scene_descr)} ;

	mScene_RT		scene_rt{&scene_descr, &scene_objects, &scene_lights, &scene_textures} ;
	cout << "\n: camera in descriptor: " << camWS ;

	get_sceneWH(scene_descr, grWidth, grHeight) ;
	cout << "\n: resolution: " << grWidth << " x " << grHeight ;
	cout << endl << "\n--- <" << scene_descr.name() << "> ready: " << std::boolalpha << scene_descr.activate() ;

	scene_rt.set_concurrency(&scene_descr, grWidth, grHeight) ;
	cout << " --- using " << scene_rt.get_concurrency()._num_of_threads << " threads for "
		<< scene_rt.get_concurrency()._slices.size() << " slices" ;

	render_RT	test(grWidth, grHeight, camWS) ;	// the Rays: camera #0: camWS
	gridImage	gIm(grWidth, grHeight) ;			// image plane
	
	// test.clear_cameras() ;
	set_cameras(scene_rt, test) ;
	
	test.fix_cameras_list() ;
	unsigned int cNumb = test.rwd_cameras_list() ;	// start from the camera at WS
	for (unsigned int i = 0 ; i < cNumb ; i++, scn_moves(scene_rt, i)) {
		if (i > 11)   {
		render_slicesV3(std::move(clF_renderSlices<mScene_RT>
								  (std::move(&test),
								   std::move(&gIm), std::move(&scene_rt))),
						scene_rt.get_concurrency()) ;
		grid_to_ppm(gIm, ppm_fname(extract_fname(scene_name), i)) ;
		}
		
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