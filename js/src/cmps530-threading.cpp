#include <iostream>
#include <thread>

//#define DOUT2

#ifdef DOUT 
#define dout cout << __FILE__ << "(" << __LINE__ << ") DEBUG: "
#else
#define dout 0 && std::cout
#endif /* DEBUG */

#ifdef DOUT
#define dprintf(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#else
#define dprinf(fmt, ...) 0
#endif

using namespace std;

#define PUSH_COPY(v)             do { *regs.sp++ = v; assertSameCompartment(cx, regs.sp[-1]); } while (0)
#define PUSH_COPY_SKIP_CHECK(v)  *regs.sp++ = v
#define PUSH_NULL()              regs.sp++->setNull()
#define PUSH_UNDEFINED()         regs.sp++->setUndefined()
#define PUSH_BOOLEAN(b)          regs.sp++->setBoolean(b)
#define PUSH_DOUBLE(d)           regs.sp++->setDouble(d)
#define PUSH_INT32(i)            regs.sp++->setInt32(i)
#define PUSH_STRING(s)           do { regs.sp++->setString(s); assertSameCompartment(cx, regs.sp[-1]); } while (0)
#define PUSH_OBJECT(obj)         do { regs.sp++->setObject(obj); assertSameCompartment(cx, regs.sp[-1]); } while (0)
#define PUSH_OBJECT_OR_NULL(obj) do { regs.sp++->setObjectOrNull(obj); assertSameCompartment(cx, regs.sp[-1]); } while (0)
#define PUSH_HOLE()              regs.sp++->setMagic(JS_ARRAY_HOLE)
#define POP_COPY_TO(v)           v = *--regs.sp
#define POP_RETURN_VALUE()       regs.fp()->setReturnValue(*--regs.sp)

# define BEGIN_CASE2(OP)     case OP: 
//# define BEGIN_CASE2(OP)     case OP: printf(#OP); printf("\n");
JS_NEVER_INLINE void
ThreadInterpret(int id, jsbytecode* start_pc, JSContext *cx, FrameRegs * orig_regs, int offset, jsbytecode *original_pc, jsbytecode *stop_pc, 
        RootedValue *rootValue0, RootedValue *rootValue1, RootedObject *rootObject0, RootedObject *rootObject1, RootedObject *rootObject2, RootedId *rootId0)
{
//    JSContext
    // sleep(1);
    Rooted<JSScript*> script(cx);
//    SET_SCRIPT(regs.fp()->script());
    FrameRegs regs = cx->regs();


    //FrameRegs regs = *orig_regs;
    regs.pc = start_pc;
    // Copy stack.
    Value temp = *(regs.sp);
    regs.sp = &temp;
    dprintf("[New Thread] ID: %d, Start: %d, Stop: %d\n", id, regs.pc - original_pc, stop_pc - original_pc);
    // dout << "Thread " << id << ", PC: " << regs.pc - original_pc << ", Stop: " << stop_pc - original_pc << endl;
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
        op = (JSOp) *(regs.pc); // Get the opcode
        dprintf("[Thread %d] PC: %d  Opcode: %d\n", id, offset, op);
        // dout << "PC:" << offset << " Opcode: " << op << std::endl;

      do_op:
        if (regs.pc == stop_pc) {
//            (*orig_regs) = regs;
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

BEGIN_CASE2(JSOP_LOOPHEAD)
#ifdef TRACEIT
printf("TRACE: JSOP_LOOPHEAD\n");
#endif
END_EMPTY_CASES
BEGIN_CASE2(JSOP_LOOPENTRY)
#ifdef TRACEIT
printf("TRACE: JSOP_LOOPENTRY\n");
#endif
END_EMPTY_CASES

BEGIN_CASE2(JSOP_LABEL)
END_CASE(JSOP_LABEL)

check_backedge:
{
    CHECK_BRANCH();
    if (op != JSOP_LOOPHEAD)
        DO_OP();

    DO_OP();
}

/* ADD_EMPTY_CASE is not used here as JSOP_LINENO_LENGTH == 3. */
BEGIN_CASE2(JSOP_LINENO)
END_CASE(JSOP_LINENO)

BEGIN_CASE2(JSOP_UNDEFINED)
#ifdef TRACEIT
    printf("TRACE: JSOP_UNDEFINED\n");
#endif
    regs.sp++->setUndefined();
END_CASE(JSOP_UNDEFINED)

BEGIN_CASE2(JSOP_BINDGNAME)
#ifdef TRACEIT
        printf("TRACE: JSOP_BINDGNAME\n");
#endif
	/* CAL Determine which name belongs to which global variable (???)
	 * A global object or THE global object? */
    regs.sp++->setObject(regs.fp()->global()); //assertSameCompartment(cx, regs.sp[-1]);
	// PUSH_OBJECT(regs.fp()->global());
END_CASE(JSOP_BINDGNAME)

BEGIN_CASE2(JSOP_GETGNAME)
BEGIN_CASE2(JSOP_CALLGNAME)
BEGIN_CASE2(JSOP_NAME)
BEGIN_CASE2(JSOP_CALLNAME)
{
#ifdef TRACEIT
    printf("TRACE: JSOP_{GETGNAME,CALLGNAME,NAME,CALLNAME}\n");
#endif
    /* CAL
	for (std::map<jsbytecode*, int>::iterator it = visited_pc.begin(); it != visited_pc.end(); it++){
        printf("PC: %u\tCount: %d\n", it->first, it->second);
    }
    */
    RootedValue &rval = *rootValue0;

    // if (!NameOperation(cx, script, regs.pc, rval.address()))
    //     goto error;

    *(regs.sp++) = rval; //assertSameCompartment(cx, regs.sp[-1]); } while (0)
    // TypeScript::Monitor(cx, script, regs.pc, rval);
}
END_CASE(JSOP_NAME)

BEGIN_CASE2(JSOP_ONE)
#ifdef TRACEIT
    printf("TRACE: JSOP_ONE\n");
#endif
    regs.sp++->setInt32(1);
END_CASE(JSOP_ONE)

BEGIN_CASE2(JSOP_ADD)
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

BEGIN_CASE2(JSOP_SETGNAME)
BEGIN_CASE2(JSOP_SETNAME)
{
#ifdef TRACEIT
    printf("TRACE JSOP_{SETGNAME,SETNAME}\n");
#endif
    RootedObject &scope = *rootObject0;
    scope = &regs.sp[-2].toObject();

    HandleValue value = HandleValue::fromMarkedLocation(&regs.sp[-1]);

    // if (!SetNameOperation(cx, script, regs.pc, scope, value))
    //     goto error;

    regs.sp[-2] = regs.sp[-1];
    regs.sp--;
}
END_CASE(JSOP_SETNAME)

BEGIN_CASE2(JSOP_POP)
#ifdef TRACEIT
    printf("TRACE: JSOP_POP\n");
#endif
    regs.sp--;
END_CASE(JSOP_POP)

BEGIN_CASE2(JSOP_SETELEM)
{
#ifdef TRACEIT
    printf("TRACE: JSOP_SETELEM\n");
#endif
    RootedObject &obj = *rootObject0;
    // FETCH_OBJECT(cx, -3, obj);
    // vvvvv
    HandleValue val = HandleValue::fromMarkedLocation(&regs.sp[-3]);
    obj = ToObject(cx, (val));
    if (!obj) {
        cout << "Failed ToObject\n";
        goto error;
    }
    // ^^^^^
    RootedId &id = *rootId0;
    // FETCH_ELEMENT_ID(obj, -2, id);
    // vvvvvv
    const Value &idval_ = regs.sp[-2];                                     
    if (!ValueToId(cx, obj, idval_, id.address())) {
        dout << "Failed ValueToID." << endl;
        goto error;  
    }
    // ^^^^^^
    Value &value = regs.sp[-1];
    bool doexit = false;

    if (!SetObjectElementOperationThread(cx, obj, id, value, script->strictModeCode))
        goto error;

    regs.sp[-3] = value;
    regs.sp -= 2;
}
END_CASE(JSOP_SETELEM)

BEGIN_CASE2(JSOP_NEW)
BEGIN_CASE2(JSOP_CALL)
BEGIN_CASE2(JSOP_FUNCALL)
{
    std::cout << "Functions not supported in loop. " << std::endl << "Opcode: " << op  << " PC: " << regs.pc - original_pc << std::endl;
    goto error;
}
END_CASE(JSOP_NEW)
    
BEGIN_CASE2(JSOP_ZERO)
#ifdef TRACEIT
    printf("TRACE: JSOP_ZERO\n");
#endif
    PUSH_INT32(0);
END_CASE(JSOP_ZERO)

BEGIN_CASE2(JSOP_DIV)
{
    RootedValue &lval = *rootValue0; 
    RootedValue &rval = *rootValue1;
    lval = regs.sp[-2];
    rval = regs.sp[-1];
    if (!DivOperation(cx, lval, rval, &regs.sp[-2]))
        goto error;
    regs.sp--;
}
END_CASE(JSOP_DIV)

BEGIN_CASE2(JSOP_INT8)
    PUSH_INT32(GET_INT8(regs.pc));
END_CASE(JSOP_INT8)

BEGIN_CASE2(JSOP_MUL)
{
    RootedValue &lval = *rootValue0, &rval = *rootValue1;
    lval = regs.sp[-2];
    rval = regs.sp[-1];
    if (!MulOperation(cx, lval, rval, &regs.sp[-2]))
        goto error;
    regs.sp--;
}
END_CASE(JSOP_MUL)

BEGIN_CASE2(JSOP_GETELEM)
BEGIN_CASE2(JSOP_CALLELEM)
{
    std::cout << "JSOP_GETELEM AND JSOP_CALLELEM currently giving problems\n";
    goto error;
#ifdef TRACEIT
    printf("TRACE: JSOP_{GETELEM,CALLELEM)\n");
#endif
    MutableHandleValue lval = MutableHandleValue::fromMarkedLocation(&regs.sp[-2]);
    HandleValue rval = HandleValue::fromMarkedLocation(&regs.sp[-1]);

    MutableHandleValue res = MutableHandleValue::fromMarkedLocation(&regs.sp[-2]);
    bool result = GetElementOperation(cx, op, lval, rval, res);
    if (!result)
        std::cout << "Thread failed to GET or CALL ELEM. PC: " << offset << std::endl;
        goto error;
    // TypeScript::Monitor(cx, script, regs.pc, res);
    regs.sp--;
}
END_CASE(JSOP_GETELEM)


BEGIN_CASE2(JSOP_DUP)
{
#ifdef TRACEIT
    printf("TRACE: JSOP_DUP\n");
#endif
    //JS_ASSERT(regs.stackDepth() >= 1);
    const Value &rref = regs.sp[-1];
    PUSH_COPY(rref);
}
END_CASE(JSOP_DUP)

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
