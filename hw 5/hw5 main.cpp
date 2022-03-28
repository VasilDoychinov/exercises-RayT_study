// hw5 main.cpp: 
//		Uses:
//			- [../tools/geometry/] vectors_RT.h
//			- [../tools/geometry/] triangles_RT.h
//			- [../tools/geometry/] grids_RT.h 
//			
//			- [../tools/threads/] progress counter.[cpp,h] 
// 
//			- [../tools/] color_RT.h
//			- [../tools/] miscellaneous.h
// 
//			- rays_RT.h 			
//			- scene_RT.h: 
//


#include "../tools/miscellaneous.h"
#include "../tools/color_RT.h"

#include "../tools/threads/progress counter.h"

#include "../tools/geometry/vectors_RT.h"
#include "../tools/geometry/triangles_RT.h"
#include "../tools/geometry/grids_RT.h"

/*
#include "miscellaneous.h"
#include "color_RT.h"

#include "progress counter.h"

#include "vectors_RT.h"
#include "triangles_RT.h"
#include "grids_RT.h"
*/

#include "rays_RT.h"
#include "scene_RT.h"


constexpr unsigned int grWidth = 1920 ;				// Resolution
constexpr unsigned int grHeight = 1080 ;			// ...
const char *           ppmF1_Name = "ppm\\crt_output_hw5_1.ppm" ;   // Output file names
const char *           ppmF2_Name = "ppm\\crt_output_hw5_2_3.ppm" ; // ...
const char *           ppmF4_Name = "ppm\\crt_output_hw5_4.ppm" ;   // ...


using VectF = vector_RT<RT_default_type> ;
using TriaF = triangle_RT<RT_default_type> ;


std::vector<slcRays> slices_for_threads(unsigned int w, unsigned int h) ;

int
main()
{
	cout << endl << "> id of main() is: " << std::this_thread::get_id() ;
	auto iSlices = slices_for_threads(grWidth, grHeight) ;

	cout << endl << endl ;
	try   {
		render_RT<grWidth, grHeight>	test ;		// the Rays
		gridImage<grWidth, grHeight>	gIm ;		// image
		slcRays							all(coord_RASTER(0, 0), grHeight, grWidth, grWidth) ;
		mColor	colors[] = {{255, 0, 255}, {0, 255, 0}, {255, 255, 0}, {0, 0, 255}, {0, 255, 255}} ;

											// for (const auto& slc : iSlices)   cout << endl << "--- " << slc ;
		cout << endl << "> grids initiated..." ;
		
		// throw std::range_error("------ partial completion ------") ;
		{	// HW lecture 5 Task 1
			cout <<endl<< "\n\n\n-------- Task 1 (resolution " << grWidth << "x" << grHeight 
				<< "): NO concurrent threads used (with 5 used takes: < 3 sec)" ;

			mAtom_RT	tri(TriaF{VectF{-1.75f, -1.75f, -3.f},
								  VectF{1.75f, -1.75f, -3.f},
								  VectF{0.f, 1.7f, -3.f}
							},
							mColor{255, 0, 0}
			) ;
			mObject_RT	obj("1st task", std::vector<mAtom_RT>{tri}) ;	
			/* for concurrent threads
			render_imSlices(std::move(xxxxxxxx<grWidth, grHeight, mObject_RT>
									  (std::move(&test),
									   std::move(&gIm), std::move(&obj), 0)),
							iSlices);
			*/

			test.render_image(0, all, gIm, obj) ;
			grid_to_ppm(gIm, ppmF1_Name) ;	// generate .ppm file
												// --- end of Task 1 - " ;
												// throw std::range_error("------ partial completion ------") ;
		}
		{	// HW lecture 5 Tasks 2 & 3
			cout <<endl<<"\n\n\n-------- Tasks 2 & 3 (resolution " << grWidth << "x" << grHeight 
				<< "):  not using concurrency takes: appr. 26 - 27 sec";

			mObject_RT	obj23("2nd and 3d tasks") ;
			TriaF   tri{VectF{-2.65f, -2.65f, -3.9f}, VectF{0.85f, -2.65f, -3.9f}, VectF{-0.9f, 0.8f, -3.9f}} ;

			VectF	v_move{0.3f, 0.3f, 0.3f} ;

			for (int i = 0, coef = 1, j = 0 ; i < _countof(colors) ; i++) {
				j = i * coef, coef = -coef ;
				obj23.add(std::move(mAtom_RT(tri + v_move * static_cast<float>(j), colors[i]))) ;
			}
			
											// without threads: test.render_image(0, all, gIm, obj23) ;
			show_idScene(obj23) ;
			render_imSlices(std::move(xxxxxxxx<grWidth, grHeight, mObject_RT>
									  (std::move(&test),
									   std::move(&gIm), std::move(&obj23), 0)),
							iSlices);
			grid_to_ppm(gIm, ppmF2_Name) ;	// generate .ppm file
												// --- end of Tasks 2 & 3 - ", show_lapse(false) ;
			
												// throw std::range_error("------ partial completion ------") ;
			// HW lecture 5 Task 4 (combined with 2&3)
			cout <<endl<< "\n\n\n-------- Task 4 (incl. 2&3 for testing purposes)"
				<< "(resolution " << grWidth << "x" << grHeight 
				<< "):  not using concurrency takes: appr. 47.3 sec";

			VectF	v_moveX{0.1f, 0.f,  0.f} ;
			VectF	v_moveY{0.f,  0.1f, 0.f} ;
			VectF	v_moveZ{0.f,  0.f,  0.1f} ;

			mObject_RT	obj4("4th task: pyramid triangular") ;

			VectF	V0{-4.f, 1.5f, -7.f} ;
			VectF	V1{4.f,  1.5f, -7.f} ;
			VectF	V2{0.f,  1.5f, -4.f} ;
			VectF	V3{0.2f, 7.f,  -7.f} ;

			TriaF   tri0{V0, V1, V2} ;
			TriaF   tri1{V0, V2, V3} ;
			TriaF   tri2{V2, V1, V3} ;
			TriaF   tri3{V0, V1, V3} ;

			obj4.add(std::move(mAtom_RT(tri0 - v_moveY * 3, colors[3]))) ;
			obj4.add(std::move(mAtom_RT(tri1 - v_moveY * 3, colors[1]))) ;
			obj4.add(std::move(mAtom_RT(tri2 - v_moveY * 3, colors[2]))) ;
			obj4.add(std::move(mAtom_RT(tri3 - v_moveY * 3, colors[4]))) ;

			mScene_RT	scene ;
			scene.add(obj23) ;
			scene.add(obj4) ;																		
											// without threads: test.render_image(0, all, gIm, scene) ;
			show_idScene(scene) ;
			render_imSlices(std::move(xxxxxxxx<grWidth, grHeight, mScene_RT>
									  (std::move(&test),
									   std::move(&gIm), std::move(&scene), 0)),
							iSlices);

			grid_to_ppm(gIm, ppmF4_Name) ;	// .ppm file
		}
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << "--- That's it ..." ;
	return(0) ;
} // main()


std::vector<slcRays> 
slices_for_threads(unsigned int w, unsigned int h)
{
	cout << endl << "> resolution is: " << w << " x " << h ;

	unsigned int nTh = std::thread::hardware_concurrency() ;

	std::vector<slcRays>	iSlices{} ;

	int		slc_num = (nTh < 4 ? 1 : nTh - 1) ;   // 'Max concurrent supported' - 1
	int		row_num{static_cast<int>(h / slc_num)} ;
		
	int row = 0 ; 
	for (int i = 0 ; i < slc_num - 1 ; i++, row += row_num) {	// store slices
		iSlices.push_back(slcRays(coord_RASTER(row, 0), row_num, grWidth, grWidth)) ;
	}
	// store the last one with the remaining rows
	iSlices.push_back(slcRays(coord_RASTER(row, 0), h - row, w, w)) ;
	std::cout << endl << "> " << nTh << " concurrent threads are supported; use: " << iSlices.size() ;

	iSlices.shrink_to_fit() ;
	return(iSlices) ;
} // slices_for_threads()

/*
std::chrono::nanoseconds xxxrender(slcRays slc) {
	auto t1 = std::chrono::steady_clock::now() ;
	cout << endl << endl << "- rendering image: " << slc;
	
	std::this_thread::sleep_for(std::chrono::seconds{5}) ;
	return(std::chrono::steady_clock::now() - t1) ;
}
*/
// eof hw5 main.cpp