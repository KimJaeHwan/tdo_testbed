/*
 * cases_offset.c — Offset-level tracking test cases (DFB034-DFB035, DFB047-DFB049)
 *
 * These cases focus on address-computation PCode ops:
 *   PTRSUB / PTRADD / INT_ADD / INT_SUB / SUBPIECE
 * and the scenarios where naive offset matching breaks down.
 *
 * Each case is annotated with:
 *   - The High PCode pattern being exercised
 *   - The expected slicer behavior (PASS / FAIL / UNCERTAIN)
 *   - The reason it differs from the existing struct/array/heap cases
 */

#include <stdint.h>
#include "dfbench_sources_sinks.h"

/* -----------------------------------------------------------------------
 * DFB034: bitfield_access
 *
 * Bitfield read/write generates SUBPIECE + INT_RIGHT + INT_AND at High PCode.
 * The slicer must track sub-byte bit ranges to distinguish bp.flags from bp.value.
 * Without bit-range analysis, the slicer sees a single byte STORE that merges
 * both sources via a read-modify-write, and cannot isolate the contributing bits.
 *
 * High PCode (write bp.value):
 *   tmp      = LOAD(bp_addr)                      ; existing byte
 *   shifted  = INT_LEFT(source_B_val, 4)
 *   masked   = INT_AND(shifted, 0xF0)
 *   cleared  = INT_AND(tmp, 0x0F)                 ; preserve flags bits
 *   new_byte = INT_OR(cleared, masked)
 *   STORE(bp_addr, new_byte)
 *
 * High PCode (read bp.value):
 *   byte   = LOAD(bp_addr)
 *   result = INT_AND(INT_RIGHT(byte, 4), 0xF)
 *
 * Expected slicer behavior: FAIL — cannot isolate source_B from source_A
 *   through the read-modify-write merge at the byte level.
 * ----------------------------------------------------------------------- */
typedef struct DFBBitPacked {
    unsigned flags : 4;   /* bits [0..3] */
    unsigned value : 4;   /* bits [4..7] */
} DFBBitPacked;

DFB_CASE void case_DFB034_bitfield_access(void) {
    DFBBitPacked bp;
    bp.flags = (unsigned)dfb_source_A() & 0xfU;   /* bits 0-3 */
    bp.value = (unsigned)dfb_source_B() & 0xfU;   /* bits 4-7 */
    dfb_sink_int((int)bp.value);                   /* ground truth: source_B only */
}

/* -----------------------------------------------------------------------
 * DFB035: bitfield_access_zeroinit
 *
 * Identical to DFB034 except bp is zero-initialized (= {0}).
 * Comparison case: tests whether zero-initialization changes Ghidra's High PCode
 * representation of the bitfield read-modify-write sequence.
 *
 * DFB034 (uninitialized):
 *   CALL dfb_source_A out=null  +  INDIRECT out=stack:0x-38 in=[stack:0x-38, seqno]
 *   CALL dfb_source_B out=null  +  INDIRECT out=stack:0x-38 in=[stack:0x-38, seqno]
 *   CALL dfb_sink_int            in=[..., stack:0x-38]
 *   -> Ghidra drops source return values; sink receives uninitialized stack varnode.
 *
 * DFB035 (= {0} initialized):
 *   Hypothesis: the zero initializer anchors the stack varnode SSA chain.
 *   Ghidra may promote the source return values into the INDIRECT output or
 *   reconstruct the mask/shift arithmetic, changing the PCode shape.
 *   -> Actual behavior to be confirmed by pcode_dumper.py run.
 *
 * Expected slicer behavior: UNCERTAIN — depends on whether zero-init causes
 *   Ghidra to surface the CALL return varnodes in the data-flow chain.
 * ----------------------------------------------------------------------- */
DFB_CASE void case_DFB035_bitfield_access_zeroinit(void) {
    DFBBitPacked bp = {0};                         /* explicit zero-initialization */
    bp.flags = (unsigned)dfb_source_A() & 0xfU;   /* bits 0-3 */
    bp.value = (unsigned)dfb_source_B() & 0xfU;   /* bits 4-7 */
    dfb_sink_int((int)bp.value);                   /* ground truth: source_B only */
}

/* -----------------------------------------------------------------------
 * DFB047: struct_padding_offset
 *
 * Compiler alignment rules insert 3 bytes of padding after `tag` (char),
 * placing `value` (int) at absolute offset +4, not +1.
 * Ghidra's decompiler computes the correct ABI offset at analysis time and
 * emits PTRSUB(s_addr, 4) for `s.value`. The slicer must use this
 * decompiler-provided offset rather than a naive field-index calculation.
 *
 * High PCode (write s.value):
 *   STORE(PTRSUB(s_addr, 4), source_B_val)
 *
 * High PCode (read s.value):
 *   result = LOAD(PTRSUB(s_addr, 4))
 *
 * Expected slicer behavior: PASS — offsets match after Ghidra resolves padding.
 * Validates that the slicer correctly uses decompiler-computed ABI offsets.
 * ----------------------------------------------------------------------- */
typedef struct DFBPaddedS {
    char tag;     /* offset 0                    */
                  /* 3 bytes implicit padding     */
    int  value;   /* offset 4 (alignment to int) */
} DFBPaddedS;

DFB_CASE void case_DFB047_struct_padding_offset(void) {
    DFBPaddedS s;
    s.tag   = (char)dfb_source_A();   /* absolute offset 0 */
    s.value = dfb_source_B();         /* absolute offset 4 (past padding) */
    dfb_sink_int(s.value);            /* ground truth: source_B */
}

/* -----------------------------------------------------------------------
 * DFB048: cast_range_overlap
 *
 * A 4-byte int write via pointer cast covers bytes [buf+4 .. buf+7].
 * The sink reads byte [buf+6], which falls strictly inside the written range.
 * A correct slicer needs range-intersection analysis:
 *   store_offset(4) + store_size(4) > read_offset(6) >= store_offset(4)
 *   => [4..7] intersects [6..6] => data dependency exists.
 *
 * High PCode (write):
 *   STORE(PTRSUB(buf_addr, 4), source_A_val)   ; size=4
 *
 * High PCode (read):
 *   result = LOAD(PTRSUB(buf_addr, 6))          ; size=1
 *
 * Current slicer: exact offset matching — 4 != 6 => dependency not detected.
 * Expected slicer behavior: FAIL (needs range-based alias analysis).
 * ----------------------------------------------------------------------- */
DFB_CASE void case_DFB048_cast_range_overlap(void) {
    char buf[16] = {0};
    int *iptr = (int *)(buf + 4);
    *iptr = dfb_source_A();                           /* STORE 4 bytes at buf+4 (covers [4..7]) */
    dfb_sink_int((int)(unsigned char)buf[6]);          /* LOAD 1 byte at buf+6  (inside [4..7]) */
}

/* -----------------------------------------------------------------------
 * DFB049: negative_offset_arithmetic
 *
 * A pointer is advanced forward by 30, then stepped backward by 10,
 * landing at buf+20. With constant operands Ghidra's decompiler will
 * likely constant-fold: INT_ADD(buf, 30) - 10  =>  INT_ADD(buf, 20).
 * If folded, the slicer sees a plain +20 offset and should PASS.
 * If NOT folded, the slicer must evaluate:
 *   INT_ADD(INT_ADD(buf_addr, 30), INT_2COMP(10))  =>  buf+20
 * i.e., combine two addends including a negated constant (INT_NEGATE / INT_2COMP).
 *
 * High PCode (if not folded):
 *   end     = INT_ADD(buf_addr, 30)
 *   target  = INT_ADD(end, COPY(-10))      ; or INT_SUB(end, 10)
 *   STORE(target, source_A_val)
 *
 * Expected slicer behavior: UNCERTAIN — depends on Ghidra constant folding.
 *   If folded: PASS.  If not folded: FAIL (INT_SUB/NEGATE combining needed).
 * ----------------------------------------------------------------------- */
DFB_CASE void case_DFB049_negative_offset_arithmetic(void) {
    char buf[32] = {0};
    char *end = buf + 30;
    *(end - 10) = (char)dfb_source_A();        /* writes to buf[20] */
    dfb_sink_int((int)(unsigned char)buf[20]); /* reads  from buf[20] */
}
