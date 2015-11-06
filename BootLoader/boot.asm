         ;�����嵥17-1
         ;�ļ�����boot.asm
         ;�ļ�˵����Ӳ���������������� 
         ;�������ڣ�2015-09-13 11:20        ;���ö�ջ�κ�ջָ�� 

%define  BOOT_LOADER_ADDRESS				0x10000;�������ں˼��ص���ʼ�ڴ��ַ 
%define  BOOT_LOADER_SEGMENT				(BOOT_LOADER_ADDRESS>>4)    ;�������ں˼��ص���ʼ�ڴ�ε�ַ 
%define  BOOT_LOADER_OFFSET					(BOOT_LOADER_ADDRESS&0xffff);�������ں˼��ص���ʼ�ڴ��ƫ�� 

%define  BOOT_LOADER_STACK					0x00090000

; 0x00100000 +--------------------+
;            |      ROM           |
; 0x000C0000 +--------------------+
;            |                    |
; 0x000B8000 +     ��Ƶ������     +
;            |                    |
; 0x000A0000 +--------------------+
;            |        ...         |
; 0x00090000 +--------------------+
;            | BootLoader stack   |
;            |        ...         |
;            |  OsLoader code     |
; 0x00020000 +--------------------+
;            | BootLoader code    |
; 0x00010000 +--------------------+
;            |        ...         |
; 0x00007F00 +--------------------+
;            |     �ڴ�ӳ���     |
; 0x00007E00 +--------------------+
;            |     boot           |
; 0x00007C00 +--------------------+
;            |     boot��ջ       |
; 0x00000000 +--------------------+
;===============================================================================

        ORG     0x7C00
        BITS    16
        SECTION .text
boot:
;BIOS�����������
;��ڿ�����0000:7C00 �� 07C0:0000 
;�˴�ʹ�ó���ת��ͳһ����Ϊ0000:7C00
;=========================================================================
		jmp     0000:start ;5byte
;------------------------------------------------------------------------
boot_drive		  db      0  ;��������
boot_loader_main dd      0  ;boot_loaderģ����ڵ�ַ����CreateImageд��
Disk_Address_Packet:
		db		10h		;size of packet (16 bytes)
		db		00h		;always 0
Kernel_Sectors:
		dw		7Fh		;number of sectors to transfer(max 127)����CreateImageд��
		dw		BOOT_LOADER_OFFSET	;transfer buffer offset
		dw		BOOT_LOADER_SEGMENT	;transfer buffer segment 1000:0000=10000
		dd		00000001h;start sector
		dd		00000000h

;------------------------------------------------------------------------
start:
        ; Setup initial environment
        mov     ax, cs
        mov     ds, ax
        mov		ss, ax
		mov		es, ax
        mov		esp, 0x7c00

        mov     [boot_drive], dl ; Save boot drive
		mov     ah,0x08

		;clear_screen
		mov		ax,	0x0003
		int		0x10

        mov     si, boot_msg ;"BootStrap booting ..."
        call    print
				
;-----------------------------------------------------------------
;�����ں˴��������OSloader
load_osloader:
		mov     si, load_osloader_msg
        call    print
		
		mov		si, Disk_Address_Packet	; address of "disk address packet"
		mov		ah, 0x42			; AL is unused
		mov		dl, [boot_drive]	; drive number 0 (OR the drive # with 0x80)
		int		0x13
		jnc		load_osloader_ok
load_osloader_failed:
		mov     si, failed_msg
        call    print
		hlt
load_osloader_ok:
		mov     si, ok_msg
        call    print

;-----------------------------------------------------------------
;��A20 (����ģʽ��http://wiki.osdev.org/A20_Line)
         in		al,0x92                         ;����оƬ�ڵĶ˿� 
         or		al,0000_0010B
         out	0x92,al                        ;��A20
;------------------------------------------------------------------
;��ʼ��ȫ�ֶ���������Ϊ���뱣��ģʽ��׼��
		 cli      ;�˴��ر��жϷǳ���Ҫ,����VMware��ִ��lidt	[IDTR]ʱ������쳣
		 lgdt	[GDTR]
		 lidt	[IDTR]

;-------------------------------------------------------------------
;��������ģʽ
         mov		eax,cr0                  
         or			eax,1
         mov		cr0,eax                        ;����PEλ
      
         ;���½��뱣��ģʽ... ...
         jmp dword GDT32.code32:boot_code32 ;16λ��������ѡ���ӣ�32λƫ��
                                            ;����ˮ�߲����л�������

;===============================================================================
        [bits 32]               
   boot_code32:                                  
         mov		ax,		GDT32.data32                    ;�������ݶ�(4GB)ѡ����
         mov		ds,		ax
         mov		es,		ax
         mov		fs,		ax
         mov		gs,		ax
         mov		ss,		ax										;���ض�ջ��(4GB)ѡ����

		 mov     esp,  BOOT_LOADER_STACK
		 movzx   eax,  byte [boot_drive]
		 push    eax
		 call	 [ds:boot_loader_main]

		 hlt

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

align   8
GDT32:
			dq	0x0000000000000000
.code32		equ  $ - GDT32
		    dq	0x00cf9A000000ffff
.data32		equ  $ - GDT32
			dq	0x00cf92000000ffff

GDTR		dw	$ - GDT32 - 1
			dd	GDT32 ;GDT������/���Ե�ַ

IDTR		dw	0
			dd	0 ;IDT������/���Ե�ַ

boot_msg			db      'BootStrap booting ',13,10,0
load_osloader_msg	db		'Load BootLoader ',0
ok_msg				db		'OK',13,10,0
failed_msg			db		'FAILED',13,10,0

;-------------------------------------------------------------------------------                             
     times	510-($-$$) db 0
	 dw 0xAA55