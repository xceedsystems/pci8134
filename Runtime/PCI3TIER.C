/************************************************************

                Pci3tier.c


This file implements all the module entry points: 

int rtIdentify( P_IDENTITY_BLOCK* ppIdentityBlock );
int rtLoad(   UINT32 ScanRate, UINT32* rDirectCalls );
int rtOpen(   LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtReload( LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtOnLine( LPDRIVER_INST lpNet, P_ERR_PARAM lpErrors);
int rtInput(  LPDRIVER_INST lpNet);
int rtOutput( LPDRIVER_INST lpNet);
int rtSpecial(LPDRIVER_INST lpNet, LPSPECIAL_INST lpData);
int rtOffLine(LPDRIVER_INST lpNet, P_ERR_PARAM  lpErrors);
int rtClose(  LPDRIVER_INST lpNet, P_ERR_PARAM  lpErrors);
int rtUnload( );

**************************************************************/


#include "stdafx.h"


/*********************************************************/
/*                                                       */
/*                Pci3tier Sample Program                */
/*                                                       */
/*********************************************************/ 
                 
#include <rt.h>

#include "vlcport.h"
#include "CSFlat.h"     // FCoreSup
#include "DCFlat.h"     // FDriverCore
#include "driver.h"

#include "version.h"
#include "auxrut.h"
#include "pci3tier.h"   // DUAL_PORT
#include "task.h"
#include "card.h"
#include "pcistuff.h"
#include "Pci_8134.h"
#include "8134drv.h"

 PCI_INFO info;


int rtIdentify( P_IDENTITY_BLOCK* ppIdentityBlock )
{
    static IDENTITY_BLOCK IdentityBlock; 
    IdentityBlock.DriverId   = DriverPCI3TIER;
    IdentityBlock.DriverVers = PCI3TIERVERS;
    IdentityBlock.pName      = PRODUCT_NAME ", " PRODUCT_VERSION;
    *ppIdentityBlock = &IdentityBlock;
    return 0;
}

int rtLoad( UINT32 ScanRate, UINT32* rDirectCalls )
{
    // Executing the LOAD PROJECT command

    #if defined( _DEBUG )
        SetDebuggingFlag( 1 );  // Disable the VLC watchdog, so we can step through our code. 
    #endif  // _DEBUG


    // Use direct calls for very fast applications.  
    // With the appropriate bit set, Input(), Output() and/or Special()
    //  can be directly called from the engine thread, 
    //  saving the delay introduced by a task switch. 
    // Note:  Functions exectuted in the engine thread cannot call 
    //  some C stdlib APIs, like sprintf(), malloc(), ...
    
    // *rDirectCalls = ( DIRECT_INPUT | DIRECT_OUTPUT | DIRECT_SPECIAL );



    EROP( "rtLoad() ScanRate=%d, rDirectCalls=%x", ScanRate, *rDirectCalls, 0, 0 );

    return 0;
}

int rtOpen( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the LOAD PROJECT command

    int rc = SUCCESS;
    LPDEVICE_INST pDevice;


	int      bn, AxisNum,x;
    U16 cbn=0;
    if( pNet->Sentinel != RT3_SENTINEL )
        rc = IDS_VLCRTERR_ALIGNMENT;

    if( rc == SUCCESS )
    {
        UINT32* pSentinel = BuildUiotPointer( pNet->ofsSentinel );
        if( *pSentinel != RT3_SENTINEL )
            rc = IDS_VLCRTERR_ALIGNMENT;
    }

    EROP( "rtOpen() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );

    if( rc == SUCCESS )
	{
        pNet->pDeviceList = BuildUiotPointer( pNet->ofsDeviceList );
		for( pDevice = pNet->pDeviceList; pDevice->Type && ( rc == SUCCESS ) ; pDevice++ )

		{
			if( pDevice->Sentinel != RT3_SENTINEL )
				rc = IDS_VLCRTERR_ALIGNMENT;
			else
			{
                // Create UIOT pointers
				pDevice->pName = BuildUiotPointer( pDevice->ofsName );
                if( pDevice->Input.bUsed )
                    pDevice->Input.pDst  = BuildUiotPointer( pDevice->Input.ofsUiot );
                if( pDevice->Output.bUsed )
                    pDevice->Output.pDst = BuildUiotPointer( pDevice->Output.ofsUiot );
			}
		}
	}

    if( !pNet->bSimulate )
	{
     	bn= (pNet->PciIndex) ;
		bn=bn- 1;


//	if (bn==0)

//		rc = InitPCI( pNet, pErrors, &info  );  // Load the physical address of the PCI card in pNet

		// init Pci motion card

		// init Once

	if (bn==0)
	{

		EROP( "_8134_Initial , cbn =%d\n",cbn,0, 0, 0 );


		rc=	_8134_Initial( &cbn,  &info  );
		EROP( "cbn = %d \n",cbn,0, 0, 0 );

#if defined( _DEBUG )
		x=0;
		for (;x < cbn*4 ; x++ )
		{

		EROP( "1st info.baseAddr[%d] = %x \n",x, info.baseAddr[0],0,  0 );
		EROP( "info.lcrAddr[%d] = %x \n",x,info.lcrAddr[0],0,  0 );
		EROP( "info.irqNo[%d] = %x \n",x,info.irqNo[0],0,  0 );
		}

#endif
		
		x=0;
		for (;x < cbn*4 ; x++ )
		{
//		AxisNum = ((bn * 4)+x) ; 
		AxisNum = x;

//		_8134_pclstr(x);
		rc= set_cnt_src((U16) AxisNum, 0);
		rc= set_pls_iptmode((U16) AxisNum, 2);
		rc= set_pls_outmode((U16) AxisNum, 0);

		rc=set_home_config((U16)AxisNum,0,1,0,1);

		// rc= set_inp_logic((U16) AxisNum,1,0);
		rc= set_alm_logic((U16) AxisNum,0,0);
		// rc=set_sd_logic((U16) AxisNum,1,0,0);

		rc= set_move_ratio(	(U16)AxisNum,1.0);
		set_manu_iptmode((U16)AxisNum,2,0);
		
		rc = fix_max_speed((U16) AxisNum, 2500000);

		rc=set_erc_enable((U16) AxisNum,0);


		}


	}

        if( rc == SUCCESS )
            rc = CreateBackgroundTask(pNet);

	}

    return rc;
}

int rtReload( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the LOAD PROJECT command
    EROP( "rtReload() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0);
    if( !pNet->bSimulate )
    {
        InitLinkedList(&pNet->Pend);
        InitLinkedList(&pNet->Done);
    }

    // make sure pNet is in the same state as after rtOpen(). 
    return 0;
}

int rtOnLine( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    int	rc = SUCCESS;

	EROP( "rtOnLine() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );
    pNet->bFirstCycle = 1;
    pNet->bGoOffLine  = 0;

    if( !pNet->bSimulate )
    {
        /* Check all devices. If critical devices are offline,  rc = IDS_PCI3TIER_DEVICE_OFFLINE */

        rc = TestConfig( pNet, pErrors);
    }

	EROP( "rtOnLine(). exit.", 0, 0, 0, 0 );

    return rc;

}


int rtInput( LPDRIVER_INST pNet ) 
{

	int	bn;
    int     rc       = SUCCESS;

	// EROP( "rtInput() pNet=%p", pNet, 0, 0, 0 );
    // This is the beginning of the VLC scan cycle
    if( !pNet->bSimulate )
    {


		// test get io status

	
 

		// Copy new input data from the hw to the driver input image in the UIOT. 
		LPDEVICE_INST pDevice = pNet->pDeviceList;

		bn=(pNet->PciIndex) -1;

		for( ; pDevice->Type ; pDevice++ )
		{
			if( pDevice->Input.bUsed )
			{

				// Start Read IO,INT,POS from adlink card
				// bn crd number 0~3
				rc= ADlinkReadIO(pDevice, bn, pDevice->Input.pDst );

			}
		}



    VerifyDoneList(&pNet->Done);    // Flush the completed background functions
    }

//	EROP( "rtInput(). exit", 0, 0, 0, 0 );

    return SUCCESS;
}


int rtOutput( LPDRIVER_INST pNet)
{
   // This is the end of the VLC scan cycle
    if( !pNet->bSimulate )
    {
        // Copy new output data from the UIOT driver output image to the hw.
		LPDEVICE_INST pDevice = pNet->pDeviceList;
    
		for( ; pDevice->Type ; pDevice++ )
            if( pDevice->Output.bUsed )
//                CardCopy( pDevice->Output.pDst, pDevice->Output.pSrc, pDevice->Output.Size);

        if( pNet->bFirstCycle )     // first Output() ?
        {
            //  Only now we have a valid output image in the DPR. 
            //    EnableOutputs(dp);  enable outputs (if our hardware lets us) 
            
            pNet->bFirstCycle = 0;
        }       
    }

 //   EROP( "rtOutput() pNet=%p", pNet, 0, 0, 0 );

    return SUCCESS;
}

int rtSpecial( LPDRIVER_INST pNet, LPSPECIAL_INST pData)
{
    // A trapeziodal block has been hit, function found in card.c

    UINT16  Result = 0;
    UINT16  Status = VLCFNCSTAT_OK;
    
	// get devicelist
	LPDEVICE_INST pDevice = pNet->pDeviceList;


    EROP( "rtSpecial() pNet=%p, pData=%p", pNet, pData, 0, 0 );

    if( !pNet->bSimulate )
    {
        int  FunctionId = pData->User.paramHeader.FunctionId;
        switch( FunctionId ) 
        {
			case DRVF_MOTION:
				AdLinkMotion( pNet, pData );
				break;       

			case DRVF_SETGET:
				AdLinkSetGet( pNet, pData );
				break;       

			case DRVF_OTHERS:
				AdLinkOthers( pNet, pData );
				break;       
         	default:
                    Status = VLCFNCSTAT_WRONGPARAM;
                    break;
        }
    
        EROP("Special();  FunId= %d, Status= %d, pData= %p", FunctionId, Status, pData, 0);
    }
    else
    {
		UINT16* pResult = BuildUiotPointer( pData->User.paramHeader.ofsResult );
        if( pResult )   // some functions may not have the Result param implemented
		    *pResult = (UINT32) SUCCESS;

        Status = VLCFNCSTAT_SIMULATE;
    }

    if( pData->User.paramHeader.ofsStatus )   // some functions may not have the status param implemented
	{
		UINT16* pStatus = BuildUiotPointer( pData->User.paramHeader.ofsStatus );
		*pStatus = Status;
	}
    
    return SUCCESS;
}

int rtOffLine( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    // Executing the STOP PROJECT command
    int rc = SUCCESS;
	UINT16 bn=0;

    EROP( "rtOffLine() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );

    pNet->bGoOffLine = 1;
    if( !pNet->bSimulate )
    {

		bn= (pNet->PciIndex) - 1;
		// init Pci motion card

		rc=	stop_motion();
//		_8134_Close( bn );
//		_8134_Software_Reset(bn);


        rc = WaitForAllFunctionCompletion(pNet);  /* wait for the backgroung task to calm down */
        
        if( rc == SUCCESS )
        {
            /*
            DUAL_PORT far *  dp  = (DUAL_PORT far *)pNet->pDpr;
            if( pNet->StopState == 1 )
                rc = stop scanning;
    
            DisableOutputs(dp, &pNet->trans_count);
            DisableWD(dp); 
            */
        }
        
    }    

    EROP("rtOffLine(). exit  rc= %d", rc, 0, 0, 0);

    return rc;
}

/*   if Open() fails, Close() is not automatically called for this instance.
     if lpErrors == NULL, do not report any error, keep the Open()'s error code and params.  
 */ 
int rtClose( LPDRIVER_INST pNet, P_ERR_PARAM pErrors)
{
    int rc = SUCCESS;

		UINT16 bn=0;

    // Executing the UNLOAD PROJECT command
    if( !pNet->bSimulate )
    {
        EROP("rtClose(). start. pNet= %p", pNet, 0, 0, 0);


				bn= (pNet->PciIndex) - 1;
		// init Pci motion card

		rc=	stop_motion();
		_8134_Close( bn );
//		_8134_Software_Reset(bn);



        /*
        {
            DUAL_PORT far* const dp = (DUAL_PORT far *)pNet->pDpr;     / * pointer to the dualport * /
            Reset the board;
        }
        */
        
        //DeleteInterruptTask( pNet );
        DeleteBackgroundTask( pNet );
    
		if( pNet->pDpr )
        {
            FreeDpr( pNet->pDpr );
            pNet->pDpr = NULL;
        }

    }

    EROP( "rtClose() pNet=%p, pErrors=%p", pNet, pErrors, 0, 0 );
    return SUCCESS;
}

int rtUnload()
{
    // Executing the UNLOAD PROJECT command
    EROP( "rtUnload()", 0,0,0,0 );
    return 0;
}



