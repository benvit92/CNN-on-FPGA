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
void uart_receive(u8 * buff,int RecvCount){
	int counter = 0;
	while(counter < RecvCount){
		while (!XUartPs_IsReceiveData(Config->BaseAddress));
			;//waiting for input
		XUartPs_Recv(&uart,&buff[counter],1);
		counter++;
	}
}

//function to send data to UART port
void uart_send(char* res_hw,int num){
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

	//local status variable
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

	//setting receiver buffer
	u8 buff[(conv_channel * conv_in_x * conv_in_y) * sizeof(float)];
	//char* in = malloc(256);
	//float* in = malloc(conv_channel * conv_in_x * conv_in_y * sizeof(float));
	//float in[1][16][16] = {{{0,0,0.003921568627451,0.031372549019608,0,0.21176470588235,0.69411764705882,0,0.011764705882353,0,0.003921568627451,0,0.03921568627451,0,0,0.015686274509804},{0.015686274509804,0.03921568627451,0,0.019607843137255,0.26666666666667,0.94509803921569,0.26666666666667,0.047058823529412,0,0,0,0,0.015686274509804,0,0,0},{0.003921568627451,0,0,0.098039215686275,0.85882352941176,0.47450980392157,0.054901960784314,0,0.011764705882353,0.027450980392157,0.023529411764706,0.015686274509804,0.003921568627451,0.015686274509804,0.003921568627451,0},{0.003921568627451,0.03921568627451,0,0.71764705882353,0.76078431372549,0,0.058823529411765,0,0.019607843137255,0,0,0,0,0.015686274509804,0,0},{0.007843137254902,0,0.4,0.90196078431373,0.15686274509804,0,0.015686274509804,0.027450980392157,0,0,0.035294117647059,0.050980392156863,0,0,0,0.031372549019608},{0.007843137254902,0,0.93725490196078,0.34117647058824,0,0.050980392156863,0,0,0.082352941176471,0,0,0.003921568627451,0.13725490196078,0.18039215686275,0.18039215686275,0},{0.015686274509804,0.43529411764706,0.83137254901961,0.019607843137255,0.031372549019608,0,0,0.023529411764706,0,0.027450980392157,0.41960784313725,0.62745098039216,0.9921568627451,1,0.96862745098039,0.52156862745098},{0,0.89019607843137,0.4078431372549,0,0.023529411764706,0,0.023529411764706,0,0.10196078431373,0.53725490196078,0.94901960784314,0.46274509803922,0.11372549019608,0.066666666666667,0.57254901960784,0.69803921568627},{0.15294117647059,0.99607843137255,0.12156862745098,0,0,0.050980392156863,0,0.058823529411765,0.6,0.89411764705882,0.070588235294118,0,0.027450980392157,0.090196078431373,0.85882352941176,0.14117647058824},{0.41176470588235,0.90980392156863,0.007843137254902,0.015686274509804,0,0,0,0.28627450980392,0.85098039215686,0.23529411764706,0.015686274509804,0.027450980392157,0,0.65098039215686,0.68627450980392,0},{0.54117647058824,0.58039215686275,0,0,0,0.062745098039216,0.047058823529412,0.63529411764706,0.46666666666667,0,0,0,0.50196078431373,0.76470588235294,0.10980392156863,0},{0.66274509803922,0.62352941176471,0.094117647058824,0,0.007843137254902,0.011764705882353,0.17647058823529,0.96470588235294,0.031372549019608,0.007843137254902,0.082352941176471,0.6,0.73333333333333,0.035294117647059,0,0.015686274509804},{0.38823529411765,0.70196078431373,0,0,0.074509803921569,0,0.38823529411765,0.7843137254902,0,0.086274509803922,0.61960784313725,0.72156862745098,0.10980392156863,0.003921568627451,0,0},{0.22745098039216,0.87058823529412,0.33725490196078,0,0,0.070588235294118,0.62745098039216,0.68627450980392,0.62352941176471,0.92549019607843,0.50588235294118,0,0,0.027450980392157,0,0.098039215686275},{0,0.63529411764706,0.96862745098039,0.73725490196078,0.66666666666667,0.75294117647059,0.94509803921569,0.85490196078431,0.57647058823529,0.047058823529412,0.031372549019608,0,0.019607843137255,0,0.031372549019608,0},{0.035294117647059,0,0.22745098039216,0.52156862745098,0.65490196078431,0.43529411764706,0.33333333333333,0.023529411764706,0,0.047058823529412,0,0.003921568627451,0.007843137254902,0.027450980392157,0,0.023529411764706}}};
	//float* result = malloc(output_size * sizeof(float));
	//always available when started
	char *in =malloc(256*4);
	int i;
   	while(1){
	   	//receiving input
	   	uart_receive(buff,(conv_channel*conv_in_x*conv_in_y)*sizeof(float));

	   	/*for(i=0;i<10;i++){
	   		result[i]= -1;
	   	}*/
	   	//casting the buffer to float
	   	in = (char *) buff;

		//usleep(300000);

	   	//Handling the DMA to PL input transfer
	   	/*do{
			status = XAxiDma_SimpleTransfer(&axiDma,(u32)in,sizeof(float) * (conv_channel*conv_in_x*conv_in_y),
						XAXIDMA_DMA_TO_DEVICE);
		}while(status != XST_SUCCESS);

	   	//waiting in order to make the transfer complete
		usleep(300000);

	    //receiving result from PL
		do{
			status = XAxiDma_SimpleTransfer(&axiDma,(u32)result,sizeof(float) * output_size,
					XAXIDMA_DEVICE_TO_DMA);
		}while(status != XST_SUCCESS);

	   	//waiting in order to make the transfer complete
		usleep(5000000);*/

		/*printf("Results are...\n");
		for(i=0;i<10;i++){
			printf("%f\n",result[i]);
		}*/

		//sending the results to the UART port
		uart_send(in,sizeof(float)*256);//output_size);

	    //resetting the DMA because SDK sucks
		//XAxiDma_Reset(&axiDma);
   }

   //free(result);
   free(in);
   return 0;
}
