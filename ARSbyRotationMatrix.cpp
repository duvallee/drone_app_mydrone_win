#include "StdAfx.h"
#include "MatrixAlgebra.h"
#include "mm.h"

static dMatrix R(3,3);		// roll, pitch, yaw 값을 가지는 3x3 회전행렬

double average_gyro[3] = {0, 0, 0, };	// 각속도의 평균 값을 저장한다. 화면에 표시하기 위해 전역변수로 설정함.
double adjust_rpy[3] = {0, 0, 0, };		// 가속도센서에서 측정된 중력의 방향으로부터 보정된 roll, pitch, yaw 값.


void InitARS()
{
	//	회전행렬을 단위행렬로 초기화 한다.
	R.unit();
}

void GyroAverage (double average_gyro[3], double gyro[3], const int max_count, const double tolerance)
{
	// 센서가 정지해 있더라도 각속도계(gyroscope)에서 읽은 값은 0의 값을 가지지 않는다.
	// 대부분 특정한 값으로 바이어스 되어있는데, 이 값을 0으로 보정하기 위하여
	// 센서가 움직이지 않을 때 각속도 값의 평균을 구한다.

	static int count = 0;

	double d0 = gyro[0] - average_gyro[0];
	double d1 = gyro[1] - average_gyro[1];
	double d2 = gyro[2] - average_gyro[2];

	if (!count || ( 
		-tolerance < d0 && d0 < tolerance && 
		-tolerance < d1 && d1 < tolerance && 
		-tolerance < d2 && d2 < tolerance )) {

		average_gyro[0]  = (average_gyro[0]*count + gyro[0])/(count+1);
		average_gyro[1]  = (average_gyro[1]*count + gyro[1])/(count+1);
		average_gyro[2]  = (average_gyro[2]*count + gyro[2])/(count+1);
	}
	if (count < max_count) ++count;
}

dMatrix ARS (double gyro[3], double accl[3], double magn[3], double dt)
{
	GyroAverage (average_gyro, gyro, 10000, 10.);

	// 자이로 센서의 바이어스 값을 보정하기 위해 각속도 값에서 각속도 평균값을 뺀다.
	// 그리고 각도의 변화량을 회전행렬로 변환한다.
	dMatrix dR = RotationMatrix (
		_DEG2RAD*(gyro[0] - average_gyro[0])*dt, 
		_DEG2RAD*(gyro[1] - average_gyro[1])*dt, 
		_DEG2RAD*(gyro[2] - average_gyro[2])*dt);

	// 회전 행렬의 곱은 두 각도를 더하는 효과를 가진다.
	// R*dR은 R만큼 회전된 좌표계를 dR만큼 더 회전하게 된다.
	R = R*dR;

	dMatrix g = dMatrix (3,1, accl);
	dMatrix m = dMatrix (3,1, magn);

	// accl과 magn은 센서 좌표계를 기준으로 측정된 값이다.
	// 센서의 자세(R)을 곱해서 전역좌표계를 기준으로 한 값으로 바꿔준다.
	g = R*g;
	m = R*m;

	// 중력가속도 값이 1근처에 있을 때 이득(k1)이 큰 값을 가지도록 한다.
	double l = sqrt (accl[0]*accl[0] + accl[1]*accl[1] + accl[2]*accl[2]) - 1.;
	double sigma = 0.1;
	double k1 = 0.1*exp(-l*l/(sigma*sigma));

#if defined ADIS_16405
	double k2 = 0.1;

	// 각도의 보정량을 계산한다.
	double dPhi   = g(2,0) ? atan (g(1,0)/g(2,0)) : 0.;
	double dTheta = (-1 < g(0,0) && g(0,0) < 1) ? asin(-g(0,0)/-1.) : 0.;
	double dPsi   = -atan2 ( m(1,0),  m(0,0));
#else
	double k2 = 0.;

	// 각도의 보정량을 계산한다.
	double dPhi   = g(2,0) ? atan (g(1,0)/g(2,0)) : 0.;
	double dTheta = (-1 < g(0,0) && g(0,0) < 1) ? asin(-g(0,0)/-1.) : 0.;
	double dPsi   = 0.;
#endif

	adjust_rpy[0] = dPhi;
	adjust_rpy[1] = dTheta;
	adjust_rpy[2] = dPsi;

	// 오일러각으로 계산한 보정량을 회전행렬로 변환한다.
	dMatrix aR = RotationMatrix (k1*dPhi, k1*dTheta, k2*dPsi);

	// 계산된 보정량(aR)은 전역좌표계를 기준으로 계산된 값이다.
	// 그러므로 R의 앞에서 aR을 곱해야 한다.
	// R*aR 과 같이 쓰지 않도록 주의한다.
	R = aR*R;

	return R;
}
