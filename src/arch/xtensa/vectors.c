#include "vectors.h"
#include "intrin.h"
#include "util/log.h"

#include <util/printf.h>

void common_exception_entry(void);

void* g_fast_interrupts[64] = {
    // start with everything
    [0 ... 63] = common_exception_entry,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The exception handler pushes everything for easier debugging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct exception_regs {
    uint32_t ar[64];        // 0
    uint32_t sar;           // 256
    uint32_t lbeg;          // 260
    uint32_t lend;          // 264
    uint32_t lcount;        // 268
    uint32_t pc;            // 272
    uint32_t ps;            // 276
    uint32_t windowbase;    // 280
    uint32_t windowstart;   // 284
    uint16_t windowmask;    // 288
    uint16_t windowsize;    // 190
} exception_regs_t;
STATIC_ASSERT(offsetof(exception_regs_t, sar) == EXCEPTION_REGS_SAR);
STATIC_ASSERT(offsetof(exception_regs_t, lbeg) == EXCEPTION_REGS_LBEG);
STATIC_ASSERT(offsetof(exception_regs_t, lend) == EXCEPTION_REGS_LEND);
STATIC_ASSERT(offsetof(exception_regs_t, lcount) == EXCEPTION_REGS_LCOUNT);
STATIC_ASSERT(offsetof(exception_regs_t, pc) == EXCEPTION_REGS_PC);
STATIC_ASSERT(offsetof(exception_regs_t, ps) == EXCEPTION_REGS_PS);
STATIC_ASSERT(offsetof(exception_regs_t, windowbase) == EXCEPTION_REGS_WINDOWBASE);
STATIC_ASSERT(offsetof(exception_regs_t, windowstart) == EXCEPTION_REGS_WINDOWSTART);
STATIC_ASSERT(offsetof(exception_regs_t, windowmask) == EXCEPTION_REGS_WINDOWMASK);
STATIC_ASSERT(sizeof(exception_regs_t) == EXCEPTION_REGS_SIZE);

/**
 * The possible exception causes, any commented out exception
 * does not exist on our board, but we have it for reference
 */
typedef enum exception_cause {
    IllegalInstructionCause     = 0,
    SyscallCause                = 1,
    InstructionFetchErrorCause  = 2,
    LoadStoreErrorCause         = 3,
    Level1InterruptCause        = 4,
    AllocaCause                 = 5,
    IntegerDivideByZero         = 6,
    //                          = 7,
    PrivilegedCause             = 8,
    LoadStoreAlignmentCause     = 9,
    //                          = 10,
    //                          = 11,
    InstrPIFDataErrorCause      = 12,
    LoadStorePIFDataErrorCause  = 13,
    InstrPIFAddrErrorCause      = 14,
    LoadStorePIFAddrErrorCause  = 15,
    InstTLBMissCause            = 16,
    InstTLBMultiHitCause        = 17,
    InstFetchPrivilegeCause     = 18,
    //                          = 19,
    InstFetchProhibitedCause    = 20,
    //                          = 21,
    //                          = 22,
    //                          = 23,
    LoadStoreTLBMissCause       = 24,
    LoadStoreTLBMultiHitCause   = 25,
    LoadStorePrivilegeCause     = 26,
    //                          = 27,
    LoadProhibitedCause         = 28,
    StoreProhibitedCause        = 29,
    //                          = 30,
    //                          = 31,
    Coprocessor0Disabled        = 32,
    Coprocessor1Disabled        = 33,
    Coprocessor2Disabled        = 34,
    Coprocessor3Disabled        = 35,
    Coprocessor4Disabled        = 36,
    Coprocessor5Disabled        = 37,
    Coprocessor6Disabled        = 38,
    Coprocessor7Disabled        = 39,
} exception_cause_t;

static const char* m_cause_str[] = {
    [IllegalInstructionCause] = "IllegalInstructionCause",
    [SyscallCause] = "SyscallCause",
    [InstructionFetchErrorCause] = "InstructionFetchErrorCause",
    [LoadStoreErrorCause] = "LoadStoreErrorCause",
    [Level1InterruptCause] = "Level1InterruptCause",
    [AllocaCause] = "AllocaCause",
    [IntegerDivideByZero] = "IntegerDivideByZero",
    [PrivilegedCause] = "PrivilegedCause",
    [LoadStoreAlignmentCause] = "LoadStoreAlignmentCause",
    [InstrPIFDataErrorCause] = "InstrPIFDataErrorCause",
    [LoadStorePIFDataErrorCause] = "LoadStorePIFDataErrorCause",
    [InstrPIFAddrErrorCause] = "InstrPIFAddrErrorCause",
    [LoadStorePIFAddrErrorCause] = "LoadStorePIFAddrErrorCause",
    [InstTLBMissCause] = "InstTLBMissCause",
    [InstTLBMultiHitCause] = "InstTLBMultiHitCause",
    [InstFetchPrivilegeCause] = "InstFetchPrivilegeCause",
    [InstFetchProhibitedCause] = "InstFetchProhibitedCause",
    [LoadStoreTLBMissCause] = "LoadStoreTLBMissCause",
    [LoadStoreTLBMultiHitCause] = "LoadStoreTLBMultiHitCause",
    [LoadStorePrivilegeCause] = "LoadStorePrivilegeCause",
    [LoadProhibitedCause] = "LoadProhibitedCause",
    [StoreProhibitedCause] = "StoreProhibitedCause",
    [Coprocessor0Disabled] = "Coprocessor0Disabled",
    [Coprocessor1Disabled] = "Coprocessor1Disabled",
    [Coprocessor2Disabled] = "Coprocessor2Disabled",
    [Coprocessor3Disabled] = "Coprocessor3Disabled",
    [Coprocessor4Disabled] = "Coprocessor4Disabled",
    [Coprocessor5Disabled] = "Coprocessor5Disabled",
    [Coprocessor6Disabled] = "Coprocessor6Disabled",
    [Coprocessor7Disabled] = "Coprocessor7Disabled",
};

void* xthal_memcpy(void *dst, const void *src, unsigned len);

typedef enum opcode_format {
    XTENSA_RRR,
    XTENSA_RRI4,
    XTENSA_RRI8,
    XTENSA_RI16,
    XTENSA_RSR,
    XTENSA_CALL,
    XTENSA_CALLX,
    XTENSA_BRI8,
    XTENSA_BRI12,
    XTENSA_RRRN,
    XTENSA_RI7,
    XTENSA_RI6,
} opcode_format_t;

typedef union xtensa_opcode {
    uint8_t op0 : 4;
    struct {
        uint8_t op0 : 4;
        uint8_t t : 4;
        uint8_t s : 4;
        uint8_t r : 4;
        uint8_t imm8;
    } rri8;
    uint32_t value;
} PACKED xtensa_opcode_t;

static void print_opcodes(void* ptr, int32_t len) {
    while (len > 0) {
        xtensa_opcode_t opcode = { .value = 0 };
        xthal_memcpy(&opcode, ptr, 3);

        printf_("[-] %08x: ", ptr);

        switch (opcode.op0) {
            case 0b0010: {
                switch (opcode.rri8.r) {
                    case 0b0010: printf_("L32I a%d, a%d, %u", opcode.rri8.t, opcode.rri8.s, opcode.rri8.imm8); len += 3; break;
                    default: printf_("Invalid opcode %08x", opcode.value); len -= 3; break;
                }
            } break;
            default: printf_("Invalid opcode %08x", opcode.value); len -= 3; break;
        }

        printf_("\n\r");
    }
}
typedef struct reg_offset {
    const char* name;
    uint32_t offset;
} reg_offset_t;

static reg_offset_t m_reg_offsets[] = {
        { "LBEG", offsetof(exception_regs_t, lbeg) },
        { "LEND", offsetof(exception_regs_t, lend) },
        { "LCOUNT", offsetof(exception_regs_t, lcount) },
        { "SAR", offsetof(exception_regs_t, sar) },
        { "WINDOWBASE", offsetof(exception_regs_t, windowbase) },
        { "WINDOWSTART", offsetof(exception_regs_t, windowstart) },
        { "PS", offsetof(exception_regs_t, ps) },
};

void exception_regs_dump(exception_regs_t* regs) {
    LOG_TRACE("PC=%08x", regs->pc);

    LOG_TRACE("");

    for (int i = 0; i < ALIGN_UP(ARRAY_LENGTH(m_reg_offsets), 4) / 4; i++) {
        printf_("[*] ");
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx >= ARRAY_LENGTH(m_reg_offsets)) break;

            size_t value = *(size_t*)((void*)regs + m_reg_offsets[idx].offset);
            const char* name = m_reg_offsets[idx].name;
            printf_("%12s=%08x ", name, value);
        }
        printf_("\n\r");
    }

    LOG_TRACE("");

    for (int i = 0; i < 4; i++) {
        printf_("[*] ");
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            printf_(" A%02d=%08x", idx, regs->ar[idx]);
        }
        printf_("\n\r");
    }

    LOG_TRACE("");

    for (int i = 0; i < 64 / 4; i++) {
        printf_("[*] ");
        for (int j = 0; j < 4; j++) {
            int abs_idx = i * 4 + j;

            int real_idx = abs_idx - regs->windowbase * 4;
            if (real_idx < 0) {
                real_idx = 64 + real_idx;
            }

            printf_("AR%02d=%08x ", abs_idx, regs->ar[real_idx]);

            if ((abs_idx % 4) == 3) {
                // check for windowbase/windowstart
                bool ws = (regs->windowstart & (1 << (abs_idx / 4))) != 0;
                bool cw = regs->windowbase == (abs_idx / 4);
                printf_("%c%c\n\r", ws ? '<' : ' ', cw ? '=' : ' ');
            }
        }
    }
}

void common_exception_handler(exception_regs_t* regs) {
    int cause = RSR(EXCCAUSE);

    // print the cuase
    printf_("[-] got exception ");
    if (cause < ARRAY_LENGTH(m_cause_str) && m_cause_str[cause] != NULL) {
        printf_("%s (%d)", m_cause_str[cause], cause);
    } else {
        printf_("%d", cause);
    }

    // print the vaddr if needed
    if (
        cause == InstructionFetchErrorCause ||
        cause == LoadStoreErrorCause ||
        cause == LoadStoreAlignmentCause ||
        cause == InstrPIFDataErrorCause ||
        cause == LoadStorePIFDataErrorCause ||
        cause == InstrPIFAddrErrorCause ||
        cause == LoadStorePIFAddrErrorCause ||
        cause == InstTLBMissCause ||
        cause == InstTLBMultiHitCause ||
        cause == InstFetchPrivilegeCause ||
        cause == InstFetchProhibitedCause ||
        cause == LoadStoreTLBMissCause ||
        cause == LoadStoreTLBMultiHitCause ||
        cause == LoadStorePrivilegeCause ||
        cause == LoadProhibitedCause ||
        cause == StoreProhibitedCause
    ) {
        printf_(" EXCVADDR=%08x", RSR(EXCVADDR));
        printf_("\n\r");
    }

    printf_("\n\r");

    // TODO: translate if has mmu
    print_opcodes((void*)regs->pc, 16);
    LOG_ERROR("");

    // dump everything
    exception_regs_dump(regs);

    // TODO: kill the task that caused the problem

    while(1);
}
