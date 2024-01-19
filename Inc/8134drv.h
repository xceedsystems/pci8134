/****************************************************************************/
/*  Copyright (c) 1995-1998, ADLink Technology Inc.  All rights reserved.   */
/*                                                                          */
/*  File Name   :   PCI_8134.H                                              */
/*  Purpose     :   Header file for PCI_8134.C module                       */
/*  Date        :   04/29/1999                                              */
/*  Revision    :   1.00                                                    */
/*  Programmer  :   Yi_Tun Huang                                           */
/****************************************************************************/
/****************************************************************************/
/*      Typedef  Definitions                                                */
/****************************************************************************/
typedef unsigned char   U8;
typedef short           I16;
typedef unsigned short  U16;
typedef long            I32;
typedef unsigned long   U32;
typedef float           F32;
typedef double          F64;
typedef char            Boolean;

// #define     MAX_PCI_CARDS       4
#define     MAX_PCI_CARDS       12      // Ver 2.0, Stephen

/****************************************************************************/
/*      PCL_5022 Register Structure Type Define                             */
/****************************************************************************/
typedef struct  cmddt {
        unsigned int  strcmd;           /* Start Command */
        unsigned int  stpcmd;           /* Stop Command */
        unsigned int  schgcmd;          /* Speed Change Command */
        unsigned int  srescmd;
        unsigned int  outbit;
        unsigned int  concmd;
}CMD_TBL;

typedef struct  workunit {
        U32     runadr;
       U32     i_oadr;
       U32     bitadr;
        U32     regadr;
        U32     i_obadr0;
        U32     i_obadr1;
}PCL_ADR;

/****************************************************************************/
/*      PCL_5022 Register Structure Declare External                       */
/*      Init in pci_8134.c
/****************************************************************************/
extern unsigned long sts_dt[2*MAX_PCI_CARDS];
extern CMD_TBL cmd_tbl[4*MAX_PCI_CARDS];
extern PCL_ADR pcl_adr[2*MAX_PCI_CARDS];

/****************************************************************************/
/*      Motion Control ChipSet Define                                       */
/****************************************************************************/
#define         _PCL_5022       0               // Ver 2.0, Stephen
#define         _PCL_5023       1               // Ver 2.0, Stephen
/****************************************************************************/
/*      Constant Definitions                                                */
/****************************************************************************/
#define         PCI_8134        0x8134
#define         PCL_CS0         0x00
#define         PCL_CS1         0x80

/* ------------------------------------------------------------------------ */

#define         R0      0x00
#define         R1      0x01
#define         R2      0x02
#define         R3      0x03
#define         R4      0x04
#define         R5      0x05
#define         R6      0x06
#define         R7      0x07
#define         R8      0x08
#define         R9      0x09
#define         R10     0x0A
#define         R11     0x0B
#define         R12     0x0C
#define         R13     0x0D
#define         R14     0x0E
#define         R15     0x0F
#define         R3C     0x3C
#define         RMD     0x3F
#define         R3D     0x3D
#define         R16     0x10            // Ver 2.0 Stephen
#define         R17     0x11            // Ver 2.0 Stephen

/* ------------------------------------------------------------------------ */

#define         CHGtoFL         0x00
#define         CHGtoFH         0x01
#define         DECtoFL         0x02
#define         ACCtoFH         0x03

#define         STOP            0x09
#define         DECtoSTOP       0x0a

#define         RunFL           0x10
#define         RunFH           0x11
#define         RunVRSPD        0x13

//--------------------- Ver 2.0, Stephen -------------------------------
#define         SuspVRSPD       0x23

#define         AllSTART        0x30
#define         AllSTOP         0x38
//----------------------------------------------------------------------
/****************************************************************************/
/*      Function  Declerations                                              */
/****************************************************************************/
extern void _8134_Software_Reset(U16 cardNo);
extern void _PLX_Generate_sw_int( U16 cardNo );

extern U16  _8134_sts(int axis_n);                       /* Status Read */
extern U16  _8134_pclcmd(int axis_n, U8 cmd_dt);         /* PCL Command Set */
extern U16  _8134_pclrun(int axis_n,U8 cmd_dt);          /* PCL START/STOP */
extern U16  _8134_pclstr(int axis_n);                    /* START */
extern U16  _8134_pclstp(int axis_n);                    /* STOP */
extern U16  _8134_wreg(int axis_n,U8 r_num,U32 r_dat);   /* Write Register */
extern U16  _8134_rreg(int axis_n,U8 r_num,U32 *r_dat);  /* Read Register */

extern U16 _8134_Set_Vel(int axis, double Str_Vel, double Max_Vel);
extern U16 _8134_Set_Mult(int axis, double Max_Vel);
//--------------------- Ver 2.0, Stephen -------------------------------
extern U16 _8134_Spd_Chg(int axis, double Max_Vel);
//----------------------------------------------------------------------
extern U16 _8134_Set_Acc(int axis, double accel);
extern U16 _8134_Set_Dec(int axis, double Decel);
extern U16 _8134_Set_Sacc(int axis, double Tlacc, double Tsacc);// Ver 2.0, Stephen
extern U16 _8134_Set_Sdec(int axis, double Tldec, double Tsdec);// Ver 2.0, Stephen
extern U16 _8134_Set_Pos(int axis, double Pos);
extern U16 _8134_Set_RMD(int axis, int RMD_no);
extern U16 _8134_Get_Pos(int axis, double *act_pos);
extern U16 _8134_Clr_Pos(int axis);

