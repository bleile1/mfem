// Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-443211. All Rights
// reserved. See file COPYRIGHT for details.
//
// This file is part of the MFEM library. For more information and source code
// availability see http://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License (as published by the Free
// Software Foundation) version 2.1 dated February 1999.

//This file contains useful functions to compute fluxes for DG methods.

#include <vector>
#include "fem.hpp"

using std::vector;
using std::pair;

namespace mfem
{

/**
*	Returns the canonical coordinate vectors e_1 and e_2.
*/
void getBaseVector2D(Vector& e1, Vector& e2)
{
	e1.SetSize(2);
	e1(0) = 1;
	e1(1) = 0;
	e2.SetSize(2);
	e2(0) = 0;
	e2(1) = 1;
}

/**
*	Returns the canonical coordinate vectors e_1, e_2 and e_3.
*/
void getBaseVector3D(Vector& e1, Vector& e2, Vector& e3)
{
	e1.SetSize(2);
	e1(0) = 1;
	e1(1) = 0;
	e1(2) = 0;
	e2.SetSize(2);
	e2(0) = 0;
	e2(1) = 1;
	e2(2) = 0;
	e3.SetSize(2);
	e3(0) = 0;
	e3(1) = 0;
	e3(2) = 1;
}

/** A function that initialize the local coordinate base for a face with
*   indice face_ind.
*   This returns the local face coordinate base expressed in reference
*   element coordinate.
*/
// Highly dependent of the node ordering from geom.cpp
void InitFaceCoord2D(const int face_id, IntMatrix& base)
{
	//Vector e1,e2;
	//getBaseVector2D(e1,e2);
	base.Zero();
	switch(face_id)
	{
		case 0://SOUTH
			base(0,0)= 1;//base.SetCol(0, e1);
			base(1,1)=-1;//base.SetCol(1,-e2);
			break;
		case 1://EAST
			base(1,0)= 1;//base.SetCol(0, e2);
			base(0,1)= 1;//base.SetCol(1, e1);
			break;
		case 2://NORTH
			base(0,0)=-1;//base.SetCol(0,-e1);
			base(1,1)= 1;//base.SetCol(1, e2);
			break;
		case 3://WEST
			base(1,0)=-1;//base.SetCol(0,-e2);
			base(0,1)= 1;//base.SetCol(1, e1);
			break;
		default:
			mfem_error("The face_ind exceeds the number of faces in this dimension.");
			break;
	}
}

// Highly dependent of the node ordering from geom.cpp
void InitFaceCoord3D(const int face_id, IntMatrix& base)
{
	//Vector e1,e2,e3;
	//getBaseVector3D(e1,e2,e3);
	base.Zero();
	switch(face_id)
	{
		case 0://BOTTOM
			base(0,0)= 1;//base.SetCol(0, e1);
			base(1,1)=-1;//base.SetCol(1,-e2);
			base(2,2)=-1;//base.SetCol(2,-e3);
			break;		
		case 1://SOUTH
			base(0,0)= 1;//base.SetCol(0, e1);
			base(2,1)= 1;//base.SetCol(1, e3);
			base(1,2)=-1;//base.SetCol(2,-e2);
			break;
		case 2://EAST
			base(1,0)= 1;//base.SetCol(0, e2);
			base(2,1)= 1;//base.SetCol(1, e3);
			base(0,2)= 1;//base.SetCol(2, e1);
			break;
		case 3://NORTH
			base(0,0)=-1;//base.SetCol(0,-e1);
			base(2,1)= 1;//base.SetCol(1, e3);
			base(1,2)= 1;//base.SetCol(2, e2);
			break;
		case 4://WEST
			base(1,0)=-1;//base.SetCol(0,-e2);
			base(2,1)= 1;//base.SetCol(1, e3);
			base(0,2)=-1;//base.SetCol(2,-e1);
			break;
		case 5://TOP
			base(0,0)= 1;//base.SetCol(0, e1);
			base(1,1)= 1;//base.SetCol(1, e2);
			base(2,2)= 1;//base.SetCol(2, e3);
			break;
		default:
			mfem_error("The face_ind exceeds the number of faces in this dimension.");
			break;
	}
}

/** Maps the coordinate vectors of the first face to the coordinate vectors of the second face.
*   nb_rot is the number of rotation to opperate so that the first node of each face match.
*   The result map contains pairs of int, where the first int is the cofficient, and the 
*   second int is the indice of the second face vector.
*/
// There shouldn't be any rotation in 2D.
void GetLocalCoordMap2D(vector<pair<int,int> >& map, const int nb_rot)
{
	map.resize(2);
	//First and second coordinate vectors should always be of opposite direction in 2D.
	map[0] = pair<int,int>(-1,1);
	map[1] = pair<int,int>(-1,2);
}

// Default parameter nb_rot=0 should be only use with a structured mesh.
// Rotations follow the ordering of the nodes.
void GetLocalCoordMap3D(vector<pair<int,int> >& map, const int nb_rot)
{
	map.resize(3);
	// Normal to the face are always of opposite direction
	map[2] = pair<int,int>(-1,3);
	// nb_rot determines how local coordinates are oriented from one face to the other.
	// See case 2 for an example.
	switch(nb_rot)
	{
		case 0:
			map[0] = pair<int,int>( 1,1);
			map[1] = pair<int,int>( 1,0);
			break;
		case 1:
			map[0] = pair<int,int>(-1,0);
			map[1] = pair<int,int>( 1,1);
			break;
		case 2:
			//first vector equals -1 times the second vector of the other face coordinates
			map[0] = pair<int,int>(-1,1);
			//second vector equals -1 times the first vector of the other face coordinates
			map[1] = pair<int,int>(-1,0);
			break;
		case 3:
			map[0] = pair<int,int>( 1,0);
			map[1] = pair<int,int>(-1,1);
			break;
		default:
			mfem_error("There shouldn't be that many rotations.");
			break;
	}
}

/**
*	Returns the change of matrix P from base_K2 to base_K1 according to the mapping map.
*/
void GetChangeOfBasis(const IntMatrix& base_K1, IntMatrix& base_K2,
								const vector<pair<int,int> >& map, IntMatrix& P)
{
	int dim = map.size();
	for (int j = 0; j < dim; j++)
	{
		int i = 0;
		//we look if the vector is colinear with e_j
		// Can be replaced by base_K2(j,i)!=0
		while (base_K2(j,i)!=0) i++;
		int coeff = map[i].first;
		int ind = map[i].second;
		for (int k = 0; k < dim; ++k)
		{
			P(k,j) = coeff * base_K1(k,ind);
		}
	}
}

/**
*	Returns the face_id that identifies the face on the reference element, and nb_rot the
*  "rotations" the face did between reference to physical spaces.
*/
void GetIdRotInfo(const int face_info, int& face_id, int& nb_rot){
	int orientation = face_info % 64;
	face_id = face_info / 64;
	// Test if ny understanding of mfem code is correct, error if not
	MFEM_ASSERT(orientation % 2 == 0, "Unexpected inside out face");
	nb_rot = orientation / 2;
}

}