#pragma once

extern double average_gyro[3];
extern double adjust_rpy[3];

extern void InitARS();
extern dMatrix ARS (double gyro[3], double accel[3], double magn[3], double dt);

