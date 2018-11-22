/**
  ******************************************************************************
  * @file    STM32F0xx_IAP/inc/common.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-May-2012
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#ifndef __YMODEM_EXPORT_H__
#define __YMODEM_EXPORT_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "ff.h"
#ifdef PLATFORM_ZYNQ
#include "xuartps.h"
#include "xil_printf.h"
#define UART_DEVICE_ID              XPAR_XUARTPS_1_DEVICE_ID
#define Y_MODEM_BAUD_RATE (460800)
#endif

/********************************************************************************
 *                   The  Public APIs                                           *
 ********************************************************************************/
void Ymodem_Main_Entrance(void);

void logd(char * log, ...);
void logi(char * log, ...);
void logw(char * log, ...);

/********************************************************************************
 *                   The  Porting APIs Must Be Implemented                      *
 ********************************************************************************/
uint32_t   SerialKeyPressed(unsigned char *key);
void             SerialPutChar(unsigned char c);
unsigned char    SerialReadByte(void);

void             ymodem_init(void);
uint32_t    ymodem_get_receive_maxsize(void);
uint32_t     ymodem_get_transmit_size(void);

//0 - success ; -1 - fail
int              ymodem_recv_start_cb(const char * filename, const uint32_t filesize);
int              ymodem_recv_processing_cb(const uint8_t * buffer, const uint32_t buff_size);
int              ymodem_recv_end_cb(void);

#endif  /* __YMODEM_EXPORT_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
