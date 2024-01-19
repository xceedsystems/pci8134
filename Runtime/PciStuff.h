/***************************************************************

                PciStuff.h             

   This file contains the interface to the Pcilib.lib

****************************************************************/


#ifndef  __PCISTUFF_H__
#define  __PCISTUFF_H__

#include "Pci_8134.h"


int InitPCI( LPDRIVER_INST pNet, P_ERR_PARAM pErrors,  PCI_INFO *pciInfo);


#endif      /* __PCISTUFF_H__ */

