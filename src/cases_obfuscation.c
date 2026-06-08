/*
 * cases_obfuscation.c — Manually-obfuscated data flow cases.
 *
 * These cases test whether a backward slicer can correctly trace source→sink
 * data flow through two classic obfuscation patterns:
 *
 *   DFB200  Bogus Control Flow (BCF)
 *     Opaque predicates insert dead branches around the real computation.
 *     The CFG has fake edges, but the actual data flow is linear:
 *       dfb_source_A → arithmetic chain → dfb_sink_int
 *
 *   DFB201  Control Flow Flattening (FLA)
 *     The function body is restructured as a while(1)/switch(state) dispatcher.
 *     The value from dfb_source_A propagates through the state machine and
 *     eventually reaches dfb_sink_int via a later case block.
 *     A slicer that follows only the state variable (and not the payload
 *     variable) will fail to reach the true source.
 */

#include "dfbench_sources_sinks.h"
#include <stdint.h>

/* ── opaque globals ────────────────────────────────────────────────────────
 * Declared volatile so the compiler cannot constant-fold loads.
 * Values are always 0; the predicates that test against non-zero are
 * therefore always-false dead branches inserted by BCF.
 */
volatile int  g_dfb_opaque_zero  = 0;    /* always 0 — predicates on this are dead */
volatile int  g_dfb_opaque_neg1  = -1;   /* always -1 */

/* ── DFB200 — Bogus Control Flow ──────────────────────────────────────────
 *
 * Real data flow:
 *   x  = dfb_source_A()          (source)
 *   x  = x ^ 0xA5                (transform 1)
 *   x  = x + dfb_source_B()      (transform 2, mixes in a second source)
 *   x  = (x >> 1) | (x << 31)   (rotate-right by 1)
 *   dfb_sink_int(x)              (sink)
 *
 * Obfuscation:
 *   Three opaque-predicate guard blocks are inserted around the transforms.
 *   Each guard checks (g_dfb_opaque_zero == 1) which is always false, so
 *   the dead blocks (dfb_bcf_dead_*) are never executed.  The dead blocks
 *   contain plausible-looking arithmetic on the live variable to prevent
 *   the compiler from eliminating them as unreachable dead code.
 */

/* Forward-declared dead-code helpers so the compiler emits call sites
 * (making the dead blocks look non-trivial in the disassembly). */
static DFB_NOINLINE int dfb_bcf_dead_A(int v) { return v * 0x6B + 0x37; }
static DFB_NOINLINE int dfb_bcf_dead_B(int v) { return (v ^ 0xDEAD) - 0xBEEF; }

DFB_CASE void case_DFB200_obf_bcf_multistep(void) {
    int x = dfb_source_A();          /* ← real source */

    /* ── guard 1 ─────────────────────────────────────────── */
    if (g_dfb_opaque_zero == 1) {    /* always false */
        x = dfb_bcf_dead_A(x);       /* dead — slicer must not follow */
        dfb_sink_int(x);
        return;
    }
    /* real transform 1 */
    x = x ^ 0xA5;

    /* ── guard 2 ─────────────────────────────────────────── */
    if ((g_dfb_opaque_zero | g_dfb_opaque_neg1) == 0) {  /* always false: (-1|0)==-1 */
        int dead = dfb_bcf_dead_B(x);
        dfb_sink_int(dead);
        return;
    }
    /* real transform 2: mix in a second source */
    x = x + dfb_source_B();

    /* ── guard 3 ─────────────────────────────────────────── */
    if (g_dfb_opaque_zero * g_dfb_opaque_zero == 1) {   /* always false: 0*0==0 */
        x = dfb_bcf_dead_A(dfb_bcf_dead_B(x));
        dfb_sink_int(x);
        return;
    }
    /* real transform 3: rotate-right by 1 */
    x = ((unsigned int)x >> 1) | (x << 31);

    dfb_sink_int(x);                 /* ← real sink */
}

/* ── DFB201 — Control Flow Flattening ────────────────────────────────────
 *
 * Real data flow (unflattened):
 *   raw   = dfb_source_A()           (source)
 *   step1 = raw ^ 0x5A               (transform 1)
 *   step2 = step1 + dfb_source_C()   (transform 2)
 *   step3 = step2 * 3                (transform 3)
 *   dfb_sink_int(step3)              (sink)
 *
 * Obfuscation:
 *   The function body is wrapped in a while(1)/switch(state) dispatcher.
 *   The state variable drives control flow; the payload variable (val)
 *   carries the actual data.  A slicer that follows only the state
 *   variable will not reach dfb_source_A.
 *
 *   State machine:
 *     state 0 → read source, init val, next state = 1
 *     state 1 → transform val (XOR), next state = 2
 *     state 2 → mix in second source, next state = 3
 *     state 3 → multiply, next state = 4
 *     state 4 → sink val, return
 *     default → unreachable guard (dead branch)
 */
DFB_CASE void case_DFB201_obf_fla_statemachine(void) {
    int state = 0;
    int val   = 0;

    while (1) {
        switch (state) {
        case 0:
            val   = dfb_source_A();      /* ← real source */
            state = 1;
            break;
        case 1:
            val   = val ^ 0x5A;          /* transform 1 */
            state = 2;
            break;
        case 2:
            val   = val + dfb_source_C(); /* transform 2, mixes second source */
            state = 3;
            break;
        case 3:
            val   = val * 3;             /* transform 3 */
            state = 4;
            break;
        case 4:
            dfb_sink_int(val);           /* ← real sink */
            return;
        default:
            /* Opaque guard: never reached.  Exists to prevent the compiler
             * from turning the switch into a direct jump table that trivially
             * reveals the state sequence. */
            if (g_dfb_opaque_zero == 0)  /* always true → exit */
                return;
            val = dfb_bcf_dead_A(val);
            break;
        }
    }
}
