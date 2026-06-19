/*
 * cases_recursion.c — Recursion pattern test cases (DFB061-DFB065)
 *
 * DFB060 (tail recursion, loop-equivalent) is in cases_interproc.c.
 * These cases cover recursion patterns that require genuine fixed-point
 * reasoning beyond what a simple loop model handles:
 *
 *   DFB061 — non-tail recursion (self-referential summary)
 *   DFB062 — mutual recursion (SCC ≥ 2, joint fixed-point)
 *   DFB063 — side-effect recursion (global accumulation)
 *   DFB064 — indirect recursion (function pointer self-call)
 *   DFB065 — tree recursion (two recursive call sites per frame)
 */

#include "dfbench_sources_sinks.h"

/* -----------------------------------------------------------------------
 * DFB061 — non_tail_recursion
 *
 * The recursive call result is combined with seed before returning,
 * so the exit depends on itself (self-referential summary).
 * A correct slicer must reach fixed-point on the self-referential edge
 * rather than looping indefinitely.
 *
 * ground truth: source_A (seed). source_C controls depth only.
 * ----------------------------------------------------------------------- */
DFB_HELPER int dfb_rec_combine(int seed, int n) {
    if (n <= 1) return seed;
    return dfb_rec_combine(seed, n - 1) + seed;
}

DFB_CASE void case_DFB061_non_tail_recursion(void) {
    int a = dfb_source_A();
    int n = (dfb_source_C() & 3) + 1;
    dfb_sink_int(dfb_rec_combine(a, n));   /* ground truth: source_A */
}

/* -----------------------------------------------------------------------
 * DFB062 — mutual_recursion (SCC size 2)
 *
 * dfb_mr_even and dfb_mr_odd form a call-graph SCC of size 2.
 * A correct slicer must treat the entire SCC as a co-recursive unit
 * and compute a joint fixed-point (§18.1 / §19.2).
 *
 * ground truth: source_A. source_C controls parity / depth only.
 * ----------------------------------------------------------------------- */
DFB_HELPER int dfb_mr_odd(int x, int n);

DFB_HELPER int dfb_mr_even(int x, int n) {
    return (n == 0) ? x : dfb_mr_odd(x, n - 1);
}

DFB_HELPER int dfb_mr_odd(int x, int n) {
    return (n == 0) ? x : dfb_mr_even(x, n - 1);
}

DFB_CASE void case_DFB062_mutual_recursion(void) {
    int a = dfb_source_A();
    int n = (dfb_source_C() & 3) + 1;
    dfb_sink_int(dfb_mr_even(a, n));   /* ground truth: source_A */
}

/* -----------------------------------------------------------------------
 * DFB063 — recursion_global_effect
 *
 * Each recursive frame writes to a global accumulator (side effect).
 * Requires fixed-point on memory/global effects, not just registers.
 * If recursive summaries are unavailable the slicer should emit
 * recursive_summary_unavailable and stop soundly.
 *
 * ground truth: source_A. source_C controls depth only.
 * ----------------------------------------------------------------------- */
static volatile int g_dfb_rec_acc = 0;

DFB_HELPER void dfb_rec_accumulate(int x, int n) {
    if (n <= 0) return;
    g_dfb_rec_acc += x;
    dfb_rec_accumulate(x, n - 1);
}

DFB_CASE void case_DFB063_recursion_global_effect(void) {
    int a = dfb_source_A();
    g_dfb_rec_acc = 0;
    dfb_rec_accumulate(a, (dfb_source_C() & 3) + 1);
    dfb_sink_int(g_dfb_rec_acc);   /* ground truth: source_A */
}

/* -----------------------------------------------------------------------
 * DFB064 — indirect_recursion
 *
 * The function calls itself via a void* function-pointer argument,
 * creating a CALLIND edge that forms a self-loop SCC.
 * Requires SCC detection across indirect edges (CALLIND resolved
 * at least partially to identify the back-edge).
 *
 * ground truth: source_A.
 * ----------------------------------------------------------------------- */
typedef int (*dfb_rec_fn_t)(void *self, int x, int n);

DFB_HELPER int dfb_indirect_rec(void *self, int x, int n) {
    dfb_rec_fn_t f = (dfb_rec_fn_t)self;
    if (n <= 0) return x;
    return f(self, x, n - 1);
}

DFB_CASE void case_DFB064_indirect_recursion(void) {
    int a = dfb_source_A();
    dfb_sink_int(dfb_indirect_rec((void *)dfb_indirect_rec, a, 3));
}

/* -----------------------------------------------------------------------
 * DFB065 — tree_recursion
 *
 * Two recursive call sites per frame (fibonacci-style).
 * source_A is injected at the base case; both sub-results are joined
 * by addition before returning upward. The slicer must join results
 * from both recursive branches.
 *
 * ground truth: source_A (base case). source_C controls depth only.
 * ----------------------------------------------------------------------- */
DFB_HELPER int dfb_tree_rec(int n) {
    if (n <= 1) return dfb_source_A();   /* source injected at base case */
    return dfb_tree_rec(n - 1) + dfb_tree_rec(n - 2);
}

DFB_CASE void case_DFB065_tree_recursion(void) {
    int n = (dfb_source_C() & 3) + 2;
    dfb_sink_int(dfb_tree_rec(n));   /* ground truth: source_A */
}
