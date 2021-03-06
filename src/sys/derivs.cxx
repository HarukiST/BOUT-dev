/**************************************************************************
 * Basic derivative methods
 *
 *
 * Four kinds of differencing methods:
 *
 * 1. First derivative DD*
 *    Central differencing e.g. Div(f)
 *
 * 2. Second derivatives D2D*2
 *    Central differencing e.g. Delp2(f)
 *
 * 3. Upwinding VDD*
 *    Terms like v*Grad(f)
 *
 * 4. Flux methods FDD* (e.g. flux conserving, limiting)
 *    Div(v*f)
 *
 **************************************************************************
 * Copyright 2010 B.D.Dudson, S.Farley, M.V.Umansky, X.Q.Xu
 *
 * Contact: Ben Dudson, bd512@york.ac.uk
 *
 * This file is part of BOUT++.
 *
 * BOUT++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BOUT++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOUT++.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************/

#include <globals.hxx>
#include <derivs.hxx>
#include <stencils.hxx>
#include <utils.hxx>
#include <fft.hxx>
#include <interpolation.hxx>
#include <bout/constants.hxx>
#include <msg_stack.hxx>

#include <cmath>
#include <string.h>
#include <stdlib.h>

#include <output.hxx>

/*******************************************************************************
 * First central derivatives
 *******************************************************************************/

////////////// X DERIVATIVE /////////////////

const Field3D DDX(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method) {
  Field3D result =  mesh->indexDDX(f,outloc, method) / mesh->coordinates()->dx;
  
  if(mesh->IncIntShear) {
    // Using BOUT-06 style shifting
    result += mesh->coordinates()->IntShiftTorsion * DDZ(f, outloc);
  }
  
  return result;
}

const Field3D DDX(const Field3D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return DDX(f, outloc, method);
}

const Field3D DDX(const Field3D &f, DIFF_METHOD method) {
  return DDX(f, CELL_DEFAULT, method);
}

const Field2D DDX(const Field2D &f) {
  return mesh->coordinates()->DDX(f);
}

////////////// Y DERIVATIVE /////////////////

const Field3D DDY(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexDDY(f,outloc, method) / mesh->coordinates()->dy;
}

const Field3D DDY(const Field3D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return DDY(f, outloc, method);
}

const Field3D DDY(const Field3D &f, DIFF_METHOD method) {
  return DDY(f, CELL_DEFAULT, method);
}

const Field2D DDY(const Field2D &f) {
  return mesh->coordinates()->DDY(f);
}

////////////// Z DERIVATIVE /////////////////

const Field3D DDZ(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method, bool inc_xbndry) {
  return mesh->indexDDZ(f,outloc, method, inc_xbndry) / mesh->coordinates()->dz;
}

const Field3D DDZ(const Field3D &f, DIFF_METHOD method, CELL_LOC outloc, bool inc_xbndry) {
  return DDZ(f, outloc, method, inc_xbndry);
}

const Field3D DDZ(const Field3D &f, DIFF_METHOD method, bool inc_xbndry) {
  return DDZ(f, CELL_DEFAULT, method, inc_xbndry);
}

const Field3D DDZ(const Field3D &f, bool inc_xbndry) {
  return DDZ(f, CELL_DEFAULT, DIFF_DEFAULT, inc_xbndry);
}

const Field2D DDZ(const Field2D &UNUSED(f)) {
  return Field2D(0.0);
}

const Vector3D DDZ(const Vector3D &v, CELL_LOC outloc, DIFF_METHOD method) {
  Vector3D result;

  Coordinates *metric = mesh->coordinates();

  if(v.covariant){
    // From equation (2.6.32) in D'Haeseleer
    result.x = DDZ(v.x, outloc, method) - v.x*metric->G1_13 - v.y*metric->G2_13 - v.z*metric->G3_13;
    result.y = DDZ(v.y, outloc, method) - v.x*metric->G1_23 - v.y*metric->G2_23 - v.z*metric->G3_23;
    result.z = DDZ(v.z, outloc, method) - v.x*metric->G1_33 - v.y*metric->G2_33 - v.z*metric->G3_33;
    result.covariant = true;
  }
  else{
    // From equation (2.6.31) in D'Haeseleer
    result.x = DDZ(v.x, outloc, method) + v.x*metric->G1_13 + v.y*metric->G1_23 + v.z*metric->G1_33;
    result.y = DDZ(v.y, outloc, method) + v.x*metric->G2_13 + v.y*metric->G2_23 + v.z*metric->G2_33;
    result.z = DDZ(v.z, outloc, method) + v.x*metric->G3_13 + v.y*metric->G3_23 + v.z*metric->G3_33;
    result.covariant = false;
  }

  return result;
}

const Vector3D DDZ(const Vector3D &v, DIFF_METHOD method, CELL_LOC outloc) {
  return DDZ(v, outloc, method);
}

const Vector2D DDZ(const Vector2D &v) {
  Vector2D result;

  result.covariant = v.covariant;

  // Vector 2D is constant in the z direction
  // Gx_y3 contains z-derivatives (where G is the Christoffel symbol of the
  // second kind, and x and y in {1, 2, 3})
  result.x = 0.;
  result.y = 0.;
  result.z = 0.;

  return result;
}

/*******************************************************************************
 * 2nd derivative
 *******************************************************************************/

////////////// X DERIVATIVE /////////////////

const Field3D D2DX2(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method) {
  
  Field3D result = mesh->indexD2DX2(f, outloc, method) / SQ(mesh->coordinates()->dx);
  
  if(mesh->coordinates()->non_uniform) {
    // Correction for non-uniform mesh
    result += mesh->coordinates()->d1_dx * mesh->indexDDX(f, outloc, DIFF_DEFAULT)/mesh->coordinates()->dx;
  }

  return result;
}

const Field3D D2DX2(const Field3D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return D2DX2(f, outloc, method);
}

const Field2D D2DX2(const Field2D &f) {
  Field2D result = mesh->indexD2DX2(f) / SQ(mesh->coordinates()->dx);
  
  if(mesh->coordinates()->non_uniform) {
    // Correction for non-uniform mesh
    result += mesh->coordinates()->d1_dx * mesh->indexDDX(f) / mesh->coordinates()->dx;
  }
  
  return result;
}

////////////// Y DERIVATIVE /////////////////

const Field3D D2DY2(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method) {
  
  Field3D result = mesh->indexD2DY2(f, outloc, method) / SQ(mesh->coordinates()->dy);

  if(mesh->coordinates()->non_uniform) {
    // Correction for non-uniform mesh
    result += mesh->coordinates()->d1_dy * mesh->indexDDY(f, outloc, DIFF_DEFAULT) / mesh->coordinates()->dy;
  }

  return interp_to(result, outloc);
}

const Field3D D2DY2(const Field3D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return D2DY2(f, outloc, method);
}

const Field2D D2DY2(const Field2D &f) {
  Field2D result = mesh->indexD2DY2(f) / SQ(mesh->coordinates()->dy);
  if(mesh->coordinates()->non_uniform) {
    // Correction for non-uniform mesh
    result += mesh->coordinates()->d1_dy * mesh->indexDDY(f) / mesh->coordinates()->dy;
  }
  
  return result;
}

////////////// Z DERIVATIVE /////////////////

const Field3D D2DZ2(const Field3D &f, CELL_LOC outloc, DIFF_METHOD method, bool inc_xbndry) {
  return mesh->indexD2DZ2(f, outloc, method, inc_xbndry) / SQ(mesh->coordinates()->dz);
}

const Field3D D2DZ2(const Field3D &f, DIFF_METHOD method, CELL_LOC outloc, bool inc_xbndry) {
  return D2DZ2(f, outloc, method, inc_xbndry);
}

const Field3D D2DZ2(const Field3D &f, bool inc_xbndry) {
  return D2DZ2(f, CELL_DEFAULT, DIFF_DEFAULT, inc_xbndry);
}

const Field2D D2DZ2(const Field2D &UNUSED(f)) {
  return Field2D(0.0);
}

/*******************************************************************************
 * Fourth derivatives
 *******************************************************************************/

const Field3D D4DX4(const Field3D &f) {
  return mesh->indexD4DX4(f) / SQ(SQ(mesh->coordinates()->dx));
}

const Field2D D4DX4(const Field2D &f) {
  return mesh->indexD4DX4(f) / SQ(SQ(mesh->coordinates()->dx));
}

const Field3D D4DY4(const Field3D &f) {
  return mesh->indexD4DY4(f) / SQ(SQ(mesh->coordinates()->dy));
}

const Field2D D4DY4(const Field2D &f) {
  return mesh->indexD4DY4(f) / SQ(SQ(mesh->coordinates()->dy));
}

const Field3D D4DZ4(const Field3D &f) {
  return mesh->indexD4DZ4(f) / SQ(SQ(mesh->coordinates()->dz));
}

const Field2D D4DZ4(const Field2D &f) {
  CELL_LOC loc = f.getLocation() ;
  Field2D result = Field2D(0.0);
  result.setLocation(loc) ;
  return result ;
}

/*******************************************************************************
 * Mixed derivatives
 *******************************************************************************/

/*!
 * Mixed derivative in X and Y
 *
 * This first takes derivatives in X, then in Y.
 *
 * ** Applies Neumann boundary in Y, communicates
 */
const Field2D D2DXDY(const Field2D &f) {
  Field2D dfdy = DDY(f);
  mesh->communicate(dfdy);
  return DDX(dfdy);
}

/*!
 * Mixed derivative in X and Y
 *
 * This first takes derivatives in X, then in Y.
 *
 * ** Applies Neumann boundary in Y, communicates
 */
const Field3D D2DXDY(const Field3D &f) {
  Field3D dfdy = DDY(f);
  mesh->communicate(dfdy);
  return DDX(dfdy);
}

const Field2D D2DXDZ(const Field2D &UNUSED(f)) {
  return Field2D(0.0);
}

/// X-Z mixed derivative
const Field3D D2DXDZ(const Field3D &f) {
  Field3D result;

  // Take derivative in Z, including in X boundaries. Then take derivative in X
  // Maybe should average results of DDX(DDZ) and DDZ(DDX)?
  result = DDX(DDZ(f, true));

  return result;
}

const Field2D D2DYDZ(const Field2D &UNUSED(f)) {
  return Field2D(0.0);
}

const Field3D D2DYDZ(const Field3D &f) {
  Field3D result;
  result.allocate();
  for(int i=mesh->xstart;i<=mesh->xend;i++)
    for(int j=mesh->ystart;j<=mesh->yend;j++) 
      for(int k=0;k<mesh->LocalNz;k++) {
        int kp = (k+1) % (mesh->LocalNz);
        int km = (k-1+mesh->LocalNz) % (mesh->LocalNz);
        result(i,j,k) = 0.25*( +(f(i,j+1,kp) - f(i,j-1,kp))/(mesh->coordinates()->dy(i,j+1))
                               -(f(i,j+1,km) - f(i,j-1,km))/(mesh->coordinates()->dy(i,j-1)) )
          / mesh->coordinates()->dz;
      }
  return result;
}

/*******************************************************************************
 * Advection schemes
 *
 * Jan 2009  - Re-written to use Set*Stencil routines
 *******************************************************************************/

////////////// X DERIVATIVE /////////////////

/// Special case where both arguments are 2D. Output location ignored for now
const Field2D VDDX(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexVDDX(v, f, outloc, method) / mesh->coordinates()->dx;
}

const Field2D VDDX(const Field2D &v, const Field2D &f, DIFF_METHOD method) {
  return VDDX(v, f, CELL_DEFAULT, method);
}

/// General version for 2 or 3-D objects
const Field3D VDDX(const Field &v, const Field &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexVDDX(v, f, outloc, method) / mesh->coordinates()->dx;
}

const Field3D VDDX(const Field &v, const Field &f, DIFF_METHOD method, CELL_LOC outloc) {
  return VDDX(v, f, outloc, method);
}

////////////// Y DERIVATIVE /////////////////

// special case where both are 2D
const Field2D VDDY(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexVDDY(v, f, outloc, method) / mesh->coordinates()->dy;
}

const Field2D VDDY(const Field2D &v, const Field2D &f, DIFF_METHOD method) {
  return VDDY(v, f, CELL_DEFAULT, method);
}

// general case
const Field3D VDDY(const Field &v, const Field &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexVDDY(v, f, outloc, method) / mesh->coordinates()->dy;
}

const Field3D VDDY(const Field &v, const Field &f, DIFF_METHOD method, CELL_LOC outloc) {
  return VDDY(v, f, outloc, method);
}

////////////// Z DERIVATIVE /////////////////

// special case where both are 2D
const Field2D VDDZ(const Field2D &UNUSED(v), const Field2D &UNUSED(f)) {
  return Field2D(0.0);
}

// Note that this is zero because no compression is included
const Field2D VDDZ(const Field3D &UNUSED(v), const Field2D &UNUSED(f)) {
  return Field2D(0.0);
}

// general case
const Field3D VDDZ(const Field &v, const Field &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexVDDZ(v, f, outloc, method) / mesh->coordinates()->dz;
}

const Field3D VDDZ(const Field &v, const Field &f, DIFF_METHOD method, CELL_LOC outloc) {
  return VDDZ(v, f, outloc, method);
}

/*******************************************************************************
 * Flux conserving schemes
 *******************************************************************************/

const Field2D FDDX(const Field2D &v, const Field2D &f) {
  return FDDX(v, f, DIFF_DEFAULT, CELL_DEFAULT);
}

const Field2D FDDX(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexFDDX(v, f, outloc, method) / mesh->coordinates()->dx;
}

const Field2D FDDX(const Field2D &v, const Field2D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return FDDX(v, f, outloc, method);
}

const Field3D FDDX(const Field3D &v, const Field3D &f) {
  return FDDX(v, f, DIFF_DEFAULT, CELL_DEFAULT);
}

const Field3D FDDX(const Field3D &v, const Field3D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexFDDX(v, f, outloc, method) / mesh->coordinates()->dx;
}

const Field3D FDDX(const Field3D &v, const Field3D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return FDDX(v, f, outloc, method);
}

/////////////////////////////////////////////////////////////////////////

const Field2D FDDY(const Field2D &v, const Field2D &f) {
  return FDDY(v, f, DIFF_DEFAULT, CELL_DEFAULT);
}

const Field2D FDDY(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexFDDY(v, f, outloc, method) / mesh->coordinates()->dy;
}

const Field2D FDDY(const Field2D &v, const Field2D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return FDDY(v, f, outloc, method);
}

const Field3D FDDY(const Field3D &v, const Field3D &f) {
  return FDDY(v, f, DIFF_DEFAULT, CELL_DEFAULT);
}

const Field3D FDDY(const Field3D &v, const Field3D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexFDDY(v, f, outloc, method) / mesh->coordinates()->dy;
}

const Field3D FDDY(const Field3D &v, const Field3D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return FDDY(v, f, outloc, method);
}

/////////////////////////////////////////////////////////////////////////

const Field2D FDDZ(const Field2D &v, const Field2D &f) {
  return FDDZ(v, f, DIFF_DEFAULT, CELL_DEFAULT);
}

const Field2D FDDZ(const Field2D &v, const Field2D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return FDDZ(v, f, method, outloc);
}

const Field2D FDDZ(const Field2D &UNUSED(v), const Field2D &UNUSED(f), DIFF_METHOD UNUSED(method), CELL_LOC UNUSED(outloc)) {
  return Field2D(0.0);
}

const Field3D FDDZ(const Field3D &v, const Field3D &f) {
  return FDDZ(v, f, DIFF_DEFAULT, CELL_DEFAULT);
}

const Field3D FDDZ(const Field3D &v, const Field3D &f, CELL_LOC outloc, DIFF_METHOD method) {
  return mesh->indexFDDZ(v, f, outloc, method) / mesh->coordinates()->dz;
}

const Field3D FDDZ(const Field3D &v, const Field3D &f, DIFF_METHOD method, CELL_LOC outloc) {
  return FDDZ(v, f, outloc, method);
}
