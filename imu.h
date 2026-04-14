/*
 * imu.h
 *
 *  Created on: Apr 14, 2026
 *      Author: PSS
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

typedef struct{
		float acc_x;
		float acc_y;
		float acc_z;
}imu_acc;

typedef struct{
		float g_x;
		float g_y;
		float g_z;
}imu_g;

typedef struct{
		float gx_e;
		float gy_e;
		float gz_e;
		float ax_e;
		float ay_e;
}imu_of;

void mpu6050_init();

void mpu6050_calib();

void mpu6050_offsets(float gx,float gy,float gz,float ax,float ay);

imu_acc mpu6050_read_acc();

imu_g mpu6050_read_g();

void mpu6050_interrupt_enable();

float mpu6050_pitch(imu_acc a);
float mpu6050_roll(imu_acc a);

#endif /* INC_IMU_H_ */
