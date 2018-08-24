#include "StdAfx.h"
#include "MatrixAlgebra.h"
#include "mm.h"

static dMatrix x(3,1);	// EKF의 상태 벡터, 여기서는 roll, pitch, yaw 값을 가지는 행렬이다.
static dMatrix P(3,3);	// 상태 벡터의 공분산 행렬

double average_gyro[3] = {0, 0, 0, };	// 각속도의 평균 값을 저장한다. 화면에 표시하기 위해 전역변수로 설정함.
double adjust_rpy[3] = {0, 0, 0, };		// 가속도센서에서 측정된 중력의 방향으로부터 보정된 roll, pitch, yaw 값. 여기서는 사용하지 않음.

void InitARS()
{
	// EKF의 상태 벡터와 공분산 행렬을 초기화 한다.
	x.null();		// 상태벡터는 0으로 초기화
	P.unit();		// 공분산 행렬은 단위행렬로 초기화
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

dMatrix GetCInv(double phi, double theta)
{
	// 오일러각 변화율과 각속도각의 관계를 표현하는 행렬을 계산한다.

	double sin_phi = sin(phi),		cos_phi = cos(phi);
	double sin_tht = sin(theta),	cos_tht = cos(theta),	tan_tht = tan(theta);

	dMatrix C(3,3);

	C(0,0) = 1;		C(0,1) = sin_phi*tan_tht;	C(0,2) = cos_phi*tan_tht;
	C(1,0) = 0;		C(1,1) = cos_phi;			C(1,2) = -sin_phi;
	C(2,0) = 0;		C(2,1) = sin_phi/cos_tht;	C(2,2) = cos_phi/cos_tht;

	return C;
}

dMatrix GetStateTransitionMatrix(double phi, double theta, double wy, double wz, double dt)
{
	// 시스템 모델 f의 자코비안을 계산한다.

	double sin_phi = sin(phi),		cos_phi = cos(phi);
	double sin_tht = sin(theta),	cos_tht = cos(theta);

	dMatrix A(3,3);

	A(0,0) = 1 + sin_tht*(wy*cos_phi - wz*sin_phi)/cos_tht;		
	A(0,1) = 0 + (wy*sin_phi + wz*cos_phi)/(cos_tht*cos_tht);
	A(0,2) = 0;
	A(1,0) = 0 - wy*sin_phi - wz*cos_phi;
	A(1,1) = 1;
	A(1,2) = 0;
	A(2,0) = 0 + (wy*cos_phi - wz*sin_phi)/cos_tht;		
	A(2,1) = 0 + sin_tht*(wy*sin_phi + wz*cos_phi)/(cos_tht*cos_tht);
	A(2,2) = 1;

	return A*dt;
}

dMatrix ARS (double gyro[3], double accl[3], double magn[3], double dt)
{
	GyroAverage (average_gyro, gyro, 10000, 10.);

	// 자이로 센서의 바이어스 값을 보정하기 위해 각속도 값에서 각속도 평균값을 뺀다.
	dMatrix w = MakeMatrix (dVector3 (
		_DEG2RAD*(gyro[0] - average_gyro[0]), 
		_DEG2RAD*(gyro[1] - average_gyro[1]), 
		_DEG2RAD*(gyro[2] - average_gyro[2])));

	/////////////////////////////////////////////////////////////////////////
	// Predict 과정

	// 각속도를 오일러각 변화율로 변환하는 행렬
	dMatrix Cinv = GetCInv (x(0,0), x(1,0));

	// state transition matrix
	dMatrix A = GetStateTransitionMatrix (x(0,0), x(1,0), w(1,0), w(2,0), dt);
	dMatrix B = Cinv*dt;
	
	const double _q[3][3] = { 
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1} 
	};

	// Covariance matrix of porcess noises
	const dMatrix Q(3, 3, &_q[0][0]);

	// Predicted state estimate
	x = x + Cinv*w*dt;
	// Predicted estimate covariance
	P = A*P*~A + B*Q*~B;

	/////////////////////////////////////////////////////////////////////////
	// Update 과정

	double phi_e   = atan2(-accl[1],-accl[2]);
	double theta_e = (-1 < accl[0] && accl[0] < 1) ? asin(-accl[0]/-1.) : 0;

	// 중력가속도 값이 1근처에 있을 때 이득(k1)이 큰 값을 가지도록 한다.
	double v = sqrt (accl[0]*accl[0] + accl[1]*accl[1] + accl[2]*accl[2]) - 1.;
	double s = exp(-v*v);
	double e = pow(2.*accl[0],8.);

	double _r[2][2] = { 
		{0.01*(1/s+e), 0},
		{0, 0.01*(1/s+e)} };

	// Covariance matrix of observation noises
	dMatrix R(2, 2, &_r[0][0]);

	// measurement value
	dMatrix z(2,1);
	z(0,0) = phi_e;
	z(1,0) = theta_e;

	//  predicted measurement from the predicted state
	dMatrix h(2,1);
	h(0,0) = x(0,0);
	h(1,0) = x(1,0);

	// measurement residual
	// y = z - h
	dMatrix y(2,1);
	y(0,0) = DeltaRad(z(0,0), h(0,0));
	y(1,0) = DeltaRad(z(1,0), h(1,0));

	const double _h[2][3] = { 
		{1, 0, 0},
		{0, 1, 0} };

	// observation matrix
	const dMatrix H(2, 3, &_h[0][0]);

	// Kalman gain
	dMatrix K = P*~H*!(H*P*~H + R);

	// Updated state estimate
	x = x + K*y;
	// Updated estimate covariance
	P = P - K*H*P;

	return RotationMatrix (x(0,0), x(1,0), x(2,0));
}
