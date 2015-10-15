         ;�����嵥17-1
         ;�ļ�����c17_mbr.asm
         ;�ļ�˵����Ӳ���������������� 
         ;�������ڣ�2012-07-13 11:20        ;���ö�ջ�κ�ջָ�� 
		 ;%include "E:/OS/ShellcodeOS/kernel/include/kernel.h"

%define  KERNEL_LOAD_SEGMENT				  0x1000   ;�������ں˼��ص���ʼ�ڴ�ε�ַ 
%define  KERNEL_LOAD_ADDRESS				  0X10000;�������ں˼��ص���ʼ�ڴ��ַ 

;%define  KERNEL_MOVE_ADDRESS			      0x100000	
;%define  KERNEL_START_ADDRESS			  0x11c50	

%define  		 ARDS_COUNT				  0x00007e00 ;���ڴ�ֲ����ݴ���ڴ˴�,�Ա�kernel_main�н��д���
%define  		 ARDS_BUF			  	  0x00007e04

; 0x00100000 +--------------------+
;            |      ROM           |
; 0x000C0000 +--------------------+
;            |                    |
; 0x000B8000 |     ��Ƶ������     |
;            |                    |
; 0x000A0000 +--------------------+
;            |     ......         |
; 0x00090000 +--------------------+
;            |     ......         |
; 0x00033000 +--------------------+
;            |  PageTable[0x300]  |
; 0x00032000 +--------------------+
;            |  PageTable[0]      |
; 0x00031000 +--------------------+
;            |     PageDir        |
; 0x00030000 +--------------------+
;            |     OsLoader       |
; 0x00020000 +--------------------+
;            |     BootLoader     |
; 0x00010000 +--------------------+
;            |        ...         |
; 0x00007F00 +--------------------+
;            |     �ڴ�ӳ���     |
; 0x00007E00 +--------------------+
;            |     boot           |
; 0x00007C00 +--------------------+
;            |     ��ջ           |
; 0x00000000 +--------------------+
;===============================================================================
        ORG     0x7c00
        BITS    16
        SECTION .text
; Entry point for initial bootstap code
boot:
          jmp     0:start ;5byte
bootdrv	  db      0
boot_loader_main:
          dd      0
;Offset	Size	Description
; 0	1	size of packet (16 bytes)
; 1	1	always 0
; 2	2	number of sectors to transfer (max 127 on some BIOSes)
; 4	4	-> transfer buffer (16 bit segment:16 bit offset) (see note #1)
; 8	4	starting LBA
;12	4	used for upper part of 48 bit LBAs
Disk_Address_Packet:
		db	10h; size of packet (16 bytes)
		db  00h; always 0
Kernel_Sectors:
		dw  7Fh; number of sectors to transfer
		dw  0000h ;transfer buffer offset
		dw  1000h ;transfer buffer segment
		dd  00000001h
		dd  00000000h
start:
        ; Setup initial environment
		cli
		cld
        mov     ax, cs
        mov     ds, ax
        mov		ss, ax
		mov		es, ax
        mov		esp, 0x7c00

        ; Save boot drive
        mov     [bootdrv], dl

		call		clear_screen

        mov     si, boot_msg
        call    print
		
;-----------------------------------------------------------------
;�����ں˼�����osloader
		mov		si, Disk_Address_Packet		; address of "disk address packet"
		mov		ah, 0x42			; AL is unused
		mov		dl, [bootdrv]	; drive number 0 (OR the drive # with 0x80)
		int		0x13
		jnc		load_osloader_ok
load_osloader_failed:
		mov     si, load_osloader_failed_msg
        call    print
		jmp		$

load_osloader_ok:
		mov     si, load_osloader_ok_msg
        call    print
;-----------------------------------------------------------------
; ͨ��BIOS INT 15H ��ȡ�ڴ�ֲ���Ϣ
		;ARDS_BUF�Ľṹ
		;uint32 ARDS_COUNT
		;struct RAM_ARDS // Address Range Descriptor Structure
		;{
		;	uint32 Size; ���ֶ����Լ���ӵ�,����Ҫ
		;	uint64 BaseAddr;
		;	uint64 Length;
		;	uint32 Type;
		;}[];

get_memory_map:
		mov	ebx, 0                 ;ebx�д�ź���ֵ, ��һ�ε���ʱebx ����Ϊ 0;
		mov	edi,  ARDS_BUF;es:di ָ��һ�λ��������������ARDS��Address Range Descriptor Structure�������ڴ�ֲ�������
		mov	dword [ARDS_COUNT], 0

next_mem_block:
		mov	eax, 0E820h
		mov	ecx, 64 ;ecx����������(ARDS)�������Ĵ�С�����ֽ�Ϊ��λ��������ͨ�������BIOS�������20�ֽڵ���Ϣ��ARDS��
		mov	edx, 0534D4150h ; 'SMAP' �ж�Ҫ��
		int	15h
		;	CFλ��λ�������ó��������޴� 
		;	eax���"SMAP" ��ASCII��
		;	es:di���������Ϣ
		;	ecx���ARDS�������ֽ���
		;	bx�����һ��ַ����������ĺ���ֵ�����ebx��ֵΪ0��˵���Ѿ��������һ����ַ��Χ�������ˡ�
		jc	LABEL_MEM_CHK_FAIL
		add	edi, ecx
		inc	dword [ARDS_COUNT]
		cmp	ebx, 0
		jne	next_mem_block
		jmp	LABEL_MEM_CHK_OK

LABEL_MEM_CHK_FAIL:
		mov     si, check_memory_failed_msg
        call    print
		jmp		$

LABEL_MEM_CHK_OK:
		mov     si, check_memory_ok_msg
        call    print
;-----------------------------------------------------------------
;��A20
         in al,0x92                         ;����оƬ�ڵĶ˿� 
         or al,0000_0010B
         out 0x92,al                        ;��A20
;------------------------------------------------------------------
;��ʼ��ȫ�ֶ���������Ϊ���뱣��ģʽ��׼��
		 cli
		 lgdt [gdtr]

;-------------------------------------------------------------------
;��������ģʽ
         mov eax,cr0                  
         or  eax,1
         mov cr0,eax                        ;����PEλ
      
         ;���½��뱣��ģʽ... ...
         jmp dword 0x0008:flush             ;16λ��������ѡ���ӣ�32λƫ��
                                            ;����ˮ�߲����л�������
         [bits 32]               
  flush:                                  
         mov eax,	0x00010                    ;�������ݶ�(4GB)ѡ����
         mov ds,		eax
         mov es,		eax
         mov fs,		eax
         mov gs,		eax
         mov ss,		eax										;���ض�ջ��(4GB)ѡ����
		 
		 ;mov ah,    0xac
		 ;mov edi,   0xb8000
		 ;mov esi,   ARDS_COUNT;KERNEL_START_ADDRESS
		 ;mov ecx,   4+20+20
next_hex_char:		 
		 ;lodsb
		 ;call print_hex
		 ;loop next_hex_char 
		 mov     esp,  0x90000
		 push    dword [ARDS_COUNT]
		 push    ARDS_BUF
		 call	 [ds:boot_loader_main]
		 hlt
;-----------------------------------------------------------
; clear the screen via an interrupt
clear_screen:
		mov     al, 02h		; al = 02h, code for video mode (80x25)
		mov     ah, 00h		; code for the change video mode function
		int     10h		; trigger interrupt to call function
		ret

;-----------------------------------------------------------
; Print string to console
; si = ptr to first character of a null terminated string
print:
        cld
        mov     ah, 0x0e
next_char:
        lodsb
        cmp     al, 0
        je      print_done
        int     0x10
        jmp     next_char
print_done:
        ret

;----------------------------------------------------------------------
;32λ����ģʽ����ʾ16�����ַ�
;AH = ��ʾ��ɫ
;AL = ��ʾ�ַ�
;EDI= ��ʾ������ָ��
print_hex:
		 ;mov bl,al
		 ;mov dl,al
		 
		 ;shr bl,4
		 ;and ebx,0x0000000f
		 ;mov al,[ebx + hex_chars]
		 ;stosw

		 ;and edx,0x0000000f
		 ;mov al,[edx + hex_chars]
		 ;stosw
		 ;ret
;-------------------------------------------------------------------------------
gdtr		dw	(0x18-1)
			dd	GDT_NULL ;GDT������/���Ե�ַ

GDT_NULL	dd	00
			dd	00
GDT_CODE	dd	0x0000ffff
			dd	0x00cf9800
GDT_DATA	dd	0x0000ffff
			dd	0x00cf9200

; Message strings
boot_msg:
        db      'Booting ...',13,10,0
load_osloader_ok_msg:
		db		'Load boot-loader OK',13,10,0
load_osloader_failed_msg:
		db		'Load boot-loader failed',13,10,0
check_memory_ok_msg:
		db		'Get memory map OK',13,10,0
hex_chars:
        ;db      '0123456789ABCDEF'
check_memory_failed_msg:
		db		'Get memory map failed',13,10,0
;-------------------------------------------------------------------------------                             
         times	510-($-$$) db 0
sign0x55aa:
		 db 0x55,0xaa