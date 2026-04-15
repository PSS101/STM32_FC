/*
 * nrf24.h
 *
 *  Created on: Apr 14, 2026
 *      Author: PSS
 */

#ifndef INC_NRF24_H_
#define INC_NRF24_H_

#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02

#define SETUP_AW 0x03
#define SETUP_RETR 0x04

#define RF_CH 0x05
#define RF_SETUP 0x06

#define STATUS 0x07
#define OBSERVE_TX 0x08
#define RPD 0x09

#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F

#define TX_ADDR 0x10

#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16

#define FIFO_STATUS 0x17
#define DYNPD 0x1C
#define FEATURE 0x1D

#define R_REGISTER 0b00000000
#define W_REGISTER 0b00100000
#define R_RX_PAYLOAD 0b01100001
#define W_TX_PAYLOAD 0b10100000
#define FLUSH_TX 0b11100001
#define FLUSH_RX 0b11100011
#define  R_RX_PL_WID 0b01100000
#define W_ACK_PAYLOAD 0b10101000
#define W_TX_PAYLOAD_NOACK 0b10110000
#define NOP 0b11111111

void CE_EN();
void CE_DIS();

void CSN_Select();
void CSN_UnSelect();

void nrf24_Write_Reg(uint8_t reg,uint8_t data);
void nrf24_Write_Reg_Multi(uint8_t reg,uint8_t *data,int size);

uint8_t nrf24_Read_Reg(uint8_t reg);
void nrf24_Read_Reg_Multi(uint8_t reg,uint8_t *data,int size);

void nrf24_Write_Cmd(uint8_t cmd);
void nrf24_Write_Cmd_Data(uint8_t cmd,uint8_t *data);
void nrf24_init();

void nrf24_TX(uint8_t*addr,uint8_t ch);
uint8_t nrf24_Transmit(uint8_t *data);

void nrf24_debug();
#endif /* INC_NRF24_H_ */
