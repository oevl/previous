/***************************************************************************

    i860.h

    Interface file for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)
    Released for general non-commercial use under the MAME license
    with the additional requirement that you are free to use and
    redistribute this code in modified or unmodified form, provided
    you list me in the credits.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __I860_H__
#define __I860_H__

#include <stdint.h>

#define TRACE_I860           0
#define TRACE_RDWR_MEM       0
#define TRACE_PAGE_FAULT     1
#define TRACE_UNDEFINED_I860 0
#define TRACE_UNALIGNED_MEM  0
#define TRACE_EXT_INT        0
        
typedef uint64_t UINT64;
typedef int64_t INT64;

typedef uint32_t UINT32;
typedef int32_t INT32;

typedef uint16_t UINT16;
typedef int16_t INT16;

typedef uint8_t  UINT8;
typedef int8_t  INT8;

typedef int64_t offs_t;

extern "C" {
    UINT8  nd_board_bget(UINT32 addr);
    void   nd_board_bput(UINT32 addr, UINT8 val);
    UINT16 nd_board_wget(UINT32 addr);
    void   nd_board_wput(UINT32 addr, UINT16 val);
    UINT32 nd_board_lget(UINT32 addr);
    void   nd_board_lput(UINT32 addr, UINT32 val);
    int    nd_process_interrupts(int nHostCycles);
    bool   i860_trace(UINT32 addr);
}

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum {
	I860_PC = 1,

	I860_FIR,
	I860_PSR,
	I860_DIRBASE,
	I860_DB,
	I860_FSR,
	I860_EPSR,

	I860_R0,  I860_R1,  I860_R2,  I860_R3,  I860_R4,  I860_R5,  I860_R6,  I860_R7,  I860_R8,  I860_R9,
	I860_R10, I860_R11, I860_R12, I860_R13, I860_R14, I860_R15, I860_R16, I860_R17, I860_R18, I860_R19,
	I860_R20, I860_R21, I860_R22, I860_R23, I860_R24, I860_R25, I860_R26, I860_R27, I860_R28, I860_R29,
	I860_R30, I860_R31,

	I860_F0,  I860_F1,  I860_F2,  I860_F3,  I860_F4,  I860_F5,  I860_F6,  I860_F7,  I860_F8,  I860_F9,
	I860_F10, I860_F11, I860_F12, I860_F13, I860_F14, I860_F15, I860_F16, I860_F17, I860_F18, I860_F19,
	I860_F20, I860_F21, I860_F22, I860_F23, I860_F24, I860_F25, I860_F26, I860_F27, I860_F28, I860_F29,
	I860_F30, I860_F31,
};

const int STATE_SZ = I860_F31+1;

/* Defines for pending_trap.  */
enum {
    TRAP_NORMAL        = 0x01,
    TRAP_IN_DELAY_SLOT = 0x02,
    TRAP_WAS_EXTERNAL  = 0x04
};

/* Macros for accessing register fields in instruction word.  */
#define get_isrc1(bits) (((bits) >> 11) & 0x1f)
#define get_isrc2(bits) (((bits) >> 21) & 0x1f)
#define get_idest(bits) (((bits) >> 16) & 0x1f)
#define get_fsrc1(bits) (((bits) >> 11) & 0x1f)
#define get_fsrc2(bits) (((bits) >> 21) & 0x1f)
#define get_fdest(bits) (((bits) >> 16) & 0x1f)
#define get_creg(bits) (((bits) >> 21) & 0x7)

/* Macros for accessing immediate fields.  */
/* 16-bit immediate.  */
#define get_imm16(insn) ((insn) & 0xffff)

/* A mask for all the trap bits of the PSR (FT, DAT, IAT, IN, IT, or
 bits [12..8]).  */
#define PSR_ALL_TRAP_BITS_MASK 0x00001f00

/* A mask for PSR bits which can only be changed from supervisor level.  */
#define PSR_SUPERVISOR_ONLY_MASK 0x0000fff3


/* PSR: BR flag (PSR[0]):  set/get.  */
#define GET_PSR_BR()  ((m_cregs[CR_PSR] >> 0) & 1)
#define SET_PSR_BR(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 0)) | (((val) & 1) << 0))

/* PSR: BW flag (PSR[1]):  set/get.  */
#define GET_PSR_BW()  ((m_cregs[CR_PSR] >> 1) & 1)
#define SET_PSR_BW(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 1)) | (((val) & 1) << 1))

/* PSR: Shift count (PSR[21..17]):  set/get.  */
#define GET_PSR_SC()  ((m_cregs[CR_PSR] >> 17) & 0x1f)
#define SET_PSR_SC(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~0x003e0000) | (((val) & 0x1f) << 17))

/* PSR: CC flag (PSR[2]):  set/get.  */
#define GET_PSR_CC()  ((m_cregs[CR_PSR] >> 2) & 1)
#define SET_PSR_CC(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 2)) | (((val) & 1) << 2))

/* PSR: IT flag (PSR[8]):  set/get.  */
#define GET_PSR_IT()  ((m_cregs[CR_PSR] >> 8) & 1)
#define SET_PSR_IT(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 8)) | (((val) & 1) << 8))

/* PSR: IN flag (PSR[9]):  set/get.  */
#define GET_PSR_IN()  ((m_cregs[CR_PSR] >> 9) & 1)
#define SET_PSR_IN(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 9)) | (((val) & 1) << 9))

/* PSR: IAT flag (PSR[10]):  set/get.  */
#define GET_PSR_IAT()  ((m_cregs[CR_PSR] >> 10) & 1)
#define SET_PSR_IAT(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 10)) | (((val) & 1) << 10))

/* PSR: DAT flag (PSR[11]):  set/get.  */
#define GET_PSR_DAT()  ((m_cregs[CR_PSR] >> 11) & 1)
#define SET_PSR_DAT(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 11)) | (((val) & 1) << 11))

/* PSR: FT flag (PSR[12]):  set/get.  */
#define GET_PSR_FT()  ((m_cregs[CR_PSR] >> 12) & 1)
#define SET_PSR_FT(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 12)) | (((val) & 1) << 12))

/* PSR: DS flag (PSR[13]):  set/get.  */
#define GET_PSR_DS()  ((m_cregs[CR_PSR] >> 13) & 1)
#define SET_PSR_DS(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 13)) | (((val) & 1) << 13))

/* PSR: DIM flag (PSR[14]):  set/get.  */
#define GET_PSR_DIM()  ((m_cregs[CR_PSR] >> 14) & 1)
#define SET_PSR_DIM(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 14)) | (((val) & 1) << 14))

/* PSR: LCC (PSR[3]):  set/get.  */
#define GET_PSR_LCC()  ((m_cregs[CR_PSR] >> 3) & 1)
#define SET_PSR_LCC(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 3)) | (((val) & 1) << 3))

/* PSR: IM (PSR[4]):  set/get.  */
#define GET_PSR_IM()  ((m_cregs[CR_PSR] >> 4) & 1)
#define SET_PSR_IM(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 4)) | (((val) & 1) << 4))

/* PSR: PIM (PSR[5]):  set/get.  */
#define GET_PSR_PIM()  ((m_cregs[CR_PSR] >> 5) & 1)
#define SET_PSR_PIM(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 5)) | (((val) & 1) << 5))

/* PSR: U (PSR[6]):  set/get.  */
#define GET_PSR_U()  ((m_cregs[CR_PSR] >> 6) & 1)
#define SET_PSR_U(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 6)) | (((val) & 1) << 6))

/* PSR: PU (PSR[7]):  set/get.  */
#define GET_PSR_PU()  ((m_cregs[CR_PSR] >> 7) & 1)
#define SET_PSR_PU(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~(1 << 7)) | (((val) & 1) << 7))

/* PSR: Pixel size (PSR[23..22]):  set/get.  */
#define GET_PSR_PS()  ((m_cregs[CR_PSR] >> 22) & 0x3)
#define SET_PSR_PS(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~0x00c00000) | (((val) & 0x3) << 22))

/* PSR: Pixel mask (PSR[31..24]):  set/get.  */
#define GET_PSR_PM()  ((m_cregs[CR_PSR] >> 24) & 0xff)
#define SET_PSR_PM(val)  (m_cregs[CR_PSR] = (m_cregs[CR_PSR] & ~0xff000000) | (((val) & 0xff) << 24))

/* EPSR: WP bit (EPSR[14]):  set/get.  */
#define GET_EPSR_WP()  ((m_cregs[CR_EPSR] >> 14) & 1)
#define SET_EPSR_WP(val)  (m_cregs[CR_EPSR] = (m_cregs[CR_EPSR] & ~(1 << 14)) | (((val) & 1) << 14))

/* EPSR: INT bit (EPSR[17]):  set/get.  */
#define GET_EPSR_INT()  ((m_cregs[CR_EPSR] >> 17) & 1)
#define SET_EPSR_INT(val)  (m_cregs[CR_EPSR] = (m_cregs[CR_EPSR] & ~(1 << 17)) | (((val) & 1) << 17))


/* EPSR: OF flag (EPSR[24]):  set/get.  */
#define GET_EPSR_OF()  ((m_cregs[CR_EPSR] >> 24) & 1)
#define SET_EPSR_OF(val)  (m_cregs[CR_EPSR] = (m_cregs[CR_EPSR] & ~(1 << 24)) | (((val) & 1) << 24))

/* EPSR: BE flag (EPSR[23]):  set/get.  */
#define GET_EPSR_BE()  ((m_cregs[CR_EPSR] >> 23) & 1)
#define SET_EPSR_BE(val)  (m_cregs[CR_EPSR] = (m_cregs[CR_EPSR] & ~(1 << 23)) | (((val) & 1) << 23))

/* DIRBASE: ATE bit (DIRBASE[0]):  get.  */
#define GET_DIRBASE_ATE()  (m_cregs[CR_DIRBASE] & 1)

/* DIRBASE: CS8 bit (DIRBASE[7]):  get.  */
#define GET_DIRBASE_CS8()  ((m_cregs[CR_DIRBASE] >> 7) & 1)

/* FSR: FTE bit (FSR[5]):  set/get.  */
#define GET_FSR_FTE()  ((m_cregs[CR_FSR] >> 5) & 1)
#define SET_FSR_FTE(val)  (m_cregs[CR_FSR] = (m_cregs[CR_FSR] & ~(1 << 5)) | (((val) & 1) << 5))

/* FSR: SE bit (FSR[8]):  set/get.  */
#define GET_FSR_SE()  ((m_cregs[CR_FSR] >> 8) & 1)
#define SET_FSR_SE(val)  (m_cregs[CR_FSR] = (m_cregs[CR_FSR] & ~(1 << 8)) | (((val) & 1) << 8))

/* Control register numbers.  */
enum {
    CR_FIR     = 0,
    CR_PSR     = 1,
    CR_DIRBASE = 2,
    CR_DB      = 3,
    CR_FSR     = 4,
    CR_EPSR    = 5
};

class i860_reg {
    UINT32        id;
    const char*   name;
    const char*   format;
    const UINT32* reg;
public:
    i860_reg() : id(0), name(0), format(0), reg(&id) {}
    
    bool valid() {
        return name;
    }
    
    void formatstr(const char* format) {
        this->format = format;
    }
    
    void set(int regId, const char* name, const UINT32 * reg) {
        this->id   = regId;
        this->name = name;
        this->reg  = reg;
    }
    
    UINT32 get() const {
        return *reg;
    }
    
    const char* get_name() {
        return name;
    }
};

class i860_cpu_device {
public:
	// construction/destruction
	i860_cpu_device();

    void init();
    void run_cycle();
    
	/* This is the external interface for asserting an external interrupt
	   to the i860.  */
	void i860_gen_interrupt();

    /* This is the external interface for clearing an external interrupt
     to the i860.  */
    void i860_clr_interrupt();

	/* This is the external interface for asserting/deasserting a pin on
	   the i860.  */
	void i860_set_pin(int, int);

	/* Hard or soft reset.  */
	void reset_i860();
    
    // device_disasm_interface overrides
    virtual UINT32 disasm_min_opcode_bytes() const { return 4; }
    virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
    virtual offs_t disasm(char *buffer, offs_t pc);
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 8; }
	virtual UINT32 execute_input_lines() const { return 0; }

    void get_state(UINT32* state) {
        for(int i = 0; i < STATE_SZ; i++)
            state[i] = m_regs[i].get();
    }
    
    i860_reg & state(int regId, const char* name, UINT32 * reg) {
        m_regs[regId].set(regId, name, reg);
        return m_regs[regId];
    }
    
    void state_delta(char* buffer, const UINT32* oldstate, const UINT32* newstate);
private:
    i860_reg m_regs[STATE_SZ];
    
	/* Integer registers (32 x 32-bits).  */
	UINT32 m_iregs[32];

	/* Floating point registers (32 x 32-bits, 16 x 64 bits, or 8 x 128 bits).
	   When referenced as pairs or quads, the higher numbered registers
	   are the upper bits. E.g., double precision f0 is f1:f0.  */
	UINT8 m_frg[32 * 4];

	/* Control registers (6 x 32-bits).  */
	UINT32 m_cregs[6];

	/* Program counter (1 x 32-bits).  Reset starts at pc=0xffffff00.  */
	UINT32 m_pc;

	/* Special registers (4 x 64-bits).  */
	union
	{
		float s;
		double d;
	} m_KR, m_KI, m_T;
	UINT64 m_merge;

	/* The adder pipeline, always 3 stages.  */
	struct
	{
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Adder result precision (1 = dbl, 0 = sgl).  */
			char arp;
		} stat;
	} m_A[3];

	/* The multiplier pipeline. 3 stages for single precision, 2 stages
	   for double precision, and confusing for mixed precision.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Multiplier result precision (1 = dbl, 0 = sgl).  */
			char mrp;
		} stat;
	} m_M[3];

	/* The load pipeline, always 3 stages.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Load result precision (1 = dbl, 0 = sgl).  */
			char lrp;
		} stat;
	} m_L[3];

	/* The graphics/integer pipeline, always 1 stage.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Integer/graphics result precision (1 = dbl, 0 = sgl).  */
			char irp;
		} stat;
	} m_G;

	/* Pins.  */
	int m_pin_bus_hold;
	int m_pin_reset;

	/*
	 * Other emulator state.
	 */
	int m_exiting_readmem;
	int m_exiting_ifetch;

	/* Indicate a control-flow instruction, so we know the PC is updated.  */
	int m_pc_updated;

	/* Indicate an instruction just generated a trap, so we know the PC
	   needs to go to the trap address.  */
	int m_pending_trap;

	/* This is 1 if the next fir load gets the trap address, otherwise
	   it is 0 to get the ld.c address.  This is set to 1 only when a
	   non-reset trap occurs.  */
	int m_fir_gets_trap_addr;

	/* Single stepping flag for internal use.  */
	int m_single_stepping;

    /* memory access */
    
    inline UINT8 rd8(UINT32 addr) {
        return nd_board_bget(addr);
    }
    
    inline UINT16 rd16le(UINT32 addr) {
        UINT16 result = nd_board_bget(addr+1); result <<= 8;
        result       |= nd_board_bget(addr+0);
        return result;
    }
    
    inline UINT32 rd32le(UINT32 addr) {
        UINT32 result = nd_board_bget(addr+3); result <<= 8;
        result       |= nd_board_bget(addr+2); result <<= 8;
        result       |= nd_board_bget(addr+1); result <<= 8;
        result       |= nd_board_bget(addr+0);
        return result;
    }
    
    inline UINT16 rd16(UINT32 addr) {
        return GET_EPSR_BE() ? nd_board_wget(addr) : rd16le(addr);
    }
    
    inline UINT32 rd32(UINT32 addr) {
        return GET_EPSR_BE() ? nd_board_lget(addr) : rd32le(addr);
    }

    inline void wr8(UINT32 addr, UINT8 val) {
        nd_board_bput(addr, val);
    }
    
    inline void wr16le(UINT32 addr, UINT16 val) {
        nd_board_wput(addr+1, val >> 8);
        nd_board_bput(addr+0, val);
    }
    
    inline void wr32le(UINT32 addr, UINT32 val) {
        nd_board_bput(addr+3, val >> 24);
        nd_board_bput(addr+2, val >> 16);
        nd_board_bput(addr+1, val >> 8);
        nd_board_bput(addr+0, val);
    }
    
    inline void wr16(UINT32 addr, UINT16 val) {
        if(GET_EPSR_BE())
            nd_board_wput(addr, val);
        else
            wr16le(addr, val);
    }
    
    inline void wr32(UINT32 addr, UINT32 val) {
        if(GET_EPSR_BE())
            nd_board_lput(addr, val);
        else
            wr32le(addr, val);
    }

    /*
	 * MAME-specific stuff.
	 */
	UINT32 m_ppc;
	int m_icount;
    int* m_icountptr;
	// For debugger
	UINT32 m_freg[32];

	void writememi_emu (UINT32 addr, int size, UINT32 data);
	void fp_readmem_emu (UINT32 addr, int size, UINT8 *dest);
	void fp_writemem_emu (UINT32 addr, int size, UINT8 *data, UINT32 wmask);
	void dump_pipe (int type);
	void dump_state ();
	void unrecog_opcode (UINT32 pc, UINT32 insn);
	void insn_ld_ctrl (UINT32 insn);
	void insn_st_ctrl (UINT32 insn);
	void insn_ldx (UINT32 insn);
	void insn_stx (UINT32 insn);
	void insn_fsty (UINT32 insn);
	void insn_fldy (UINT32 insn);
	void insn_pstd (UINT32 insn);
	void insn_ixfr (UINT32 insn);
	void insn_addu (UINT32 insn);
	void insn_addu_imm (UINT32 insn);
	void insn_adds (UINT32 insn);
	void insn_adds_imm (UINT32 insn);
	void insn_subu (UINT32 insn);
	void insn_subu_imm (UINT32 insn);
	void insn_subs (UINT32 insn);
	void insn_subs_imm (UINT32 insn);
	void insn_shl (UINT32 insn);
	void insn_shl_imm (UINT32 insn);
	void insn_shr (UINT32 insn);
	void insn_shr_imm (UINT32 insn);
	void insn_shra (UINT32 insn);
	void insn_shra_imm (UINT32 insn);
	void insn_shrd (UINT32 insn);
	void insn_and (UINT32 insn);
	void insn_and_imm (UINT32 insn);
	void insn_andh_imm (UINT32 insn);
	void insn_andnot (UINT32 insn);
	void insn_andnot_imm (UINT32 insn);
	void insn_andnoth_imm (UINT32 insn);
	void insn_or (UINT32 insn);
	void insn_or_imm (UINT32 insn);
	void insn_orh_imm (UINT32 insn);
	void insn_xor (UINT32 insn);
	void insn_xor_imm (UINT32 insn);
	void insn_xorh_imm (UINT32 insn);
	void insn_trap (UINT32 insn);
	void insn_intovr (UINT32 insn);
	void insn_bte (UINT32 insn);
	void insn_bte_imm (UINT32 insn);
	void insn_btne (UINT32 insn);
	void insn_btne_imm (UINT32 insn);
	void insn_bc (UINT32 insn);
	void insn_bnc (UINT32 insn);
	void insn_bct (UINT32 insn);
	void insn_bnct (UINT32 insn);
	void insn_call (UINT32 insn);
	void insn_br (UINT32 insn);
	void insn_bri (UINT32 insn);
	void insn_calli (UINT32 insn);
	void insn_bla (UINT32 insn);
	void insn_flush (UINT32 insn);
	void insn_fmul (UINT32 insn);
	void insn_fmlow (UINT32 insn);
	void insn_fadd_sub (UINT32 insn);
	void insn_dualop (UINT32 insn);
	void insn_frcp (UINT32 insn);
	void insn_frsqr (UINT32 insn);
	void insn_fxfr (UINT32 insn);
	void insn_ftrunc (UINT32 insn);
	void insn_famov (UINT32 insn);
	void insn_fiadd_sub (UINT32 insn);
	void insn_fcmp (UINT32 insn);
	void insn_fzchk (UINT32 insn);
	void insn_form (UINT32 insn);
	void insn_faddp (UINT32 insn);
	void insn_faddz (UINT32 insn);
	void decode_exec (UINT32 insn, UINT32 non_shadow);
	void disasm (UINT32 addr, int len);
	void dbg_db (UINT32 addr, int len);
	float get_fregval_s (int fr);
	double get_fregval_d (int fr);
	void set_fregval_s (int fr, float s);
	void set_fregval_d (int fr, double d);
	int has_delay_slot(UINT32 insn);
	UINT32 ifetch (UINT32 pc);
	UINT32 get_address_translation (UINT32 vaddr, int is_dataref, int is_write);
	UINT32 readmemi_emu (UINT32 addr, int size);
	float get_fval_from_optype_s (UINT32 insn, int optype);
	double get_fval_from_optype_d (UINT32 insn, int optype);

	typedef void (i860_cpu_device::*insn_func)(UINT32);
	struct decode_tbl_t
	{
		/* Execute function for this opcode.  */
		insn_func insn_exec;
		/* Flags for this opcode.  */
		char flags;
	};
	static const decode_tbl_t decode_tbl[64];
	static const decode_tbl_t core_esc_decode_tbl[8];
	static const decode_tbl_t fp_decode_tbl[128];
};


/* i860 pins.  */
enum {
	DEC_PIN_BUS_HOLD,       /* Bus HOLD pin.      */
	DEC_PIN_RESET           /* System reset pin.  */
};

/* disassembler */
int i860_disassembler(UINT32 pc, UINT32 insn, char* buffer);

#endif /* __I860_H__ */
