/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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
#include "file.h"
#include "cmd_unit.h"
#include "cmd_gun.h"
#include "gfx_sprite.h"
#include "lin_time.h"
#include "cmd_unitenum.h"

#include "gfx_hud.h"

void Unit::Init()
{
	meshdata = NULL;
	subunits = NULL;
	aistate = NULL;
	//weapons = NULL;
	numsubunit = 0;
	time = 0;
	active = TRUE;

	Identity(tmatrix);
	ResetVectors(pp,pq,pr);
	ppos = Vector(0,0,0);
	fpos = 0;
	mass = 0;
	fuel = 0;
	
	MomentOfInertia = 0;
	AngularVelocity = Vector(0,0,0);
	Velocity = Vector(0,0,0);

	NetTorque = Vector(0,0,0);
	NetForce = Vector(0,0,0);
}

Unit::Unit()
{
	Init();
}
Unit::Unit(char *filename):Mesh()
{
	Init();

	debugName = strdup((filename + string(" - Unit")).c_str());
	/*Insert file loading stuff here*/
	LoadFile(filename);

	ReadInt(nummesh);
	meshdata = new Mesh*[nummesh];
	for(int meshcount = 0; meshcount < nummesh; meshcount++)
	{
		int meshtype;
		ReadInt(meshtype);
		char meshfilename[64];
		float x,y,z;
		ReadMesh(meshfilename, x,y,z);
		if(meshtype == 0)
			meshdata[meshcount] = new Mesh(meshfilename);
		else
			meshdata[meshcount] = new Sprite(meshfilename);
		meshdata[meshcount]->SetPosition(x,y,z);
	}

	ReadInt(numsubunit);
	for(int unitcount = 0; unitcount < numsubunit; unitcount++)
	{
		char unitfilename[64];
		float x,y,z;
		int type;
		ReadUnit(unitfilename, type, x,y,z);
		switch((UnitType)type)
		{
		case TEXTPLANE:
			subunits[unitcount] = new TextPlane(unitfilename);
			break;
		case LIGHT:
			subunits[unitcount] = new Unit(unitfilename);	//how to group turrets like fighters??? maybes stick in special cases
														//Translation: This ROYALLY screws up the idea of having every object as a class...
														//perhaps use a switch statement? that would get rather big, although it would be straightforward
														//maybe have a behavior variable in each turret that specifies its stats, but that sucks too
														//or, make a special exception to the copy constructor rule... *frumple* this sucks
			break;
		case GUN:
			subunits[unitcount] = new Gun(unitfilename);
			break;
		default:
		  printf ("unit type not supported");
		}
		subunits[unitcount]->SetPosition(x,y,z);
	}
	int restricted;
	float min, max;
	ReadInt(restricted); //turrets and weapons
	//ReadInt(restricted); // What's going on here? i hsould have 2, but that screws things up

	ReadRestriction(restricted, min, max);
	if(restricted)
		RestrictYaw(min,max);

	ReadRestriction(restricted, min, max);
	if(restricted)
		RestrictPitch(min,max);

	ReadRestriction(restricted, min, max);
	if(restricted)
		RestrictRoll(min,max);

	float maxspeed, maxaccel, mass;
	ReadFloat(maxspeed);
	ReadFloat(maxaccel);
	ReadFloat(mass);

	fpos = ::GetPosition();

	CloseFile();

}

Unit::~Unit()
{
	if(meshdata&&nummesh>0)
	{
		for(int meshcount = 0; meshcount < nummesh; meshcount++)
			delete meshdata[meshcount];
		delete [] meshdata;
	}
	if(subunits)
	{
		for(int subcount = 0; subcount < numsubunit; subcount++)
			delete subunits[subcount];
		delete [] subunits;
	}
	if(aistate)
		delete aistate;
/*
	if(weapons)
	{
		for(int weapcount = 0; weapcount < numweap; subcount++)
			delete subsidiary[subcount];
		delete [] subsidiary;
	}
*/
}

bool Unit::querySphere (const Vector &pnt, float err) {
  UpdateMatrix();
  int i;
  Vector TargetPoint;
  for (i=0;i<nummesh;i++) {
    TargetPoint = Transform(transformation,meshdata[i]->Position())-pnt;
    if (sqrtf (TargetPoint.Dot (TargetPoint))< err+meshdata[i]->rSize())
      return true;
  }
  for (int i=0;i<numsubunit;i++) {
    if (querySphere (transformation,pnt,err))
      return true;
  }
  return false;
}

bool Unit::querySphere (Matrix t,const Vector &pnt, float err) {
  UpdateMatrix();
  int i;
  Matrix tmpo;
  MultMatrix (tmpo,t,transformation);
  
  Vector TargetPoint;
  for (i=0;i<nummesh;i++) {
    TargetPoint = Transform (tmpo,meshdata[i]->Position())-pnt;
    if (sqrtf (TargetPoint.Dot (TargetPoint))< err+meshdata[i]->rSize())
      return true;
  }
  for (int i=0;i<numsubunit;i++) {
    if (querySphere (tmpo,pnt,err))
      return true;
  }
  return false;
}

bool Unit::queryBoundingBox (const Vector &pnt, float err) {
  UpdateMatrix();
  int i;
  BoundingBox * bbox=NULL;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (transformation);
    if (bbox->Within(pnt,err)) {
      delete bbox;
      return true;
    }
    delete bbox;
  }
  for (i=0;i<numsubunit;i++) {
    if (subunits[i]->queryBoundingBox (transformation,pnt,err)) 
      return true;
  }
  return false;
}

bool Unit::queryBoundingBox (Matrix t,const Vector &pnt, float err) {
  int i;
  Matrix tmpo;
  MultMatrix (tmpo,t, transformation);
  BoundingBox *bbox=0;
  for (i=0;i<nummesh;i++) {
    bbox = meshdata[i]->getBoundingBox();
    bbox->Transform (tmpo);
    if (bbox->Within(pnt,err)){
      delete bbox;
      return true;
    }
    delete bbox;
  }
  if (numsubunit>0) {
    for (i=0;i<numsubunit;i++) {
      if (subunits[i]->queryBoundingBox (tmpo,pnt,err))
	return true;
    }
  }
  return false;

}

void Unit::Draw()
{
	//glMatrixMode(GL_MODELVIEW);
	
	/*
	if(changed)
		MultMatrix(transformation, translation, orientation);
	glMultMatrixf(transformation);
	*/
	time += GetElapsedTime();
	UpdateMatrix();
	Vector np = Transform(pp,pq,pr,p), 
		nq = Transform(pp,pq,pr,q),
		nr = Transform(pp,pq,pr,r),
		npos = ppos+pos;

	Matrix currentMatrix;
	GFXGetMatrix(MODEL, currentMatrix);
	for (int i=0;i<nummesh;i++) {
	  GFXLoadMatrix(MODEL, currentMatrix); // not a problem with overhead if the mesh count is kept down
	  //meshdata[i]->Draw(np, nq, nr, npos);
	  meshdata[i]->Draw();
	}
	for(int subcount = 0; subcount < numsubunit; subcount++)
		subunits[subcount]->Draw(tmatrix, np, nq, nr, npos);
	if(aistate)
		aistate = aistate->Execute();
}

void Unit::DrawStreak(const Vector &v)
{
	Vector v1 = v;
	int steps = (int)v.Magnitude()*10;
	v1 = v1 * (1.0/steps);
	Vector opos = pos;
	GFXColor(0.5, 0.5, 0.5, 0.5);
	for(int a = 0; a < steps; a++) {
		Draw();
		pos+=v1;
	}
	GFXColor(1.0, 1.0, 1.0, 1.0);
	pos = opos;
}

void Unit::Draw(Matrix tmatrix)
{
	CopyMatrix(this->tmatrix, tmatrix);
	Draw();
}

void Unit::Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos)
{
	//Matrix orientation;
	//Matrix translation;
	//Matrix tmatrix;

	this->pp = pp;
	this->pq = pq;
	this->pr = pr;
	this->ppos = ppos;

	/*
	  VectorToMatrix(orientation, pp,pq,pr);
	  Translate(translation, ppos.i,ppos.j,ppos.k);
	  
	  MultMatrix(tmatrix, translation, orientation);
	
	  CopyMatrix(this->tmatrix, tmatrix);
	*/
	Draw();
}

void Unit::SetAI(AI *newAI)
{
	if(aistate)
		delete aistate;
	aistate = newAI;
	aistate->SetParent(this);
}
