#ifndef PCI_8134_H
    #define PCI_8134_H
/****************************************************************************/
/*  Copyright (c) 1995-1999, ADLink Technology Inc.  All rights reserved.   */
/*                                                                          */
/*  File Name   :   PCI_8134.H                                              */
/*  Purpose     :   Header file for PCI_8134 Card                           */
/*  Date        :   11/28/1999                                              */
/*  Revision    :   2.01                                                    */
/*  Programmer  :   Yi_Tun Huang                                           */
/****************************************************************************/

/****************************************************************************/
/*      Typedef  Definitions                                                */
/****************************************************************************/


// #include <YVALS.H>
// #include <yvals.h>

#include <pcibus.h>

typedef unsigned char   U8;
typedef short           I16;
typedef unsigned short  U16;
typedef long            I32;
typedef unsigned long   U32;
typedef float           F32;
typedef double          F64;
// typedef char            Boolean;

#define     MAX_PCI_CARDS       12      // Ver 2.0, Stephen

#define     TRUE                1
#define     FALSE               0

#define     HIGH                1
#define     LOW                 0

typedef struct
{
        U32 baseAddr[MAX_PCI_CARDS];
        U32 lcrAddr[MAX_PCI_CARDS];
        U16 irqNo   [MAX_PCI_CARDS];
//        void    (*old_isr[MAX_PCI_CARDS]) (void);
} PCI_INFO;

extern  PCI_INFO       pcinfo;

/* ------------ I/O Status Definition for get_io_status(); ---------------- */
#define         pos_limit       0x01
#define         neg_limit       0x02
#define         psd_switch      0x04
#define         nsd_switch      0x08
#define         org_limit       0x10
#define         idx_switch      0x20
#define         alm_switch      0x40
#define         svon_sts        0x80
#define         rdy_sts         0x100
#define         int_sts         0x200
#define         erc_sts         0x400
#define         inp_sts         0x800

/* ------------ Motion Done Reurn Value Definition for motion_done(); ----- */
#define         BUSY            0x00
#define         DONE            0x01
#define         POS_LIM         0x02
#define         NEG_LIM         0x03
#define         ORG_LIM         0x04
#define         ALARM           0x05

/****************************************************************************/
/*      Function  Declarations                                              */
/****************************************************************************/

extern int     axes[49];        // Ver 2.0, Stephen
extern int     Max_AxisNo;


extern void _8134_Software_Reset(U16 cardNo);

extern U16  _8134_Initial(U16 *existCards, PCI_INFO *pciInfo);

extern U16  _8134_Set_SVON(int axis, int on_off);

extern void _8134_Get_IRQ_Status (U16 cardNo, U16 *ch1 );
extern void _8134_Get_IRQ_Channel(U16 cardNo, U16 *irq_no );
extern void _8134_Get_Base_Addr(U16 cardNo, U16 *base_addr );
extern void _8134_Set_INT_Enable(U16 cardNo, U16 intFlag );
extern void _8134_Close(U16 cardNo);


extern U16 _8134_Set_Config(void);

extern U16 start_aa_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 accel);
extern U16 start_a_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tacc);
extern U16 a_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tacc);
extern U16 start_r_move(I16 axis, F64 distance, F64 str_vel, F64 max_vel, F64 Tacc);
extern U16 r_move(I16 axis, F64 distance, F64 str_vel, F64 max_vel, F64 Tacc);
extern U16 start_t_move(I16 axis, F64 distance, F64 str_vel, F64 max_vel, F64 Tacc, F64 Tdec);
extern U16 t_move(I16 axis, F64 distance, F64 str_vel, F64 max_vel, F64 Tacc, F64 Tdec);
extern U16 wait_for_done(I16 axis);

//--------------------- Ver 2.0, Stephen -------------------------------
extern U16 start_ta_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tacc, F64 Tdec);
extern U16 ta_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tacc, F64 Tdec);
extern U16 start_s_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tlacc, F64 Tsacc);
extern U16 s_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tlacc, F64 Tsacc);
extern U16 start_rs_move(I16 axis, F64 distance, F64 str_vel, F64 max_vel, F64 Tlacc, F64 Tsacc);
extern U16 rs_move(I16 axis, F64 distance, F64 str_vel, F64 max_vel, F64 Tlacc, F64 Tsacc);
extern U16 start_tas_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tlacc, F64 Tsacc, F64 Tldec, F64 Tsdec);
extern U16 tas_move(I16 axis, F64 pos, F64 str_vel, F64 max_vel, F64 Tlacc, F64 Tsacc, F64 Tldec, F64 Tsdec);
extern U16 start_move_all(I16 len, I16 *map_array, F64 *pos, F64 *str_vel, F64 *max_vel, F64 *Tacc);
extern U16 move_all(I16 len, I16 *map_array, F64 *pos, F64 *str_vel, F64 *max_vel, F64 *Tacc);
extern U16 wait_for_all(I16 len, I16 *map_array);
//----------------------------------------------------------------------

extern U16 set_move_ratio(I16 axis, F64 ratio);
extern U16 get_position(I16 axis, F64 *pos);
extern U16 set_position(I16 axis, F64 pos);
extern U16 get_command(I16 axis, F64 *pos);
extern U16 set_command(I16 axis, F64 pos);

extern U16 v_move(I16 axis, F64 str_vel, F64 max_vel, F64 Tacc);
//--------------------- Ver 2.0, Stephen -------------------------------
extern U16 sv_move(I16 axis, F64 str_vel, F64 max_vel, F64 Tlacc, F64 Tsacc);
extern U16 v_change(I16 axis, F64 max_vel, F64 accel);
extern U16 fix_max_speed(I16 axis, F64 max_vel);
//----------------------------------------------------------------------
extern U16 v_stop(I16 axis, F64 Tdec);

extern U16 get_io_status(I16 axis, U16 *io_sts);

extern U16 motion_done(I16 axis);

extern U16 map_axes(I16 n_axes, I16 *map_array);
//--------------------- Ver 2.0, Stephen -------------------------------
extern U16 set_move_mode(int mode);
extern U16 set_move_pos(double pos);
extern U16 set_move_speed(double str_vel, double max_vel);
extern U16 set_move_accel(double accel);
extern U16 set_move_saccel(double tlacc, double tsacc);
extern U16 start_motion(void);
extern U16 stop_motion(void);
extern U16 set_sync_mode(I16 ext_trig, I16 trig_mode);
//----------------------------------------------------------------------

extern U16 set_arc_division(I16 axis, F64 degrees);
extern U16 arc_optimization(int optimize);
extern U16 move_xy(int cardNo, double x, double y);
extern U16 move_zu(int cardNo, double z, double u);
extern U16 arc_xy(int cardNo, double x_center, double y_center, double angle);
extern U16 arc_zu(int cardNo, double z_center, double u_center, double angle);
extern U16 start_move_xy(int cardNo, double z, double u);
extern U16 start_move_xy(int cardNo, double z, double u);
extern U16 recover_xy(int cardNo);
extern U16 recover_zu(int cardNo);

extern U16 set_home_config(int axis,int home_mode,int org_logic,int org_latch,int EZ_logic);
extern U16 home_move(int axis,double str_vel,double max_vel,double Tacc);

extern U16 set_manu_iptmode(int axis, int manu_iptmode, int op_mode);
extern U16 manu_move(int axis, double max_vel);

extern U16 set_pls_outmode(int axis, int pls_outmode);
extern U16 set_pls_iptmode(int axis, int pls_iptmode);
extern U16 set_cnt_src(int axis, int cnt_src);

extern U16 set_alm_logic(int axis, int alm_logic, int alm_mode);
extern U16 set_inp_logic(int axis, int inp_logic, int inp_enable);
//--------------------- Ver 2.0, Stephen -------------------------------
// extern U16 set_sd_logic(int axis, int sd_logic, int sd_enable);
extern U16 set_sd_logic(int axis, int sd_logic, int sd_latch, int sd_enable); // Ver 2.0, Stephen
extern U16 set_erc_enable(int axis, int erc_enable);    // Ver 2.0, Stephen
//----------------------------------------------------------------------

extern U16 set_int_factor(U16 axis, U32 int_factor );
extern U16 get_int_axis(U16 *int_axis );
extern U16 get_int_status(U16 axis, U32 *int_status );
extern U16 read_int_status(U16 axis, U32 *ri_status );
extern U16 read_int_axis(U16 *int_axis );

extern U8 PciFinxDevice(PCIDEV_OLD *pPciDevice);

/****************************************************************************/
/*      ISR Function Declaration                                            */
/****************************************************************************/
//void interrupt _8134_isr0 (void);
//void interrupt _8134_isr1 (void);
//void interrupt _8134_isr2 (void);
//void interrupt _8134_isr3 (void);
//void interrupt _8134_isr4 (void);
//void interrupt _8134_isr5 (void);
//void interrupt _8134_isr6 (void);
//void interrupt _8134_isr7 (void);
//void interrupt _8134_isr8 (void);
//void interrupt _8134_isr9 (void);
//void interrupt _8134_isra (void);
//void interrupt _8134_isrb (void);



#endif
