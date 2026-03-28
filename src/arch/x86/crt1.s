	.section .text
	.global _start

_start:
	# clear frame pointer – same trick as the 64‑bit version
	xor    %ebp, %ebp          # EBP = 0

	# -------------------------------------------------
	# 32‑bit calling convention (cdecl):
	#   arguments are passed on the stack, right‑to‑left.
	#   we want to pass the current stack pointer (ESP)
	#   as the first argument to the C entry point.
	# -------------------------------------------------
	# push the current ESP value
	push   %esp                # arg0 = current stack pointer

	# Align the stack to a 16‑byte boundary before the call.
	# After the push, ESP is 4 bytes lower than it was at entry.
	# The BIOS/bootloader leaves ESP 4‑byte aligned, so we need
	# to subtract another 12 bytes to reach a 16‑byte multiple.
	# (4 + 12 = 16)
	sub    $12, %esp           # make ESP 16‑byte aligned

	call   _start_c            # call the C entry point

_hang:
	hlt                         # stop the processor
	jmp    _hang                # keep looping if we somehow continue
