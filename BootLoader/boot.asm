         ;�����嵥17-1
         ;�ļ�����boot.asm
         ;�ļ�˵����Ӳ���������������� 
         ;�������ڣ�2015-09-13 11:20        ;���ö�ջ�κ�ջָ�� 

%define  BOOT_LOADER_ADDRESS				0x10000;�������ں˼��ص���ʼ�ڴ��ַ 
%define  BOOT_LOADER_SEGMENT				(BOOT_LOADER_ADDRESS>>4)    ;�������ں˼��ص���ʼ�ڴ�ε�ַ 
%define  BOOT_LOADER_OFFSET					(BOOT_LOADER_ADDRESS&0xffff);�������ں˼��ص���ʼ�ڴ��ƫ�� 

%define  BOOT_LOADER_STACK				0x00090000

%define  MEMORY_PARAMS					0x00007e00
%define  MEM_MAP_COUNT					0x00007e00 ;���ڴ�ֲ����ݴ���ڴ˴�,�Ա�kernel_main�н��д���
%define  MEM_MAP_BUF		  			0x00007e04

%define  DISK_INFO						0x00008000
%define  BOOT_LOAD_DRIVER				0x00008000
%define  DISK_INFO_DRIVER				0x00008000
%define  DISK_INFO_TYPE					0x00008001
%define  DISK_INFO_CYLINDERS			0x00008002
%define  DISK_INFO_HEADS				0x00008004
%define  DISK_INFO_SECTORS				0x00008005
%define  DISK_PARAM_SEG					0x00008006
%define  DISK_PARAM_OFFSET				0x00008008

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
		nop
;bootdrv			 db      0  ;��������
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

        mov     [DISK_INFO_DRIVER], dl ; Save boot drive
		mov     ah,0x08

		;clear_screen
		mov		ax,	0x0003
		int		0x10

        mov     si, boot_msg ;"BootStrap booting ..."
        call    print
		
;-----------------------------------------------------------------
;��ȡ�������̲���
;INT 13H��ͳ�жϣ�AH=08H
;��1��  ��ȡ����������
;AH��08H
;��ڣ�
;	DL����������00H~7FH�����̣�80H~0FFH��Ӳ��
;���أ�
;   CF��1��������ʧ�ܣ�AH��״̬����
;   CF��0 �ɹ�
;   BL��01H �� 360K
;     ��02H �� 1.2M
;     ��03H �� 720K
;     ��04H �� 1.44M
;     ��05H   ����
;     ��06H  2.88M
;     ��10H  ATAPI���ƶ�����
;   CH���������ŵĵ�8λ   low eight bits of maximum cylinder number������Ŵ�0��ʼ�㣩
;   CL��λ7-6���������ŵĸ�2λ high two bits of maximum cylinder number
;   CL��λ5-0����������� maximum sector number�������Ŵ�1��ʼ�㣩
;   DH������ͷ�� maximum head number����ͷ�Ŵ�0��ʼ�㣩
;   DL���������� number of drives
;   ES:DI�������������������ַ��ֻ������
		mov     si, get_disk_params_msg
		call    print
		mov     ah, 0x08
		mov     dl, [DISK_INFO_DRIVER]
		int     0x13
		jnc     get_disk_params_ok
		mov     si, failed_msg
        call    print
		hlt
get_disk_params_ok:
		mov     si, ok_msg
        call    print
		mov     [DISK_INFO_HEADS], dh
		mov     ah,	cl
		shr     ah,	6
		mov     al,	ch
		mov     [DISK_INFO_CYLINDERS], ax
		and     cl, 00111111b
		mov     [DISK_INFO_SECTORS], cl
		mov     [DISK_INFO_TYPE], bl
		;mov		ax,es
		;mov     [DISK_PARAM_SEG], ax
		;mov     [DISK_PARAM_OFFSET], di
				
;-----------------------------------------------------------------
;�����ں˴��������OSloader
		mov     si, load_osloader_msg
        call    print
		
		mov		si, Disk_Address_Packet	; address of "disk address packet"
		mov		ah, 0x42			; AL is unused
		mov		dl, [DISK_INFO_DRIVER]	; drive number 0 (OR the drive # with 0x80)
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
; ͨ��BIOS INT 15H ��ȡ�ڴ�ֲ���Ϣ
		;MEM_MAP_BUF�Ľṹ
		;uint32 MEM_MAP_COUNT
		;struct RAM_ARDS // Address Range Descriptor Structure
		;{
		;	uint32 Size; ���ֶ����Լ���ӵ�,����Ҫ
		;	uint64 BaseAddr;
		;	uint64 Length;
		;	uint32 Type;
		;}[];
get_memory_map:
		mov     si, check_memory_msg
		call    print
		mov		ebx, 0                 ;ebx�д�ź���ֵ, ��һ�ε���ʱebx ����Ϊ 0;
		mov		edi,  MEM_MAP_BUF;es:di ָ��һ�λ��������������ARDS��Address Range Descriptor Structure�������ڴ�ֲ�������
		mov		dword [MEM_MAP_COUNT], 0
next_mem_block:
		mov		eax, 0E820h
		mov		ecx, 64 ;ecx����������(ARDS)�������Ĵ�С�����ֽ�Ϊ��λ��������ͨ�������BIOS�������20�ֽڵ���Ϣ��ARDS��
		mov		edx, 0534D4150h ; 'SMAP' �ж�Ҫ��
		int		15h
		jc		mem_check_failed ;	CFλ��λ�������ó��������޴� 
		add		edi, ecx
		inc		dword [MEM_MAP_COUNT]
		cmp		ebx, 0;	bx�����һ��ַ����������ĺ���ֵ�����ebx��ֵΪ0��˵���Ѿ��������һ����ַ��Χ�������ˡ�
		jne		next_mem_block
		jmp		mem_check_ok
mem_check_failed:
		mov     si, failed_msg
		call    print
		hlt
mem_check_ok:
		mov     si, ok_msg
        call    print

;-----------------------------------------------------------------
;��A20 (����ģʽ��http://wiki.osdev.org/A20_Line)
         in		al,0x92                         ;����оƬ�ڵĶ˿� 
         or		al,0000_0010B
         out		0x92,al                        ;��A20
;------------------------------------------------------------------
;��ʼ��ȫ�ֶ���������Ϊ���뱣��ģʽ��׼��
		 cli      ;�˴��ر��жϷǳ���Ҫ,����VMware��ִ��lidt	[IDTR]ʱ������쳣
		 lgdt	[GDTR]
		 lidt	[IDTR]

;-------------------------------------------------------------------
;��������ģʽ
         mov		eax,cr0                  
         or		eax,1
         mov		cr0,eax                        ;����PEλ
      
         ;���½��뱣��ģʽ... ...
         jmp dword GDT32.code32:boot_code32 ;16λ��������ѡ���ӣ�32λƫ��
                                            ;����ˮ�߲����л�������

;===============================================================================
        [bits 32]               
;-------------------------------------------------------------------------------
   boot_code32:                                  
         mov		ax,		GDT32.data32                    ;�������ݶ�(4GB)ѡ����
         mov		ds,		ax
         mov		es,		ax
         mov		fs,		ax
         mov		gs,		ax
         mov		ss,		ax										;���ض�ջ��(4GB)ѡ����

		 ;mov ah,    0xac
		 ;mov edi,   0xb8000
		 ;mov esi,   MEM_MAP_COUNT;KERNEL_START_ADDRESS
		 ;mov ecx,   4+20+20
next_hex_char:		 
		 ;lodsb
		 ;call print_hex
		 ;loop next_hex_char 
		 mov     esp,  BOOT_LOADER_STACK
		 push    MEMORY_PARAMS
		 push    DISK_INFO
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
align   8
GDT32:
			dq	0x0000000000000000
.code32		equ  $ - GDT32
		    dq	0x00cf9A000000ffff
.data32		equ  $ - GDT32
			dq	0x00cf92000000ffff
.real_code  equ  $ - GDT32
			dq	0x00009A010000ffff
.real_data  equ  $ - GDT32
			dq	0x000092010000ffff

GDTR		dw	$ - GDT32 - 1
			dd	GDT32 ;GDT������/���Ե�ַ

IDTR		dw	0
			dd	0 ;IDT������/���Ե�ַ

; Message strings
;boot_msg			db      'BootStrap booting ',13,10,0
boot_msg			db      'Booting ',13,10,0
get_disk_params_msg db      'Get disk params ',0
load_osloader_msg	db		'Load BootLoader ',0
check_memory_msg	db		'Get memory map ',0
ok_msg				db		'OK',13,10,0
failed_msg			db		'FAILED',13,10,0
;hex_chars			db      '0123456789ABCDEF dddddddddddddddddd                    '

;-------------------------------------------------------------------------
	times 510-64-($-$$) db 0
;disk_partition_table1:
;disk_partition_table2:
;disk_partition_table3:
;disk_partition_table4:

;-------------------------------------------------------------------------------                             
     times	510-($-$$) db 0
	 dw 0xAA55