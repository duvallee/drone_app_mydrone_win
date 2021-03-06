//////////////////////////////////////////////////////
// Kinematics & Dynamics files for Robot applications
//
// Copyright (c) 2002-2008. All Rights Reserved.
// Division of Applied Robot Technology KITECH
// Web: http://dart.kitech.re.kr
// Written by KwangWoong Yang<page365@gmail.com>
//
/*
 *  File: OglCommon.h
 *
 * Modified by duvallee.lee in 2018
 *
 */

#pragma once

#include "OglCommon.h"
#include <vector>

using namespace std;

extern void oglPlane(double size, double stride);
extern void oglCoordinate(double size);

extern void oglLine(double sx, double sy, double sz, double ex, double ey, double ez);
extern void oglLineStrip(vector <CPoint3d> &line);
extern void oglSphere(double radius);
extern void oglSphere(double rx, double ry, double rz);
extern void oglCapsule(double height, double radius);
extern void oglCylinder(double height, double radius);
extern void oglCylinder(double height, double baseRadius, double topRadius, double xyRatio);
extern void oglBox(double dx, double dy, double dz);

extern void oglBracketJoint(double d, double theta, double a, double b, double alpha, double radius);
extern void oglRevoluteJoint(double d, double theta, double a, double b, double alpha, double radius);
extern void oglPrismaticJoint(double d, double theta, double a, double b, double alpha, double radius);

//typedef double dVector[4];

extern void oglRay(double length);
extern void oglMotor(double height, double radius);

extern void oglMesh(float *fVertex, float *fNormal, float *fTextureCoord, int iTriangleCount);

