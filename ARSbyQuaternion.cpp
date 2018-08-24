#include "StdAfx.h"
#include "MatrixAlgebra.h"
#include "Quaternion.h"
#include "mm.h"

static Quaternion _Q(0,0,0);		// roll, pitch, yaw 값을 가지는 쿼터니언
double average_gyro[3] = {0, 0, 0, };	// 각속도의 평균 값을 저장한다. 화면에 표시하기 위해 전역변수로 설정함.
double adjust_rpy[3] = {0, 0, 0, };		// 가속도센서에서 측정된 중력의 방향으로부터 보정된 roll, pitch, yaw 값. 여기서는 사용안함

void InitARS()
{
	// 단위 쿼터니언으로 초기화 한다.

	_Q.s_ = 1.;
	_Q.v_[0] = 0.;
	_Q.v_[1] = 0.;
	_Q.v_[2] = 0.;
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
	// 그리고 각도의 변화량을 
	Quaternion dQ(
		_DEG2RAD*(gyro[0] - average_gyro[0])*dt, 
		_DEG2RAD*(gyro[1] - average_gyro[1])*dt, 
		_DEG2RAD*(gyro[2] - average_gyro[2])*dt);
	
	// 현재 센서의 자세를 나타내는 쿼터니언(_Q)에 자세 변화량 쿼터니언을 곱해서
	// 자세의 변화를 업데이트 한다.
	//_Q = _Q * dQ;

	// 센서가 움직이지 않는 상황에서 항상 중력 가속도는 아래 방향(-z)으로 작용하고 있기
	// 때문에, 센서가 움직이지 않을 때는 가속도 벡터(g)는 중력과 동일하다. 그래서
	// 가속도 벡터의 자세로부터 현재 자세 _Q의 roll과 pitch를 보정할 수 있다.
	dVector g0 = dVector3(0., 0., -1.);
	Quaternion Qa (0, accl[0], accl[1], accl[2]);

	// g는 센서 좌표계를 기준으로 한 벡터이므로, 이들을 전역 좌표계 기준으로 표시하기 
	// 위해서는 _Q을 곱해서 전역 좌표계 기준으로 변환한다.
	Quaternion Qg = _Q * Qa * _Q.i();

	// 전역 좌표계에서 가속도 센서가 측정한 중력 방향(Qg)과 이상적인 중력 방향(g0)이 이루는 
	// normal vector(ng)와 사잇각(alpha)를 구한다.
	dVector ng = Cross (Qg.v(), g0);
	double alpha = asin(Norm2(ng)/Norm2(Qg.v()));

	// 중력으로 찾은 각도의 오차를 업데이트 하는 비율(이득)을 정한다. 
	// 중력 벡터의 크기가 1.근처일 때 이득이 커야하고 1.에서 멀어질 수록 이득이 적어야 한다.
	double l = sqrt (accl[0]*accl[0] + accl[1]*accl[1] + accl[2]*accl[2]) - 1.;
	double sigma = 0.1;
	double Kg = 0.1*exp(-l*l/(sigma*sigma));

	_Q = Quaternion (Kg*alpha, ng) * _Q;

#if defined ADIS_16405
	{
		// 지자기는 항상 일정한 방향으로 작용하고 있기때문에, 지자기 센서의 지자기 
		// 벡터(m)로는 yaw를 보정할 수 있다.
		Quaternion Qm (0, magn[0], magn[1], magn[2]);

		// m은 센서 좌표계를 기준으로 한 벡터이므로, 이들을 전역 좌표계 기준으로 표시하기 
		// 위해서는 _Q을 곱해서 전역 좌표계 기준으로 변환한다.
		Qm = _Q * Qm * _Q.i();

		dVector mn (0., 0., 1.);
		double alpha = -atan2 (Qm.v()[1], Qm.v()[0]);
		const double Ka = 0.1;
		
		_Q = Quaternion (Ka*alpha, mn) * _Q;
	}
#endif

	return _Q.RotationMatrix ();
}
