/*
** Copyright (c) 2007-2015 by Silicon Laboratories
**
** $Id: si3217x_revb_intf.h 4727 2015-02-10 22:35:54Z nizajerk $
**
** Si3217x RevB ProSLIC interface header file
**
** Author(s): 
** cdp
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the Si3217x Rev B chip set.
**
**
*/
#ifndef SI3217X_REVB_INTF_H
#define SI3217X_REVB_INTF_H


/*
** Function: Si3217x_RevB_GenParamUpdate
**
** Description: 
** Update Si3217x general parameters and other required modifications
** to default reg/ram values
**
** Returns:
** int (error)
**
*/
int Si3217x_RevB_GenParamUpdate(proslicChanType_ptr hProslic,initSeqType seq);


/*
** Function: Si3217x_RevB_SelectPatch
**
** Description: 
** Select appropriate patch based on general parameters
**
** Returns:
** int (error)
**
*/
int Si3217x_RevB_SelectPatch(proslicChanType_ptr hProslic,const proslicPatch **patch);


/*
** Function: Si3217x_RevB_ConverterSetup
**
** Description: 
** Program revision specific settings before powering converter
**
** Returns:
** int (error)
**
*/
int Si3217x_RevB_ConverterSetup(proslicChanType_ptr hProslic);


/*
** Function: Si3217x_RevB_PulseMeterSetup
**
** Description: 
** configure pulse metering
*/
int Si3217x_RevB_PulseMeterSetup (proslicChanType_ptr hProslic, int preset);

/*
** Function: Si3217x_RevB_PulseMeterStop
**
** Description: 
** stop pulse meter tone
*/
int Si3217x_RevBPulseMeterStop (proslicChanType_ptr hProslic);
#endif

