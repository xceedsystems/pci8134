/********************************************************************

                                PciStuff.c

    Interface specific code. 
    This file only should touch the hardware.

*********************************************************************/


#include "stdafx.h"

#include <rt.h>
#include <pcibus.h>     // 
#include <string.h>     // strlen()
#include <stdio.h>      // sprintf()
#include <stdlib.h>     // malloc()

#include "vlcport.h"
#include "dcflat.h"             // EROP()
#include "driver.h"             // SEMAPHORE
#include "errors.h"             // IDS_SYNERGETIC_HW_TEST
#include "auxrut.h"             // StartTimeout(), IsTimeout(), EROP
#include "pcistuff.h"           // Init()





/******************* Local definitions *******************************/



/******************* Initialization  *******************************/


/*
// ****************************************************************
//
//  PCI helper functions:
//
///////////////////////////////////////////////////////////////
//
//  Configuration registers in general and BARs in particular
//
*/

/* Make sure the configuration access parameters are valid */
static int CheckConfigRegister( UINT32 Offset, UINT32 Size, UINT8* pSizeCode )
{
    int rc = SUCCESS;

    if( Offset + Size > 256 )
        rc = IDS_ERR_PCI_OUTOF_IOIMAGE;
    else
    {
        switch( Size )
        {
            case 1:
                    *pSizeCode = T_BYTE;
                    break;
            case 2:
                    *pSizeCode = T_WORD;
                    break;
            case 4:
                    *pSizeCode = T_DWORD;
                    break;
            default:
                    *pSizeCode = 0;
                    rc = IDS_ERR_PCI_INVALID_REGSIZE;
        }
    }

    return rc;
}

static int WriteConfigRegister( PCIDEV* pPciDev, UINT32 Offset, UINT32 Size, UINT32 Value )
{
    UINT8 SizeCode;
    int rc = CheckConfigRegister( Offset, Size, &SizeCode );
    if( rc == SUCCESS )
        PciSetConfigRegister( pPciDev, (UINT8)Offset, SizeCode, (DWORD)Value );
    return rc;
}

static int ReadConfigRegister( PCIDEV* pPciDev, UINT32 Offset, UINT32 Size, UINT32* pValue )
{
    UINT8 SizeCode;
    int rc = CheckConfigRegister( Offset, Size, &SizeCode );
    if( rc == SUCCESS )
        *pValue = PciGetConfigRegister( pPciDev, (UINT8)Offset, SizeCode );
    return rc;
}



int vlcWriteConfigRegister( UINT32 PciBusNum, UINT32 PciSlotNum, UINT32 Offset, UINT32 Size, UINT32 Value )
{
    PCIDEV PciDev;
    memset( &PciDev, 0, sizeof( PCIDEV ) );
    PciDev.wBusNum    = (UINT16)PciBusNum;
    PciDev.wDeviceNum = (UINT16)PciSlotNum;
    return WriteConfigRegister( &PciDev, Offset, Size, Value );
}

int vlcReadConfigRegister( UINT32 PciBusNum, UINT32 PciSlotNum, UINT32 Offset, UINT32 Size, UINT32* pValue )
{
    PCIDEV PciDev;
    memset( &PciDev, 0, sizeof( PCIDEV ) );
    PciDev.wBusNum    = (UINT16)PciBusNum;
    PciDev.wDeviceNum = (UINT16)PciSlotNum;
    return ReadConfigRegister( &PciDev, Offset, Size, pValue );
}


static int ReadHeaderType( PCIDEV* pPciDev, UINT8* pHeaderType )
{
    UINT32 HeaderType;
    UINT32 Offset = 0x0e;
    int rc = ReadConfigRegister( pPciDev, Offset, 1, &HeaderType );
    if( rc == SUCCESS )
        *pHeaderType = (UINT8)HeaderType;
    return rc;
}

static int ReadBar( PCIDEV* pPciDev, int BarIndex, UINT32* pValue )
{
    UINT32 Offset = BarIndex * sizeof(UINT32) + 0x10;
    return ReadConfigRegister( pPciDev, Offset, 4, pValue );
}

static int WriteBar( PCIDEV* pPciDev, int BarIndex, UINT32 Value )
{
    UINT32 Offset = BarIndex * sizeof(UINT32) + 0x10;
    return WriteConfigRegister( pPciDev, Offset, 4, Value );
}



/*
///////////////////////////////////////////////////////////////
//
//  Find the configuration for the specified 
//  Vendor/Device/Function set.
//
*/

/* returns TRUE if the next index is found. */
static int vlcPciFindNextDeviceOrDuplicate( PCIDEV* pPciDev )
{

    EROP( "pPciDev->wDeviceIndex  =%d,\n",pPciDev->wDeviceIndex,0, 0, 0 );

    pPciDev->wDeviceIndex++;
    return PciFindDevice( pPciDev );
}


/* returns TRUE if the next different index is found. */
static int vlcPciFindNextDevice( PCIDEV* pPciDev )
{
    int bFound = 0;
    if( pPciDev->wFunction )
    {
        /* Multi-function device */
        bFound = vlcPciFindNextDeviceOrDuplicate( pPciDev );
    }
    else
    {
        /* Function 0. */
        UINT8 HeaderType;
        if( ReadHeaderType( pPciDev, &HeaderType ) == SUCCESS )
        {
            int bMultiFunctionDevice = HeaderType & 0x80;   /* see PCI specs for Header Type */

    EROP( "bMultiFunctionDevice  =%x,\n",bMultiFunctionDevice,0, 0, 0 );


            if( bMultiFunctionDevice )
            {
                /* Multi-function device  */
                bFound = vlcPciFindNextDeviceOrDuplicate( pPciDev );
            }
            else
            {
	            /*
                    Single function device.  May show up as more dups, which we need to skip.
                    All duplicates share the same slot. 
                */
                UINT16 BusNum    = pPciDev->wBusNum;
	            UINT16 DeviceNum = pPciDev->wDeviceNum;
                int    bSameSlot;
                
                do
                {
                    bFound    = vlcPciFindNextDeviceOrDuplicate( pPciDev );
                    bSameSlot = ( ( BusNum == pPciDev->wBusNum ) && ( DeviceNum == pPciDev->wDeviceNum ) );

						EROP( "pPciDev->wBusNum  =%d,pPciDev->wDeviceNum  =%d  \n",pPciDev->wBusNum,pPciDev->wDeviceNum , 0, 0 );


                } while( bFound && bSameSlot );     /* skip all duplicates */
            }
        }
    }
    
    return bFound;
}


/* returns TRUE if there is at least 1 device. */
static int vlcPciFindFirstDevice( PCIDEV* pPciDev, int VendorId, int DeviceId )
{
    memset( pPciDev, 0, sizeof(PCIDEV) );
    pPciDev->wVendorId = VendorId;
    pPciDev->wDeviceId = DeviceId;
    return PciFindDevice( pPciDev );
}




/* BoardIndex is 1 based */
/* called by InitPCI() */
static int FindPCIBoard( LPDRIVER_INST pNet, P_ERR_PARAM pErrors, PCIDEV* pPciDev )
{
    int rc = SUCCESS; 

    UINT16 const  BoardIndex         = pNet->PciIndex;	/* 1-based index,  key to our board. */
    UINT16 const  ExpectedBoardCount = pNet->PciCount;	/* 0 --> any # of boards.  For check purposes only. */
    UINT16        FoundBoardCount    = 0;               /* how many boards are pluged in this system        */

    if( rc == SUCCESS )
    {
        PCIDEV PciDev;

        /*
            TO DO: 
            Define here he VENDOR ID and the DEVICE ID  for our PCI card. 
        */

        const   int VendorId = 0x144a;  // TO DO: define here the VENDOR ID
        const   int DeviceId = 0x8134;  // TO DO: define here the DEVICE ID

        int     bDone  = FALSE;                          /* when to stop looking for new boards of this type. */
        int     bFound = vlcPciFindFirstDevice( &PciDev, VendorId, DeviceId );
        for( ;  bFound && !bDone ; bFound = vlcPciFindNextDevice( &PciDev ) )
        {
            if( ++FoundBoardCount == BoardIndex )
            {
                /* Found our board.  PciDev contains a handle to it. */
                if( !ExpectedBoardCount )   /* if no BoardCount check requested,                   */
                    bDone = TRUE;           /* no need to find the exact # of boards of this type. */

				EROP( "FOUND pPciDev->wBusNum  =%d,pPciDev->wDeviceNum  =%d  \n",pPciDev->wBusNum,pPciDev->wDeviceNum , 0, 0 );

                memcpy( pPciDev, &PciDev, sizeof( PCIDEV ) );
            }
        }
    }

    if( rc == SUCCESS )
    {
        if( FoundBoardCount < BoardIndex )
        {
            /* Not enough boards available on this system */
            rc = IDS_ERR_PCI_CANNOT_FIND_PCIDEVICE;
            sprintf( pErrors->Param3, "%d", BoardIndex );
            sprintf( pErrors->Param4, "%d", FoundBoardCount );
        }
    }

    if( rc == SUCCESS )
    {
        if( ExpectedBoardCount && ( ExpectedBoardCount != FoundBoardCount ) )
        {
            /* BoardCount check requested and failed. */
            rc = IDS_ERR_PCI_BOARD_NO_MISMATCH;
            sprintf( pErrors->Param3, "%d", ExpectedBoardCount );
            sprintf( pErrors->Param4, "%d", FoundBoardCount    );
        }
    }

    return rc;
}


U8 PciFinxDevice(PCIDEV_OLD *pPciDevice)
{

	PCIDEV PciDevice;
	U8 rc = 0;

   EROP( "PciFinxDevice \n",0,0, 0, 0 );

	PciDevice.wDeviceIndex = pPciDevice->wDeviceIndex;
	PciDevice.wVendorId  = pPciDevice->wVendorId;
	PciDevice.wDeviceId  = pPciDevice->wDeviceId;



	rc = PciFindDevice( &PciDevice );


	//pPciDevice->byCLS

	pPciDevice->wBusNum = PciDevice.wBusNum;
	pPciDevice->wDeviceNum  = PciDevice.wDeviceNum;
	pPciDevice->wFunction  = PciDevice.wFunction;
	pPciDevice->wVendorId  = PciDevice.wVendorId;
	pPciDevice->wDeviceId  = PciDevice.wDeviceId;
	pPciDevice->wDeviceIndex = PciDevice.wDeviceIndex;
//	pPciDevice->wCommand  = PciDevice.;
	pPciDevice->wClassId  = PciDevice.wClassId;
	pPciDevice->byInterfaceId = PciDevice.byInterfaceId;
	pPciDevice->byRevId = PciDevice.byRevId;
	pPciDevice->byCLS = PciDevice.byCLS;
	pPciDevice->byLatency = PciDevice.byLatency;
	pPciDevice->dwBaseAddr[0] = PciDevice.dwBaseAddr[0];
	pPciDevice->dwBaseAddr[1] = PciDevice.dwBaseAddr[1];
	pPciDevice->dwBaseAddr[2] = PciDevice.dwBaseAddr[2];
	pPciDevice->dwBaseAddr[3] = PciDevice.dwBaseAddr[3];
	pPciDevice->dwBaseAddr[4] = PciDevice.dwBaseAddr[4];
	pPciDevice->dwBaseAddr[5] = PciDevice.dwBaseAddr[5];

	pPciDevice->dwCIS = PciDevice.dwCIS;
	pPciDevice->wSubSystemVendorId = PciDevice.wSubSystemVendorId;
	pPciDevice->wSubSystemId = PciDevice.wSubSystemId;
	pPciDevice->dwRomBaseAddr = PciDevice.dwRomBaseAddr;
	pPciDevice->byIntLine = PciDevice.byIntLine;
	pPciDevice->byIntPin = PciDevice.byIntPin;
	pPciDevice->byMaxLatency = PciDevice.byMaxLatency;
	pPciDevice->byMinGrant = PciDevice.byMinGrant;


    EROP( "PciDev.dwBaseAddr[ 0 ]  =%x,\n",pPciDevice->dwBaseAddr[ 0 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 1 ]  =%x,\n",pPciDevice->dwBaseAddr[ 1 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 2 ]  =%x,\n",pPciDevice->dwBaseAddr[ 2 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 3 ]  =%x,\n",pPciDevice->dwBaseAddr[ 3 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 4 ]  =%x,\n",pPciDevice->dwBaseAddr[ 4 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 5 ]  =%x,\n",pPciDevice->dwBaseAddr[ 5 ],0, 0, 0 );
	EROP( "pPciDev->wBusNum  =%d,pPciDev->wDeviceNum  =%d  \n",pPciDevice->wBusNum,pPciDevice->wDeviceNum , 0, 0 );


 return rc;
}


/* called by rtOpen() */
int InitPCI( LPDRIVER_INST pNet, P_ERR_PARAM pErrors , PCI_INFO *pciInfo )
{
    PCIDEV PciDev;                             /* work variable */
int rc =0;
			WORD level=0;
			BYTE irq=0;
			int	n = 0;

    		PciInitialize(0);
	 rc = FindPCIBoard( pNet, pErrors, &PciDev );


    if( rc == SUCCESS )
    { // get interrupt IRQ
	//	pNet->PhyAddr = PciDev.dwBaseAddr[ 0 ] & 0xfffffff0;

		PciEnableDevice( &PciDev );
		/* or intime < 3.0
		wCmd = (WORD)PciGetConfigRegister(&PciDev,0x04,T_WORD);
		wCmd |=0x0003;
		PciSetConfigRegister(&PciDev,T_WORD,(DWORD)wCmd);
		*/

			if (PciDev.byIntLine !=0 && PciDev.byIntLine !=255)
			{
				irq = PciDev.byIntLine ;
				level = (irq <8) ? (irq*16 + 8) : ((1+irq/8)*16 + (irq%8));// GetPciInterruptLevel( &dev );
			}

			// adlink 8134 needs 

			n = (pNet->PciIndex) -1;

			pNet->PhyAddr = irq;
			pciInfo->baseAddr[n] = PciDev.dwBaseAddr[ 2 ] &0xfffc;
			pciInfo->lcrAddr[n] = PciDev.dwBaseAddr[ 1 ] &0x0fffc;
			pciInfo->irqNo[n] = irq;

	}
	

    EROP( "PciDev.dwBaseAddr[ 0 ]  =%x,\n",PciDev.dwBaseAddr[ 0 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 1 ]  =%x,\n",PciDev.dwBaseAddr[ 1 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 2 ]  =%x,\n",PciDev.dwBaseAddr[ 2 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 3 ]  =%x,\n",PciDev.dwBaseAddr[ 3 ],0, 0, 0 );
    EROP( "PciDev.dwBaseAddr[ 4 ]  =%x,\n",PciDev.dwBaseAddr[ 4 ],0, 0, 0 );

    EROP( "pciInfo->baseAddr[%d]  =%x,\n",n,pciInfo->baseAddr[n], 0, 0 );
    EROP( "pciInfo->lcrAddr[%d]  =%x,\n",n,pciInfo->lcrAddr[n], 0, 0 );
    EROP( "pciInfo->irqNo[%d]  =%x,\n",n,pciInfo->irqNo[n], 0, 0 );


    EROP( "irq =%d,\n",irq,0, 0, 0 );
    EROP( "level  =%d,\n",level,0, 0, 0 );
    EROP( "n  =%d, (pNet->PciIndex) = % d \n",n,(pNet->PciIndex), 0, 0 );


    return rc;
}



/*
dwMemAddr = dev.dwBaseAddr[x];
if (! (dwMemAddr % 0x1000L)){
	dwOffset = 0;
		dwPageBase = dwMemAddr;
		dwMapSize = 0x100000L;

}else{
	dwPageBase = (dwMemAddr / 0x1000L) * 0x1000L;
	dwOffset = dwMemAddr - dwPageBase;
	dwMapSize = 0x100000L + (0x1000L*2);
}

  dwp = (DWORD*)MapRtPhysicalMemory(dwPageBase,dwMapSize);

  http://www.mnc.co.jp/english/INtime/faq07-2_kanren/mechanism_of_PCIdevices_and_its_control.htm

*/
























