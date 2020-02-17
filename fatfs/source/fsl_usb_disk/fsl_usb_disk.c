/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffconf.h"
#include "ff.h"
/* This fatfs subcomponent is disabled by default
 * To enable it, define following macro in ffconf.h */
#ifdef USB_DISK_ENABLE

#include "fsl_usb_disk.h" /* FatFs lower layer API */
#include "fsl_clock.h"
#include "fsl_sysmpu.h"

/*******************************************************************************
 * Definitons
 ******************************************************************************/

typedef enum _usb_host_app_state
{
    kStatus_DEV_Idle = 0, /*!< there is no device attach/detach */
    kStatus_DEV_Attached, /*!< device is attached */
    kStatus_DEV_Running, /*!< device is running */
} usb_host_app_state_t;

/*! @brief USB host msd fatfs instance structure */
typedef struct _usb_host_msd_fatfs_instance
{
    usb_host_configuration_handle configHandle; /*!< configuration handle */
    usb_device_handle deviceHandle;             /*!< device handle */
    usb_host_class_handle classHandle;          /*!< class handle */
    usb_host_interface_handle interfaceHandle;  /*!< interface handle */
    uint8_t prevDeviceState;                    /*!< device attach/detach previous status */
    uint8_t deviceState;                        /*!< device Idle/Attached/Running*/
} usb_host_msd_fatfs_instance_t;


#if ((defined USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI))
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif /* USB_HOST_CONFIG_KHCI */
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif /* USB_HOST_CONFIG_EHCI */
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI))
#define CONTROLLER_ID kUSB_ControllerOhci0
#endif /* USB_HOST_CONFIG_OHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS))
#define CONTROLLER_ID kUSB_ControllerIp3516Hs0
#endif /* USB_HOST_CONFIG_IP3516HS */

#if defined(__GIC_PRIO_BITS)
#define USB_HOST_INTERRUPT_PRIORITY (25U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief host msd ufi command callback.
 *
 * This function is used as callback function for ufi command .
 *
 * @param param      NULL.
 * @param data       data buffer pointer.
 * @param dataLength data length.
 * @status           transfer result status.
 */
static void USB_HostMsdUfiCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);
/*!
 * @brief
 *
 *
 */
usb_status_t USB_HostMsdEvent(usb_device_handle deviceHandle,
                              usb_host_configuration_handle configurationHandle,
                              uint32_t eventCode);
/*!
 * @brief
 *
 *
 */
void USB_HostMsdControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_host_msd_fatfs_instance_t g_MsdFatfsInstance;
usb_host_handle g_HostHandle;

usb_host_class_handle g_UsbFatfsClassHandle;
static uint32_t s_FatfsSectorSize;
/* command on-going state. It should set to 1 when start command, it is set to 0 in the callback */
static volatile uint8_t ufiIng;
/* command callback status */
static volatile usb_status_t ufiStatus;

#if defined(USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_UsbTransferBuffer[FF_MAX_SS];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_UsbTransferBuffer[20];
#endif



/*! @brief msd class handle array for fatfs */
extern usb_host_class_handle g_UsbFatfsClassHandle;

usb_host_msd_fatfs_instance_t g_MsdFatfsInstance; /* global msd fatfs instance */

/*******************************************************************************
 * Code
 ******************************************************************************/

static void USB_HostMsdUfiCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    ufiIng = 0;
    ufiStatus = status;
}

static inline void USB_HostControllerTaskFunction(usb_host_handle hostHandle)
{
#if ((defined USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI))
    USB_HostKhciTaskFunction(hostHandle);
#endif /* USB_HOST_CONFIG_KHCI */
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
    USB_HostEhciTaskFunction(hostHandle);
#endif /* USB_HOST_CONFIG_EHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
        USB_HostIp3516HsTaskFunction(g_HostHandle);
#endif /* USB_HOST_CONFIG_IP3516HS */
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI > 0U))
        USB_HostOhciTaskFunction(g_HostHandle);
#endif /* USB_HOST_CONFIG_OHCI */
}

DSTATUS USB_HostMsdInitializeDisk(BYTE pdrv)
{
    uint32_t address;

    /* test unit ready */
    ufiIng = 1;
    if (g_UsbFatfsClassHandle == NULL)
    {
        return RES_ERROR;
    }
    if (USB_HostMsdTestUnitReady(g_UsbFatfsClassHandle, 0, USB_HostMsdUfiCallback, NULL) != kStatus_USB_Success)
    {
        return STA_NOINIT;
    }
    while (ufiIng) /* wait the command */
    {
        USB_HostControllerTaskFunction(g_HostHandle);
    }

    /*request sense */
    ufiIng = 1;
    if (g_UsbFatfsClassHandle == NULL)
    {
        return RES_ERROR;
    }
    if (USB_HostMsdRequestSense(g_UsbFatfsClassHandle, 0, s_UsbTransferBuffer, sizeof(usb_host_ufi_sense_data_t),
                                USB_HostMsdUfiCallback, NULL) != kStatus_USB_Success)
    {
        return STA_NOINIT;
    }
    while (ufiIng) /* wait the command */
    {
        USB_HostControllerTaskFunction(g_HostHandle);
    }

    /* get the sector size */
    ufiIng = 1;
    if (g_UsbFatfsClassHandle == NULL)
    {
        return RES_ERROR;
    }
    if (USB_HostMsdReadCapacity(g_UsbFatfsClassHandle, 0, s_UsbTransferBuffer, sizeof(usb_host_ufi_read_capacity_t),
                                USB_HostMsdUfiCallback, NULL) != kStatus_USB_Success)
    {
        return STA_NOINIT;
    }
    else
    {
        while (ufiIng)
        {
            USB_HostControllerTaskFunction(g_HostHandle);
        }
        if (ufiStatus == kStatus_USB_Success)
        {
            address = (uint32_t)&s_UsbTransferBuffer[0];
            address = (uint32_t)((usb_host_ufi_read_capacity_t *)(address))->blockLengthInBytes;
            s_FatfsSectorSize = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(((uint8_t *)address));
        }
        else
        {
            s_FatfsSectorSize = 512;
        }
    }

    return 0x00;
}

DSTATUS USB_HostMsdGetDiskStatus(BYTE pdrv)
{
	//usb_echo("%d\n",g_MsdFatfsInstance.deviceState);
	if (pdrv != USBDISK)
	{
		return STA_NOINIT;
	}

	if(g_MsdFatfsInstance.deviceState!=kStatus_DEV_Running)
		return STA_NODISK;
	else
		return STA_OK;
}

DRESULT USB_HostMsdReadDisk(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    DRESULT fatfs_code = RES_ERROR;
    usb_status_t status = kStatus_USB_Success;
    uint32_t retry = USB_HOST_FATFS_RW_RETRY_TIMES;
    uint8_t *transferBuf;
    uint32_t sectorCount;
    uint32_t sectorIndex;
#if defined(USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE)
    uint32_t index;
#endif

    if (!count)
    {
        return RES_PARERR;
    }

#if defined(USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE)
    transferBuf = s_UsbTransferBuffer;
    sectorCount = 1;
    for (index = 0; index < count; ++index)
    {
        sectorIndex = sector + index;
#else
        transferBuf = buff;
        sectorCount = count;
        sectorIndex = sector;
#endif
        retry = USB_HOST_FATFS_RW_RETRY_TIMES;
        while (retry--)
        {
            ufiIng = 1;
            if (g_UsbFatfsClassHandle == NULL)
            {
                return RES_ERROR;
            }
            status = USB_HostMsdRead10(g_UsbFatfsClassHandle, 0, sectorIndex, (uint8_t *)transferBuf,
                                       (uint32_t)(s_FatfsSectorSize * sectorCount), sectorCount, USB_HostMsdUfiCallback, NULL);
            if (status != kStatus_USB_Success)
            {
                fatfs_code = RES_ERROR;
            }
            else
            {
                while (ufiIng)
                {
                    USB_HostControllerTaskFunction(g_HostHandle);
                }
                if (ufiStatus == kStatus_USB_Success)
                {
                    fatfs_code = RES_OK;
                    break;
                }
                else
                {
                    fatfs_code = RES_NOTRDY;
                }
            }
        }
#if defined(USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE)
        memcpy(buff + index * s_FatfsSectorSize, s_UsbTransferBuffer, s_FatfsSectorSize);
    }
#endif
    return fatfs_code;
}

DRESULT USB_HostMsdWriteDisk(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    DRESULT fatfs_code = RES_ERROR;
    usb_status_t status = kStatus_USB_Success;
    uint32_t retry = USB_HOST_FATFS_RW_RETRY_TIMES;
    const uint8_t *transferBuf;
    uint32_t sectorCount;
    uint32_t sectorIndex;
#if defined(USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE)
    uint32_t index;
#endif

    if (!count)
    {
        return RES_PARERR;
    }

#if defined(USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE)
    transferBuf = (const uint8_t *)s_UsbTransferBuffer;
    sectorCount = 1;
    for (index = 0; index < count; ++index)
    {
        sectorIndex = sector + index;
        memcpy(s_UsbTransferBuffer, buff + index * s_FatfsSectorSize, s_FatfsSectorSize);
#else
        transferBuf = buff;
        sectorCount = count;
        sectorIndex = sector;
#endif
        while (retry--)
        {
            ufiIng = 1;
            if (g_UsbFatfsClassHandle == NULL)
            {
                return RES_ERROR;
            }
            status = USB_HostMsdWrite10(g_UsbFatfsClassHandle, 0, sectorIndex, (uint8_t *)transferBuf,
                                        (uint32_t)(s_FatfsSectorSize * sectorCount), sectorCount, USB_HostMsdUfiCallback, NULL);
            if (status != kStatus_USB_Success)
            {
                fatfs_code = RES_ERROR;
            }
            else
            {
                while (ufiIng)
                {
                    USB_HostControllerTaskFunction(g_HostHandle);
                }
                if (ufiStatus == kStatus_USB_Success)
                {
                    fatfs_code = RES_OK;
                    break;
                }
                else
                {
                    fatfs_code = RES_NOTRDY;
                }
            }
        }
#if defined(USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE)
    }
#endif
    return fatfs_code;
}

DRESULT USB_HostMsdIoctlDisk(BYTE pdrv, BYTE cmd, void *buff)
{
    uint32_t address;
    DRESULT fatfs_code = RES_ERROR;
    usb_status_t status = kStatus_USB_Success;
    uint32_t value;

    switch (cmd)
    {
        case GET_SECTOR_COUNT:
        case GET_SECTOR_SIZE:
            if (!buff)
            {
                return RES_ERROR;
            }

            ufiIng = 1;
            if (g_UsbFatfsClassHandle == NULL)
            {
                return RES_ERROR;
            }
            status = USB_HostMsdReadCapacity(g_UsbFatfsClassHandle, 0, s_UsbTransferBuffer,
                                             sizeof(usb_host_ufi_read_capacity_t), USB_HostMsdUfiCallback, NULL);
            if (status != kStatus_USB_Success)
            {
                fatfs_code = RES_ERROR;
            }
            else
            {
                while (ufiIng)
                {
                    USB_HostControllerTaskFunction(g_HostHandle);
                }
                if (ufiStatus == kStatus_USB_Success)
                {
                    fatfs_code = RES_OK;
                }
                else
                {
                    fatfs_code = RES_NOTRDY;
                }
            }

            if (fatfs_code == RES_OK)
            {
                if (GET_SECTOR_COUNT == cmd) /* Get number of sectors on the disk (DWORD) */
                {
                    address = (uint32_t)&s_UsbTransferBuffer[0];
                    address = (uint32_t)((usb_host_ufi_read_capacity_t *)(address))->lastLogicalBlockAddress;
                    value = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(((uint8_t *)address));
                    ((uint8_t *)buff)[0] = ((uint8_t*)&value)[0];
                    ((uint8_t *)buff)[1] = ((uint8_t*)&value)[1];
                    ((uint8_t *)buff)[2] = ((uint8_t*)&value)[2];
                    ((uint8_t *)buff)[3] = ((uint8_t*)&value)[3];
                }
                else /* Get the sector size in byte */
                {
                    address = (uint32_t)&s_UsbTransferBuffer[0];
                    address = (uint32_t)((usb_host_ufi_read_capacity_t *)(address))->blockLengthInBytes;
                    value = USB_LONG_FROM_BIG_ENDIAN_ADDRESS(((uint8_t *)address));
                    ((uint8_t *)buff)[0] = ((uint8_t*)&value)[0];
                    ((uint8_t *)buff)[1] = ((uint8_t*)&value)[1];
                    ((uint8_t *)buff)[2] = ((uint8_t*)&value)[2];
                    ((uint8_t *)buff)[3] = ((uint8_t*)&value)[3];
                }
            }
            break;

        case GET_BLOCK_SIZE:
            if (!buff)
            {
                return RES_ERROR;
            }
            *(uint32_t *)buff = 0;
            fatfs_code = RES_OK;
            break;

        case CTRL_SYNC:
            fatfs_code = RES_OK;
            break;

        default:
            fatfs_code = RES_PARERR;
            break;
    }
    return fatfs_code;
}


status_t USB_DiskSetUp(void)
{
	usb_status_t status = kStatus_USB_Success;
    SystemCoreClockUpdate();
    CLOCK_EnableUsbfs0Clock(kCLOCK_UsbSrcPll0, CLOCK_GetFreq(kCLOCK_PllFllSelClk));

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    status = USB_HostInit(CONTROLLER_ID, &g_HostHandle, USB_HostMsdEvent);//DRIVER FUNCTION()usb_hosthci, se le pasa un callback
	    if (status != kStatus_USB_Success)
	    {
	        usb_echo("host init error\r\n");
	        return kStatus_Fail;
	    }

	 /*-------*/
	uint8_t irqNumber;

	uint8_t usbHOSTKhciIrq[] = USB_IRQS;
	irqNumber = usbHOSTKhciIrq[CONTROLLER_ID - kUSB_ControllerKhci0];

	/* Install isr, set priority, and enable IRQ. */
	#if defined(__GIC_PRIO_BITS)
		GIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
	#else
		NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
	#endif
		status=EnableIRQ((IRQn_Type)irqNumber);
		if (status != kStatus_USB_Success)
		{
			usb_echo("host init error\r\n");
			return kStatus_Fail;
		}

		 usb_echo("host init done\r\n");
		return status;

}

void USB_HostMsdControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_msd_fatfs_instance_t *msdFatfsInstance = (usb_host_msd_fatfs_instance_t *)param;

    if (msdFatfsInstance->deviceState == kStatus_DEV_Attached) /* set interface finish */
    {
    	msdFatfsInstance->deviceState = kStatus_DEV_Running;
    }
}

usb_status_t USB_HostMsdEvent(usb_device_handle deviceHandle,
                              usb_host_configuration_handle configurationHandle,
                              uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    usb_host_configuration_t *configuration;
    uint8_t interfaceIndex;
    usb_host_interface_t *interface;
    uint32_t infoValue;
    uint8_t id;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                interface = &configuration->interfaceList[interfaceIndex];
                id = interface->interfaceDesc->bInterfaceClass;
                if (id != USB_HOST_MSD_CLASS_CODE)
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceSubClass;
                if ((id != USB_HOST_MSD_SUBCLASS_CODE_UFI) && (id != USB_HOST_MSD_SUBCLASS_CODE_SCSI))
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceProtocol;
                if (id != USB_HOST_MSD_PROTOCOL_BULK)
                {
                    continue;
                }
                else
                {
                    if (g_MsdFatfsInstance.deviceState == kStatus_DEV_Idle)
                    {
                        /* the interface is supported by the application */
                        g_MsdFatfsInstance.deviceHandle = deviceHandle;
                        g_MsdFatfsInstance.interfaceHandle = interface;
                        g_MsdFatfsInstance.configHandle = configurationHandle;
                        return kStatus_USB_Success;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
        	status = kStatus_USB_NotSupported;
        	break;

        case kUSB_HostEventEnumerationDone:
            if (g_MsdFatfsInstance.configHandle == configurationHandle)
            {
                if ((g_MsdFatfsInstance.deviceHandle != NULL) && (g_MsdFatfsInstance.interfaceHandle != NULL))
                {
                    /* the device enumeration is done */
                    if (g_MsdFatfsInstance.deviceState == kStatus_DEV_Idle)
                    {
                        //-------------------ESTO SOLO SON PRINTS
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &infoValue);
                        usb_echo("mass storage device attached:pid=0x%x", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &infoValue);
                        usb_echo("vid=0x%x ", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &infoValue);
                        usb_echo("address=%d\r\n", infoValue);
                        //--------------------
                        //g_MsdFatfsInstance.deviceState = kStatus_DEV_Attached;
                    	//----
                        g_MsdFatfsInstance.deviceState = kStatus_DEV_Attached;
                        usb_status_t statusA=USB_HostMsdInit(g_MsdFatfsInstance.deviceHandle,
                    			&g_MsdFatfsInstance.classHandle); /* msd class initialization */
                    	g_UsbFatfsClassHandle =g_MsdFatfsInstance.classHandle;
                    	if (statusA != kStatus_USB_Success)
                    	{
                    		usb_echo("usb host msd init fail\r\n");
                    		break;
                    	}

						statusA = USB_HostMsdSetInterface(g_MsdFatfsInstance.classHandle, g_MsdFatfsInstance.interfaceHandle, 0,
														 USB_HostMsdControlCallback, &g_MsdFatfsInstance);
						if (statusA != kStatus_USB_Success)
						{
							usb_echo("set interface fail\r\n");
						}
                    }
                    else
                    {
                        usb_echo("not idle msd instance\r\n");
                        status = kStatus_USB_Error;
                    }
                }
            }
            break;

        case kUSB_HostEventDetach:
            if (g_MsdFatfsInstance.configHandle == configurationHandle)
            {
                /* the device is detached */
                g_UsbFatfsClassHandle = NULL;
                g_MsdFatfsInstance.configHandle = NULL;
                if (g_MsdFatfsInstance.deviceState != kStatus_DEV_Idle)
                {
                    g_MsdFatfsInstance.deviceState = kStatus_DEV_Idle;
                }
                USB_HostMsdDeinit(g_MsdFatfsInstance.deviceHandle,
                		g_MsdFatfsInstance.classHandle); /* msd class de-initialization */
                g_MsdFatfsInstance.classHandle = NULL;

				usb_echo("mass storage device detached\r\n");
            }
            break;

        default:
            break;
    }
    return status;
}
void USB_HostTaskFn()
{
    USB_HostKhciTaskFunction(g_HostHandle);
}
void USB0_IRQHandler(void)
{
    USB_HostKhciIsrFunction(g_HostHandle);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}
#endif /* USB_DISK_ENABLE */
