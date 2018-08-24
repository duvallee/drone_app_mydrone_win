#include "StdAfx.h"
#include "MatrixAlgebra.h"
#include "Quaternion.h"
#include "mm.h"

static Quaternion _Q(0,0,0);		// roll, pitch, yaw ���� ������ ���ʹϾ�
double average_gyro[3] = {0, 0, 0, };	// ���ӵ��� ��� ���� �����Ѵ�. ȭ�鿡 ǥ���ϱ� ���� ���������� ������.
double adjust_rpy[3] = {0, 0, 0, };		// ���ӵ��������� ������ �߷��� �������κ��� ������ roll, pitch, yaw ��. ���⼭�� ������

void InitARS()
{
	// ���� ���ʹϾ����� �ʱ�ȭ �Ѵ�.

	_Q.s_ = 1.;
	_Q.v_[0] = 0.;
	_Q.v_[1] = 0.;
	_Q.v_[2] = 0.;
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
	// �׸��� ������ ��ȭ���� 
	Quaternion dQ(
		_DEG2RAD*(gyro[0] - average_gyro[0])*dt, 
		_DEG2RAD*(gyro[1] - average_gyro[1])*dt, 
		_DEG2RAD*(gyro[2] - average_gyro[2])*dt);
	
	// ���� ������ �ڼ��� ��Ÿ���� ���ʹϾ�(_Q)�� �ڼ� ��ȭ�� ���ʹϾ��� ���ؼ�
	// �ڼ��� ��ȭ�� ������Ʈ �Ѵ�.
	//_Q = _Q * dQ;

	// ������ �������� �ʴ� ��Ȳ���� �׻� �߷� ���ӵ��� �Ʒ� ����(-z)���� �ۿ��ϰ� �ֱ�
	// ������, ������ �������� ���� ���� ���ӵ� ����(g)�� �߷°� �����ϴ�. �׷���
	// ���ӵ� ������ �ڼ��κ��� ���� �ڼ� _Q�� roll�� pitch�� ������ �� �ִ�.
	dVector g0 = dVector3(0., 0., -1.);
	Quaternion Qa (0, accl[0], accl[1], accl[2]);

	// g�� ���� ��ǥ�踦 �������� �� �����̹Ƿ�, �̵��� ���� ��ǥ�� �������� ǥ���ϱ� 
	// ���ؼ��� _Q�� ���ؼ� ���� ��ǥ�� �������� ��ȯ�Ѵ�.
	Quaternion Qg = _Q * Qa * _Q.i();

	// ���� ��ǥ�迡�� ���ӵ� ������ ������ �߷� ����(Qg)�� �̻����� �߷� ����(g0)�� �̷�� 
	// normal vector(ng)�� ���հ�(alpha)�� ���Ѵ�.
	dVector ng = Cross (Qg.v(), g0);
	double alpha = asin(Norm2(ng)/Norm2(Qg.v()));

	// �߷����� ã�� ������ ������ ������Ʈ �ϴ� ����(�̵�)�� ���Ѵ�. 
	// �߷� ������ ũ�Ⱑ 1.��ó�� �� �̵��� Ŀ���ϰ� 1.���� �־��� ���� �̵��� ����� �Ѵ�.
	double l = sqrt (accl[0]*accl[0] + accl[1]*accl[1] + accl[2]*accl[2]) - 1.;
	double sigma = 0.1;
	double Kg = 0.1*exp(-l*l/(sigma*sigma));

	_Q = Quaternion (Kg*alpha, ng) * _Q;

#if defined ADIS_16405
	{
		// ���ڱ�� �׻� ������ �������� �ۿ��ϰ� �ֱ⶧����, ���ڱ� ������ ���ڱ� 
		// ����(m)�δ� yaw�� ������ �� �ִ�.
		Quaternion Qm (0, magn[0], magn[1], magn[2]);

		// m�� ���� ��ǥ�踦 �������� �� �����̹Ƿ�, �̵��� ���� ��ǥ�� �������� ǥ���ϱ� 
		// ���ؼ��� _Q�� ���ؼ� ���� ��ǥ�� �������� ��ȯ�Ѵ�.
		Qm = _Q * Qm * _Q.i();

		dVector mn (0., 0., 1.);
		double alpha = -atan2 (Qm.v()[1], Qm.v()[0]);
		const double Ka = 0.1;
		
		_Q = Quaternion (Ka*alpha, mn) * _Q;
	}
#endif

	return _Q.RotationMatrix ();
}
