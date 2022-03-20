// hw4_main.cpp:  
//		
//		- tools used are in:
//			- [../tools/]coords_RT.h
//			- [../tools/]vectors_RT.h: for the representation of vectors
//			- [../tools/]triangles_RT.h: ...					   triangles
//

#include <iostream>
using std::cout ;
using std::endl ;

#include <exception>
#include <time.h>
#include <math.h>

#include <functional>

#include "../tools/vectors_RT.h"			// in GitHub
#include "../tools/triangles_RT.h"			// ...
// OR: #include "vectors_RT.h"				// otherwise
//     #include "triangles_RT.h"			// ...

using VectF = vector_RT<double> ;
using TriaF = triangle_RT<double> ;

VectF task_1_1(VectF A, VectF B) ;  // returns ( A X B )

int
main()
{
	cout << "> homework lecture 4 ...";

	try {
		
		cout << endl << endl << "- TASK 1: see [../tools/]triangles.h" ;
		{	cout << endl << "- TASK 2.1: calculate Cross Product of vectors:" << endl ;
			/*
			 *	Изчислете векторното произведение(AxB) между двата вектора А = (3.5, 0, 0) и B = (1.75, 3.5, 0)
			 *	Изчислете векторното произведение(AxB) между двата вектора А = (3, -3, 1) и B = (4, 9, 3)
			*/
			task_1_1(VectF{3.5, 0., 0.}, VectF{1.75, 3.5, 0}) ;

			VectF v(task_1_1(VectF{3, -3, 1}, VectF{4, 9, 3})) ;

			/*
			 *	Изчислете лицето на успоредника, който се формира с векторите А = (3, -3, 1) и B = (4, 9, 3)
			 *	Изчислете лицето на успоредника, който се формира с векторите А = (3, -3, 1) и B = (-12, 12, -4)
			*/
			cout << endl << endl << "- TASK 2.2: calculate area of paralellograms:" ;

			cout << endl << "--- CHECK: area of the paralellogram defined by A(10, 0, 0) & B(0, 10, 0) is: "
				<< length(crossRHS(VectF{10, 0, 0}, VectF{0, 10, 0})) ;

			cout << endl << endl << "--- area of the paralellogram defined by A(3, -3, 1) & B(4, 9, 3) is: " << v.length() ;

			cout << endl << "--- area of the paralellogram defined by A(3, -3, 1) & B(-12, 12, -4) is: " 
				<< length(crossRHS(VectF{3, -3, 1}, VectF{-12, 12, -4})) ;
		}
		{	cout << endl << endl << endl << "- TASK 3(and task 1): triangles:" << endl ;
			/*
			 * Намерете нормал вектор за триъгълник със следните върхове :
								- A = (-1.75, -1.75, -3)
								- B = (1.75, -1.75, -3)
								- C = (0, 1.75, -3)
			*/
			cout << endl << TriaF{VectF{-1.75, -1.75, -3}, VectF{1.75, -1.75, -3}, VectF{0, 1.75, -3}} ;
			/* 
			 * Намерете нормал вектор за триъгълник със следните върхове :
								- A = (0, 0, -1)
								- B = (1, 0, 1)
								- C = (-1, 0, 1)
			*/
			cout << endl << TriaF{VectF{0, 0, -1}, VectF{1, 0, 1}, VectF{-1, 0, 1}} ;
			/*
			 * Намерете нормал вектор за триъгълник със следните върхове :
								 - A = (0.56, 1.11, 1.23)
								 - B = (0.44, -2.368, -0.54)
								 - C = (-1.56, 0.15, -1.92)
			*/
			cout << endl << TriaF{VectF{0.56f, 1.11f, 1.23f}, 
								  VectF{0.44f, -2.368f, -0.54f}, 
								  VectF{-1.56f, 0.15f, -1.92f}} ;

			/* Изчислете лицата на триъгълниците: already shown by << */
		}
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	}

	cout << endl << endl << endl << "--- That's it ..." ;
	return(0) ;
} // main()


VectF
task_1_1(VectF	A, VectF B)
{
	VectF   v(crossRHS(A, B)) ;
	
	cout << endl << "--- for A" << A << " X  B" << B << " the result is VC" << v ;
	cout << endl << "----   checks -> A . VC = " << dotPR(A, v) << "; B . VC = " << dotPR(B, v) ;
	return(v) ;
}

// eof hw4 main.cpp