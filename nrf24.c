/*
 * nrf24.c
 *
 *  Created on: Apr 14, 2026
 *      Author: PSS
 */

#include<stdint.h>
#include<nrf24.h>
#include"main.h"
#include<stdio.h>

extern SPI_HandleTypeDef hspi1;
#define CE GPIO_PIN_0
#define CE_PORT GPIOB
#define CSN GPIO_PIN_1
#define CSN_PORT GPIOB
#define NRF24_SPI &hspi1



void CE_EN(){
	HAL_GPIO_WritePin(CE_PORT,CE,GPIO_PIN_SET);
}

void CE_DIS(){
	HAL_GPIO_WritePin(CE_PORT,CE,GPIO_PIN_RESET);
}

void CSN_Select(){
	HAL_GPIO_WritePin(CSN_PORT,CSN,GPIO_PIN_RESET);
}

void CSN_UnSelect(){
	HAL_GPIO_WritePin(CSN_PORT,CSN,GPIO_PIN_SET);
}

void nrf24_Write_Reg(uint8_t reg,uint8_t data){
	uint8_t buf[2];
	buf[0] = reg|(1<<5);
	buf[1] = data;

	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,buf,2,500);

	CSN_UnSelect();
}

void nrf24_Write_Reg_Multi(uint8_t reg,uint8_t *data,int size){
	uint8_t buf[2];
	buf[0] = reg|(1<<5);
	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,buf,1,500);
	HAL_SPI_Transmit(NRF24_SPI,data,size,500);

	CSN_UnSelect();
}

uint8_t nrf24_Read_Reg(uint8_t reg){
	uint8_t data=0;
	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,&reg,1,100);
	HAL_SPI_Receive(NRF24_SPI,&data,1,100);

	CSN_UnSelect();
	return data;
}

void nrf24_Read_Reg_Multi(uint8_t reg,uint8_t *data,int size){
	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,&reg,1,100);
	HAL_SPI_Receive(NRF24_SPI,data,size,100);

	CSN_UnSelect();

}

void nrf24_Write_Cmd(uint8_t cmd){
	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,&cmd,1,100);

	CSN_UnSelect();
}

void nrf24_Write_Cmd_Data(uint8_t cmd,uint8_t *data){
	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,&cmd,1,100);
	HAL_SPI_Transmit(NRF24_SPI,data,32,1000);

	CSN_UnSelect();
}
void nrf24_Read_Cmd_Data(uint8_t cmd,uint8_t *data){
	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,&cmd,1,100);
	HAL_SPI_Receive(NRF24_SPI,data,32,1000);

	CSN_UnSelect();
}

void nrf24_init(uint8_t speed,uint8_t pa){
	CE_DIS();
	CSN_UnSelect();
	HAL_Delay(5);

	nrf24_Write_Reg(CONFIG,0x0E);

	nrf24_Write_Reg(EN_AA,0x01);
	nrf24_Write_Reg(EN_RXADDR,0x01);

	nrf24_Write_Reg(SETUP_AW,0x03);
	nrf24_Write_Reg(SETUP_RETR,0x0);

	nrf24_Write_Reg(RF_CH,115);
	uint8_t rf_setup;
	if(speed == nrf24_250Kbps){
		rf_setup = (1<<5)|pa;
	}
	else{
		rf_setup = (speed<<3)|pa;
	}
	nrf24_Write_Reg(RF_SETUP,rf_setup);



}

void nrf24_TX(uint8_t* addr,uint8_t ch){
	CE_DIS();
	//nrf24_Write_Reg(RF_SETUP,0x26);
	nrf24_Write_Reg(RF_CH,ch);
	nrf24_Write_Reg(EN_RXADDR,0x01);
	nrf24_Write_Reg(EN_AA,0x0);
	nrf24_Write_Reg_Multi(TX_ADDR,addr,5);
	//nrf24_Write_Reg_Multi(RX_ADDR_P0,addr,5);

	uint8_t config = nrf24_Read_Reg(CONFIG);
	config = config | (1<<1);
	config &= ~(1<<0);
	nrf24_Write_Reg(CONFIG,config);


	HAL_Delay(5);



}

void nrf24_RX(uint8_t* addr,uint8_t ch){
	CE_DIS();
	//nrf24_Write_Reg(RF_SETUP,0x26);
	nrf24_Write_Reg(RF_CH,ch);

	uint8_t rx_addr = nrf24_Read_Reg(EN_RXADDR);
	rx_addr = rx_addr | (1<<1);
	nrf24_Write_Reg(EN_RXADDR,rx_addr);

	nrf24_Write_Reg_Multi(RX_ADDR_P1,addr,5);

	nrf24_Write_Reg(RX_PW_P1,32);

	nrf24_Write_Reg(EN_AA,0x0);
	nrf24_Write_Reg_Multi(TX_ADDR,addr,5);
	//nrf24_Write_Reg_Multi(RX_ADDR_P0,addr,5);

	uint8_t config = nrf24_Read_Reg(CONFIG);
	config = config | (1<<1) | (1<<0);
	nrf24_Write_Reg(CONFIG,config);


	HAL_Delay(5);
	CE_EN();

}

uint8_t is_data_Ready(uint8_t pipe){
	uint8_t status = nrf24_Read_Reg(STATUS);
	if(status & (1<<6) && (status&(pipe<<1))){
		nrf24_Write_Reg(STATUS,1<<6);
		return 1;
	}
	else{
		return 0;
	}
}



uint8_t nrf24_Transmit(uint8_t *data){
	CE_DIS();
	nrf24_Write_Reg(STATUS,0x70);

	nrf24_Write_Cmd_Data(W_TX_PAYLOAD,data);

	CE_EN();
	HAL_Delay(10);
	CE_DIS();

	uint8_t fifo_status = nrf24_Read_Reg(FIFO_STATUS);
	uint8_t status = nrf24_Read_Reg(STATUS);
	if(status & (1<<5)){
		//printf("transmitted\n");
		nrf24_Write_Reg(STATUS,1<<5);
		return 1;
	}
	if((status&(1<<4)) && (fifo_status & (1<<4)) ){;
		nrf24_Write_Reg(STATUS,1<<4);
		nrf24_Write_Cmd(FLUSH_TX);
		return 0;
	}


}

void nrf24_Receive(uint8_t* data){
	nrf24_Read_Cmd_Data(R_RX_PAYLOAD,data);
	HAL_Delay(5);
	nrf24_Write_Cmd(FLUSH_RX);
}
void nrf24_debug(){
	printf("----------------------------------------\n");
	printf("config			0x%x\n", nrf24_Read_Reg(CONFIG));
	printf("EN_AA			0x%x\n", nrf24_Read_Reg(EN_AA));
	printf("RXADDR			0x%x\n", nrf24_Read_Reg(EN_RXADDR));
	printf("SETUP_AW		0x%x\n", nrf24_Read_Reg(SETUP_AW));
	printf("SETUP_RETR		0x%x\n", nrf24_Read_Reg(SETUP_RETR));
	printf("RF_CH			0x%x\n", nrf24_Read_Reg(RF_CH));
	printf("RF_SETUP		0x%x\n", nrf24_Read_Reg(RF_SETUP));
	printf("STATUS			0x%x\n", nrf24_Read_Reg(STATUS));
	uint8_t buf[5];
	nrf24_Read_Reg_Multi(TX_ADDR,buf,5);
	printf("TX_ADDR			");
	for(int i=0;i<5;i++){
		printf("0x%x ",buf[i]);
	}
	printf("\n");
	printf("FIFIO_STATUS		0x%x\n", nrf24_Read_Reg(FIFO_STATUS));
	printf("----------------------------------------\n");

}
