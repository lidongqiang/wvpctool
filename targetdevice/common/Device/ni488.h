
#ifndef NI488_H       // ensure we are only included once
#define NI488_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Global variables.                                                      */
/**************************************************************************/

#define ibsta	(*m_Pibsta)
#define iberr	(*m_Piberr)
#define ibcntl	(*m_Pibcntl)
#define ibcnt	(*m_Pibcntl)

/*  functions                      */
typedef int (__stdcall * ibdev_fun)(int boardID, int pad, int sad, int tmo, int eot, int eos);
typedef int (__stdcall * ibln_fun)(int ud, int pad, int sad, short *listen);
typedef int (__stdcall * ibwrt_fun)(int ud, void *buf, long cnt);
typedef int (__stdcall * ibrd_fun)(int ud, void *buf, long cnt);
typedef int (__stdcall * ibrsp_fun)(int ud,void *spr);
typedef int (__stdcall * ibonl_fun)(int ud, int v);
typedef int (__stdcall * ibeos_fun)(int ud, int v);
typedef int (__stdcall * ibeot_fun)(int ud, int v);
typedef int (__stdcall * ibtmo_fun)(int ud, int v);
typedef int (__stdcall * ibppc_fun)(int ud, int v);
typedef int (__stdcall * ibpad_fun)(int ud, int v);
typedef int (__stdcall * ibsad_fun)(int ud, int v);
typedef int (__stdcall * ibrpp_fun)(int ud, void *ppr);
typedef int (__stdcall * ibwait_fun)(int ud, int mask);
typedef int (__stdcall * ibloc_fun)(int ud); 
typedef int (__stdcall * ibpct_fun)(int ud);
typedef int (__stdcall * ibtrg_fun)(int ud);
typedef int (__stdcall * ibclr_fun)(int ud);
typedef int (__stdcall * ibconfig_fun)(int ud, int option,int value); 

typedef int (__stdcall *ibask_fun)(int ud, int option, PINT v);
typedef int (__stdcall *ibcac_fun)(int ud, int v);
typedef void(__stdcall *FindLstn_fun)(int boardID, short *addrlist, short *results, int limit);
typedef int (__stdcall *ibfind_fun)(char *ud);
typedef int(__stdcall *ibsic_fun)(int ud);

/* GPIB status bit vector :                                */
/*       global variable ibsta and wait mask               */

#define ERR     (1<<15) /* Error detected                  */
#define TIMO    (1<<14) /* Timeout                         */
#define END     (1<<13) /* EOI or EOS detected             */
#define SRQI    (1<<12) /* SRQ detected by CIC             */
#define RQS     (1<<11) /* Device needs service            */
#define SPOLL   (1<<10) /* Board has been serially polled  */
#define EVENT   (1<<9)  /* An event has occured            */
#define CMPL    (1<<8)  /* I/O completed                   */
#define LOK     (1<<7)  /* Local lockout state             */
#define REM     (1<<6)  /* Remote state                    */
#define CIC     (1<<5)  /* Controller-in-Charge            */
#define ATN     (1<<4)  /* Attention asserted              */
#define TACS    (1<<3)  /* Talker active                   */
#define LACS    (1<<2)  /* Listener active                 */
#define DTAS    (1<<1)  /* Device trigger state            */
#define DCAS    (1<<0)  /* Device clear state              */

/* Error messages returned in global variable iberr        */

#define EDVR  0  /* Operating system error                 */
#define ECIC  1  /* Function requires GPIB board to be CIC */
#define ENOL  2  /* Write function detected no Listeners   */
#define EADR  3  /* Interface board not addressed correctly*/
#define EARG  4  /* Invalid argument to function call      */
#define ESAC  5  /* Function requires GPIB board to be SAC */
#define EABO  6  /* I/O operation aborted                  */
#define ENEB  7  /* Non-existent interface board           */
#define EDMA  8  /* Error performing DMA buffer            */
#define EOIP 10  /* I/O operation started before previous  */
/* operation completed                    */
#define ECAP 11  /* No capability for intended operation   */
#define EFSO 12  /* File system operation error            */
#define EBUS 14  /* Command error during device call       */
#define ESTB 15  /* Serial poll status byte lost           */
#define ESRQ 16  /* SRQ remains asserted                   */
#define ETAB 20  /* The return buffer is full.             */
#define ELCK 21  /* Address or board is locked.            */
#define EARM 22  /* ibnotify callback failed to rearm	   */
#define EHDL 23  /* Input handle is invalid				   */
#define ECFG 24  /* The GPIB-ENET was already on-line, and */
/* the default board configuration sent   */
/* to it differs from than configuration  */
/* under which it was already operating.  */
/* This is only a warning.  The board     */
/* configuration has not been changed,    */
/* but the operation has otherwise        */
/* completed successfully.                */
#define EWIP 26 //Wait in progress on specified input handle
#define ERST 27 //The event notification was cancelled due to a reset of the interface
#define EPWR 28 //The interface lost power


#define ELNK 200 /* The GPIB library was not linked;       */
/* dummy functions were linked instead    */
#define EDLL 201 /* Error loading GPIB32.DLL;  ibcnt       */
/* contains the MS Windows error code     */
#define EFNF 203 /* Unable to find the function in         */
/* in GPIB32.DLL;  ibcnt contains the     */
/* MS Windows error code                  */
#define EGLB 205 /* Globals or copyright string not found  */
/* in GPIB32.DLL.                         */
#define ENNI 206 /* Not a National Instruments GPIB32.DLL  */
#define EMTX 207 /* Unable to acquire Mutex for loading    */
/* DLL.  The MS Windows error is in ibcnt */

#define EMSG 210 /* Unable to register callback function   */
/* with Windows                           */
#define ECTB 211 /* Callback table is full                 */

/*  The following constants are used for the second parameter of the
*  ibconfig function.  They are the "option" selection codes.
*/
#define  IbcPAD        0x0001      /* Primary Address                      */
#define  IbcSAD        0x0002      /* Secondary Address                    */
#define  IbcTMO        0x0003      /* Timeout Value                        */
#define  IbcEOT        0x0004      /* Send EOI with last data byte?        */
#define  IbcPPC        0x0005      /* Parallel Poll Configure              */
#define  IbcREADDR     0x0006      /* Repeat Addressing                    */
#define  IbcAUTOPOLL   0x0007      /* Disable Auto Serial Polling          */
#define  IbcCICPROT    0x0008      /* Use the CIC Protocol?                */
#define  IbcIRQ        0x0009      /* Use PIO for I/O                      */
#define  IbcSC         0x000A      /* Board is System Controller?          */
#define  IbcSRE        0x000B      /* Assert SRE on device calls?          */
#define  IbcEOSrd      0x000C      /* Terminate reads on EOS               */
#define  IbcEOSwrt     0x000D      /* Send EOI with EOS character          */
#define  IbcEOScmp     0x000E      /* Use 7 or 8-bit EOS compare           */
#define  IbcEOSchar    0x000F      /* The EOS character.                   */
#define  IbcPP2        0x0010      /* Use Parallel Poll Mode 2.            */
#define  IbcTIMING     0x0011      /* NORMAL, HIGH, or VERY_HIGH timing.   */
#define  IbcDMA        0x0012      /* Use DMA for I/O                      */
#define  IbcReadAdjust 0x0013      /* Swap bytes during an ibrd.           */
#define  IbcWriteAdjust 0x014      /* Swap bytes during an ibwrt.          */
#define  IbcSendLLO    0x0017      /* Enable/disable the sending of LLO.      */
#define  IbcSPollTime  0x0018      /* Set the timeout value for serial polls. */
#define  IbcPPollTime  0x0019      /* Set the parallel poll length period.    */
#define  IbcEndBitIsNormal 0x001A  /* Remove EOS from END bit of IBSTA.       */
#define  IbcUnAddr         0x001B  /* Enable/disable device unaddressing.     */
#define  IbcSignalNumber   0x001C  /* Set UNIX signal number - unsupported */
#define  IbcBlockIfLocked  0x001D  /* Enable/disable blocking for locked boards/devices */
#define  IbcHSCableLength  0x001F  /* Length of cable specified for high speed timing.*/
#define  IbcIst        0x0020      /* Set the IST bit.                     */
#define  IbcRsv        0x0021      /* Set the RSV byte.                    */
#define  IbcLON        0x0022      /* Enter listen only mode               */

/*
*    Constants that can be used (in addition to the ibconfig constants)
*    when calling the ibask() function.
*/

#define  IbaPAD            IbcPAD
#define  IbaSAD            IbcSAD
#define  IbaTMO            IbcTMO
#define  IbaEOT            IbcEOT
#define  IbaPPC            IbcPPC
#define  IbaREADDR         IbcREADDR
#define  IbaAUTOPOLL       IbcAUTOPOLL
#define  IbaCICPROT        IbcCICPROT
#define  IbaIRQ            IbcIRQ
#define  IbaSC             IbcSC
#define  IbaSRE            IbcSRE
#define  IbaEOSrd          IbcEOSrd
#define  IbaEOSwrt         IbcEOSwrt
#define  IbaEOScmp         IbcEOScmp
#define  IbaEOSchar        IbcEOSchar
#define  IbaPP2            IbcPP2
#define  IbaTIMING         IbcTIMING
#define  IbaDMA            IbcDMA
#define  IbaReadAdjust     IbcReadAdjust
#define  IbaWriteAdjust    IbcWriteAdjust
#define  IbaSendLLO        IbcSendLLO
#define  IbaSPollTime      IbcSPollTime
#define  IbaPPollTime      IbcPPollTime
#define  IbaEndBitIsNormal IbcEndBitIsNormal
#define  IbaUnAddr         IbcUnAddr
#define  IbaSignalNumber   IbcSignalNumber
#define  IbaBlockIfLocked  IbcBlockIfLocked
#define  IbaHSCableLength  IbcHSCableLength
#define  IbaIst            IbcIst
#define  IbaRsv            IbcRsv
#define  IbaLON            IbcLON
#define  IbaSerialNumber   0x0023

#ifdef __cplusplus
}
#endif


#endif   // NI488_H

