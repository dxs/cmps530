#include <iostream>
#include <thread>

#define DEBUG

#ifdef DEBUG
#define dout cout << __FILE__ << "(" << __LINE__ << ") DEBUG: "
#else
#define dout 0 && cout
#endif /* DEBUG */

using namespace std;

JS_NEVER_INLINE void
ThreadInterpret(int id, JSContext *cx, FrameRegs regs, int offset, jsbytecode *original_pc, jsbytecode *stop_pc)
{
    std::dout << "Thread " << id << ", PC: " << regs.pc - original_pc << ", Stop: " << stop_pc - original_pc << endl;
#include "interp-defines.h"
    /*
     * It is important that "op" be initialized before calling DO_OP because
     * it is possible for "op" to be specially assigned during the normal
     * processing of an opcode while looping. We rely on DO_NEXT_OP to manage
     * "op" correctly in all other cases.
     */
    JSOp op;
    int32_t len=0;
    int switchOp;
    register int switchMask = 0;

    DO_NEXT_OP(len);

    /*
     * This is a loop, but it does not look like a loop. The loop-closing
     * jump is distributed throughout goto *jumpTable[op] inside of DO_OP.
     * When interrupts are enabled, jumpTable is set to interruptJumpTable
     * where all jumps point to the interrupt label. The latter, after
     * calling the interrupt handler, dispatches through normalJumpTable to
     * continue the normal bytecode processing.
     */

    /* CAL Main interpret loop
     * Like the above comment says, this doesn't look like it, but it's a loop.
     * The main loop in fact.  regs.pc += len moves the program counter (len is
     * set from a table by the operation last executed).  The next line grabs the
     * opcode and the switch selects the proper action.
     */
    for (;;) {
      advance_pc_by_one:
        JS_ASSERT(js_CodeSpec[op].length == 1);
        len = 1;
      advance_pc:
        js::gc::MaybeVerifyBarriers(cx);
        regs.pc += len; // Set pc (len set by last op to execute)
        offset = regs.pc - original_pc ;
        op = (JSOp) *regs.pc; // Get the opcode
        std::dout << "PC:" << offset << " Opcode: " << op << std::endl;

      do_op:
        if (regs.pc == stop_pc) {
            return;
        }

#ifdef TRACKPC
        printf("PC:\t%d\n", offset);
#endif
        CHECK_PCCOUNT_INTERRUPTS();
        switchOp = int(op) | switchMask; // ??
      //do_switch:
        switch (switchOp) { // CAL Main instruction switch

/* No-ops for ease of decompilation. */
ADD_EMPTY_CASE(JSOP_NOP)
ADD_EMPTY_CASE(JSOP_CONDSWITCH)
ADD_EMPTY_CASE(JSOP_TRY)

#ifdef TRACEIT
printf("TRACE: EMPTY\n");
#endif
END_EMPTY_CASES

BEGIN_CASE(JSOP_LOOPHEAD)
#ifdef TRACEIT
printf("TRACE: JSOP_LOOPHEAD\n");
#endif
END_EMPTY_CASES
BEGIN_CASE(JSOP_LOOPENTRY)
#ifdef TRACEIT
printf("TRACE: JSOP_LOOPENTRY\n");
#endif
END_EMPTY_CASES

BEGIN_CASE(JSOP_LABEL)
END_CASE(JSOP_LABEL)

check_backedge:
{
    CHECK_BRANCH();
    if (op != JSOP_LOOPHEAD)
        DO_OP();

    DO_OP();
}

/* ADD_EMPTY_CASE is not used here as JSOP_LINENO_LENGTH == 3. */
BEGIN_CASE(JSOP_LINENO)
END_CASE(JSOP_LINENO)

BEGIN_CASE(JSOP_UNDEFINED)
#ifdef TRACEIT
    printf("TRACE: JSOP_UNDEFINED\n");
#endif
    PUSH_UNDEFINED();
END_CASE(JSOP_UNDEFINED)

          default:
          {
              std::cout << "Unimplemented opcode: " << op << std::endl;
            // char numBuf[12];
            // JS_snprintf(numBuf, sizeof numBuf, "%d", op);
            // JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL,
            //                      JSMSG_BAD_BYTECODE, numBuf);
            // exit(1);
            goto error;
          }

        } /* switch (op) */
    } /* for (;;) */

error:
exit(1);

}
