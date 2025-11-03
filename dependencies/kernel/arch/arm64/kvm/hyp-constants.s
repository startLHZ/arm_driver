	.arch armv8-a
	.file	"hyp-constants.c"
// GNU C11 (Arm GNU Toolchain 12.2.Rel1 (Build arm-12.24)) version 12.2.1 20221205 (aarch64-none-linux-gnu)
//	compiled by GNU C version 4.8.5 20150623 (Red Hat 4.8.5-44), GMP version 6.2.1, MPFR version 3.1.6, MPC version 1.0.3, isl version isl-0.15-1-g835ea3a-GMP

// GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
// options passed: -mlittle-endian -mgeneral-regs-only -mabi=lp64 -mbranch-protection=pac-ret+leaf -mstack-protector-guard=sysreg -mstack-protector-guard-reg=sp_el0 -mstack-protector-guard-offset=1232 -O2 -std=gnu11 -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-delete-null-pointer-checks -fno-allow-store-data-races -fstack-protector-strong -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftrivial-auto-var-init=zero -fno-stack-clash-protection -fpatchable-function-entry=2 -fno-strict-overflow -fstack-check=no -fconserve-stack
	.text
	.section	.text.startup,"ax",@progbits
	.align	2
	.p2align 4,,11
	.global	main
	.type	main, %function
main:
	.section	__patchable_function_entries,"awo",@progbits,main
	.align	3
	.8byte	.LPFE1
	.section	.text.startup
.LPFE1:
	nop	
	nop	
	hint	25 // paciasp
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kvm/hyp/hyp-constants.c:8: 	DEFINE(STRUCT_HYP_PAGE_SIZE,	sizeof(struct hyp_page));
#APP
// 8 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kvm/hyp/hyp-constants.c" 1
	
.ascii "->STRUCT_HYP_PAGE_SIZE 4 sizeof(struct hyp_page)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kvm/hyp/hyp-constants.c:10: }
#NO_APP
	mov	w0, 0	//,
	hint	29 // autiasp
	ret	
	.size	main, .-main
	.ident	"GCC: (Arm GNU Toolchain 12.2.Rel1 (Build arm-12.24)) 12.2.1 20221205"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align	3
	.word	4
	.word	16
	.word	5
	.string	"GNU"
	.word	3221225472
	.word	4
	.word	2
	.align	3
