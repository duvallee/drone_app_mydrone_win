//////////////////////////////////////////////////////
// Kinematics & Dynamics files for Robot applications
// 
// Copyright (c) 2002-2008. All Rights Reserved.
// Division of Applied Robot Technology KITECH
// Web: http://dart.kitech.re.kr
// Written by KwangWoong Yang<page365@gmail.com>
//

/*
 *  File: OglObjects.cpp
 *
 * Modified by duvallee.lee in 2018
 *
 */

#include "StdAfx.h"
#include <math.h>
#include <gl/glu.h>
#include "OglObjects.h" 

#ifndef M_PI
#define M_PI                                             3.14159265358979323846        // pi 
#endif

#define _RAD2DEG                                         (180. / M_PI)
#define _DEG2RAD                                         (M_PI / 180.)

/* --------------------------------------------------------------------------
 * Name : oglPlane()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglPlane(double size, double stride)
{
   size                                                  /= 2.;
   int n                                                 = (int)(size / stride);

   glDisable(GL_LIGHTING);

   glColor3d(0.5, 0.5, 0.5);

   for (int i = -n ; i <= n; ++i)
   {
      oglLine(i * stride, -size, 0, i * stride, (i == 0) ? 0 : size, 0);
      oglLine(-size, i * stride, 0, (i == 0) ? 0 : size, i * stride, 0); 
   }

   // x-axis
   glColor3d(1., 0., 0.);
   oglLine(0, 0, 0, size, 0, 0);

   //y-axis
   glColor3d(0., 1., 0.);
   oglLine (0, 0, 0, 0, size, 0);

   // z-axis
   glColor3d(0., 0., 1.);
   oglLine (0, 0, 0, 0, 0, size);

   glEnable(GL_LIGHTING);
}

/* --------------------------------------------------------------------------
 * Name : oglCoordinate()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglCoordinate(double size)
{
   glDisable(GL_LIGHTING);

   // The Z-axis
   glColor3d(0., 0., 1.);
   oglLine (0., 0., 0., 0., 0., size);

   // The Y-axis
   glColor3d(0., 1., 0.);
   oglLine(0., 0., 0., 0., size, 0.);

   // The X-axis
   glColor3d(1., 0., 0.);
   oglLine(0., 0., 0., size, 0., 0.);

   glEnable(GL_LIGHTING);
}

/* --------------------------------------------------------------------------
 * Name : oglLine()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglLine (double sx, double sy, double sz, double ex, double ey, double ez)
{   
   glBegin(GL_LINES);

   glVertex3d(sx, sy, sz);
   glVertex3d(ex, ey, ez);

   glEnd();
}

/* --------------------------------------------------------------------------
 * Name : oglLineStrip()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglLineStrip(vector <CPoint3d> &line)
{
   glBegin(GL_LINE_STRIP);

   for(unsigned int i = 0; i < line.size(); i++)
   {
      glVertex3d(line[i].x, line[i].y, line[i].z);
   }

   glEnd();
}

/* --------------------------------------------------------------------------
 * Name : oglSphere()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglSphere (double radius) 
{
   int slices                                            = 15;
   GLUquadricObj *obj                                    = gluNewQuadric();
   
   gluSphere(obj, radius, slices, slices);

   gluDeleteQuadric(obj);
}

/* --------------------------------------------------------------------------
 * Name : oglSphere()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglSphere(double rx, double ry, double rz) 
{
   int lats                                              = 15;
   int longs                                             = 15;

   for(int i = 1; i <= lats; i++)
   {
      double lat0                                        = M_PI * (0.5 + (double) (i - 1) / lats);
      double z0                                          = rz * sin(lat0);
      double zr0                                         = cos(lat0);

      double lat1                                        = M_PI * (0.5 + (double) i / lats);
      double z1                                          = rz * sin(lat1);
      double zr1                                         = cos(lat1);

      glBegin(GL_QUAD_STRIP);

      for(int j = 0; j <= longs; j++)
      {
         double lng                                      = 2 * M_PI * (double) (j - 1) / longs;
         double x                                        = rx * cos(lng);
         double y                                        = ry * sin(lng);

         glNormal3d(x * zr0, y * zr0, z0);
         glVertex3d(x * zr0, y * zr0, z0);
         glNormal3d(x * zr1, y * zr1, z1);
         glVertex3d(x * zr1, y * zr1, z1);
      }
      glEnd();
    }
}

/* --------------------------------------------------------------------------
 * Name : oglCapsule()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglCapsule(double height, double radius) 
{
   height                                                /= 2.;

   int slices                                            = 15;
   GLUquadricObj *obj                                    = gluNewQuadric();
   
   glTranslated(0., 0., -height);
   oglSphere(radius);
   
   gluQuadricOrientation(obj, GLU_OUTSIDE);
   gluCylinder(obj, radius, radius, height * 2, slices, 1);

   glTranslated(0., 0., height * 2);
   oglSphere(radius);

   glTranslated(0., 0., -height);

   gluDeleteQuadric(obj);
}

/* --------------------------------------------------------------------------
 * Name : oglCylinder()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglCylinder(double height, double radius) 
{
   height                                                /= 2.;

   int slices                                            = 15;
   GLUquadricObj *obj                                    = gluNewQuadric();
   
   glTranslated(0., 0., -height);
   gluQuadricOrientation(obj, GLU_INSIDE);
   gluDisk(obj, 0, radius, slices, 1);
   
   gluQuadricOrientation(obj, GLU_OUTSIDE);
   gluCylinder(obj, radius, radius, height * 2, slices, 1);
   
   glTranslated(0., 0., height * 2);
   gluDisk(obj, 0., radius, slices, 1);

   glTranslated(0., 0., -height);

   gluDeleteQuadric(obj);
}

/* --------------------------------------------------------------------------
 * Name : oglCylinder()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglCylinder(double height, double baseRadius, double topRadius, double xyRatio) 
{
   height                                                /= 2.;

   int lats                                              = 15;
   int longs                                             = 15;

   for(int i = 1; i <= lats; i++)
   {
      double lat0                                        = -0.5 + (double) (i - 1) / lats;
      double z0                                          = height * lat0;
      double zr0                                         = (topRadius - baseRadius) * lat0 + baseRadius;

      double lat1                                        = -0.5 + (double) i / lats;
      double z1                                          = height * lat1;
      double zr1                                         = (topRadius - baseRadius) * lat1 + baseRadius;

      glBegin(GL_QUAD_STRIP);

      for(int j = 0; j <= longs; j++)
      {
         double lng                                      = 2 * M_PI * (double) (j - 1) / longs;
         double x                                        = cos(lng);
         double y                                        = xyRatio * sin(lng);

         glNormal3d(x * zr0, y * zr0, z0);
         glVertex3d(x * zr0, y * zr0, z0);
         glNormal3d(x * zr1, y * zr1, z1);
         glVertex3d(x * zr1, y * zr1, z1);
      }
      glEnd();
    }
}

/* --------------------------------------------------------------------------
 * Name : oglBox()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglBox(double dx, double dy, double dz) 
{
   dx                                                    /= 2.;
   dy                                                    /= 2.;
   dz                                                    /= 2.;

   glBegin(GL_QUADS);

   glNormal3d(0., 0., 1.);
   glTexCoord2f(  0.0f,   0.0f); glVertex3d( dx,  dy,  dz);   
   glTexCoord2f(100.0f,   0.0f); glVertex3d( dx, -dy,  dz);
   glTexCoord2f(100.0f, 100.0f); glVertex3d(-dx, -dy,  dz);
   glTexCoord2f(  0.0f, 100.0f); glVertex3d(-dx,  dy,  dz);

   glNormal3d(0., 0., -1.);
   glTexCoord2f(  0.0f,   0.0f); glVertex3d(-dx,  dy, -dz);
   glTexCoord2f(100.0f,   0.0f); glVertex3d(-dx, -dy, -dz);
   glTexCoord2f(100.0f, 100.0f); glVertex3d( dx, -dy, -dz);
   glTexCoord2f(  0.0f, 100.0f); glVertex3d( dx,  dy, -dz);

   glNormal3d(1., 0., 0.);
   glTexCoord2f(  0.0f,   0.0f); glVertex3d( dx,  dy,  dz);
   glTexCoord2f(100.0f,   0.0f); glVertex3d( dx,  dy, -dz);
   glTexCoord2f(100.0f, 100.0f); glVertex3d( dx, -dy, -dz);
   glTexCoord2f(  0.0f, 100.0f); glVertex3d( dx, -dy,  dz);

   glNormal3d(-1., 0., 0.);
   glTexCoord2f(  0.0f,   0.0f); glVertex3d(-dx,  dy,  dz);
   glTexCoord2f(100.0f,   0.0f); glVertex3d(-dx,  dy, -dz);
   glTexCoord2f(100.0f, 100.0f); glVertex3d(-dx, -dy, -dz);
   glTexCoord2f(  0.0f, 100.0f); glVertex3d(-dx, -dy,  dz);

   glNormal3d(0., 1., 0.);
   glTexCoord2f(  0.0f,   0.0f); glVertex3d(-dx,  dy,  dz);
   glTexCoord2f(100.0f,   0.0f); glVertex3d( dx,  dy,  dz);
   glTexCoord2f(100.0f, 100.0f); glVertex3d( dx,  dy, -dz);
   glTexCoord2f(  0.0f, 100.0f); glVertex3d(-dx,  dy, -dz);

   glNormal3d(0., -1., 0.);
   glTexCoord2f(  0.0f,   0.0f); glVertex3d(-dx, -dy,  dz);
   glTexCoord2f(100.0f,   0.0f); glVertex3d( dx, -dy,  dz);
   glTexCoord2f(100.0f, 100.0f); glVertex3d( dx, -dy, -dz);
   glTexCoord2f(  0.0f, 100.0f); glVertex3d(-dx, -dy, -dz);

   glEnd();
}

/* --------------------------------------------------------------------------
 * Name : oglBracketJoint()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglBracketJoint(double d, double theta, double a, double b, double alpha, double radius)
{
   int slices                                            = 15;

   glPushMatrix(); 
   {
      GLUquadricObj *obj                                 = gluNewQuadric();
      
      double rz                                          = theta + _RAD2DEG * atan2(b, a);
      double ry                                          = 90. - _RAD2DEG * atan2(d, sqrt(a * a + b * b));
      double height                                      = sqrt(d * d + a * a + b * b);

      glRotated(rz, 0, 0, 1);
      glRotated(ry, 0, 1, 0);

      gluQuadricOrientation(obj, GLU_OUTSIDE);
      gluCylinder(obj, radius, radius, height, slices, 1);

      glTranslated(0, 0, height);      
      gluDisk(obj, 0., radius, slices, 1);

      gluDeleteQuadric(obj);
   }
   glPopMatrix();
}

/* --------------------------------------------------------------------------
 * Name : oglRevoluteJoint()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglRevoluteJoint(double d, double theta, double a, double b, double alpha, double radius)
{
   int slices                                            = 15;
   double inrad                                          = radius * 0.;
   double outrad                                         = radius * 1.3;

   glPushMatrix();
   {
      GLUquadricObj *obj                                 = gluNewQuadric();
      
      // draw joint
      glTranslated(0., 0., -radius);
      gluQuadricOrientation(obj, GLU_INSIDE);
      gluDisk(obj, 0, outrad, slices, 1);
      
      gluQuadricOrientation(obj, GLU_OUTSIDE);
      gluCylinder(obj, outrad, outrad, radius * 2, slices, 1);

      glTranslated(0., 0., radius * 2);
      gluDisk(obj, 0, outrad, slices, 1);
      glTranslated(0., 0., -radius);

      // draw link
      gluQuadricOrientation(obj, GLU_OUTSIDE);

      double rz                                          = theta + _RAD2DEG * atan2(b, a);
      double ry                                          = 90. - _RAD2DEG * atan2(d, sqrt(a * a + b * b));
      double height                                      = sqrt(d * d + a * a + b * b);

      glRotated(rz, 0, 0, 1);
      glRotated(ry, 0, 1, 0);
      gluCylinder(obj, radius, radius, height, slices, 1);

      glTranslated(0, 0, height);      
      gluDisk(obj, 0., radius, slices, 1);
   
      gluDeleteQuadric(obj);
   }
   glPopMatrix();
}

/* --------------------------------------------------------------------------
 * Name : oglPrismaticJoint()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglPrismaticJoint(double d, double theta, double a, double b, double alpha, double radius)
{
   int slices                                            = 15;
   double  inrad                                         = radius * 1.;
   double outrad                                         = radius * 1.3;

   glPushMatrix(); 
   {
      GLUquadricObj *obj                                 = gluNewQuadric();
      
      gluQuadricOrientation(obj, GLU_OUTSIDE);

      double rz                                          = theta + _RAD2DEG * atan2(b, a);
      double ry                                          = 90. - _RAD2DEG * atan2(d, sqrt(a * a + b * b));
      double height                                      = sqrt(d * d + a * a + b * b);

      glRotated(rz, 0, 0, 1);
      glRotated(ry, 0, 1, 0);
      oglBox(outrad, outrad, outrad);
      glTranslated (0, 0, height / 2);      
      oglBox(inrad, inrad, height / 2);

      gluDeleteQuadric(obj);
   }
   glPopMatrix();
}

/* --------------------------------------------------------------------------
 * Name : oglMotor()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglMotor(double height, double radius) 
{
   height                                                /= 2.;

   int slices                                            = 15;
   GLUquadricObj *gl_obj                                 = gluNewQuadric();

   glTranslated(0., 0., -height);
   gluQuadricOrientation(gl_obj, GLU_INSIDE);
   gluDisk(gl_obj, 0, radius, slices, 1);
   gluQuadricOrientation(gl_obj, GLU_OUTSIDE);
   gluCylinder(gl_obj, radius, radius, height * 2, slices, 1);

   glTranslated(0., 0., height * 2);
   gluDisk(gl_obj, 0., radius, slices, 1);
   glTranslated(0., 0., -height);
   glRotated(90, 1, 0, 0);

   glTranslated(0., 0., -height);
   gluQuadricOrientation(gl_obj, GLU_INSIDE);
   gluDisk(gl_obj, 0, radius, slices, 1);
   gluQuadricOrientation(gl_obj, GLU_OUTSIDE);
   gluCylinder(gl_obj, radius, radius, height * 2, slices, 1);

   glTranslated(0., 0., height * 2);
   gluDisk(gl_obj, 0., radius, slices, 1);
   glTranslated(0., 0., -height);
   glRotated(90, 0, 1, 0);

   glTranslated(0., 0., -height);
   gluQuadricOrientation(gl_obj, GLU_INSIDE);
   gluDisk(gl_obj, 0, radius, slices, 1);
   gluQuadricOrientation(gl_obj, GLU_OUTSIDE);
   gluCylinder(gl_obj, radius, radius, height * 2, slices, 1);

   glTranslated(0., 0., height * 2);
   gluDisk(gl_obj, 0., radius, slices, 1);
   glTranslated(0., 0., -height);

   gluDeleteQuadric(gl_obj);
}

/* --------------------------------------------------------------------------
 * Name : oglRay()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglRay(double length)
{
   glDisable(GL_LIGHTING);
   glLineWidth(2.0f);

   glBegin(GL_LINE_LOOP);

   glVertex3d(0., 0., 0.);
   glVertex3d(0., 0., length);
   
   glEnd();

   glLineWidth(1.0f);

   glEnable(GL_LIGHTING);
}

/* --------------------------------------------------------------------------
 * Name : oglMesh()
 *
 *
 *
 * -------------------------------------------------------------------------- */
void oglMesh(float *fVertex, float *fNormal, float *fTextureCoord, int iTriangleCount)
{
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, fVertex);

   glEnableClientState(GL_NORMAL_ARRAY);
   glNormalPointer(GL_FLOAT, 0, fNormal);

   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glTexCoordPointer(3, GL_FLOAT, 0, fTextureCoord);
   glDrawArrays(GL_TRIANGLES, 0, 3 * iTriangleCount);

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_NORMAL_ARRAY);
}

