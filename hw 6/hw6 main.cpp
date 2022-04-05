// hw5 main.cpp: 
//		Uses:
//			- [../tools/geometry/] vectors_RT.h
//			- [../tools/geometry/] triangles_RT.h
//			- [../tools/geometry/] grids_RT.h 
//			- [../tools/geometry/] matrices_RT.h 
// 
//			- [../tools/threads/] progress counter.[cpp,h] 
// 
//			- [../tools/] color_RT.h
//			- [../tools/] miscellaneous.h
// 
//			- rays_RT.h 			
//			- scene_RT.h: 
// 
//			- cameras.*:
//			- 
//


#include "../tools/miscellaneous.h"
#include "../tools/color_RT.h"

#include "../tools/threads/progress counter.h"

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/triangles_RT.h"
#include "../tools/geometry/grids_RT.h"
#include "../tools/geometry/matrices_RT.h"

/*
#include "miscellaneous.h"
#include "color_RT.h"

#include "progress counter.h"

#include "vectors_RT.h"
#include "triangles_RT.h"
#include "grids_RT.h"
#include "matrices_RT.h"
*/


#include "types_RT.h"
#include "rays_RT.h"
#include "scene_RT.h"

using VectF = vector_RT<RT_default_type> ;
using TriaF = triangle_RT<RT_default_type> ;
using MatrixF = mMatrix<RT_default_type> ;


constexpr unsigned int grWidth = 1920 ;				// Resolution
constexpr unsigned int grHeight = 1080 ;			// ...

const char *			ppm_Dir = "ppm/" ;
const char *			ppm_Form = "crt_output.ppm" ;
#include <string>
using std::string ;

string ppm_fname(string&& str, int cam_id, string batch_id={}) ; // .ppm file name as: ppm/task/task_camID.ppm

mObject_RT object_task_6_3() ;
mObject_RT object_task_6_4() ;

int
main()
{
	cout << endl << endl ;
	cout << "> id of main() is: " << std::this_thread::get_id() ;
	auto iSlices = slices_to_render(grWidth, grHeight, 150) ;
	std::cout << endl << "> " << std::thread::hardware_concurrency()
				<< " concurrent threads are supported;" ;
	cout << endl << "> " << iSlices.size() << " slices to work on" ;
										// for (const auto& slc : iSlices)   cout << endl << "--- " << slc ;
	cout << endl << "> resolution is: " << grWidth << " x " << grHeight ;
	cout << endl ;
	
	{ // Task 1:
		ray_RT		v{0, 0, -1} ;
		camera_RT	cam{pan(30.)} ;
		cout << endl << "- Task 1: transform vector" << v << " with camera pan(deg 30):" ;
		cout << endl << "--------- the transformed vector: " << (cam.camera_ray(v), v) ;
	} // end of Task 6_1

	try   {
												// throw std::range_error("------ partial completion ------") ;
		render_RT<grWidth, grHeight>	test ;		// the Rays: camera at (0,0,0)
		gridImage<grWidth, grHeight>	gIm ;		// use for an image
		slcRays							all(coord_RASTER(0, 0), grHeight, grWidth, grWidth) ;

		cout << endl << "--- grids set..." ;
												// cout << endl << test ; 
												// throw std::range_error("------ partial completion ------") ;
		{	// Task 6_2
			cout <<endl<< "\n\n\n-------- Task 6_2 (resolution " << grWidth << "x" << grHeight 
				<< "): NO concurrent threads used" ;

			mAtom_RT	tri(TriaF{VectF{-1.75f, -1.75f, -3.f},
								  VectF{1.75f, -1.75f, -3.f},
								  VectF{0.f, 1.75f, -3.f}
							},
							mColor{255, 0, 0}
			) ;
			mObject_RT	obj("task 6_2", std::vector<mAtom_RT>{tri}) ;	
																		// cout << endl ; show_idScene(obj) ;
			test.add_camera(dolly(50, truck(-50, boom(-30)))) ; 
			test.fix_cameras_list() ;			// release the unnecessary memory, actually
			// show_cameras(cout, test) << endl ;
			
			unsigned int cNumb = test.rwd_cameras_list() ;	// start from the camera at WS
			for (unsigned int i = 0 ; i < cNumb ; i++) {
				test.render_image(all, gIm, obj), grid_to_ppm(gIm, ppm_fname("task 6_2", i)) ;
				test.next_camera() ;
			}						
											// throw std::range_error("------ partial completion ------") ;
		} // end - task 6_2

		{	// Task 6_3
			cout << endl << "\n\n\n-------- Tasks 6_3 (resolution " << grWidth << "x" << grHeight << ")" ;
			cout << endl << "         (the frame for camera #0 shows the initial for all the others)" ;
			cout << endl << "         (...           camera #1 :: dolly(3) -> roll(90)" ;
			cout << endl << "         (...           camera #2 :: dolly(1) -> truck(2)" ;
			cout << endl << "         (...           camera #3 :: dolly(2) -> tilt(15)" ;
			cout << endl << "         (...           camera #4 :: dolly(1) -> boom(1)" ;
			cout << endl << "         (...           camera #5 :: dolly(2) -> pan(15)" ;
			cout << endl << "         (...           camera #6 :: dolly(5)" ;

			mObject_RT	obj(object_task_6_3()) ;
																		// cout << endl ; show_idScene(obj) ;
			test.clear_cameras() ;
			test.add_camera(roll(90, dolly(3))) ;
			test.add_camera(truck(2, dolly(1))) ;
			test.add_camera(tilt(15, dolly(2))) ;
			test.add_camera(boom(1, dolly(1))) ;
			test.add_camera(pan(15, dolly(2))) ;
			test.add_camera(dolly(5)) ;
			
			test.fix_cameras_list() ;			
																		// show_cameras(cout, test) << endl ;
			unsigned int cNumb = test.rwd_cameras_list() ;	// start from the camera at WS
			iSlices = slices_for_threads(grWidth, grHeight) ;
			for (unsigned int i = 0 ; i < cNumb ; i++) {
				render_imSlices(std::move(clF_RenderSlice<grWidth, grHeight, mObject_RT>
											  (std::move(&test),
											   std::move(&gIm), std::move(&obj))),
									iSlices);
															// no concurrency: test.render_image(all, gIm, obj) 
				grid_to_ppm(gIm, ppm_fname("task 6_3", i)) ;

				test.next_camera() ;
			}
		}	// --- end of Task 6_3 - " ;
											// throw std::range_error("------ partial completion ------") ;
		{	// Task 6_4
			cout << endl << "\n\n\n-------- Tasks 6_4 (resolution " << grWidth << "x" << grHeight << ")" ;

			mObject_RT	obj(object_task_6_4()) ;
																		// cout << endl ; show_idScene(obj) ;
			test.clear_cameras() ;
			test.add_camera(tilt(-0.5f, boom(-1., dolly(-1., roll(3.65f, roll(-5., boom(1.5,
										pan(-32.f, truck(-0.9f, dolly(-1.f,
										boom(1.f, truck(-3, dolly(2))))))))))))
			) ;
			
			test.fix_cameras_list() ;
																		// show_cameras(cout, test) << endl ;
			unsigned int cNumb = test.rwd_cameras_list() ;	// start from the camera at WS
			iSlices = slices_for_threads(grWidth, grHeight) ;
			for (unsigned int i = 0 ; i < cNumb ; i++) {
				render_imSlices(std::move(clF_RenderSlice<grWidth, grHeight, mObject_RT>
										  (std::move(&test),
										   std::move(&gIm), std::move(&obj))),
										iSlices);
				grid_to_ppm(gIm, ppm_fname("task 6_4", i)) ;
				test.next_camera() ;
			}
		}	// --- end of Task 6_4 - " ;
											// throw std::range_error("------ partial completion ------") ;
		{	// Task 6_5
			cout << endl << "\n\n\n-------- Tasks 6_5 (resolution " << grWidth << "x" << grHeight << ")" ;
			cout << endl <<       "         (presents 5 batches of 'movements' of the object from 6_4" ;

			mObject_RT	obj(object_task_6_4()) ;			// cout << endl ; show_idScene(obj) ;
			std::initializer_list<std::pair<std::pair<int, int>, string>>	b_ilist{
				{{0,9}, "0"}, {{10,18},"1"}, {{19,37}, "2"}, {{38,42}, "3"}, {{43,48}, "4"}
			};
			batchesCameras batches {b_ilist} ;

			test.clear_cameras() ;

			camera_RT		c_cam{} ;
			c_cam = test.add_Cameras(roll, (30.f), 9) ;		// batch 0: 0 - 9
			test.add_Cameras(dolly, (1.5f), 9, c_cam) ;		// batch 1: 10 - 18
			
															// batch 2: 19 - 37
															//  all direction reverse and,
															//  pan, for example, acts like roll()at WS
			test.add_camera(c_cam = tilt(90.f, camera_to_position(ray_RT{0.f, 5.25f, -3.5f}))) ;
															
			c_cam = test.add_Cameras(pan, 10.f, 9, c_cam) ;	
			c_cam = test.add_Cameras(dolly, 1.f, 9, c_cam) ; 
															// batch 3: 38 - 42 
			c_cam = camera_to_position(ray_RT{-1.75f, -1.75f, 0.f}) ;
			test.add_camera(c_cam) ;
			test.add_camera(c_cam = boom(0.35f, dolly(-0.6f, c_cam))) ;
			c_cam = test.add_Cameras(boom, 0.95f, 2, c_cam) ; 
			test.add_camera(c_cam = pan(20.f, c_cam)) ;
															// batch 4: 43 - 
			c_cam = camera_to_position(ray_RT{-4.f, 0.75f, -3.f}) ;
			test.add_camera(c_cam = pan(-120.f, c_cam)) ;
			c_cam = test.add_Cameras(pan, 10, 5, c_cam) ;

			test.fix_cameras_list() ;									// show_cameras(cout, test) << endl ;
			unsigned int cNumb = test.rwd_cameras_list() ;	// rewind to WS, number of cameras in cNumb ;
					
												// throw std::range_error("------ partial completion ------") ;
			iSlices = slices_to_render(grWidth, grHeight, 150) ;
			for (unsigned int i = 0 ; i < cNumb ; i++) {
				if (i >= batches.start(0) && i <= batches.end(4)) {
					render_imSlcV2(std::move(clF_RenderSlice<grWidth, grHeight, mObject_RT>
											  (std::move(&test),
											   std::move(&gIm), std::move(&obj))),
									iSlices);
					grid_to_ppm(gIm, ppm_fname("task 6_5", i, batches[i])) ;
				}
				test.next_camera() ;
			}
		}
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << "--- That's it ..." ;
	return(0) ;
} // main()

mObject_RT
object_task_6_3()
{
	mColor	colors[] = {{255, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 255, 255}, {233, 233, 233}} ;

	TriaF tri{VectF{-1.75f, -1.75f, -3.f}, VectF{1.75f, -1.75f, -3.f}, VectF{0.f, 1.75f, -3.f}};

	VectF	v_moveX{1.f, 0.f,  0.f} ; 
	VectF	v_moveY{0.f,  1.f, 0.f} ;
	VectF	v_moveZ{0.f,  0.f,  1.f} ;

	mObject_RT	obj4("task 6_3") ;

	obj4.add(std::move(mAtom_RT(tri - v_moveZ, mColor{255, 0, 0}))) ;
	obj4.add(std::move(mAtom_RT(tri - v_moveX * 3 - v_moveY * 2 + v_moveZ * 0.2f, colors[1]))) ;
	obj4.add(std::move(mAtom_RT(tri - v_moveX * 5 + v_moveY * 3 - v_moveZ * 3.f, colors[3]))) ;
	obj4.add(std::move(mAtom_RT(tri + v_moveX * 4 - v_moveY * 2 - v_moveZ,       colors[2]))) ;
	obj4.add(std::move(mAtom_RT(tri + v_moveX * 7 + v_moveY * 4 - v_moveZ * 3.f, colors[4]))) ;

	return(obj4) ;
}

mObject_RT
object_task_6_4()
{
	mColor	colors[] = {{255, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 255, 255}, {233, 233, 233}} ;

	TriaF triXY {VectF{-1.75f, -1.75f, -3.5f}, VectF{1.75f, -1.75f, -3.5f}, VectF{0.f, 1.75f, -3.5f}};
	TriaF triYZ {VectF{0.f, -1.75f, -1.75f}, VectF{0.f, -1.75f, -6.f}, VectF{0.f, 1.75f, -3.5f}};
	TriaF triXZ1{VectF{0.f, 0.f, -1.75f}, VectF{0.f, 0.f, -5.25f}, VectF{-1.75f, 0.f, -3.5f}};
	TriaF triXZ2{VectF{0.f, 0.f, -1.75f}, VectF{1.75f, 0.f, -3.5f}, VectF{0.f, 0.f, -5.25f}};

	mObject_RT	obj4("task 6_4") ;

	obj4.add(std::move(mAtom_RT(triXY, colors[1]))) ;
	obj4.add(std::move(mAtom_RT(triYZ, colors[0]))) ;
	obj4.add(std::move(mAtom_RT(triXZ1, colors[3]))) ;
	obj4.add(std::move(mAtom_RT(triXZ2, colors[2]))) ;

	return(obj4) ;
}

#include <sstream>
#include <iomanip>
string ppm_fname(string&& str, int cam_id, string batch_id)		// to generate a .ppm file name in sub-folder ppm_Dir
{
	string	camera_id{} ;

	if (cam_id >= 0) {
		std::stringstream sss ; sss << std::setw(2) << std::setfill('0') << std::to_string(cam_id) ;

		camera_id = "_(" + batch_id + "_camera #" + sss.str() + ")" ;
	}
	return(ppm_Dir + str +'/'+ str + camera_id + ppm_Form) ;
} // ppm_fname()

// eof hw5 main.cpp