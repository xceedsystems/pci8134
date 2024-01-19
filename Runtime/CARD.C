/********************************************************************

                                Card.c

    Interface specific code. 
    This file only should touch the hardware.

*********************************************************************/


#include "stdafx.h"

#include <rt.h>
#include <string.h>     // strlen()
#include <stdio.h>      // sprintf()

#include "vlcport.h"
#include "dcflat.h"     // EROP()
#include "driver.h"     /* SEMAPHORE */
#include "errors.h"     /* IDS_RT_DP_RW_TEST                     */
#include "auxrut.h"     /* StartTimeout(), IsTimeout(), EROP     */
#include "pci3tier.h"   /* DUAL_PORT                             */
#include "card.h"       /* Init()                                */
#include "pci_8134.h"
//#include "8134def.h"

static UINT32 INTarray[MAX_PCI_CARDS *4];



/******************* Card specific  Functions  *******************************/


/******************* Initialization  *******************************/


static int TestAndFill(UINT8* pc, const int Size, const int test, const int fill)   /* test == 1  --> no test */
{
    int i  = 0;
    for(; i < Size;  *pc++ = fill, i++)
    {
        int c = *pc & 255;
        if(test != 1  &&  test != c)
        {
            EROP("Ram Error.  Address %p, is 0x%02x, and should be 0x%02x", pc, c, test, 0);
            return IDS_PCI3TIER_HW_TEST;
        }
    }
    return SUCCESS;
}


int  Init( LPDRIVER_INST pNet, P_ERR_PARAM const lpErrors)
{
    int rc = SUCCESS;

    return rc;
}



/****************************************************************************************
    IN:     pName   --> pointer to the device user name
            Address --> device's network address
            pBuf    --> pointer to the destination buffer
            szBuf   --> size of the destination buffer

    OUT:    *pBuf   <-- "Address xx (usr_name)".  
    Note:   The device user name may be truncated!
*/
static void LoadDeviceName( char* pName, UINT16 Address, char* pBuf, size_t szBuf )
{
    if( szBuf && (pBuf != NULL) )
    {
        char* format = "Address %d";

        *pBuf = '\0';

        if( szBuf > (strlen(format)+3) )    /* Address may need more digits */
        {
            size_t  len;

            sprintf(pBuf, format, Address & 0xffff);

            len = strlen( pBuf ); 

            if( pName && ((szBuf - len) > 10) )     /* if we still have 10 free bytes  */
            {
                strcat(pBuf, " (");
                len += 2;
                strncat( pBuf, pName, szBuf-len-2 );
                *(pBuf + szBuf - 2) = '\0';
                strcat( pBuf, ")" );
            }
        }
    }
}



int  TestConfig( LPDRIVER_INST const pNet, P_ERR_PARAM const lpErrors )
{
    int rc = SUCCESS;

    LPDEVICE_INST pDevice = (LPDEVICE_INST)pNet->pDeviceList;
        
    for( ; pDevice->Type && (rc == SUCCESS); pDevice++ )
    {
        
		// TO DO:
		DUAL_PORT* const dp = NULL; // (DUAL_PORT*)pNet->pDpr;     /* pointer to the dualport */

        pDevice->bPresent = 1;

        /*
        Check pDevice. 
        if( the device is not on the network )
            pDevice->bPresent = 0;
        */

		/*
		printf( "TO DO File=%s, line=%d \n" __FILE__, __LINE__ );
        
        if( !pDevice->bPresent)
        {
            LoadDeviceName( pDevice->pName, pDevice->Address, lpErrors->Param3, sizeof(lpErrors->Param3) );
            rc = IDS_PCI3TIER_DEVICE_OFFLINE; 
        }
		*/
    }

    return rc;
}


/********************* runtime specific card access functions ********************/


int	DoCollect( LPDRIVER_INST pNet, LPSPECIAL_INST pData)
{
    int     rc       = SUCCESS;
//	int		channel;
//	UINT16	*ChanBuff[16];
//	UINT16	NumSamples = pData->Work.paramCommand.NumSamps;
	UINT16* pResult = BuildUiotPointer( pData->Work.paramHeader.ofsResult );
   
//	for (channel = 0; (channel < 16) && (rc == SUCCESS); channel++)
//	{
//		LPPTBUFFER pRBuffer = &pData->Work.paramCommand.Buffers[channel];

//		printf("Channel %d  Length %d  Offset %d\n", 
//			channel, pRBuffer->Size, pRBuffer->Offset);

//		ChanBuff[channel] = BuildUiotPointer( pRBuffer->Offset );
//		if( pRBuffer->Size > NumSamples)
//		{
//			rc = IDS_PCI3TIER_READ_SIZE;
//		}
//	}

	// At this time, ChanBuf[i] is a pointer to the buffer area for channel i.
	// Insert your code here.
	*pResult = rc;
	return  (rc);
}


int ADlinkReadIO( LPDEVICE_INST const pDevice, int bn, VOID *Dest )
{

    int     rc       = SUCCESS;
	UINT16 ValueGet16;
	UINT16	AxisNum; //	IntAxisNum;
	double	ValuePos;
	float	ValuePos32;
//	UINT32	IntValue;


 switch(pDevice->Type ) 
    {
        case DEVICE_CARD_POS:
		//0~3
		AxisNum = (bn * 4) + pDevice->Address;

		rc= get_position(AxisNum, &ValuePos);
		ValuePos32=ValuePos;	// conver to 32 bits

		*((UINT32  volatile*)Dest) = (UINT32)ValuePos;
		
		break;

		case DEVICE_CARD_IO_STATUS1:
		AxisNum = (bn * 4) + pDevice->Address;


		rc= get_io_status(AxisNum, &ValueGet16);

		*((UINT16  volatile*)Dest) = (UINT16) ValueGet16;

		ValueGet16= motion_done(AxisNum);
		*((UINT16  volatile*)Dest+1) = (UINT16) ValueGet16;

			
		break;

		case DEVICE_CARD_INT_STATUS1:
		AxisNum = (bn * 4) + pDevice->Address;

//			rc= get_int_axis( &IntAxisNum);

//			if ((rc==0)  && (IntAxisNum < MAX_PCI_CARDS *4))
//			{
//			rc= get_int_status(IntAxisNum, &IntValue );
//			INTarray[IntAxisNum] = (UINT32) IntValue;
//			}

			ValueGet16= motion_done(AxisNum);


			*((UINT32  volatile*)Dest) = (UINT32) ValueGet16;

		//	if ((IntAxisNum == AxisNum) && (rc==0))
		//	{
		//	rc= get_int_status(IntAxisNum, &IntValue );
		// 	 *((UINT32  volatile*)Dest) = (UINT32) IntValue ;
		//	}

			
			//			else
//			 *((UINT32  volatile*)Dest) = (UINT32) 0 ;


		break;            
    }




	return  (rc);

}


void AdLinkMotion( const LPDRIVER_INST pNet, SPECIAL_INST* const pData )
{
    int rc = SUCCESS ;

	UINT16 bn;
		
    SPECIAL_INST_COMMAND* const pUser = &pData->User.paramCommand;

    UINT16* pResult = BuildUiotPointer( pUser->Header.ofsResult );
 	*pResult=rc;	

//	bn=(((pNet->PciIndex) -1) *4) + pUser->Address; // axis number

	bn=(((pNet->PciIndex) -1) *4) + pUser->Address; // axis number

 switch(pUser->Function  ) 
    {

	
	case	START_AA_MOVE:
	*pResult= start_aa_move(bn, pUser->fPos , pUser->stVel, pUser->maxVel, pUser->Accl);
	break;
	case	START_A_MOVE:
	*pResult=	start_a_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl);
		break;
	case	A_MOVE:
	*pResult=	a_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl);
		break;
	case	START_R_MOVE:
	*pResult=	start_r_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl);
		break;
	case	R_MOVE:
	*pResult=	r_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl);
		break;
	case	START_T_MOVE: 
	*pResult=	start_t_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl, pUser->TDeccl);
	
		break;
	case	T_MOVE:
	*pResult=	t_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl, pUser->TDeccl);
		break;
	case	START_TA_MOVE:
	*pResult=	start_ta_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl, pUser->TDeccl);
		break;
	case	TA_MOVE:
	*pResult=	ta_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->Accl, pUser->TDeccl);
		break;
	case	START_S_MOVE:
	*pResult=	start_s_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->T1Accl, pUser->TSAccl);
		break;
	case	S_MOVE:
	*pResult=	s_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->T1Accl, pUser->TSAccl);
		break;
	case	START_RS_MOVE:
	*pResult=	start_rs_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->T1Accl, pUser->TSAccl);
		break;
	case	RS_MOVE:
	*pResult=	rs_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->T1Accl, pUser->TSAccl);
		break;
	case	START_TAS_MOVE:
	*pResult=	start_tas_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->T1Accl, pUser->TSAccl,pUser->T1Deccl,pUser->TSDeccl);
		break;
	case	TAS_MOVE:
	*pResult=	tas_move(bn, pUser->fPos, pUser->stVel, pUser->maxVel, pUser->T1Accl, pUser->TSAccl,pUser->T1Deccl,pUser->TSDeccl);
		break;
	case	V_MOVE:
	*pResult=	v_move(bn, pUser->stVel, pUser->maxVel, pUser->Accl);
		break;
	case	SV_MOVE:
	*pResult=	sv_move(bn, pUser->stVel, pUser->maxVel, pUser->T1Accl, pUser->TSAccl);
		break;
	case	HOMEMOVE:
	*pResult=	home_move(bn, pUser->stVel,pUser->maxVel,pUser->Accl);
	
		break;

	case	V_CHANGE:
	*pResult=	v_change(bn, pUser->maxVel, pUser->Accl);
		break;
	case	MANU_MOVE:
	*pResult=	manu_move(bn, pUser->fPos);
		break;

	}

}

void AdLinkSetGet( const LPDRIVER_INST pNet, SPECIAL_INST* const pData )
{
    int rc = SUCCESS ;
	UINT16 bn;
//	F64 PosRead;		
    SPECIAL_INST_SETGET* pUser = &pData->User.paramSetGet;

    double* pGetValue = BuildUiotPointer( pUser->GetValue  );

    UINT16* pResult = BuildUiotPointer( pUser->Header.ofsResult );
	*pResult=rc;	



	bn=(((pNet->PciIndex) -1) *4) + pUser->Address; // axis number
	

 switch(pUser->Function  ) 
    {
	case	STARTMOTION:
	*pResult= start_motion();
	*pGetValue = (double) *pResult;

	break;
	case	STOPMOTION:
	*pResult=stop_motion();
	*pGetValue = (double) *pResult;

	break;
	case	V_STOP: 
	*pResult=v_stop(bn,pUser->SetValue );
	*pGetValue = (double) *pResult;

	break;
	case	AXISMOTIONDONE:
	*pResult=motion_done(bn);
	*pGetValue = (double) *pResult;

	break;
	case	WAITAXISDONE:
	*pResult=wait_for_done(bn);
	*pGetValue = (double) *pResult;

	break;
	case	SETMAXVEL:
	*pResult=fix_max_speed(bn, pUser->SetValue );
	*pGetValue = (double) *pResult;

	break;
	case	GETPOS:
	*pResult=get_position(bn,  pGetValue);
	
	break;
	case	SETPOS:
	*pResult=set_position(bn, pUser->SetValue );
	*pGetValue = (double) *pResult;

	break;
	case	GETCOMMAND:
	*pResult=get_command(bn, pGetValue );
	break;
	case	SETCOMMAND:
	*pResult=set_command(bn,  pUser->SetValue);
	*pGetValue = (double) *pResult;

	break;

	case SETMOVERATIO:
	*pResult= set_move_ratio(bn, pUser->SetValue);
	*pGetValue = (double) *pResult;

	break;

	}


}


void AdLinkOthers( const LPDRIVER_INST pNet, SPECIAL_INST* const pData )
{
    int rc = SUCCESS ;
	int bn;
//	F64 PosRead;		
    SPECIAL_INST_OTHERS* pUser = &pData->User.paramOthers;

    int* pGetValue = BuildUiotPointer( pUser->GetValue  );

    UINT16* pResult = BuildUiotPointer( pUser->Header.ofsResult );
	*pResult=rc;	

//	*pGetValue=*pGetValue+1;

	bn=(((pNet->PciIndex) -1) *4) + pUser->Address; // axis number
	

 switch(pUser->Function  ) 
    {
	case	SETHOMECONFIG:
	*pResult= set_home_config(bn, pUser->SetValue1, pUser->SetValue2, pUser->SetValue3, pUser->SetValue4);
	*pGetValue = (int) *pResult;
		break;

		case	SETSVONOFF:
	*pResult =_8134_Set_SVON(bn, pUser->SetValue1);
	*pGetValue = (int) *pResult;
		break;

		case	SETMAN_IPMODE:
	*pResult = set_manu_iptmode(bn, pUser->SetValue1, pUser->SetValue2);
	*pGetValue = (int) *pResult;
		break;

		case	SET_PLS_OUTMODE:

	*pResult = set_pls_outmode(bn,pUser->SetValue1);
	*pGetValue = (int) *pResult;
		break;
		case	SET_PLS_INMODE:

	*pResult = set_pls_iptmode(bn,pUser->SetValue1);
	*pGetValue = (int) *pResult;
		break;
		case	SET_CNTMODE:
	*pResult = set_cnt_src(bn,pUser->SetValue1);
	*pGetValue = (int) *pResult;
		break;
		case	SETALARM_LOGIC:
	*pResult = set_alm_logic(bn,pUser->SetValue1,pUser->SetValue2);
	*pGetValue = (int) *pResult;
		break;
		case	SETIN_LOGIC:

	*pResult = set_inp_logic(bn,pUser->SetValue1,pUser->SetValue2);
	*pGetValue = (int) *pResult;
		break;

		case	SETINTZERO:

		INTarray[bn] = 	INTarray[bn] & pUser->SetValue1;

		*pGetValue = INTarray[bn] ; //(int) *pResult;
		break;
 }


}


/*
// Win32
int _81345_initial(I16 *existCards)
{

   I16 cn;
   I16 i,CardNo;

#if define WIN32ONLY

   if( hMap == NULL ) return ERR_OtherProcessExist;
#endif

  for(cn=0;cn<MAX_PCI_CARDS;cn++)
  {
	pci_Info[cn].initFlag = FALSE;
	TotalCard=0;
	TotalAxes=0;
  }

  for(cn=0;cn<MAX_PCI_CARDS;cn++)
  {
		CardNo=i_wdm_get_card_info(cn);
	   	if( CardNo < 0 ) continue;

		PLX_USR[TotalCard][0] = i_get_plx_usr(TotalCard);
		PLX_USR[TotalCard][0] &= 0xf7ff;
		PLX_USR[TotalCard][1] = PLX_USR[TotalCard][0] | 0x0800;

	   i_set_user_io_pin3( CardNo , 1 );

	   // Set Int Polarity be Active Low, and Disable all INT at Init.
	   i_hardware_int_control(CardNo,1,0,1,0);

		pci_Info[TotalCard].CPLD_Ctrl=0x00;
		i_cpld_ctrl(TotalCard, pci_Info[TotalCard].CPLD_Ctrl);

	   // Set Other parameters
	   for(i=AXIS_PER_CARD*CardNo;i<AXIS_PER_CARD*CardNo+AXIS_PER_CARD;i++)
			i_set_pci_info(i);

		TotalCard++;

	} // End of search every card

  *existCards=TotalCard;
  TotalAxes=AXIS_PER_CARD*TotalCard-1;

  if ( TotalCard == 0  )
 	 return ERR_NoCardFound;

	return ERR_NoError;
}

*/
