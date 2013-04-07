#include <iostream>

#include "jsprvtd.h"
#include "jsscript.h"
// For SrcNoteType
#include "frontend/BytecodeEmitter.h"

using namespace js;

typedef enum LoopType {
    JSLOOP_FOR,
    JSLOOP_WHILE,
    JSLOOP_FORIN
} LoopType;

struct Loop {
    jsbytecode *loophead;   // Loop body head
    jsbytecode *loopentry;  // condition check, update already done 
    jsbytecode *update;     // condition update.
                                // for loop only
                                // while loop: unknown
                                // for-in loop: nextiter immediately after loophead
    LoopType looptype;
};

char *SrcNoteTypeNames[] {
    (char *)"SRC_NULL",        
    (char *)"SRC_IF,BREAK,INITPROP",        
    (char *)"SRC_GENEXP",        
    (char *)"SRC_IF_ELSE,FOR_IN",        
    (char *)"SRC_FOR",        
    (char *)"SRC_WHILE",        
    (char *)"SRC_CONTINUE",        
    (char *)"SRC_DECL,DESTRUCT",        
    (char *)"SRC_PCDELTA,GROUPASSIGN,DESTRUCTLET",        
    (char *)"SRC_ASSIGNOP",        
    (char *)"SRC_COND",        
    (char *)"SRC_BRACE",       
    (char *)"SRC_HIDDEN",       
    (char *)"SRC_PCBASE",       
    (char *)"SRC_LABEL",       
    (char *)"SRC_LABELBRACE",       
    (char *)"SRC_ENDBRACE",       
    (char *)"SRC_BREAK2LABEL",       
    (char *)"SRC_CONT2LABEL",       
    (char *)"SRC_SWITCH",       
    (char *)"SRC_SWITCHBREAK",       
    (char *)"SRC_FUNCDEF",       
    (char *)"SRC_CATCH",       
    (char *)"SRC_COLSPAN",       
    (char *)"SRC_NEWLINE",       
    (char *)"SRC_SETLINE",       
    (char *)"SRC_XDELTA"
};

SrcNoteType GetInstructionType(jssrcnote *note, int offset) {

    SrcNoteType sn_type = (SrcNoteType) SN_TYPE(note + offset);
    //std::cout << "PC: " << offset << " NoteType: " << SrcNoteTypeNames[sn_type] << "\n";
    std::cout << "PC: " << offset << " NoteType: " << js_SrcNoteSpec[sn_type].name << "\n";

    return sn_type;
} 

class Notes {
    JSContext *cx;
    JSScript *script;
    //Vector<int> list;

  public:
    Notes(JSContext *c, JSScript *s) : cx(c), script(s)
    {
        //"ofs", "line", "pc", "delta", "desc", "args");
        unsigned offset = 0;
        unsigned colspan = 0;
        unsigned lineno = script->lineno;
        jssrcnote *notes = script->notes();
        unsigned switchTableEnd = 0, switchTableStart = 0;
        for (jssrcnote *sn = notes; !SN_IS_TERMINATOR(sn); sn = SN_NEXT(sn)) {
            unsigned delta = SN_DELTA(sn);
            offset += delta;
            SrcNoteType type = (SrcNoteType) SN_TYPE(sn);
            const char *name = js_SrcNoteSpec[type].name;
            if (type == SRC_LABEL) {
                /* Check if the source note is for a switch case. */
                if (switchTableStart <= offset && offset < switchTableEnd)
                    name = "case";
            }
            //Sprint(sp, "%3u: %4u %5u [%4u] %-8s", unsigned(sn - notes), lineno, offset, delta, name);
            switch (type) {
              case SRC_COLSPAN:
                colspan = js_GetSrcNoteOffset(sn, 0);
                if (colspan >= SN_COLSPAN_DOMAIN / 2)
                    colspan -= SN_COLSPAN_DOMAIN;
                //Sprint(sp, "%d", colspan);
                break;
              case SRC_SETLINE:
                lineno = js_GetSrcNoteOffset(sn, 0);
                //Sprint(sp, " lineno %u", lineno);
                break;
              case SRC_NEWLINE:
                ++lineno;
                break;
              case SRC_FOR:
                //Sprint(sp, " cond %u update %u tail %u",
                       //unsigned(js_GetSrcNoteOffset(sn, 0)),
                       //unsigned(js_GetSrcNoteOffset(sn, 1)),
                       //unsigned(js_GetSrcNoteOffset(sn, 2)));
                break;
              case SRC_IF_ELSE:
                //Sprint(sp, " else %u elseif %u",
                       //unsigned(js_GetSrcNoteOffset(sn, 0)),
                       //unsigned(js_GetSrcNoteOffset(sn, 1)));
                break;
              case SRC_COND:
              case SRC_WHILE:
              case SRC_PCBASE:
              case SRC_PCDELTA:
              case SRC_DECL:
              case SRC_BRACE:
                //Sprint(sp, " offset %u", unsigned(js_GetSrcNoteOffset(sn, 0)));
                break;
              case SRC_LABEL:
              case SRC_LABELBRACE:
              case SRC_BREAK2LABEL:
              case SRC_CONT2LABEL: {
                uint32_t index = js_GetSrcNoteOffset(sn, 0);
                JSAtom *atom = script->getAtom(index);
                //Sprint(sp, " atom %u (", index);
                size_t len = PutEscapedString(NULL, 0, atom, '\0');
                //if (char *buf = sp->reserve(len)) {
                //    PutEscapedString(buf, len + 1, atom, 0);
                //    buf[len] = 0;
                // }
                //Sprint(sp, ")");
                break;
              }
              case SRC_FUNCDEF: {
                uint32_t index = js_GetSrcNoteOffset(sn, 0);
                JSObject *obj = script->getObject(index);
                JSFunction *fun = obj->toFunction();
                JSString *str = JS_DecompileFunction(cx, fun, JS_DONT_PRETTY_PRINT);
                JSAutoByteString bytes;
                //if (!str || !bytes.encode(cx, str))
                //    ReportException(cx);
                //Sprint(sp, " function %u (%s)", index, !!bytes ? bytes.ptr() : "N/A");
                break;
              }
              case SRC_SWITCH: {
                JSOp op = JSOp(script->code[offset]);
                if (op == JSOP_GOTO)
                    break;
                //Sprint(sp, " length %u", unsigned(js_GetSrcNoteOffset(sn, 0)));
                unsigned caseOff = (unsigned) js_GetSrcNoteOffset(sn, 1);
                if (caseOff)
                    //Sprint(sp, " first case offset %u", caseOff);
                //UpdateSwitchTableBounds(cx, script, offset, &switchTableStart, &switchTableEnd);
                break;
              }
              case SRC_CATCH:
                delta = (unsigned) js_GetSrcNoteOffset(sn, 0);
                //if (delta) {
                    //if (script->main()[offset] == JSOP_LEAVEBLOCK)
                        //Sprint(sp, " stack depth %u", delta);
                    //else
                        //Sprint(sp, " guard delta %u", delta);
                //}
                break;
              default:;
            }
            //Sprint(sp, "\n");
        }
    }
};

