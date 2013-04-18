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
ThreadInterpret(int id, JSContext *cx, FrameRegs *orig_regs, int offset, jsbytecode *original_pc, jsbytecode *stop_pc)
{
    FrameRegs regs = *orig_regs;
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

    RootedValue rootValue0(cx), rootValue1(cx);
    RootedObject rootObject0(cx), rootObject1(cx), rootObject2(cx);
    RootedId rootId0(cx);

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
        op = (JSOp) *(regs.pc); // Get the opcode
        std::dout << "PC:" << offset << " Opcode: " << op << std::endl;

      do_op:
        if (regs.pc == stop_pc) {
            (*orig_regs) = regs;
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
    regs.sp++->setUndefined();
END_CASE(JSOP_UNDEFINED)

BEGIN_CASE(JSOP_BINDGNAME)
#ifdef TRACEIT
        printf("TRACE: JSOP_BINDGNAME\n");
#endif
	/* CAL Determine which name belongs to which global variable (???)
	 * A global object or THE global object? */
    regs.sp++->setObject(regs.fp()->global()); //assertSameCompartment(cx, regs.sp[-1]);
	// PUSH_OBJECT(regs.fp()->global());
END_CASE(JSOP_BINDGNAME)

BEGIN_CASE(JSOP_GETGNAME)
BEGIN_CASE(JSOP_CALLGNAME)
BEGIN_CASE(JSOP_NAME)
BEGIN_CASE(JSOP_CALLNAME)
{
#ifdef TRACEIT
    printf("TRACE: JSOP_{GETGNAME,CALLGNAME,NAME,CALLNAME}\n");
#endif
    /* CAL
	for (std::map<jsbytecode*, int>::iterator it = visited_pc.begin(); it != visited_pc.end(); it++){
        printf("PC: %u\tCount: %d\n", it->first, it->second);
    }
    */
    RootedValue &rval = rootValue0;

    // if (!NameOperation(cx, script, regs.pc, rval.address()))
    //     goto error;

    *(regs.sp++) = rval; //assertSameCompartment(cx, regs.sp[-1]); } while (0)
    // TypeScript::Monitor(cx, script, regs.pc, rval);
}
END_CASE(JSOP_NAME)

BEGIN_CASE(JSOP_ONE)
#ifdef TRACEIT
    printf("TRACE: JSOP_ONE\n");
#endif
    regs.sp++->setInt32(1);
END_CASE(JSOP_ONE)

BEGIN_CASE(JSOP_ADD)
{
#ifdef TRACEIT
    printf("TRACE: JSOP_ADD\n");
#endif
    Value lval = regs.sp[-2];
    Value rval = regs.sp[-1];
    if (!AddOperation(cx, lval, rval, &regs.sp[-2]))
        goto error;
    regs.sp--;
}
END_CASE(JSOP_ADD)

BEGIN_CASE(JSOP_SETGNAME)
BEGIN_CASE(JSOP_SETNAME)
{
#ifdef TRACEIT
    printf("TRACE JSOP_{SETGNAME,SETNAME}\n");
#endif
    RootedObject &scope = rootObject0;
    scope = &regs.sp[-2].toObject();

    HandleValue value = HandleValue::fromMarkedLocation(&regs.sp[-1]);

    // if (!SetNameOperation(cx, script, regs.pc, scope, value))
    //     goto error;

    regs.sp[-2] = regs.sp[-1];
    regs.sp--;
}
END_CASE(JSOP_SETNAME)

BEGIN_CASE(JSOP_POP)
#ifdef TRACEIT
    printf("TRACE: JSOP_POP\n");
#endif
    regs.sp--;
END_CASE(JSOP_POP)

BEGIN_CASE(JSOP_SETELEM)
{
#ifdef TRACEIT
    printf("TRACE: JSOP_SETELEM\n");
#endif
    RootedObject &obj = rootObject0;
    // FETCH_OBJECT(cx, -3, obj);
    // vvvvv
    HandleValue val = HandleValue::fromMarkedLocation(&regs.sp[-3]);
    obj = ToObject(cx, (val));
    if (!obj) {
        cout << "Failed ToObject\n";
        goto error;
    }
    // ^^^^^
    RootedId &id = rootId0;
    // FETCH_ELEMENT_ID(obj, -2, id);
    // vvvvvv
    const Value &idval_ = regs.sp[-2];                                     
    if (!ValueToId(cx, obj, idval_, id.address())) {
        dout << "Failed ValueToID." << endl;
        goto error;  
    }
    // ^^^^^^
    Value &value = regs.sp[-1];
    // if (!SetObjectElementOperation(cx, obj, id, value, script->strictModeCode))
    //     goto error;
    regs.sp[-3] = value;
    regs.sp -= 2;
}
END_CASE(JSOP_SETELEM)

BEGIN_CASE(JSOP_NEW)
BEGIN_CASE(JSOP_CALL)
BEGIN_CASE(JSOP_FUNCALL)
{
    std::cout << "Functions not supported in loop. Opcode: " << op << std::endl;
    goto error;
}
END_CASE(JSOP_NEW)
    
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
cout << "Bailing out\n";
exit(1);

}
