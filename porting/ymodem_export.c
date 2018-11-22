/**
 ******************************************************************************
 * @file    STM32F0xx_IAP/src/common.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    29-May-2012
 * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "ymodem_export.h"

/** @addtogroup STM32F0xx_IAP
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/********************************************************************************
 *                     Porting APIs
 ********************************************************************************/
typedef enum {
	TRANS_FILE = 0, IAP_IMAGE = 1,
} YMODEM_ACTION_E;

typedef struct {
	XUartPs ymodem_uart;
	XUartPs_Config * ymodem_uart_config_p;
	YMODEM_ACTION_E ymodem_action;
	uint32_t ymodem_receive_supported_maxsize;
	uint32_t ymodem_transmit_size;

	bool ymodem_file_inited;
	uint32_t ymodem_file_total_size;
	uint32_t ymodem_file_handled_size;
	FATFS ymodem_drive_handle;
	FIL ymodem_file_handle;
	uint8_t ymodem_file_tmppath[_MAX_LFN];
} YMODEM_DATA_T;

static YMODEM_DATA_T s_ymodem_data;

void ymodem_init(void) {
	memset(&s_ymodem_data, 0, sizeof(YMODEM_DATA_T));

	//don't wait for a line
#ifdef USE_STDIO
	setvbuf(stdin, NULL, _IONBF, 0);
#else
#ifdef PLATFORM_ZYNQ

	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	s_ymodem_data.ymodem_uart_config_p = XUartPs_LookupConfig(UART_DEVICE_ID);
	assert(NULL !=s_ymodem_data.ymodem_uart_config_p);

	int Status = XUartPs_CfgInitialize(&s_ymodem_data.ymodem_uart,
			s_ymodem_data.ymodem_uart_config_p,
			s_ymodem_data.ymodem_uart_config_p->BaseAddress);
	assert(XST_SUCCESS == Status);


	/* Check hardware build. */
	Status = XUartPs_SelfTest(&s_ymodem_data.ymodem_uart);
	assert(Status == XST_SUCCESS);


	Status = XUartPs_SetBaudRate(&s_ymodem_data.ymodem_uart, Y_MODEM_BAUD_RATE);
	assert(Status == XST_SUCCESS);
	/* Ensure not in loopback*/
	XUartPs_SetOperMode(&s_ymodem_data.ymodem_uart, XUARTPS_OPER_MODE_NORMAL);

#endif
#endif

	s_ymodem_data.ymodem_action = TRANS_FILE;

	switch (s_ymodem_data.ymodem_action) {
	case TRANS_FILE:
		s_ymodem_data.ymodem_receive_supported_maxsize = 1024 * 1024 * 1024;
		s_ymodem_data.ymodem_transmit_size = 0;
		break;

	case IAP_IMAGE:
#warning "MUST implement GOT transmit SIZE!"
		s_ymodem_data.ymodem_receive_supported_maxsize = 1024 * 1024 * 5;
		s_ymodem_data.ymodem_transmit_size = 0;
		break;

	default:
		//Not supported
		break;
	}
}
uint32_t ymodem_get_receive_maxsize(void) {
	return s_ymodem_data.ymodem_receive_supported_maxsize;
}

uint32_t ymodem_get_transmit_size(void) {
	return s_ymodem_data.ymodem_transmit_size;
}

//0 - success ; -1 - fail
int ymodem_recv_start_cb(const char * filename, const uint32_t filesize) {
	FRESULT fres;
	const char * prefix = (const char*) "0:/";
	char *filepath = (char*) &s_ymodem_data.ymodem_file_tmppath[0];
	uint32_t filepath_size = sizeof(s_ymodem_data.ymodem_file_tmppath);

	switch (s_ymodem_data.ymodem_action) {
	case TRANS_FILE:
		if (strlen(prefix) + strlen((char*) filename) >= filepath_size) {
			return -1;      //path length too long
		}
		memset(&s_ymodem_data.ymodem_file_tmppath[0], 0, filepath_size);
		filepath = strcat((char*) filepath, prefix);
		filepath = strcat((char*) filepath, (char*) filename);

		static uint32_t fs_mounted = 0;

		/*
		 * only mount on the first time
		 */
		if (0 == fs_mounted++) {
			/*
			 * Register volume work area, initialize device
			 */
			FRESULT res = f_mount(&s_ymodem_data.ymodem_drive_handle, prefix,
					0);

			if (FR_OK != res) {
				fs_mounted = 0;
				return -1;
			}
		}

		if (FR_OK != (fres = f_open(&s_ymodem_data.ymodem_file_handle, filepath,
		FA_WRITE | FA_CREATE_ALWAYS))) {
			xil_printf("Ymodem Start: create file(%s) fail(%d)...\n", filepath,
					fres);
			return -1;
		} else {
			xil_printf("Ymodem Start: create file(%s) ok(%d)...\n", filepath,
					fres);

			s_ymodem_data.ymodem_file_inited = TRUE;
			s_ymodem_data.ymodem_file_total_size = filesize;
			s_ymodem_data.ymodem_file_handled_size = 0;
			return 0;
		}
		//break;

	case IAP_IMAGE:
		/* erase user application area */
		//FLASH_If_Erase(APPLICATION_ADDRESS);
		break;

	default:
		//Not supported
		break;
	}
	return -1;
}

int ymodem_recv_processing_cb(const uint8_t * buffer, const uint32_t buff_size) {
	FRESULT fres;
	uint32_t to_write_size = 0;
	uint32_t writted_size = 0;

	switch (s_ymodem_data.ymodem_action) {
	case TRANS_FILE:
		to_write_size = s_ymodem_data.ymodem_file_total_size
				- s_ymodem_data.ymodem_file_handled_size;
		to_write_size = (buff_size > to_write_size) ? to_write_size : buff_size;

		if (FR_OK
				!= (fres = f_write(&s_ymodem_data.ymodem_file_handle, buffer,
						to_write_size, &writted_size))) {
			xil_printf("Ymodem process: write file(%d - %d) fail(%d)...\n",
					to_write_size, writted_size, fres);
			return -1;
		} else {
			s_ymodem_data.ymodem_file_handled_size += to_write_size;
			//xil_printf("Ymodem process: write file(%d/%d) ok(%d)...\n",	s_ymodem_data.ymodem_file_handled_size,	s_ymodem_data.ymodem_file_total_size, fres);
			return 0;
		}
		//break;

	case IAP_IMAGE:
		//if (FLASH_If_Write(&flashdestination, (uint32_t*) ramsource, (uint16_t) packet_length/4)  == 0)
		break;

	default:
		//Not supported
		break;
	}
	return -1;
}

int ymodem_recv_end_cb(void) {
	FRESULT fres;
	FILINFO fno;

	switch (s_ymodem_data.ymodem_action) {
	case TRANS_FILE:
		if (TRUE != s_ymodem_data.ymodem_file_inited)
			return -1;

		fres = f_close(&s_ymodem_data.ymodem_file_handle);
		xil_printf("Ymodem End: close file res(%d)...\n", fres);

		fres = f_stat((const TCHAR *) s_ymodem_data.ymodem_file_tmppath, &fno);
		if (fres != FR_OK) {
			xil_printf("Get File Status Fail(%d)...\n", fres);
		} else {
			xil_printf("Get File Status ok(%d), file size(%d) Bytes...\n", fres,
					fno.fsize);
		}

		memset(&s_ymodem_data.ymodem_file_handle, 0, sizeof(FIL));
		memset(&s_ymodem_data.ymodem_file_tmppath[0], 0,
				sizeof(s_ymodem_data.ymodem_file_tmppath));
		s_ymodem_data.ymodem_file_total_size = 0;
		s_ymodem_data.ymodem_file_handled_size = 0;
		s_ymodem_data.ymodem_file_inited = FALSE;
		return 0;
		//break;

	case IAP_IMAGE:
		/* erase user application area */
		//FLASH_If_Erase(APPLICATION_ADDRESS);
		break;

	default:
		//Not supported
		break;
	}
	return -1;
}
/**
 * @brief  Test to see if a key has been pressed on the HyperTerminal
 * @param  key: The key pressed
 * @retval 1: Correct
 *         0: Error
 */
uint32_t SerialKeyPressed(uint8_t *key) {

	uint8_t rx_byte = 0;
	uint32_t rx_try = 500;

	//wait 10ms at most
#ifdef USE_STDIO
	*key = getchar();
#else
#ifdef PLATFORM_ZYNQ
	static uint32_t debug_byte_idx = 0;
	while (0 == XUartPs_Recv(&s_ymodem_data.ymodem_uart, &rx_byte, 1)) {
		if (0 == (rx_try--)) {
			return 0;
		}
	}
	if (0x1 == rx_byte) {
		debug_byte_idx = 0;
	}
	//xil_printf("%u\t0x%x-0c%c-0d%d\r\n",debug_byte_idx++,rx_byte,rx_byte,rx_byte);
#else
#error "no serial device specified"
#endif
#endif

	*key = rx_byte;

	return 1;

}
//fail - 0xff; success -other value
uint8_t SerialReadByte(void) {

	uint8_t rx_byte = 0;
#ifdef USE_STDIO
	rx_byte = getchar();
#else
#ifdef PLATFORM_ZYNQ
	while (0 == XUartPs_Recv(&s_ymodem_data.ymodem_uart, &rx_byte, 1)) {
		/* NOP */
	}
	//xil_printf("%x-%c ",rx_byte,rx_byte);
#else
#error "no serial device specified"
#endif
#endif
	return rx_byte;
}

/**
 * @brief  Print a character on the HyperTerminal
 * @param  c: The character to be printed
 * @retval None
 */
void SerialPutChar(uint8_t c) {
	uint32_t send_count = 0;
	uint8_t byte_to_send = c;
#ifdef USE_STDIO
	return putchar(c);
#else
#ifdef PLATFORM_ZYNQ

	while (XUartPs_IsSending(&s_ymodem_data.ymodem_uart)) {
		/* NOP */
	}

	send_count = XUartPs_Send(&s_ymodem_data.ymodem_uart, &byte_to_send, 1);
	assert(1 == send_count);
	//xil_printf("t%x ",byte_to_send);
#else
#error "no serial device specified"
#endif
#endif
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
