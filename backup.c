/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include "stdlib.h"
#include "platform.h"
#include "xuartps.h"
#include <stdio.h>
#include "xparameters.h" // System parameter definitions
#include "xaxidma.h" // Device driver API for AXI DMA
#include <assert.h>
#include "sleep.h"

#define conv_channel 1
#define conv_in_x 16
#define conv_in_y 16
#define conv_feat_map 6
#define conv_f_x 5
#define conv_f_y 5
#define conv_o_x 12
#define conv_o_y 12
#define output_size 10

//DMA instance
XAxiDma axiDma;
//UART driver instance
XUartPs uart;
//UART configuration instance
XUartPs_Config *Config;


// Helper function forward declarations
int init_dma(XAxiDma *axiDma);
static int init_uart();
void uart_receive(float in[conv_channel*conv_in_x*conv_in_y],int RecvCount);
void uart_send(float* res_hw,int num);


//Function to initialize the UART port
int init_uart(){
	int Status;
	//Look up the configuration in the config table and then initialize it.
	Config = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPs_CfgInitialize(&uart, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;

	//setting receiver reset after each operation
	//XUartPs_SetOptions(&uart,XUARTPS_OPTION_RESET_RX);
	//setting the sender reset after each operation
	//XUartPs_SetOptions(&uart,XUARTPS_OPTION_RESET_TX);
}

// A function that wraps all AXI DMA initialization related API calls
int init_dma(XAxiDma *axiDmaPtr){
   XAxiDma_Config *CfgPtr;
   int status;
   // Get pointer to DMA configuration
   CfgPtr = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
   if(!CfgPtr){
      print("Error looking for AXI DMA config\n\r");
      return XST_FAILURE;
   }
   // Initialize the DMA handle
   status = XAxiDma_CfgInitialize(axiDmaPtr,CfgPtr);
   if(status != XST_SUCCESS){
      print("Error initializing DMA\n\r");
      return XST_FAILURE;
   }
   //check for scatter gather mode - this example must have simple mode only
   if(XAxiDma_HasSg(axiDmaPtr)){
	  print("Error DMA configured in SG mode\n\r");
	  return XST_FAILURE;
   }
   //disable the interrupts
   XAxiDma_IntrDisable(axiDmaPtr, XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DEVICE_TO_DMA);
   XAxiDma_IntrDisable(axiDmaPtr, XAXIDMA_IRQ_ALL_MASK,XAXIDMA_DMA_TO_DEVICE);

   return XST_SUCCESS;
}

//function to receive from UART port
void uart_receive(float* in,int RecvCount){
	u8 buff[RecvCount];
	int counter = 0;
	while(counter < RecvCount){
		while (!XUartPs_IsReceiveData(Config->BaseAddress));
			;//waiting for input
		XUartPs_Recv(&uart,&buff[counter],1);
		counter++;
	}
	in =(float*) buff;
}

//function to send data to UART port
void uart_send(float* res_hw,int num){
	u8* buff = (u8*)res_hw;
	int counter = 0;
	while(counter < num){
		while(!XUartPs_IsTransmitEmpty(&uart));
			;//waiting for other transfers to end
		XUartPs_Send(&uart,&buff[counter],1);
		counter++;
	}
}


// Program entry point
int main()
{
	init_platform();

	int status;

   	//Initialize uart port
   	status = init_uart(uart);
   	if(status == XST_FAILURE){
	   	printf("ERROR INITIALIZING UART PORT...EXITING...\n");
	   	exit(-1);
   	}
	
	// Initialize the (simple) DMA engine
   	status = init_dma(&axiDma);
   	if (status != XST_SUCCESS) {
	  	exit(-1);
   	}

	//Disabling the cache
	Xil_DCacheDisable();
	Xil_ICacheDisable();


	int i=0;
   	int j = 0;
	//always available when started
	float* in = malloc(conv_channel * conv_in_x * conv_in_y * sizeof(float));
	float* result = malloc(output_size * sizeof(float));

	//receiving result from PL
			//status = XAxiDma_SimpleTransfer(&axiDma,(u32)result,sizeof(float) * output_size,
				//	XAXIDMA_DEVICE_TO_DMA);

	for(i=0;i<10;i++){
			//	result[i]= i;
		   		printf("%f\n",(float)result[i]);
		   	}

   	while(j++<5){


    	/*for(i=0;i<10;i++){
			printf("%f\n",(float)result[i]);
		}*/

    	for(i=0;i<256;i++){
			in[i] = (float)/*i/100;*/((i*j)+2+i)/1000;
    	}

	   	//receiving input
	   	//uart_receive(in,(conv_channel*conv_in_x*conv_in_y)*sizeof(float));

   		//printf("test is %c\n",test);

	   	printf("*** ATTEMPT NUMBER %d ***\n",j);

		//flushing the relative cache range
	   	//Xil_DCacheFlushRange((unsigned)in,(conv_channel*conv_in_x*conv_in_y) * sizeof(float));

	   	//Handling the DMA to PL input transfer
		status = XAxiDma_SimpleTransfer(&axiDma,(u32)in,sizeof(float) * (conv_channel*conv_in_x*conv_in_y),
					XAXIDMA_DMA_TO_DEVICE);


		usleep(3000000);

	    /*printf("BEFORE\n");
	    for(i=0;i<10;i++){
	    	   		printf("%f\n",(float)result[i]);
		}*/


	    //receiving result from PL
		status = XAxiDma_SimpleTransfer(&axiDma,(u32)result,sizeof(float) * output_size,
				XAXIDMA_DEVICE_TO_DMA);

		//invalidating the relative cache range in order to receive again
	    //Xil_DCacheInvalidateRange((unsigned)result,output_size * sizeof(float));

		//printing results on screen
		//printf("AFTER\n");
	   	for(i=0;i<10;i++){
		//	result[i]= i;
	   		printf("%f\n",(float)result[i]);
	   	}

		//sending the results to the UART port
	   	//uart_send(result,sizeof(float)*output_size);
		//XUartPs_Send(&uart,(u8)result,sizeof(float) * output_size);


	    //resetting the DMA because SDK sucks
	   	XAxiDma_Reset(&axiDma);
   }

	   	free(in);
	   	free(result);
   return 0;
}
