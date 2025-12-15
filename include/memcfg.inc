// ************************************************
// NAME    : memcfg_gnu.inc
// DESC    : Memory bank configuration file (GNU GAS)
// Adapted from Memcfg.inc
// ************************************************

// BWSCON
.equ DW8,  (0x0)
.equ DW16, (0x1)
.equ DW32, (0x2)
.equ WAIT, (0x1<<2)
.equ UBLB, (0x1<<3)

    .ifndef BUSWIDTH
    .error "BUSWIDTH must be defined before including this file"
    .endif

    .if BUSWIDTH == 16
.equ B1_BWSCON, (DW16)
.equ B2_BWSCON, (DW16)
.equ B3_BWSCON, (DW16)
.equ B4_BWSCON, (DW16+WAIT)
.equ B5_BWSCON, (DW16)
.equ B6_BWSCON, (DW16)
.equ B7_BWSCON, (DW16)
    .else 
    // BUSWIDTH=32		; 2440 EV board.
.equ B1_BWSCON, (DW16)	// AMD flash(AM29LV800B), 16-bit,  for nCS1
.equ B2_BWSCON, (DW16)	// PCMCIA(PD6710), 16-bit
.equ B3_BWSCON, (DW16)	// Ethernet(CS8900), 16-bit
.equ B4_BWSCON, (DW32)	// Intel Strata(28F128), 32-bit, for nCS4
.equ B5_BWSCON, (DW16)	// A400/A410 Ext, 16-bit
.equ B6_BWSCON, (DW32)	// SDRAM(K4S561632C) 32MBx2, 32-bit
.equ B7_BWSCON, (DW32)	// N.C.
    .endif

// BANK0CON

.equ B0_Tacs, 0x3	// 0clk
.equ B0_Tcos, 0x3	// 0clk
.equ B0_Tacc, 0x7	// 14clk
.equ B0_Tcoh, 0x3	// 0clk
.equ B0_Tah,  0x3	// 0clk
.equ B0_Tacp, 0x1
.equ B0_PMC,  0x0	// normal

// BANK1CON
.equ B1_Tacs, 1	// 0clk
.equ B1_Tcos, 1	// 0clk
.equ B1_Tacc, 6	// 14clk
.equ B1_Tcoh, 1	// 0clk
.equ B1_Tah,  1	// 0clk
.equ B1_Tacp, 0x0
.equ B1_PMC,  0x0	// normal

// Bank 2 parameter
.equ B2_Tacs, 1	// 0clk
.equ B2_Tcos, 1	// 0clk
.equ B2_Tacc, 6	// 14clk
.equ B2_Tcoh, 1	// 0clk
.equ B2_Tah,  1	// 0clk
.equ B2_Tacp, 0x0
.equ B2_PMC,  0x0	// normal

// Bank 3 parameter
.equ B3_Tacs, 0x1	// 0clk
.equ B3_Tcos, 0x1	// 0clk
.equ B3_Tacc, 0x6	// 14clk
.equ B3_Tcoh, 0x1	// 0clk
.equ B3_Tah,  0x1	// 0clk
.equ B3_Tacp, 0x0
.equ B3_PMC,  0x0	// normal

// Bank 4 parameter
.equ B4_Tacs, 0x1	// 0clk
.equ B4_Tcos, 0x1	// 0clk
.equ B4_Tacc, 0x6	// 14clk
.equ B4_Tcoh, 0x1	// 0clk
.equ B4_Tah,  0x1	// 0clk
.equ B4_Tacp, 0x0
.equ B4_PMC,  0x0	// normal

// Bank 5 parameter
.equ B5_Tacs, 0x1	// 0clk
.equ B5_Tcos, 0x1	// 0clk
.equ B5_Tacc, 0x6	// 14clk
.equ B5_Tcoh, 0x1	// 0clk
.equ B5_Tah,  0x1	// 0clk
.equ B5_Tacp, 0x0
.equ B5_PMC,  0x0	// normal

    .if 1	// When 100MHz HCLK is used.
// Bank 6 parameter
.equ B6_MT,   0x3	// SDRAM
.equ B6_Trcd, 0x1	// 3clk
.equ B6_SCAN, 0x1	// 9bit

// Bank 7 parameter
.equ B7_MT,   0x3	// SDRAM
.equ B7_Trcd, 0x1	// 3clk
.equ B7_SCAN, 0x1	// 9bit

// REFRESH parameter
.equ REFEN,  0x1	// Refresh enable
.equ TREFMD, 0x0	// CBR(CAS before RAS)/Auto refresh
.equ Trp,    0x1	// 3clk
.equ Tsrc,   0x1	// 5clk	Trc= Trp(3)+Tsrc(5) = 8clock
.equ Tchr,   0x2	// 3clk
.equ REFCNT, 1268	// HCLK=105Mhz, (2048+1-7.81*100);75M->1463

    .else
// Bank 6 parameter
.equ B6_MT,   0x3	// SDRAM
.equ B6_Trcd, 0x2	// 4clk
.equ B6_SCAN, 0x1	// 9bit

// Bank 7 parameter
.equ B7_MT,   0x3	// SDRAM
.equ B7_Trcd, 0x2	// 4clk
.equ B7_SCAN, 0x1	// 9bit

// REFRESH parameter
.equ REFEN,  0x1	// Refresh enable
.equ TREFMD, 0x0	// CBR(CAS before RAS)/Auto refresh
.equ Trp,    0x2	// 4clk
.equ Tsrc,   0x2	// 6clk	Trc= Trp(4)+Tsrc(6) = 10clock

.equ Tchr,   0x2	// 3clk
.equ REFCNT, 1012	// HCLK=135Mhz, (2048+1-7.8*133 = 1012)
    .endif
