/*
 * app.c
 *
 *  Created on: Aug 14, 2022
 *      Author: TK
 */

#include "app.h"

extern FDCAN_HandleTypeDef hfdcan1;

extern UART_HandleTypeDef huart2;

uint8_t counter = 0;


void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
  FDCAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];
  HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rx_header, rx_data);

  uint8_t is_get_request = rx_header.RxFrameType == FDCAN_REMOTE_FRAME || rx_header.DataLength == FDCAN_DLC_BYTES_0;

  uint16_t func_id = rx_header.Identifier >> 4;

  FDCAN_TxHeaderTypeDef tx_header;
  uint8_t tx_data[8];

  tx_header.Identifier = rx_header.Identifier;
  tx_header.IdType = FDCAN_STANDARD_ID;
  tx_header.TxFrameType = FDCAN_DATA_FRAME;
  tx_header.DataLength = FDCAN_DLC_BYTES_4;
  tx_header.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
  tx_header.BitRateSwitch = FDCAN_BRS_OFF;
  tx_header.FDFormat = FDCAN_CLASSIC_CAN;
  tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  tx_header.MessageMarker = 0;

  switch (func_id) {
    case 0x00:
      tx_header.DataLength = FDCAN_DLC_BYTES_1;
      tx_data[0] = 0xDA;
      break;
    case 0x7F:
      tx_header.DataLength = FDCAN_DLC_BYTES_4;
      tx_data[0] = 0x01;
      tx_data[1] = 0x23;
      tx_data[2] = 0x45;
      tx_data[3] = 0x67;
      break;

  }

  if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, tx_data) != HAL_OK) {
    //   HAL_UART_Transmit(&huart2, (uint8_t *) "CAN TX Error\r\n", strlen("CAN TX Error\r\n"), 100);
  }
}



void APP_init() {

  FDCAN_FilterTypeDef filter_config;
  filter_config.IdType = FDCAN_STANDARD_ID;
  filter_config.FilterIndex = 0;
  filter_config.FilterType = FDCAN_FILTER_MASK;
  filter_config.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter_config.FilterID1 = 0;
  filter_config.FilterID2 = 0;

  HAL_FDCAN_ConfigFilter(&hfdcan1, &filter_config);

if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    while (1)
    HAL_UART_Transmit(&huart2, (uint8_t *) "CAN init Error\r\n", strlen("CAN init Error\r\n"), 100);
  }

  HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

}

void APP_main() {



//  HAL_UART_Transmit(&huart2, (uint8_t *) "loop\r\n", strlen("loop\r\n"), 100);

  counter += 1;
}
