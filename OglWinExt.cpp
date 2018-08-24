
#include "StdAfx.h"
#include "OglWinExt.h"
#include "OglObjects.h"
#include <gl/glu.h>
#include "mm.h"

COglWndExt::COglWndExt () :
	sensorRotation(3,3), gyroscope(3), accelerometer(3), magnetometer(3)
{
	sensorRotation.unit();
	//gyroscope.null();
	//accelerometer.null();
	//magnetometer.null();
}

void COglWndExt::TransformAxis(dMatrix &A)
{
	if (!A.size()) return;

	double m[16];
	m[0] = A(0,0);  m[4] = A(0,1);  m[8] = A(0,2);  m[12] = 0.; 
	m[1] = A(1,0);  m[5] = A(1,1);  m[9] = A(1,2);  m[13] = 0.; 
	m[2] = A(2,0);  m[6] = A(2,1);  m[10]= A(2,2);  m[14] = 0.; 
	m[3] = 0.;      m[7] = 0.;      m[11]= 0.;      m[15] = 1.; 

	glMultMatrixd(m);
}

void COglWndExt::OnDraw(CDC* pDC) 
{ 
	SetViewport ();
	oglPlane (1., .1);

	glPushMatrix(); 

	TransformAxis (sensorRotation);
	glColor3d(0.8, 0.8, 0.8);
	oglBox (0.2, 0.03, 0.02);
	oglCoordinate (0.2);

	glDisable(GL_LIGHTING);
	glLineWidth (3.);

	glPushMatrix(); 
	glColor3d(0., 0., 0.5);
	oglLine (0, 0, 0, gyroscope[0], gyroscope[1], gyroscope[2]);
	glPopMatrix();

	glPushMatrix(); 
	glColor3d(0., 1., 1.);
	oglLine (0, 0, 0, accelerometer[0], accelerometer[1], accelerometer[2]);
	glPopMatrix();

	glPushMatrix(); 
	glColor3d(1., 0., 1.);
	oglLine (0, 0, 0, magnetometer[0], magnetometer[1], magnetometer[2]);
	glPopMatrix();

	glLineWidth (1.);
	glEnable(GL_LIGHTING);

	glPopMatrix();
}