#include "display_demo.h"
#include "display_ctrl/display_ctrl.h"
#include <stdio.h>
#include "xuartps.h"
#include "math.h"
#include <ctype.h>
#include <stdlib.h>
#include "xil_types.h"
#include "xil_cache.h"
#include "timer_ps/timer_ps.h"
#include "xparameters.h"

/*
 * XPAR redefines
 */
#define DYNCLK_BASEADDR XPAR_AXI_DYNCLK_0_BASEADDR
#define VGA_VDMA_ID XPAR_AXIVDMA_0_DEVICE_ID
#define DISP_VTC_ID XPAR_VTC_0_DEVICE_ID
#define VID_VTC_IRPT_ID XPS_FPGA3_INT_ID
#define VID_GPIO_IRPT_ID XPS_FPGA4_INT_ID
#define SCU_TIMER_ID XPAR_SCUTIMER_DEVICE_ID
#define UART_BASEADDR XPAR_PS7_UART_0_BASEADDR

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

/*
 * Display Driver structs
 */
DisplayCtrl dispCtrl;
XAxiVdma vdma;
int srcBuffer;
/*
 * Framebuffers for video data
 */
u8 frameBuf[DISPLAY_NUM_FRAMES][DEMO_MAX_FRAME];
u8 *pFrames[DISPLAY_NUM_FRAMES]; //array of pointers to the frame buffers

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */

int main(void)
{
	DemoInitialize();

	//DemoRun();

	int Status;
		XAxiVdma InstancePtr;
		double fRed, fBlue, fGreen, fColor;

		//u8 frame_buffer[2][DEMO_MAX_FRAME];

		xil_printf("\n--- Entering main() --- \r\n");
		xil_printf("Starting the first VDMA \n\r");

		srcBuffer = (int)&frameBuf[0];

		//Check Address of buffer
		xil_printf("SrcBuffer: %x\n\r", srcBuffer);
		xil_printf("Frame[%d]: %x\n\r", 0, frameBuf[0]);
		xil_printf("Frame[%d]: %x\n\r", 1, frameBuf[1]);
		xil_printf("WriteAddr: %x\n\r", srcBuffer + DEMO_MAX_FRAME);

		//Initial data in read buffer
		int j;
		for (j = 0; j < DEMO_MAX_FRAME; j = j+8){

			frameBuf[1][j] = (u8)55; //r
			frameBuf[1][j+1] = (u8)215; //g
			frameBuf[1][j+2] = (u8)55; //b
			frameBuf[1][j+3] = (u8)255; //b
			frameBuf[1][j+4] = (u8)55; //r
			frameBuf[1][j+5] = (u8)215; //g
			frameBuf[1][j+6] = (u8)55; //b
			frameBuf[1][j+7] = (u8)255; //b
			frameBuf[2][j] = (u8)55; //r
			frameBuf[2][j+1] = (u8)15; //g
			frameBuf[2][j+2] = (u8)0; //b
			frameBuf[2][j+3] = (u8)0; //b
			frameBuf[2][j+4] = (u8)55; //r
			frameBuf[2][j+5] = (u8)15; //g
			frameBuf[2][j+6] = (u8)0; //b
			frameBuf[2][j+7] = (u8)0; //b

		}


		for (j = 0; j < DEMO_MAX_FRAME; j = j+8){

					frameBuf[0][j] = (u8)(55+j); //r
					frameBuf[0][j+1] = (u8)15+j; //g
					frameBuf[0][j+2] = (u8)100+j; //b
					frameBuf[0][j+3] = (u8)110+j; //b
					frameBuf[0][j+4] = (u8)55+j; //b
					frameBuf[0][j+5] = (u8)15+j; //b
					frameBuf[0][j+6] = (u8)100+j; //b
					frameBuf[0][j+7] = (u8)110+j; //b
					frameBuf[0][j+8] = (u8)25+j; //r
					frameBuf[0][j+9] = (u8)255+j; //g
					frameBuf[0][j+10] = (u8)55+j; //b
					frameBuf[0][j+11] = (u8)255+j; //b
					frameBuf[0][j+12] = (u8)25+j; //b
					frameBuf[0][j+13] = (u8)255+j; //b
					frameBuf[0][j+14] = (u8)55+j; //b
					frameBuf[0][j+15] = (u8)255+j; //b
		}



		Xil_DCacheFlushRange((unsigned int) frameBuf[0], DEMO_MAX_FRAME);
		Xil_DCacheFlushRange((unsigned int) frameBuf[1], DEMO_MAX_FRAME);
		Xil_DCacheFlushRange((unsigned int) frameBuf[2], DEMO_MAX_FRAME);





	int nextFrame = 0;
	/* Flush UART FIFO */
	while (1)
	{
		nextFrame = dispCtrl.curFrame + 1;
								if (nextFrame >= DISPLAY_NUM_FRAMES)
								{
									nextFrame = 0;
								}
								DisplayChangeFrame(&dispCtrl, nextFrame);
								xil_printf("frame-no:%d\n\r",nextFrame);
								TimerDelay(5000000);

}
}

void DemoInitialize()
{
	int Status;
	XAxiVdma_Config *vdmaConfig;
	int i;

	/*
	 * Initialize an array of pointers to the 3 frame buffers
	 */
	for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
	{
		pFrames[i] = frameBuf[i];
	}

	/*
	 * Initialize a timer used for a simple delay
	 */
	TimerInitialize(SCU_TIMER_ID);

	/*
	 * Initialize VDMA driver
	 */
	vdmaConfig = XAxiVdma_LookupConfig(VGA_VDMA_ID);
	if (!vdmaConfig)
	{
		xil_printf("No video DMA found for ID %d\r\n", VGA_VDMA_ID);
		return;
	}
	Status = XAxiVdma_CfgInitialize(&vdma, vdmaConfig, vdmaConfig->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		xil_printf("VDMA Configuration Initialization failed %d\r\n", Status);
		return;
	}

	/*
	 * Initialize the Display controller and start it
	 */
	Status = DisplayInitialize(&dispCtrl, &vdma, DISP_VTC_ID, DYNCLK_BASEADDR, pFrames, DEMO_STRIDE);
	if (Status != XST_SUCCESS)
	{
		xil_printf("Display Ctrl initialization failed during demo initialization%d\r\n", Status);
		return;
	}
	Status = DisplayStart(&dispCtrl);
	if (Status != XST_SUCCESS)
	{
		xil_printf("Couldn't start display during demo initialization%d\r\n", Status);
		return;
	}


	return;
}








void DemoPrintTest(u8 *frame, u32 width, u32 height, u32 stride, int pattern)
{
	u32 xcoi, ycoi;
	u32 iPixelAddr;
	u8 wRed, wBlue, wGreen;
	u32 wCurrentInt;
	double fRed, fBlue, fGreen, fColor;
	u32 xLeft, xMid1,xMid2, xRight, xInt;
	u32 yLeft, yMid1, yMid2, yRight, yInt;
	double xInc, yInc;
	//char userInput = 0;
		int nextFrame;


	switch (pattern)
	{
	case DEMO_PATTERN_0:

		xInt = width / 4; //Four intervals, each with width/4 pixels
		xLeft = xInt * 3;
		xMid1 = xInt * 2 * 3;
		xMid2 = xInt * 3 * 3;
		xRight = xInt *4* 3;
		xInc = 256.0 / ((double) xInt); //256 color intensities are cycled through per interval (overflow must be caught when color=256.0)

		yInt = height / 4; //Two intervals, each with width/2 lines
		yLeft = yInt * 3;
		yMid1 = yInt * 2 * 3;
		yMid2 = yInt * 3 * 3;
		yRight = yInt * 4 * 3;
		yInc = 256.0 / ((double) yInt); //256 color intensities are cycled through per interval (overflow must be caught when color=256.0)
		for(xcoi = 0; xcoi < (width*4); xcoi+=1)
		{
			/*
			 * Convert color intensities to integers < 256, and trim values >=256
			 */
			wRed =((u8) fRed);
			wBlue = ((u8) fBlue);
			iPixelAddr = xcoi;
			for(ycoi = 0; ycoi < (height); ycoi+=1)
			{

				wGreen = ((u8) fGreen);



				frame[iPixelAddr] = (u8) fRed;
				frame[iPixelAddr + 1] = (u8)fBlue;
				frame[iPixelAddr + 2] = (u8) fGreen;



				iPixelAddr += (stride);
				}


		}
		/*
		 * Flush the framebuffer memory range to ensure changes are written to the
		 * actual memory, and therefore accessible by the VDMA.
		 */
		Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
		break;

}
}

