#pragma once

extern void divide_zero_handler(void);
extern void debug_handler(void);
extern void breakpoint_handler(void);
extern void overflow_handler(void);
extern void bound_range_handler(void);
extern void invalid_opcode_handler(void);
extern void not_available_handler(void);
extern void double_fault_handler(void);
extern void invalid_tss_handler(void);
extern void seg_np_handler(void);
extern void stack_seg_fault_handler(void);
extern void general_handler(void);
extern void page_fault_handler(void);
extern void x86_fp_handler(void);
extern void alignment_check_handler(void);
extern void machine_check_handler(void);
extern void SIMD_fp_handler(void);
extern void virtalization_handler(void);
extern void security_handler(void);

