/*
 *  File: OglWinExt.h
 *
 * Written by duvallee.lee in 2018
 *
 */

#pragma once
#include "OglWnd.h"
#include "MatrixAlgebra.h"

class COglWndExt : public COglWnd
{
public:
   dMatrix sensorRotation;
   dVector gyroscope;
   dVector accelerometer;
   dVector magnetometer;

public:
   COglWndExt();

   void TransformAxis(dMatrix &A);
   virtual void OnDraw(CDC* pDC);
};


