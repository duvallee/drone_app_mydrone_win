#include "StdAfx.h"
#include "MatrixAlgebra.h"
#include "mm.h"

static dMatrix R(3,3);		// roll, pitch, yaw ���� ������ 3x3 ȸ�����

double average_gyro[3] = {0, 0, 0, };	// ���ӵ��� ��� ���� �����Ѵ�. ȭ�鿡 ǥ���ϱ� ���� ���������� ������.
double adjust_rpy[3] = {0, 0, 0, };		// ���ӵ��������� ������ �߷��� �������κ��� ������ roll, pitch, yaw ��.


void InitARS()
{
	//	ȸ������� ������ķ� �ʱ�ȭ �Ѵ�.
	R.unit();
}

void GyroAverage (double average_gyro[3], double gyro[3], const int max_count, const double tolerance)
{
	// ������ ������ �ִ��� ���ӵ���(gyroscope)���� ���� ���� 0�� ���� ������ �ʴ´�.
	// ��κ� Ư���� ������ ���̾ �Ǿ��ִµ�, �� ���� 0���� �����ϱ� ���Ͽ�
	// ������ �������� ���� �� ���ӵ� ���� ����� ���Ѵ�.

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

	// ���̷� ������ ���̾ ���� �����ϱ� ���� ���ӵ� ������ ���ӵ� ��հ��� ����.
	// �׸��� ������ ��ȭ���� ȸ����ķ� ��ȯ�Ѵ�.
	dMatrix dR = RotationMatrix (
		_DEG2RAD*(gyro[0] - average_gyro[0])*dt, 
		_DEG2RAD*(gyro[1] - average_gyro[1])*dt, 
		_DEG2RAD*(gyro[2] - average_gyro[2])*dt);

	// ȸ�� ����� ���� �� ������ ���ϴ� ȿ���� ������.
	// R*dR�� R��ŭ ȸ���� ��ǥ�踦 dR��ŭ �� ȸ���ϰ� �ȴ�.
	R = R*dR;

	dMatrix g = dMatrix (3,1, accl);
	dMatrix m = dMatrix (3,1, magn);

	// accl�� magn�� ���� ��ǥ�踦 �������� ������ ���̴�.
	// ������ �ڼ�(R)�� ���ؼ� ������ǥ�踦 �������� �� ������ �ٲ��ش�.
	g = R*g;
	m = R*m;

	// �߷°��ӵ� ���� 1��ó�� ���� �� �̵�(k1)�� ū ���� �������� �Ѵ�.
	double l = sqrt (accl[0]*accl[0] + accl[1]*accl[1] + accl[2]*accl[2]) - 1.;
	double sigma = 0.1;
	double k1 = 0.1*exp(-l*l/(sigma*sigma));

#if defined ADIS_16405
	double k2 = 0.1;

	// ������ �������� ����Ѵ�.
	double dPhi   = g(2,0) ? atan (g(1,0)/g(2,0)) : 0.;
	double dTheta = (-1 < g(0,0) && g(0,0) < 1) ? asin(-g(0,0)/-1.) : 0.;
	double dPsi   = -atan2 ( m(1,0),  m(0,0));
#else
	double k2 = 0.;

	// ������ �������� ����Ѵ�.
	double dPhi   = g(2,0) ? atan (g(1,0)/g(2,0)) : 0.;
	double dTheta = (-1 < g(0,0) && g(0,0) < 1) ? asin(-g(0,0)/-1.) : 0.;
	double dPsi   = 0.;
#endif

	adjust_rpy[0] = dPhi;
	adjust_rpy[1] = dTheta;
	adjust_rpy[2] = dPsi;

	// ���Ϸ������� ����� �������� ȸ����ķ� ��ȯ�Ѵ�.
	dMatrix aR = RotationMatrix (k1*dPhi, k1*dTheta, k2*dPsi);

	// ���� ������(aR)�� ������ǥ�踦 �������� ���� ���̴�.
	// �׷��Ƿ� R�� �տ��� aR�� ���ؾ� �Ѵ�.
	// R*aR �� ���� ���� �ʵ��� �����Ѵ�.
	R = aR*R;

	return R;
}
