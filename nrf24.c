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
	buf[0] = reg|1<<5;
	buf[1] = data;

	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,buf,2,500);

	CSN_UnSelect();
}

void nrf24_Write_Reg_Multi(uint8_t reg,uint8_t *data,int size){
	uint8_t buf[2];
	buf[0] = reg|1<<5;

	CSN_Select();

	HAL_SPI_Transmit(NRF24_SPI,buf,2,500);
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

void nrf24_init(){
	CE_DIS();
	CSN_UnSelect();
	HAL_Delay(5);

	nrf24_Write_Reg(CONFIG,0x0A);

	nrf24_Write_Reg(EN_AA,0x0);
	nrf24_Write_Reg(EN_RXADDR,0x01);

	nrf24_Write_Reg(SETUP_AW,0x03);
	nrf24_Write_Reg(SETUP_RETR,0x0);

	nrf24_Write_Reg(RF_CH,0x0);
	nrf24_Write_Reg(RF_SETUP,0x0E);

	nrf24_Write_Reg(STATUS,0x70);


}

void nrf24_TX(uint8_t* addr,uint8_t ch){
	CE_DIS();

	nrf24_Write_Reg(RF_CH,ch);
	nrf24_Write_Reg_Multi(TX_ADDR,addr,5);
	nrf24_Write_Reg_Multi(RX_ADDR_P0,addr,5);

	uint8_t config = nrf24_Read_Reg(CONFIG);
	config = config | (1<<1);
	config &= ~(1<<0);
	nrf24_Write_Reg(CONFIG,config);


	HAL_Delay(2);

}

uint8_t nrf24_Transmit(uint8_t *data){
	uint8_t cmd = 0;
	cmd = W_TX_PAYLOAD;
	nrf24_Write_Cmd_Data(cmd,data);


	CE_EN();
	HAL_Delay(1);
	CE_DIS();


	uint8_t status = nrf24_Read_Reg(STATUS);
	if( (status & (1<<5) ) ){
		nrf24_Write_Reg(status,(1<<5));
		return 1;
	}
	else{
		return 0;
	}

}

void nrf24_debug(){
    uint8_t config    = nrf24_Read_Reg(CONFIG);    // should be 0x0A
    uint8_t setup_aw  = nrf24_Read_Reg(SETUP_AW);  // should be 0x03
    uint8_t rf_ch     = nrf24_Read_Reg(RF_CH);     // your channel
    uint8_t rf_setup  = nrf24_Read_Reg(RF_SETUP);  // should be 0x0E
    uint8_t status    = nrf24_Read_Reg(STATUS);    // should be 0x0E at startup
    uint8_t fifo      = nrf24_Read_Reg(FIFO_STATUS); // should be 0x11 (both empty)

    printf("CONFIG:    0x%02X\r\n", config);
    printf("SETUP_AW:  0x%02X\r\n", setup_aw);
    printf("RF_CH:     0x%02X\r\n", rf_ch);
    printf("RF_SETUP:  0x%02X\r\n", rf_setup);
    printf("STATUS:    0x%02X\r\n", status);
    printf("FIFO:      0x%02X\r\n", fifo);
    // STATUS register default value at power on should be 0x0E
    nrf24_Write_Reg(CONFIG, 0x0A);
    uint8_t val = nrf24_Read_Reg(CONFIG);
    printf("0x%02X",val);
}
