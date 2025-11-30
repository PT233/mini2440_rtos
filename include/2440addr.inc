// ====================================================================
// File Name : 2440addr_gnu.inc
// Function  : S3C2440 Define Address Register (Assembly for GNU GAS)
// Adapted from 2440addr.inc
// ====================================================================

//      GBLL   BIG_ENDIAN__
// BIG_ENDIAN__   SETL   {FALSE}
// In GAS, define BIG_ENDIAN__ via command line or .equ if needed.
// For now, we assume Little Endian as per original default.

// =================
// Memory control
// =================
.equ BWSCON,   0x48000000     // Bus width & wait status
.equ BANKCON0, 0x48000004     // Boot ROM control
.equ BANKCON1, 0x48000008     // BANK1 control
.equ BANKCON2, 0x4800000c     // BANK2 control
.equ BANKCON3, 0x48000010     // BANK3 control
.equ BANKCON4, 0x48000014     // BANK4 control
.equ BANKCON5, 0x48000018     // BANK5 control
.equ BANKCON6, 0x4800001c     // BANK6 control
.equ BANKCON7, 0x48000020     // BANK7 control
.equ REFRESH,  0x48000024     // DRAM/SDRAM refresh
.equ BANKSIZE, 0x48000028     // Flexible Bank Size
.equ MRSRB6,   0x4800002c     // Mode register set for SDRAM Bank6
.equ MRSRB7,   0x48000030     // Mode register set for SDRAM Bank7


// ==========================
// CLOCK & POWER MANAGEMENT
// ==========================
.equ LOCKTIME, 0x4c000000     // PLL lock time counter
.equ MPLLCON,  0x4c000004     // MPLL Control
.equ UPLLCON,  0x4c000008     // UPLL Control
.equ CLKCON,   0x4c00000c     // Clock generator control
.equ CLKSLOW,  0x4c000010     // Slow clock control
.equ CLKDIVN,  0x4c000014     // Clock divider control


// =================
// INTERRUPT
// =================
.equ SRCPND,    0x4a000000    // Interrupt request status
.equ INTMOD,    0x4a000004    // Interrupt mode control
.equ INTMSK,    0x4a000008    // Interrupt mask control
.equ PRIORITY,  0x4a00000c    // IRQ priority control
.equ INTPND,    0x4a000010    // Interrupt request status
.equ INTOFFSET, 0x4a000014    // Interruot request source offset
.equ SUSSRCPND, 0x4a000018    // Sub source pending
.equ INTSUBMSK, 0x4a00001c    // Interrupt sub mask


// =================
// I/O PORT for LED
// =================
.equ GPFCON,  0x56000050     // Port F control
.equ GPFDAT,  0x56000054     // Port F data
.equ GPFUP,   0x56000058     // Pull-up control F

.equ GPBCON,  0x56000010     // Port B control
.equ GPBDAT,  0x56000014     // Port B data


// Miscellaneous register
.equ MISCCR,   0x56000080     // Miscellaneous control
.equ DCKCON,   0x56000084     // DCLK0/1 control
.equ EXTINT0,  0x56000088     // External interrupt control register 0
.equ EXTINT1,  0x5600008c     // External interrupt control register 1
.equ EXTINT2,  0x56000090     // External interrupt control register 2
.equ EINTFLT0, 0x56000094     // Reserved
.equ EINTFLT1, 0x56000098     // Reserved
.equ EINTFLT2, 0x5600009c     // External interrupt filter control register 2
.equ EINTFLT3, 0x560000a0     // External interrupt filter control register 3
.equ EINTMASK, 0x560000a4     // External interrupt mask
.equ EINTPEND, 0x560000a8     // External interrupt pending
.equ GSTATUS0, 0x560000ac     // External pin status
.equ GSTATUS1, 0x560000b0     // Chip ID(0x32440000)
.equ GSTATUS2, 0x560000b4     // Reset type
.equ GSTATUS3, 0x560000b8     // Saved data0(32-bit) before entering POWER_OFF mode
.equ GSTATUS4, 0x560000bc     // Saved data1(32-bit) before entering POWER_OFF mode

// Added for 2440
.equ MSLCON,   0x560000cc     // Memory sleep control register

// =================
// WATCH DOG TIMER
// =================
.equ WTCON, 0x53000000       // Watch-dog timer mode
.equ WTDAT, 0x53000004       // Watch-dog timer data
.equ WTCNT, 0x53000008       // Eatch-dog timer count

// =================
// Nand Flash
// =================
.equ NFCONF,   0x4E000000    // NAND Flash configuration
.equ NFCONT,   0x4E000004    // NAND Flash control
.equ NFCMD,    0x4E000008    // NAND Flash command
.equ NFADDR,   0x4E00000C    // NAND Flash address
.equ NFDATA,   0x4E000010    // NAND Flash data
.equ NFDATA8,  0x4E000010    // NAND Flash data
.equ NFMECCD0, 0x4E000014    // NAND Flash ECC for Main Area
.equ NFMECCD1, 0x4E000018
.equ NFSECCD,  0x4E00001C    // NAND Flash ECC for Spare Area
.equ NFSTAT,   0x4E000020    // NAND Flash operation status
.equ NFESTAT0, 0x4E000024
.equ NFESTAT1, 0x4E000028
.equ NFMECC0,  0x4E00002C
.equ NFMECC1,  0x4E000030
.equ NFSECC,   0x4E000034
.equ NFSBLK,   0x4E000038    // NAND Flash Start block address
.equ NFEBLK,   0x4E00003C    // NAND Flash End block address

// =================
// PWM TIMER
// =================
.equ TCFG0,    0x51000000
.equ TCFG1,    0x51000004
.equ TCON,     0x51000008
.equ TCNTB0,   0x5100000c
.equ TCMPB0,   0x51000010
.equ TCNTO0,   0x51000014
.equ TCNTB1,   0x51000018
.equ TCMPB1,   0x5100001c
.equ TCNTO1,   0x51000020
.equ TCNTB2,   0x51000024
.equ TCMPB2,   0x51000028
.equ TCNTO2,   0x5100002c
.equ TCNTB3,   0x51000030
.equ TCMPB3,   0x51000034
.equ TCNTO3,   0x51000038
.equ TCNTB4,   0x5100003c
.equ TCNTO4,   0x51000040

// =================
// UART
// =================
.equ ULCON0,   0x50000000
.equ UCON0,    0x50000004
.equ UFCON0,   0x50000008
.equ UMCON0,   0x5000000c
.equ UTRSTAT0, 0x50000010
.equ UERSTAT0, 0x50000014
.equ UFSTAT0,  0x50000018
.equ UMSTAT0,  0x5000001c
.equ UTXH0,    0x50000020
.equ URXH0,    0x50000024
.equ UBRDIV0,  0x50000028

.equ ULCON1,   0x50004000
.equ UCON1,    0x50004004
.equ UFCON1,   0x50004008
.equ UMCON1,   0x5000400c
.equ UTRSTAT1, 0x50004010
.equ UERSTAT1, 0x50004014
.equ UFSTAT1,  0x50004018
.equ UMSTAT1,  0x5000401c
.equ UTXH1,    0x50004020
.equ URXH1,    0x50004024
.equ UBRDIV1,  0x50004028

.equ ULCON2,   0x50008000
.equ UCON2,    0x50008004
.equ UFCON2,   0x50008008
.equ UTRSTAT2, 0x50008010
.equ UERSTAT2, 0x50008014
.equ UFSTAT2,  0x50008018
.equ UTXH2,    0x50008020
.equ URXH2,    0x50008024
.equ UBRDIV2,  0x50008028

// =================
// GPIO (Remaining)
// =================
.equ GPACON,   0x56000000
.equ GPADAT,   0x56000004

.equ GPCCON,   0x56000020
.equ GPCDAT,   0x56000024
.equ GPCUP,    0x56000028

.equ GPDCON,   0x56000030
.equ GPDDAT,   0x56000034
.equ GPDUP,    0x56000038

.equ GPECON,   0x56000040
.equ GPEDAT,   0x56000044
.equ GPEUP,    0x56000048

.equ GPGCON,   0x56000060
.equ GPGDAT,   0x56000064
.equ GPGUP,    0x56000068

.equ GPHCON,   0x56000070
.equ GPHDAT,   0x56000074
.equ GPHUP,    0x56000078

.equ GPJCON,   0x560000d0
.equ GPJDAT,   0x560000d4
.equ GPJUP,    0x560000d8

// =================
// USB Device
// =================
.equ rFUNC_ADDR_REG,     0x52000140
.equ rPWR_REG,           0x52000144
.equ rEP_INT_REG,        0x52000148
.equ rUSB_INT_REG,       0x52000158
.equ rEP_INT_EN_REG,     0x5200015c
.equ rUSB_INT_EN_REG,    0x5200016c
.equ rFRAME_NUM1_REG,    0x52000170
.equ rFRAME_NUM2_REG,    0x52000174
.equ rINDEX_REG,         0x52000178
.equ rMAXP_REG,          0x52000180
.equ rEP0_CSR,           0x52000184
.equ rIN_CSR1_REG,       0x52000184
.equ rIN_CSR2_REG,       0x52000188
.equ rOUT_CSR1_REG,      0x52000190
.equ rOUT_CSR2_REG,      0x52000194
.equ rOUT_FIFO_CNT1_REG, 0x52000198
.equ rOUT_FIFO_CNT2_REG, 0x5200019c
.equ rEP0_FIFO,          0x520001c0
.equ rEP1_FIFO,          0x520001c4
.equ rEP2_FIFO,          0x520001c8
.equ rEP3_FIFO,          0x520001cc
.equ rEP4_FIFO,          0x520001d0
.equ rEP1_DMA_CON,       0x52000200
.equ rEP1_DMA_UNIT,      0x52000204
.equ rEP1_DMA_FIFO,      0x52000208
.equ rEP1_DMA_TTC_L,     0x5200020c
.equ rEP1_DMA_TTC_M,     0x52000210
.equ rEP1_DMA_TTC_H,     0x52000214
.equ rEP2_DMA_CON,       0x52000218
.equ rEP2_DMA_UNIT,      0x5200021c
.equ rEP2_DMA_FIFO,      0x52000220
.equ rEP2_DMA_TTC_L,     0x52000224
.equ rEP2_DMA_TTC_M,     0x52000228
.equ rEP2_DMA_TTC_H,     0x5200022c
.equ rEP3_DMA_CON,       0x52000240
.equ rEP3_DMA_UNIT,      0x52000244
.equ rEP3_DMA_FIFO,      0x52000248
.equ rEP3_DMA_TTC_L,     0x5200024c
.equ rEP3_DMA_TTC_M,     0x52000250
.equ rEP3_DMA_TTC_H,     0x52000254
.equ rEP4_DMA_CON,       0x52000258
.equ rEP4_DMA_UNIT,      0x5200025c
.equ rEP4_DMA_FIFO,      0x52000260
.equ rEP4_DMA_TTC_L,     0x52000264
.equ rEP4_DMA_TTC_M,     0x52000268
.equ rEP4_DMA_TTC_H,     0x5200026c

// =================
// USB Host
// =================
.equ rHcRevision,        0x49000000
.equ rHcControl,         0x49000004
.equ rHcCommonStatus,    0x49000008
.equ rHcInterruptStatus, 0x4900000c
.equ rHcInterruptEnable, 0x49000010
.equ rHcInterruptDisable,0x49000014
.equ rHcHCCA,            0x49000018
.equ rHcPeriodCurrentED, 0x4900001c
.equ rHcControlHeadED,   0x49000020
.equ rHcControlCurrentED,0x49000024
.equ rHcBulkHeadED,      0x49000028
.equ rHcBulkCurrentED,   0x4900002c
.equ rHcDoneHead,        0x49000030
.equ rHcFmInterval,      0x49000034
.equ rHcFmRemaining,     0x49000038
.equ rHcFmNumber,        0x4900003c
.equ rHcPeriodicStart,   0x49000040
.equ rHcLSThreshold,     0x49000044
.equ rHcRhDescriptorA,   0x49000048
.equ rHcRhDescriptorB,   0x4900004c
.equ rHcRhStatus,        0x49000050
.equ rHcRhPortStatus1,   0x49000054
.equ rHcRhPortStatus2,   0x49000058

// =================
// DMAC
// =================
.equ rDISRC0,     0x4b000000
.equ rDISRCC0,    0x4b000004
.equ rDIDST0,     0x4b000008
.equ rDIDSTC0,    0x4b00000c
.equ rDCON0,      0x4b000010
.equ rDSTAT0,     0x4b000014
.equ rDCSRC0,     0x4b000018
.equ rDCDST0,     0x4b00001c
.equ rDMASKTRIG0, 0x4b000020

.equ rDISRC1,     0x4b000040
.equ rDISRCC1,    0x4b000044
.equ rDIDST1,     0x4b000048
.equ rDIDSTC1,    0x4b00004c
.equ rDCON1,      0x4b000050
.equ rDSTAT1,     0x4b000054
.equ rDCSRC1,     0x4b000058
.equ rDCDST1,     0x4b00005c
.equ rDMASKTRIG1, 0x4b000060

.equ rDISRC2,     0x4b000080
.equ rDISRCC2,    0x4b000084
.equ rDIDST2,     0x4b000088
.equ rDIDSTC2,    0x4b00008c
.equ rDCON2,      0x4b000090
.equ rDSTAT2,     0x4b000094
.equ rDCSRC2,     0x4b000098
.equ rDCDST2,     0x4b00009c
.equ rDMASKTRIG2, 0x4b0000a0

.equ rDISRC3,     0x4b0000c0
.equ rDISRCC3,    0x4b0000c4
.equ rDIDST3,     0x4b0000c8
.equ rDIDSTC3,    0x4b0000cc
.equ rDCON3,      0x4b0000d0
.equ rDSTAT3,     0x4b0000d4
.equ rDCSRC3,     0x4b0000d8
.equ rDCDST3,     0x4b0000dc
.equ rDMASKTRIG3, 0x4b0000e0

// =================
// LCD CONTROLLER
// =================
.equ rLCDCON1,    0x4d000000
.equ rLCDCON2,    0x4d000004
.equ rLCDCON3,    0x4d000008
.equ rLCDCON4,    0x4d00000c
.equ rLCDCON5,    0x4d000010
.equ rLCDSADDR1,  0x4d000014
.equ rLCDSADDR2,  0x4d000018
.equ rLCDSADDR3,  0x4d00001c
.equ rREDLUT,     0x4d000020
.equ rGREENLUT,   0x4d000024
.equ rBLUELUT,    0x4d000028
.equ rDITHMODE,   0x4d00004c
.equ rTPAL,       0x4d000050
.equ rLCDINTPND,  0x4d000054
.equ rLCDSRCPND,  0x4d000058
.equ rLCDINTMSK,  0x4d00005c
.equ rTCONSEL,    0x4d000060

// =================
// ADC & TOUCH SCREEN
// =================
.equ rADCCON,     0x58000000
.equ rADCTSC,     0x58000004
.equ rADCDLY,     0x58000008
.equ rADCDAT0,    0x5800000c
.equ rADCDAT1,    0x58000010
.equ rADCUPDN,    0x58000014

// =================
// RTC
// =================
.equ rRTCCON,     0x57000040
.equ rTICNT,      0x57000044
.equ rRTCALM,     0x57000050
.equ rALMSEC,     0x57000054
.equ rALMMIN,     0x57000058
.equ rALMHOUR,    0x5700005c
.equ rALMDATE,    0x57000060
.equ rALMMON,     0x57000064
.equ rALMYEAR,    0x57000068
.equ rRTCRST,     0x5700006c
.equ rBCDSEC,     0x57000070
.equ rBCDMIN,     0x57000074
.equ rBCDHOUR,    0x57000078
.equ rBCDDATE,    0x5700007c
.equ rBCDDAY,     0x57000080
.equ rBCDMON,     0x57000084
.equ rBCDYEAR,    0x57000088

// =================
// IIC
// =================
.equ rIICCON,     0x54000000
.equ rIICSTAT,    0x54000004
.equ rIICADD,     0x54000008
.equ rIICDS,      0x5400000c
.equ rIICLC,      0x54000010

// =================
// IIS
// =================
.equ rIISCON,     0x55000000
.equ rIISMOD,     0x55000004
.equ rIISPSR,     0x55000008
.equ rIISFCON,    0x5500000c
.equ rIISFIFO,    0x55000010

// =================
// SPI
// =================
.equ rSPICON0,    0x59000000
.equ rSPISTA0,    0x59000004
.equ rSPIPIN0,    0x59000008
.equ rSPIPRE0,    0x5900000c
.equ rSPITDAT0,   0x59000010
.equ rSPIRDAT0,   0x59000014

.equ rSPICON1,    0x59000020
.equ rSPISTA1,    0x59000024
.equ rSPIPIN1,    0x59000028
.equ rSPIPRE1,    0x5900002c
.equ rSPITDAT1,   0x59000030
.equ rSPIRDAT1,   0x59000034

// =================
// SD Interface
// =================
.equ rSDICON,     0x5a000000
.equ rSDIPRE,     0x5a000004
.equ rSDICARG,    0x5a000008
.equ rSDICCON,    0x5a00000c
.equ rSDICSTA,    0x5a000010
.equ rSDIRSP0,    0x5a000014
.equ rSDIRSP1,    0x5a000018
.equ rSDIRSP2,    0x5a00001c
.equ rSDIRSP3,    0x5a000020
.equ rSDIDTIMER,  0x5a000024
.equ rSDIBSIZE,   0x5a000028
.equ rSDIDCON,    0x5a00002c
.equ rSDIDCNT,    0x5a000030
.equ rSDIDSTA,    0x5a000034
.equ rSDIFSTA,    0x5a000038
.equ rSDIDAT,     0x5a00003c
.equ rSDIIMSK,    0x5a000040

// =================
// Camera Interface
// =================
.equ rCISRCFMT,   0x4f000000
.equ rCIWINFMT,   0x4f000004
.equ rCIWINOFSH,  0x4f000008
.equ rCIWINOFSV,  0x4f00000c
.equ rCISRCNOFSH, 0x4f000010
.equ rCISRCNOFSV, 0x4f000014
.equ rCISRCOFSH,  0x4f000018
.equ rCISRCOFSV,  0x4f00001c
.equ rCIGCTRL,    0x4f000020
.equ rCICOYSA1,   0x4f000030
.equ rCICOYSA2,   0x4f000034
.equ rCICOYSA3,   0x4f000038
.equ rCICOYSA4,   0x4f00003c
.equ rCICOCBSA1,  0x4f000040
.equ rCICOCBSA2,  0x4f000044
.equ rCICOCBSA3,  0x4f000048
.equ rCICOCBSA4,  0x4f00004c
.equ rCICOCRSA1,  0x4f000050
.equ rCICOCRSA2,  0x4f000054
.equ rCICOCRSA3,  0x4f000058
.equ rCICOCRSA4,  0x4f00005c
.equ rCICOTRGFMT, 0x4f000060
.equ rCICOCTRL,   0x4f000064
.equ rCICOSCPRERATIO, 0x4f000068
.equ rCICOSCPREDST,   0x4f00006c
.equ rCICOSCCTRL,     0x4f000070
.equ rCICOAREA,       0x4f000074
.equ rCICOSTATUS,     0x4f000078
.equ rCIPRCLRSRA1,    0x4f000080
.equ rCIPRCLRSRA2,    0x4f000084
.equ rCIPRCLRSRA3,    0x4f000088
.equ rCIPRCLRSRA4,    0x4f00008c
.equ rCIPRTRGFMT,     0x4f000090
.equ rCIPRCTRL,       0x4f000094
.equ rCIPRSCPRERATIO, 0x4f000098
.equ rCIPRSCPREDST,   0x4f00009c
.equ rCIPRSCCTRL,     0x4f0000a0
.equ rCIPRAREA,       0x4f0000a4
.equ rCIPRSTATUS,     0x4f0000a8

// =================
// AC97
// =================
.equ rAC_GLBCTRL, 0x5b000000
.equ rAC_GLBSTAT, 0x5b000004
.equ rAC_CODEC_CMD, 0x5b000008
.equ rAC_CODEC_STAT, 0x5b00000c
.equ rAC_PCMADDR, 0x5b000010
.equ rAC_MICADDR, 0x5b000014
.equ rAC_PCMDATA, 0x5b000018
.equ rAC_MICDATA, 0x5b00001c
