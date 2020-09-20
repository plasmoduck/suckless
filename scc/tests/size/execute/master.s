	.globl	text1,averylongtext,text5
	.extern	text6
	.text
	.equ	text2,4
text1:	.byte	0
averylongtext:
	.byte	0
text3:	.byte	0
	.comm	text4,10
	.comm	text5,18
	.short	text6

	.globl	data1,averylongdata,data5
	.data
	.equ	data2,5
data1:	.byte	3
averylongdata:
	.byte	0
data3:	.byte	0
	.comm	data4,10
	.comm	data5,18

	.globl	bss1,averylongbss,bss5
	.bss
	.equ	bss2,5
bss1:	.byte	0
averylongbss:
	.byte	0
bss3:	.byte	0
	.comm	bss4,10
	.comm	bss5,18
