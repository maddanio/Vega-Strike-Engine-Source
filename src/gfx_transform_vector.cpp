/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Chris Fry
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* � 1998 Chris Fry & Daniel Horn*/
#include "vegastrike.h"
#include <math.h>
#include "gfx_transform_vector.h"
//#include "glob_externs.h"
#define _CZ 761.465325527
//extern Vector	_LightVector;
//extern float _LVRed;
//extern float _LVGreen;
//extern float _LVBlue;
//extern Vector _CamCoord;
//extern Vector _CamP;
//extern Vector _CamQ;
//extern Vector _CamR;


//extern float _CamTransConst;






/////////////////////////////////////////////////////////////
//   Yaws a unit vector
/////////////////////////////////////////////////////////////

void Vector::Yaw(float rad) //only works with unit vector
{
	float theta;
	
	if (i>0)
		theta = (float)atan(k/i);
	else if (i<0)
		theta = PI+(float)atan(k/i);
	else if (k<=0 && i==0)
		theta = -PI/2;
	else if (k>0 && i==0)
		theta = PI/2;

	theta += rad;
	i = cosf(theta);
	k = sinf(theta); 
}

void Vector::Roll(float rad)
{
	float theta;
	
	if (i>0)
		theta = (float)atan(j/i);
	else if (i<0)
		theta = PI+(float)atan(j/i);
	else if (j<=0 && i==0)
		theta = -PI/2;
	else if (j>0 && i==0)
		theta = PI/2;
	
	theta += rad; 
	i = cosf(theta);
	j = sinf(theta); 
}

void Vector::Pitch(float rad)
{
	float theta;
	
	if (k>0)
		theta = (float)atan(j/k);
	else if (k<0)
		theta = PI+(float)atan(j/k);
	else if (j<=0 && k==0)
		theta = -PI/2;
	else if (j>0 && k==0)
		theta = PI/2;
	
	theta += rad;
	k = cosf(theta);
	j = sinf(theta);
}

void Yaw (float rad, Vector &p,Vector &q, Vector &r)
{
	Vector temp1, temp2, temp3;
	temp1.i=1;
	temp1.j =0;
	temp1.k =0;
	temp1.Yaw (rad);
	temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp2.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	temp1.i= 0;
	temp1.j =0;
	temp1.k =1;
	temp1.Yaw (rad);
	temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp3.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	p = temp2;
	r = temp3;
}

void Pitch (float rad,Vector &p, Vector &q, Vector &r)
{
	Vector temp1, temp2, temp3;
	temp1.i=0;
	temp1.j =1;
	temp1.k =0;
	temp1.Pitch (rad);
	temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp2.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	temp1.i= 0;
	temp1.j =0;
	temp1.k =1;
	temp1.Pitch (rad);
	temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp3.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	q = temp2;
	r = temp3;
}
void Roll (float rad,Vector &p, Vector &q, Vector &r)
{
	Vector temp1, temp2, temp3;
	temp1.i=1;
	temp1.j =0;
	temp1.k =0;
	temp1.Roll (rad);
	temp2.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp2.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp2.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	temp1.i= 0;
	temp1.j =1;
	temp1.k =0;
	temp1.Roll (rad);
	temp3.i = temp1.i * p.i + temp1.j * q.i + temp1.k * r.i;
	temp3.j = temp1.i * p.j + temp1.j * q.j + temp1.k * r.j;
	temp3.k = temp1.i*p.k   + temp1.j * q.k + temp1.k * r.k;
	p = temp2;
	q = temp3;
}
void ResetVectors (Vector &p, Vector &q, Vector &r)
{
	p.i = q.j = r.k = 1;
	p.j = p.k= q.i = q.k = r.i = r.j = 0;
}

void MakeRVector (Vector &p,Vector &q, Vector &r) {
  ScaledCrossProduct (p,q,r);
  ScaledCrossProduct (r,p,q);
  Normalize (p);

}
void Orthogonize(Vector &p, Vector &q, Vector &r)
{
	Normalize(r);
	ScaledCrossProduct (r,p,q); //result of scaled cross put into q
	ScaledCrossProduct (q,r,p); //result of scaled cross put back into p
}
