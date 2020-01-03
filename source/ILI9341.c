/**
 * @file ILI9341.c
 * @brief
 *
 *
 */

#include "ILI9341.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_dspi.h"
#include "fsl_dspi_edma.h"
#include "fsl_dmamux.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "GUI.h"

#define ILI9341_SPI 				SPI0
#define DSPI_MASTER_CLK_SRC 		DSPI0_CLK_SRC
#define DSPI_MASTER_CLK_FREQ 		CLOCK_GetFreq(DSPI0_CLK_SRC)
#define ILI9341_PCS_FOR_INIT 		kDSPI_Pcs0
#define ILI9341_PCS_FOR_TRANSFER 	kDSPI_MasterPcs0

#define ILI9341_DMA 				DMA0
#define ILI9341_TX_REQ 				kDmaRequestMux0SPI0Tx
#define ILI9341_RX_REQ 				kDmaRequestMux0SPI0Rx
#define ILI9341_RX_CHN 				4
#define ILI9341_TX_CHN 				5
#define ILI9341_INT_CHN				6

#define DATA_COMMAND_GPIO 			GPIOC
#define DATA_COMMAND_PORT 			PORTC
#define DATA_COMMAND_PIN			4

#define RESET_GPIO 					GPIOC
#define RESET_PORT 					PORTC
#define RESET_PIN					12

#define TRANSFER_BAUDRATE 			50000000U



static dspi_master_edma_handle_t SPI_Handle;
static edma_handle_t SPI_DMA_RxRegToRxDataHandle;
static edma_handle_t SPI_DMA_TxDataToIntermediaryHandle;
static edma_handle_t SPI_DMA_IntermediaryToTxRegHandle;
static dspi_transfer_t transfer;
static uint32_t SPI_COMMAND;


static void ILI9341_InitSequence();



void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_edma_handle_t *handle, status_t status, void *userData)
{
	/* Esta mal el driver, creo. Segun "50.5.1 How to manage queues" en el reference manual
	 despues de mandar por dema una QUEUE tenes que flushear las FIFOS y borrar el flag de EOQ
	 cosa que en el callback EDMA_DspiMasterCallback en fsl_dspi_edma.c NO HACE! */
	 DSPI_FlushFifo(ILI9341_SPI, true, true);
	 DSPI_ClearStatusFlags(base, (uint32_t)(kDSPI_EndOfQueueFlag));
	 ILI9341_SendCommand(ILI9341_NOP);

	 GUI_FlushReady();
}


void ILI9341_Init()
{
	/* Init ports and pins */
	CLOCK_EnableClock(kCLOCK_PortC);			/* Port B Clock Gate Control: Clock enabled */
	CLOCK_EnableClock(kCLOCK_PortD);            /* Port D Clock Gate Control: Clock enabled */

	PORT_SetPinMux(PORTD, 0, kPORT_MuxAlt2);	/* PORTD0 (pin 93) is configured as SPI0_PCS0 */
	PORT_SetPinMux(PORTD, 1, kPORT_MuxAlt2);    /* PORTD1 (pin 94) is configured as SPI0_SCK */
	PORT_SetPinMux(PORTD, 2, kPORT_MuxAlt2);    /* PORTD2 (pin 95) is configured as SPI0_SOUT */
	PORT_SetPinMux(PORTD, 3, kPORT_MuxAlt2);    /* PORTD3 (pin 96) is configured as SPI0_SIN */
	PORT_SetPinMux(RESET_PORT, RESET_PIN, kPORT_MuxAsGpio);  /* PORTC4 (pin 96) is configured as GPIO*/
	PORT_SetPinMux(DATA_COMMAND_PORT, DATA_COMMAND_PIN, kPORT_MuxAsGpio); /* PORTC12 (pin 96) is configured as GPIO*/

	gpio_pin_config_t GPIOConfig =
	{
			.pinDirection = kGPIO_DigitalOutput,
			.outputLogic = 1U
	};

	GPIO_PinInit(RESET_GPIO,RESET_PIN, &GPIOConfig);
	GPIO_PinInit(DATA_COMMAND_GPIO,DATA_COMMAND_PIN,&GPIOConfig);


	/* Init SPI master */
	dspi_master_config_t masterConfig;
	masterConfig.whichCtar = kDSPI_Ctar0;
	masterConfig.ctarConfig.baudRate = TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.bitsPerFrame = 8;
	masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
	masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
	masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
	masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 100;//0*1000000000/TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 100;//0*1000000000/TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 500;// 0*1000000000/TRANSFER_BAUDRATE;
	masterConfig.whichPcs = ILI9341_PCS_FOR_INIT;
	masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;
	masterConfig.enableContinuousSCK = false;
	masterConfig.enableRxFifoOverWrite = false;
	masterConfig.enableModifiedTimingFormat = false;
	masterConfig.samplePoint = kDSPI_SckToSin0Clock;

	DSPI_MasterInit(ILI9341_SPI, &masterConfig, DSPI_MASTER_CLK_FREQ);

	// Command for pushing data to SPI buffer
	dspi_command_data_config_t commandConfig={false,kDSPI_Ctar0,ILI9341_PCS_FOR_INIT,false,false};
	SPI_COMMAND = DSPI_MasterGetFormattedCommand(&commandConfig);

	/* Init DMA*/

	DMAMUX_Init(DMAMUX);

	DMAMUX_SetSource(DMAMUX, ILI9341_TX_CHN,ILI9341_TX_REQ);
	DMAMUX_EnableChannel(DMAMUX, ILI9341_RX_CHN);

	DMAMUX_SetSource(DMAMUX, ILI9341_RX_CHN,ILI9341_RX_REQ);
	DMAMUX_EnableChannel(DMAMUX, ILI9341_TX_CHN);

	edma_config_t DMA_Config;
	EDMA_GetDefaultConfig(&DMA_Config);
	DMA_Config.enableDebugMode = false;

	EDMA_Init(ILI9341_DMA, &DMA_Config);

	EDMA_CreateHandle(&(SPI_DMA_RxRegToRxDataHandle), ILI9341_DMA, ILI9341_RX_CHN);
	EDMA_CreateHandle(&(SPI_DMA_TxDataToIntermediaryHandle), ILI9341_DMA, ILI9341_INT_CHN);
	EDMA_CreateHandle(&(SPI_DMA_IntermediaryToTxRegHandle), ILI9341_DMA, ILI9341_TX_CHN);

	DSPI_MasterTransferCreateHandleEDMA(ILI9341_SPI, &SPI_Handle, DSPI_MasterUserCallback,
										NULL, &SPI_DMA_RxRegToRxDataHandle,
										&SPI_DMA_TxDataToIntermediaryHandle,
										&SPI_DMA_IntermediaryToTxRegHandle);


	ILI9341_Reset();

	ILI9341_InitSequence();

}

void ILI9341_Reset()
{
	GPIO_PinWrite(RESET_GPIO,RESET_PIN,0);
	uint64_t c = 1320000;
	while(c>0) c--;
	GPIO_PinWrite(RESET_GPIO,RESET_PIN,1);
	c = 1320000;
	while(c>0) c--;
}

bool ILI9341_IsBusy()
{
	return SPI_Handle.state == kDSPI_Busy;
}

static void ILI9341_InitSequence()
{
	ILI9341_SendByte(COMMAND, 0xCF);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x83);
	ILI9341_SendByte(DATA, 0x30);

	ILI9341_SendByte(COMMAND, 0xED);
	ILI9341_SendByte(DATA, 0x64);
	ILI9341_SendByte(DATA, 0x03);
	ILI9341_SendByte(DATA, 0x12);
	ILI9341_SendByte(DATA, 0x81);

	ILI9341_SendByte(COMMAND, 0xE8);
	ILI9341_SendByte(DATA, 0x85);
	ILI9341_SendByte(DATA, 0x01);
	ILI9341_SendByte(DATA, 0x79);

	ILI9341_SendByte(COMMAND, 0xCB);
	ILI9341_SendByte(DATA, 0x39);
	ILI9341_SendByte(DATA, 0x2C);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x34);
	ILI9341_SendByte(DATA, 0x02);

	ILI9341_SendByte(COMMAND, 0xF7);
	ILI9341_SendByte(DATA, 0x20);

	ILI9341_SendByte(COMMAND, 0xEA);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x00);

	ILI9341_SendByte(COMMAND, ILI9341_PWCTR1);	// Power control
	ILI9341_SendByte(DATA, 0x26);
	// VRH[5:0]
	ILI9341_SendByte(COMMAND, ILI9341_PWCTR2);	// Power control
	ILI9341_SendByte(DATA, 0x11);
	// SAP[2:0];BT[3:0]
	ILI9341_SendByte(COMMAND, ILI9341_VMCTR1);	// VCM control
	ILI9341_SendByte(DATA, 0x35);
	ILI9341_SendByte(DATA, 0x3E);

	ILI9341_SendByte(COMMAND, ILI9341_VMCTR2);	// VCM control2
	ILI9341_SendByte(DATA, 0xBE);
	// --
	ILI9341_SendByte(COMMAND, ILI9341_MADCTL);	//  Memory Access Control
	ILI9341_SendByte(DATA, 0x28);

	ILI9341_SendByte(COMMAND, ILI9341_PIXFMT);
	ILI9341_SendByte(DATA, 0x55);

	ILI9341_SendByte(COMMAND, ILI9341_FRMCTR1);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x1B);

	ILI9341_SendByte(COMMAND, 242);
	ILI9341_SendByte(DATA, 0x08);

	ILI9341_SendByte(COMMAND, 0xF2);				//  3Gamma Function Disable
	ILI9341_SendByte(DATA, 0x08);

	ILI9341_SendByte(COMMAND, ILI9341_GAMMASET);	// Gamma curve selected
	ILI9341_SendByte(DATA, 0x01);

	ILI9341_SendByte(COMMAND, ILI9341_GMCTRP1);	// Set Gamma
	ILI9341_SendByte(DATA, 0x1F);
	ILI9341_SendByte(DATA, 0x1A);
	ILI9341_SendByte(DATA, 0x18);
	ILI9341_SendByte(DATA, 0x0A);
	ILI9341_SendByte(DATA, 0x0F);
	ILI9341_SendByte(DATA, 0x06);
	ILI9341_SendByte(DATA, 0x45);
	ILI9341_SendByte(DATA, 0x87);
	ILI9341_SendByte(DATA, 0x32);
	ILI9341_SendByte(DATA, 0x0A);
	ILI9341_SendByte(DATA, 0x07);
	ILI9341_SendByte(DATA, 0x02);
	ILI9341_SendByte(DATA, 0x07);
	ILI9341_SendByte(DATA, 0x05);
	ILI9341_SendByte(DATA, 0x00);

	ILI9341_SendByte(COMMAND, ILI9341_GMCTRN1);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x25);
	ILI9341_SendByte(DATA, 0x27);
	ILI9341_SendByte(DATA, 0x05);
	ILI9341_SendByte(DATA, 0x10);
	ILI9341_SendByte(DATA, 0x09);
	ILI9341_SendByte(DATA, 0x3A);
	ILI9341_SendByte(DATA, 0x78);
	ILI9341_SendByte(DATA, 0x4D);
	ILI9341_SendByte(DATA, 0x05);
	ILI9341_SendByte(DATA, 0x18);
	ILI9341_SendByte(DATA, 0x0D);
	ILI9341_SendByte(DATA, 0x38);
	ILI9341_SendByte(DATA, 0x3A);
	ILI9341_SendByte(DATA, 0x1F);

	ILI9341_SendByte(COMMAND, ILI9341_CASET);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0xEF);

	ILI9341_SendByte(COMMAND, ILI9341_PASET);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x3F);

	ILI9341_SendByte(COMMAND, ILI9341_RAMWR);
	ILI9341_SendByte(DATA, 0x07);

	ILI9341_SendByte(COMMAND, ILI9341_DFUNCTR);	//  Display Function Control
	ILI9341_SendByte(DATA, 0x0A);
	ILI9341_SendByte(DATA, 0x82);
	ILI9341_SendByte(DATA, 0x27);
	ILI9341_SendByte(DATA, 0x00);


	ILI9341_SendByte(COMMAND, ILI9341_SLPOUT);	// Exit Sleep
	uint64_t c = 1320000;
	while(c-->0);
	ILI9341_SendByte(COMMAND, ILI9341_DISPON);	// Display on
	c = 1320000;
	while(c-->0);

/*	ILI9341_SendByte(COMMAND, 0xEF);
	ILI9341_SendByte(DATA, 0x03);
	ILI9341_SendByte(DATA, 0x80);
	ILI9341_SendByte(DATA, 0x02);
	ILI9341_SendByte(COMMAND, 0xCF);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0xC1);
	ILI9341_SendByte(DATA, 0x30);
	ILI9341_SendByte(COMMAND, 0xED);
	ILI9341_SendByte(DATA, 0x64);
	ILI9341_SendByte(DATA, 0x03);
	ILI9341_SendByte(DATA, 0x12);
	ILI9341_SendByte(DATA, 0x81);
	ILI9341_SendByte(COMMAND, 0xE8);
	ILI9341_SendByte(DATA, 0x85);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x78);
	ILI9341_SendByte(COMMAND, 0xCB);
	ILI9341_SendByte(DATA, 0x39);
	ILI9341_SendByte(DATA, 0x2C);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x34);
	ILI9341_SendByte(DATA, 0x02);
	ILI9341_SendByte(COMMAND, 0xF7);
	ILI9341_SendByte(DATA, 0x20);
	ILI9341_SendByte(COMMAND, 0xEA);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(COMMAND, ILI9341_PWCTR1);	// Power control
	ILI9341_SendByte(DATA, 0x23);					// VRH[5:0]
	ILI9341_SendByte(COMMAND, ILI9341_PWCTR2);	// Power control
	ILI9341_SendByte(DATA, 0x10);					// SAP[2:0];BT[3:0]
	ILI9341_SendByte(COMMAND, ILI9341_VMCTR1);	// VCM control
	ILI9341_SendByte(DATA, 0x3e);
	ILI9341_SendByte(DATA, 0x28);
	ILI9341_SendByte(COMMAND, ILI9341_VMCTR2);	// VCM control2
	ILI9341_SendByte(DATA, 0x86);					// --
	ILI9341_SendByte(COMMAND, ILI9341_MADCTL);	//  Memory Access Control
	ILI9341_SendByte(DATA, 0x48);
	ILI9341_SendByte(COMMAND, ILI9341_PIXFMT);
	ILI9341_SendByte(DATA, 0x55);
	ILI9341_SendByte(COMMAND, ILI9341_FRMCTR1);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x18);
	ILI9341_SendByte(COMMAND, ILI9341_DFUNCTR);	//  Display Function Control
	ILI9341_SendByte(DATA, 0x08);
	ILI9341_SendByte(DATA, 0x82);
	ILI9341_SendByte(DATA, 0x27);
	ILI9341_SendByte(COMMAND, 0xF2);				//  3Gamma Function Disable
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(COMMAND, ILI9341_GAMMASET);	// Gamma curve selected
	ILI9341_SendByte(DATA, 0x01);
	ILI9341_SendByte(COMMAND, ILI9341_GMCTRP1);	// Set Gamma
	ILI9341_SendByte(DATA, 0x0F);
	ILI9341_SendByte(DATA, 0x31);
	ILI9341_SendByte(DATA, 0x2B);
	ILI9341_SendByte(DATA, 0x0C);
	ILI9341_SendByte(DATA, 0x0E);
	ILI9341_SendByte(DATA, 0x08);
	ILI9341_SendByte(DATA, 0x4E);
	ILI9341_SendByte(DATA, 0xF1);
	ILI9341_SendByte(DATA, 0x37);
	ILI9341_SendByte(DATA, 0x07);
	ILI9341_SendByte(DATA, 0x10);
	ILI9341_SendByte(DATA, 0x03);
	ILI9341_SendByte(DATA, 0x0E);
	ILI9341_SendByte(DATA, 0x09);
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(COMMAND, ILI9341_GMCTRN1);	// Set Gamma
	ILI9341_SendByte(DATA, 0x00);
	ILI9341_SendByte(DATA, 0x0E);
	ILI9341_SendByte(DATA, 0x14);
	ILI9341_SendByte(DATA, 0x03);
	ILI9341_SendByte(DATA, 0x11);
	ILI9341_SendByte(DATA, 0x07);
	ILI9341_SendByte(DATA, 0x31);
	ILI9341_SendByte(DATA, 0xC1);
	ILI9341_SendByte(DATA, 0x48);
	ILI9341_SendByte(DATA, 0x08);
	ILI9341_SendByte(DATA, 0x0F);
	ILI9341_SendByte(DATA, 0x0C);
	ILI9341_SendByte(DATA, 0x31);
	ILI9341_SendByte(DATA, 0x36);
	ILI9341_SendByte(DATA, 0x0F);
	ILI9341_SendByte(COMMAND, ILI9341_SLPOUT);	// Exit Sleep
	uint64_t c = 1320000;
	while(c-->0);
	ILI9341_SendByte(COMMAND, ILI9341_DISPON);	// Display on
	c = 1320000;
	while(c-->0);*/
}
void ILI9341_SendByte(ILI9341_MsgType type, uint8_t byte)
{
	GPIO_PinWrite(DATA_COMMAND_GPIO,DATA_COMMAND_PIN,type);
	DSPI_MasterWriteCommandDataBlocking(ILI9341_SPI, (SPI_COMMAND & 0xFFFF0000) | byte);
}

void ILI9341_SendCommand(uint16_t command)
{
	GPIO_PinWrite(DATA_COMMAND_GPIO,DATA_COMMAND_PIN,COMMAND);
	DSPI_MasterWriteCommandDataBlocking(ILI9341_SPI, (SPI_COMMAND & 0xFFFF0000) | command);
}

void ILI9341_SendDataBlocking(uint8_t * data, uint32_t len)
{
	assert(ILI9341_IsBusy()==false);

	GPIO_PinWrite(DATA_COMMAND_GPIO,DATA_COMMAND_PIN,DATA);

	uint8_t rxData[5300];

	dspi_transfer_t transfer;
	transfer.txData = data;
	transfer.rxData = rxData;
	transfer.dataSize = len;
	transfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;



	for(int i=0; i<len;i++)
		DSPI_MasterWriteCommandDataBlocking(ILI9341_SPI, (SPI_COMMAND & 0xFFFF0000) | data[i]);

	return;

	status_t s = DSPI_MasterTransferBlocking(ILI9341_SPI, &transfer);

	if(s != kStatus_Success)
		PRINTF("ERROR IN TRANSFER\n");

}

void ILI9341_SendData(uint8_t * data, uint32_t len)
{
	assert(ILI9341_IsBusy()==false);

	GPIO_PinWrite(DATA_COMMAND_GPIO,DATA_COMMAND_PIN,DATA);

	/* Config transfer */
	transfer.txData = data;
	transfer.rxData = NULL;
	transfer.dataSize = len;
	transfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;

	status_t s = DSPI_MasterTransferEDMA(ILI9341_SPI, &SPI_Handle, &transfer);

	assert(s==kStatus_Success);

}

void ILI9341_SendRepeatedData(uint8_t * data,uint8_t len,uint32_t n)
{
	assert(len < 4);
	assert(ILI9341_IsBusy()==false);
/*
  ESTO ES PARA EVITAR HACER UN ARREGLO Y LLENARLO EN FILL, NO ANDA TODAVIA

 */
	static uint32_t commandData[4];
//	static edma_tcd_t TCD[4];
	static edma_transfer_config_t transfer;

	commandData[0] = SPI_COMMAND | data[0];
    transfer.srcAddr = (uint32_t)&commandData[0];
    transfer.destAddr = DSPI_MasterGetTxRegisterAddress(ILI9341_SPI);
    transfer.srcTransferSize = kEDMA_TransferSize4Bytes;
    transfer.destTransferSize = kEDMA_TransferSize4Bytes;
    transfer.srcOffset = 0;
    transfer.destOffset = 0;
    transfer.minorLoopBytes = 4;
    transfer.majorLoopCounts = n;

    DSPI_EnableInterrupts(ILI9341_SPI, kDSPI_TxFifoFillRequestInterruptEnable);

//	EDMA_TcdReset(&TCD[0]);
//	EDMA_TcdSetTransferConfig(&TCD[0], &transfer, NULL);
	EDMA_SetTransferConfig(ILI9341_DMA, ILI9341_TX_CHN, &transfer, NULL);


	GPIO_PinWrite(DATA_COMMAND_GPIO,DATA_COMMAND_PIN,DATA);
	DSPI_EnableDMA(ILI9341_SPI, kDSPI_TxDmaEnable);
	DSPI_StartTransfer(ILI9341_SPI);

}



