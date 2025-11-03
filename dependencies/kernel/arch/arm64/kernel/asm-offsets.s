	.arch armv8-a
	.file	"asm-offsets.c"
// GNU C11 (Arm GNU Toolchain 12.2.Rel1 (Build arm-12.24)) version 12.2.1 20221205 (aarch64-none-linux-gnu)
//	compiled by GNU C version 4.8.5 20150623 (Red Hat 4.8.5-44), GMP version 6.2.1, MPFR version 3.1.6, MPC version 1.0.3, isl version isl-0.15-1-g835ea3a-GMP

// GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
// options passed: -mlittle-endian -mgeneral-regs-only -mabi=lp64 -mbranch-protection=pac-ret+leaf -O2 -std=gnu11 -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-delete-null-pointer-checks -fno-allow-store-data-races -fstack-protector-strong -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftrivial-auto-var-init=zero -fno-stack-clash-protection -fpatchable-function-entry=2 -fno-strict-overflow -fstack-check=no -fconserve-stack
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
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:30:   DEFINE(TSK_ACTIVE_MM,		offsetof(struct task_struct, active_mm));
#APP
// 30 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_ACTIVE_MM 1080 offsetof(struct task_struct, active_mm)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:31:   BLANK();
// 31 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:32:   DEFINE(TSK_TI_CPU,		offsetof(struct task_struct, thread_info.cpu));
// 32 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_CPU 24 offsetof(struct task_struct, thread_info.cpu)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:33:   DEFINE(TSK_TI_FLAGS,		offsetof(struct task_struct, thread_info.flags));
// 33 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_FLAGS 0 offsetof(struct task_struct, thread_info.flags)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:34:   DEFINE(TSK_TI_PREEMPT,	offsetof(struct task_struct, thread_info.preempt_count));
// 34 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_PREEMPT 16 offsetof(struct task_struct, thread_info.preempt_count)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:35:   DEFINE(TSK_TI_PREEMPT_LAZY,	offsetof(struct task_struct, thread_info.preempt_lazy_count));
// 35 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_TI_PREEMPT_LAZY 8 offsetof(struct task_struct, thread_info.preempt_lazy_count)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:43:   DEFINE(TSK_STACK,		offsetof(struct task_struct, stack));
// 43 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_STACK 40 offsetof(struct task_struct, stack)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:45:   DEFINE(TSK_STACK_CANARY,	offsetof(struct task_struct, stack_canary));
// 45 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TSK_STACK_CANARY 1224 offsetof(struct task_struct, stack_canary)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:47:   BLANK();
// 47 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:48:   DEFINE(THREAD_CPU_CONTEXT,	offsetof(struct task_struct, thread.cpu_context));
// 48 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->THREAD_CPU_CONTEXT 2704 offsetof(struct task_struct, thread.cpu_context)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:49:   DEFINE(THREAD_SCTLR_USER,	offsetof(struct task_struct, thread.sctlr_user));
// 49 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->THREAD_SCTLR_USER 3792 offsetof(struct task_struct, thread.sctlr_user)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:51:   DEFINE(THREAD_KEYS_USER,	offsetof(struct task_struct, thread.keys_user));
// 51 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->THREAD_KEYS_USER 3688 offsetof(struct task_struct, thread.keys_user)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:54:   DEFINE(THREAD_KEYS_KERNEL,	offsetof(struct task_struct, thread.keys_kernel));
// 54 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->THREAD_KEYS_KERNEL 3768 offsetof(struct task_struct, thread.keys_kernel)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:57:   DEFINE(THREAD_MTE_CTRL,	offsetof(struct task_struct, thread.mte_ctrl));
// 57 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->THREAD_MTE_CTRL 3784 offsetof(struct task_struct, thread.mte_ctrl)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:59:   BLANK();
// 59 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:60:   DEFINE(S_X0,			offsetof(struct pt_regs, regs[0]));
// 60 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X0 0 offsetof(struct pt_regs, regs[0])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:61:   DEFINE(S_X2,			offsetof(struct pt_regs, regs[2]));
// 61 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X2 16 offsetof(struct pt_regs, regs[2])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:62:   DEFINE(S_X4,			offsetof(struct pt_regs, regs[4]));
// 62 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X4 32 offsetof(struct pt_regs, regs[4])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:63:   DEFINE(S_X6,			offsetof(struct pt_regs, regs[6]));
// 63 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X6 48 offsetof(struct pt_regs, regs[6])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:64:   DEFINE(S_X8,			offsetof(struct pt_regs, regs[8]));
// 64 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X8 64 offsetof(struct pt_regs, regs[8])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:65:   DEFINE(S_X10,			offsetof(struct pt_regs, regs[10]));
// 65 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X10 80 offsetof(struct pt_regs, regs[10])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:66:   DEFINE(S_X12,			offsetof(struct pt_regs, regs[12]));
// 66 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X12 96 offsetof(struct pt_regs, regs[12])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:67:   DEFINE(S_X14,			offsetof(struct pt_regs, regs[14]));
// 67 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X14 112 offsetof(struct pt_regs, regs[14])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:68:   DEFINE(S_X16,			offsetof(struct pt_regs, regs[16]));
// 68 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X16 128 offsetof(struct pt_regs, regs[16])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:69:   DEFINE(S_X18,			offsetof(struct pt_regs, regs[18]));
// 69 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X18 144 offsetof(struct pt_regs, regs[18])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:70:   DEFINE(S_X20,			offsetof(struct pt_regs, regs[20]));
// 70 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X20 160 offsetof(struct pt_regs, regs[20])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:71:   DEFINE(S_X22,			offsetof(struct pt_regs, regs[22]));
// 71 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X22 176 offsetof(struct pt_regs, regs[22])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:72:   DEFINE(S_X24,			offsetof(struct pt_regs, regs[24]));
// 72 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X24 192 offsetof(struct pt_regs, regs[24])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:73:   DEFINE(S_X26,			offsetof(struct pt_regs, regs[26]));
// 73 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X26 208 offsetof(struct pt_regs, regs[26])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:74:   DEFINE(S_X28,			offsetof(struct pt_regs, regs[28]));
// 74 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_X28 224 offsetof(struct pt_regs, regs[28])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:75:   DEFINE(S_FP,			offsetof(struct pt_regs, regs[29]));
// 75 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_FP 232 offsetof(struct pt_regs, regs[29])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:76:   DEFINE(S_LR,			offsetof(struct pt_regs, regs[30]));
// 76 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_LR 240 offsetof(struct pt_regs, regs[30])"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:77:   DEFINE(S_SP,			offsetof(struct pt_regs, sp));
// 77 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_SP 248 offsetof(struct pt_regs, sp)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:78:   DEFINE(S_PSTATE,		offsetof(struct pt_regs, pstate));
// 78 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_PSTATE 264 offsetof(struct pt_regs, pstate)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:79:   DEFINE(S_PC,			offsetof(struct pt_regs, pc));
// 79 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_PC 256 offsetof(struct pt_regs, pc)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:80:   DEFINE(S_SYSCALLNO,		offsetof(struct pt_regs, syscallno));
// 80 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_SYSCALLNO 280 offsetof(struct pt_regs, syscallno)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:81:   DEFINE(S_SDEI_TTBR1,		offsetof(struct pt_regs, sdei_ttbr1));
// 81 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_SDEI_TTBR1 288 offsetof(struct pt_regs, sdei_ttbr1)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:82:   DEFINE(S_PMR_SAVE,		offsetof(struct pt_regs, pmr_save));
// 82 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_PMR_SAVE 296 offsetof(struct pt_regs, pmr_save)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:83:   DEFINE(S_STACKFRAME,		offsetof(struct pt_regs, stackframe));
// 83 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->S_STACKFRAME 304 offsetof(struct pt_regs, stackframe)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:84:   DEFINE(PT_REGS_SIZE,		sizeof(struct pt_regs));
// 84 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PT_REGS_SIZE 336 sizeof(struct pt_regs)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:85:   BLANK();
// 85 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:87:   DEFINE(COMPAT_SIGFRAME_REGS_OFFSET,		offsetof(struct compat_sigframe, uc.uc_mcontext.arm_r0));
// 87 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->COMPAT_SIGFRAME_REGS_OFFSET 32 offsetof(struct compat_sigframe, uc.uc_mcontext.arm_r0)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:88:   DEFINE(COMPAT_RT_SIGFRAME_REGS_OFFSET,	offsetof(struct compat_rt_sigframe, sig.uc.uc_mcontext.arm_r0));
// 88 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->COMPAT_RT_SIGFRAME_REGS_OFFSET 160 offsetof(struct compat_rt_sigframe, sig.uc.uc_mcontext.arm_r0)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:89:   BLANK();
// 89 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:91:   DEFINE(MM_CONTEXT_ID,		offsetof(struct mm_struct, context.id.counter));
// 91 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->MM_CONTEXT_ID 816 offsetof(struct mm_struct, context.id.counter)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:92:   BLANK();
// 92 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:93:   DEFINE(VMA_VM_MM,		offsetof(struct vm_area_struct, vm_mm));
// 93 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->VMA_VM_MM 16 offsetof(struct vm_area_struct, vm_mm)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:94:   DEFINE(VMA_VM_FLAGS,		offsetof(struct vm_area_struct, vm_flags));
// 94 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->VMA_VM_FLAGS 32 offsetof(struct vm_area_struct, vm_flags)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:95:   BLANK();
// 95 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:96:   DEFINE(VM_EXEC,	       	VM_EXEC);
// 96 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->VM_EXEC 4 VM_EXEC"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:97:   BLANK();
// 97 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:98:   DEFINE(PAGE_SZ,	       	PAGE_SIZE);
// 98 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PAGE_SZ 65536 PAGE_SIZE"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:99:   BLANK();
// 99 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:100:   DEFINE(DMA_TO_DEVICE,		DMA_TO_DEVICE);
// 100 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->DMA_TO_DEVICE 1 DMA_TO_DEVICE"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:101:   DEFINE(DMA_FROM_DEVICE,	DMA_FROM_DEVICE);
// 101 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->DMA_FROM_DEVICE 2 DMA_FROM_DEVICE"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:102:   BLANK();
// 102 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:103:   DEFINE(PREEMPT_DISABLE_OFFSET, PREEMPT_DISABLE_OFFSET);
// 103 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PREEMPT_DISABLE_OFFSET 1 PREEMPT_DISABLE_OFFSET"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:104:   DEFINE(SOFTIRQ_SHIFT, SOFTIRQ_SHIFT);
// 104 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->SOFTIRQ_SHIFT 8 SOFTIRQ_SHIFT"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:105:   DEFINE(IRQ_CPUSTAT_SOFTIRQ_PENDING, offsetof(irq_cpustat_t, __softirq_pending));
// 105 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->IRQ_CPUSTAT_SOFTIRQ_PENDING 0 offsetof(irq_cpustat_t, __softirq_pending)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:106:   BLANK();
// 106 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:107:   DEFINE(CPU_BOOT_TASK,		offsetof(struct secondary_data, task));
// 107 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->CPU_BOOT_TASK 0 offsetof(struct secondary_data, task)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:108:   BLANK();
// 108 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:109:   DEFINE(FTR_OVR_VAL_OFFSET,	offsetof(struct arm64_ftr_override, val));
// 109 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->FTR_OVR_VAL_OFFSET 0 offsetof(struct arm64_ftr_override, val)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:110:   DEFINE(FTR_OVR_MASK_OFFSET,	offsetof(struct arm64_ftr_override, mask));
// 110 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->FTR_OVR_MASK_OFFSET 8 offsetof(struct arm64_ftr_override, mask)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:111:   BLANK();
// 111 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:136:   DEFINE(CPU_CTX_SP,		offsetof(struct cpu_suspend_ctx, sp));
// 136 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->CPU_CTX_SP 104 offsetof(struct cpu_suspend_ctx, sp)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:137:   DEFINE(MPIDR_HASH_MASK,	offsetof(struct mpidr_hash, mask));
// 137 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->MPIDR_HASH_MASK 0 offsetof(struct mpidr_hash, mask)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:138:   DEFINE(MPIDR_HASH_SHIFTS,	offsetof(struct mpidr_hash, shift_aff));
// 138 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->MPIDR_HASH_SHIFTS 8 offsetof(struct mpidr_hash, shift_aff)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:139:   DEFINE(SLEEP_STACK_DATA_SYSTEM_REGS,	offsetof(struct sleep_stack_data, system_regs));
// 139 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->SLEEP_STACK_DATA_SYSTEM_REGS 0 offsetof(struct sleep_stack_data, system_regs)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:140:   DEFINE(SLEEP_STACK_DATA_CALLEE_REGS,	offsetof(struct sleep_stack_data, callee_saved_regs));
// 140 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->SLEEP_STACK_DATA_CALLEE_REGS 112 offsetof(struct sleep_stack_data, callee_saved_regs)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:142:   DEFINE(ARM_SMCCC_RES_X0_OFFS,		offsetof(struct arm_smccc_res, a0));
// 142 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_RES_X0_OFFS 0 offsetof(struct arm_smccc_res, a0)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:143:   DEFINE(ARM_SMCCC_RES_X2_OFFS,		offsetof(struct arm_smccc_res, a2));
// 143 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_RES_X2_OFFS 16 offsetof(struct arm_smccc_res, a2)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:144:   DEFINE(ARM_SMCCC_QUIRK_ID_OFFS,	offsetof(struct arm_smccc_quirk, id));
// 144 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_QUIRK_ID_OFFS 0 offsetof(struct arm_smccc_quirk, id)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:145:   DEFINE(ARM_SMCCC_QUIRK_STATE_OFFS,	offsetof(struct arm_smccc_quirk, state));
// 145 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_QUIRK_STATE_OFFS 8 offsetof(struct arm_smccc_quirk, state)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:146:   DEFINE(ARM_SMCCC_1_2_REGS_X0_OFFS,	offsetof(struct arm_smccc_1_2_regs, a0));
// 146 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X0_OFFS 0 offsetof(struct arm_smccc_1_2_regs, a0)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:147:   DEFINE(ARM_SMCCC_1_2_REGS_X2_OFFS,	offsetof(struct arm_smccc_1_2_regs, a2));
// 147 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X2_OFFS 16 offsetof(struct arm_smccc_1_2_regs, a2)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:148:   DEFINE(ARM_SMCCC_1_2_REGS_X4_OFFS,	offsetof(struct arm_smccc_1_2_regs, a4));
// 148 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X4_OFFS 32 offsetof(struct arm_smccc_1_2_regs, a4)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:149:   DEFINE(ARM_SMCCC_1_2_REGS_X6_OFFS,	offsetof(struct arm_smccc_1_2_regs, a6));
// 149 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X6_OFFS 48 offsetof(struct arm_smccc_1_2_regs, a6)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:150:   DEFINE(ARM_SMCCC_1_2_REGS_X8_OFFS,	offsetof(struct arm_smccc_1_2_regs, a8));
// 150 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X8_OFFS 64 offsetof(struct arm_smccc_1_2_regs, a8)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:151:   DEFINE(ARM_SMCCC_1_2_REGS_X10_OFFS,	offsetof(struct arm_smccc_1_2_regs, a10));
// 151 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X10_OFFS 80 offsetof(struct arm_smccc_1_2_regs, a10)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:152:   DEFINE(ARM_SMCCC_1_2_REGS_X12_OFFS,	offsetof(struct arm_smccc_1_2_regs, a12));
// 152 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X12_OFFS 96 offsetof(struct arm_smccc_1_2_regs, a12)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:153:   DEFINE(ARM_SMCCC_1_2_REGS_X14_OFFS,	offsetof(struct arm_smccc_1_2_regs, a14));
// 153 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X14_OFFS 112 offsetof(struct arm_smccc_1_2_regs, a14)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:154:   DEFINE(ARM_SMCCC_1_2_REGS_X16_OFFS,	offsetof(struct arm_smccc_1_2_regs, a16));
// 154 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM_SMCCC_1_2_REGS_X16_OFFS 128 offsetof(struct arm_smccc_1_2_regs, a16)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:155:   BLANK();
// 155 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:156:   DEFINE(HIBERN_PBE_ORIG,	offsetof(struct pbe, orig_address));
// 156 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->HIBERN_PBE_ORIG 8 offsetof(struct pbe, orig_address)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:157:   DEFINE(HIBERN_PBE_ADDR,	offsetof(struct pbe, address));
// 157 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->HIBERN_PBE_ADDR 0 offsetof(struct pbe, address)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:158:   DEFINE(HIBERN_PBE_NEXT,	offsetof(struct pbe, next));
// 158 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->HIBERN_PBE_NEXT 16 offsetof(struct pbe, next)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:159:   DEFINE(ARM64_FTR_SYSVAL,	offsetof(struct arm64_ftr_reg, sys_val));
// 159 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->ARM64_FTR_SYSVAL 24 offsetof(struct arm64_ftr_reg, sys_val)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:160:   BLANK();
// 160 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:162:   DEFINE(TRAMP_VALIAS,		TRAMP_VALIAS);
// 162 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->TRAMP_VALIAS -274916179968 TRAMP_VALIAS"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:169:   DEFINE(PTRAUTH_USER_KEY_APIA,		offsetof(struct ptrauth_keys_user, apia));
// 169 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PTRAUTH_USER_KEY_APIA 0 offsetof(struct ptrauth_keys_user, apia)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:171:   DEFINE(PTRAUTH_KERNEL_KEY_APIA,	offsetof(struct ptrauth_keys_kernel, apia));
// 171 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->PTRAUTH_KERNEL_KEY_APIA 0 offsetof(struct ptrauth_keys_kernel, apia)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:173:   BLANK();
// 173 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:176:   DEFINE(KIMAGE_ARCH_DTB_MEM,		offsetof(struct kimage, arch.dtb_mem));
// 176 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->KIMAGE_ARCH_DTB_MEM 648 offsetof(struct kimage, arch.dtb_mem)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:177:   DEFINE(KIMAGE_ARCH_EL2_VECTORS,	offsetof(struct kimage, arch.el2_vectors));
// 177 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->KIMAGE_ARCH_EL2_VECTORS 664 offsetof(struct kimage, arch.el2_vectors)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:178:   DEFINE(KIMAGE_ARCH_ZERO_PAGE,		offsetof(struct kimage, arch.zero_page));
// 178 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->KIMAGE_ARCH_ZERO_PAGE 688 offsetof(struct kimage, arch.zero_page)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:179:   DEFINE(KIMAGE_ARCH_PHYS_OFFSET,	offsetof(struct kimage, arch.phys_offset));
// 179 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->KIMAGE_ARCH_PHYS_OFFSET 696 offsetof(struct kimage, arch.phys_offset)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:180:   DEFINE(KIMAGE_ARCH_TTBR1,		offsetof(struct kimage, arch.ttbr1));
// 180 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->KIMAGE_ARCH_TTBR1 680 offsetof(struct kimage, arch.ttbr1)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:181:   DEFINE(KIMAGE_HEAD,			offsetof(struct kimage, head));
// 181 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->KIMAGE_HEAD 0 offsetof(struct kimage, head)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:182:   DEFINE(KIMAGE_START,			offsetof(struct kimage, start));
// 182 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->KIMAGE_START 24 offsetof(struct kimage, start)"	//
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:183:   BLANK();
// 183 "/home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c" 1
	
.ascii "->"
// 0 "" 2
// /home/huaizhenlv/630_v27/kernel/arch/arm64/kernel/asm-offsets.c:186: }
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
