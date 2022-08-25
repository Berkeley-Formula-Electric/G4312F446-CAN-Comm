/*
 * app.c
 *
 *  Created on: Aug 14, 2022
 *      Author: TK
 */

#include "app.h"

extern CAN_HandleTypeDef hcan1;
extern UART_HandleTypeDef huart2;

uint8_t counter = 0;

uint8_t error_code = 0;


#define SERIAL_BUFFER_SIZE    30

uint8_t serial_rx_buffer[SERIAL_BUFFER_SIZE];
uint8_t serial_tx_buffer[SERIAL_BUFFER_SIZE];



void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];
  HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, rx_data);

  serial_tx_buffer[0] = rx_header.StdId >> 8;
  serial_tx_buffer[1] = rx_header.StdId & 0xFF;
  serial_tx_buffer[2] = rx_header.DLC;
  serial_tx_buffer[3] = 0;

  for (uint8_t i=0; i<rx_header.DLC; i+=1) {
    serial_tx_buffer[i+4] = rx_data[i];
  }

  HAL_UART_Transmit_DMA(&huart2, serial_tx_buffer, rx_header.DLC+4);
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
  if (huart == &huart2) {

    CAN_TxHeaderTypeDef tx_header;
    tx_header.DLC = serial_rx_buffer[2];
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.StdId = ((uint16_t)serial_rx_buffer[0] << 8) | serial_rx_buffer[1];
    tx_header.TransmitGlobalTime = DISABLE;

    uint8_t *tx_data = serial_rx_buffer + 4 * sizeof(uint8_t);

    uint32_t tx_mailbox;

    if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox) != HAL_OK) {
      error_code = 1;
    }
  }

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, serial_rx_buffer, SERIAL_BUFFER_SIZE);
}

void APP_init() {

  uint32_t filter_id = 0;
  uint32_t filter_mask = 0x0;

  CAN_FilterTypeDef filter_config;
  filter_config.FilterBank = 0;
  filter_config.FilterMode = CAN_FILTERMODE_IDMASK;
  filter_config.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  filter_config.FilterIdHigh = filter_id << 5;
  filter_config.FilterIdLow = 0;
  filter_config.FilterMaskIdHigh = filter_mask << 5;
  filter_config.FilterMaskIdLow = 0;
  filter_config.FilterScale = CAN_FILTERSCALE_32BIT;
  filter_config.FilterActivation = ENABLE;
  filter_config.SlaveStartFilterBank = 14;

  HAL_CAN_ConfigFilter(&hcan1, &filter_config);

  if (HAL_CAN_Start(&hcan1) != HAL_OK) {
    while (1)
    HAL_UART_Transmit(&huart2, (uint8_t *) "CAN init Error\r\n", strlen("CAN init Error\r\n"), 100);
  }

  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, serial_rx_buffer, SERIAL_BUFFER_SIZE);

}

void APP_main() {
//  if (error_code == 1) {
//    HAL_UART_Transmit(&huart2, (uint8_t *) "HAL_CAN_AddTxMessage Error\r\n", strlen("HAL_CAN_AddTxMessage Error\r\n"), 100);
//  }

  HAL_Delay(1000);
}
