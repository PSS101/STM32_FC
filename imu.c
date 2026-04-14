/*
 * imu.c
 *
 *  Created on: Apr 14, 2026
 *      Author: PSS
 */
#include<main.h>
#include<imu.h>
#include<stdio.h>
#include<math.h>
extern I2C_HandleTypeDef hi2c1;
HAL_StatusTypeDef dr;
uint8_t temp;

imu_of of;


void mpu6050_init(){
	printf("----------------------------------------\n");
	 dr = HAL_I2C_IsDeviceReady(&hi2c1,0x68 <<1 ,1,100);
	if(dr == HAL_OK){
	  printf("imu ready\n");
	}
	else{
	  printf("imu not ready\n");
	}

	 temp = 0b00000000;
	dr = HAL_I2C_Mem_Write(&hi2c1, 0x68 <<1, 107, 1, &temp, 1,50);
	if(dr == HAL_OK){
		printf("exited sleep mode\n");
	}
	else{
		printf("failed to exit sleep mode\n");
	}


	temp = 0b00001000;
	dr = HAL_I2C_Mem_Write(&hi2c1, 0x68 <<1, 27, 1, &temp, 1,50);
	if(dr == HAL_OK){
		printf("gyro set to 500 d/s\n");
	}
	else{
		printf("failed to set gyro\n");
	}

	temp = 0b00010000;
	dr = HAL_I2C_Mem_Write(&hi2c1, 0x68 <<1, 28, 1, &temp, 1,50);
	if(dr == HAL_OK){
		printf("acc set to 8 g\n");
	}
	else{
		printf("failed set acc\n");
	}
	printf("----------------------------------------\n");

}

void mpu6050_interrupt_enable(){
	temp = 0b00000001;
	dr = HAL_I2C_Mem_Write(&hi2c1, 0x68 <<1, 56, 1, &temp, 1,50);
	if(dr == HAL_OK){
		printf("interrupt enabled\n");
	}
	else{
		printf("failed to set interrupt\n");
	}
	printf("----------------------------------------\n");
}

void mpu6050_calib(){
	printf("imu calib\n");
	float gx=0,gy=0,gz=0,ax=0,ay=0;
	for(int i=0;i<20000;i++){
		imu_acc a = mpu6050_read_acc();
		imu_g g = mpu6050_read_g();
		gx += g.g_x;
		gy += g.g_y;
		gz += g.g_z;

		ax += a.acc_x;
		ay += a.acc_y;
	}
	gx/=20000;gy/=20000;gz/=20000;
	ax/=20000;ay/=20000;
	printf("imu calib complete\n");

	mpu6050_offsets(gx,gy,gz,ax,ay);
	printf("----------------------------------------\n");
}

void mpu6050_offsets(float gx,float gy,float gz,float ax,float ay){
	of.gx_e = gx;
	of.gy_e = gy;
	of.gz_e = gz;
	of.ax_e = ax;
	of.ay_e = ay;
	printf("offsets\n");
	printf("%f,%f,%f,%f,%f\n",gx,gy,gz,ax,ay);
	printf("imu offsets set\n");
	printf("----------------------------------------\n");
}


imu_acc mpu6050_read_acc(){
	uint8_t data[6];
	 imu_acc acc={0};
	if(HAL_I2C_Mem_Read(&hi2c1, 0x68 <<1, 59, 1, data, 6, 50)==HAL_OK){
		int16_t acc_x_raw = (int16_t)(data[0]<<8|data[1]);
		acc.acc_x = ((float)acc_x_raw/4096.0f) - of.ax_e;

		int16_t acc_y_raw = (int16_t)(data[2]<<8|data[3]);
		acc.acc_y = ((float)acc_y_raw/4096.0f) - of.ay_e;

		int16_t acc_z_raw = (int16_t)(data[4]<<8|data[5]);
		acc.acc_z = (float)acc_z_raw/4096.0f;

		return acc;
	}


}

imu_g mpu6050_read_g(){
	uint8_t data[6];
	imu_g g={0};
	if(HAL_I2C_Mem_Read(&hi2c1, 0x68 <<1, 67, 1, data, 6, 50)==HAL_OK){
		int16_t g_x_raw = (int16_t)(data[0]<<8|data[1]);
		g.g_x = ((float)g_x_raw/65.5f) - of.gx_e;

		int16_t g_y_raw = (int16_t)(data[2]<<8|data[3]);
		g.g_y = ((float)g_y_raw/65.5f) - of.gy_e;

		int16_t g_z_raw = (int16_t)(data[4]<<8|data[5]);
		g.g_z = ((float)g_z_raw/65.5f) - of.gz_e;

		return g;
	}

}

float mpu6050_pitch(imu_acc a){
	float pitch_raw =  atan2( (-a.acc_x), sqrt((a.acc_y*a.acc_y)+(a.acc_z*a.acc_z)));
	return (pitch_raw*57.2958f);
}

float mpu6050_roll(imu_acc a){
	float roll_raw  =  atan2( (a.acc_y), sqrt((a.acc_x*a.acc_x)+(a.acc_z*a.acc_z)));
	return (roll_raw*57.2958f);
}
