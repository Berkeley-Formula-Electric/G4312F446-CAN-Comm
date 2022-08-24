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

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];
  HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, rx_data);
  if (rx_header.DLC != 0) {
    HAL_UART_Transmit(&huart2, (uint8_t *) "CAN RX\r\n", strlen("CAN RX\r\n"), 100);
  }
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

}

void APP_main() {

  uint32_t tx_mailbox;

  CAN_TxHeaderTypeDef tx_header;
  tx_header.DLC = 0;
  tx_header.IDE = CAN_ID_STD;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.StdId = 0x002;
  tx_header.TransmitGlobalTime = DISABLE;

  uint8_t tx_data[8];
  tx_data[0] = counter;
  tx_data[1] = 0x07;
  tx_data[2] = 0x08;
  tx_data[3] = 0x09;

  if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox) != HAL_OK) {
   HAL_UART_Transmit(&huart2, (uint8_t *) "CAN TX Error\r\n", strlen("CAN TX Error\r\n"), 100);
  }

  HAL_Delay(1000);
}
