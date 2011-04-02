[BITS 32]
;****************************************************************
;                        UCOS-GCC.ASM
;****************************************************************
[GLOBAL OSStartHighRdy]
[GLOBAL OSCtxSw]
[GLOBAL OSIntCtxSw]
[GLOBAL OSTickISR]
[GLOBAL OSKbdISRWraper]
[GLOBAL OSDummyISR]
[EXTERN OSIntEnter]
[EXTERN OSIntExit]
[EXTERN OSTimeTick]
[EXTERN OSTCBCur]
[EXTERN OSTCBHighRdy]
[EXTERN OSKbdISR]
;****************************************************************
;                      START MULTITASKING
;                   void OSStartHighRdy(void)
;****************************************************************
OSStartHighRdy
	    MOV    EBX,[OSTCBHighRdy]  ;Get highest prio. task
	    MOV    [OSTCBCur],EBX
	    MOV    EAX,[EBX]           ;Get ptr to top of stack
	    MOV    ESP,EAX
	    POPAD
	    IRET                       ;Return to task
;****************************************************************
;            PERFORM A CONTEXT SWITCH (From task level)
;                       void OSCtxSw(void)
;****************************************************************
OSCtxSw
	    PUSHAD                     ;Save current task's context
	    MOV    EBX,[OSTCBCur]      ;Save stack ptr in TCB
	    MOV    [EBX],ESP
	    MOV    EBX,[OSTCBHighRdy]  ;Point to HI Prio. Task Rdy
	    MOV    [OSTCBCur],EBX      ;This is now current TCB
	    MOV    ESP,[EBX]           ;Get new task's stack ptr
	    POPAD
	    IRET                       ;Return to new task
;****************************************************************
;            PERFORM A CONTEXT SWITCH (From an ISR)
;                     void OSIntCtxSw(void)
;****************************************************************
OSIntCtxSw
	    ADD    ESP,8               ;Ignore calls to OSIntExit,
				       ;OSIntCtxSw and locals.
	    MOV    EBX,[OSTCBCur]      ;Save stack ptr in TCB
	    MOV    [EBX],ESP
	    MOV    EBX,[OSTCBHighRdy]  ;Point to HI Prio. Task Rdy
	    MOV    [OSTCBCur],EBX      ;This is now current TCB
	    MOV    ESP,[EBX]           ;Get new task's stack ptr
	    POPAD
	    IRET
;****************************************************************
;                        HANDLE TICK ISR
;                      void OSTickISR(void)
;****************************************************************
OSTickISR
	    PUSHAD                     ;Save current task's context
	    MOV     AL,0x20
	    OUT     0x20,AL
	    STI                        ;Enable interrupt nesting
	    CALL    OSIntEnter
	    CALL    OSTimeTick
	    CALL    OSIntExit
	    POPAD
	    IRET                       ;Return to interrupted task

;****************************************************************
;                      HANDLE KEYBOARD INTERRUPT
;                      void OSKbdISRWraper(void)
;****************************************************************
OSKbdISRWraper
	    PUSHAD                     ;Save current task's context
	    ;IN      AL,0x60
	    ;IN      AL,0x61
	    ;OR      AL,10000000b
	    ;OUT     0x61,AL
	    ;AND     AL,01111111b
	    ;OUT     0x61,AL
	    MOV     AL,0x20
	    OUT     0x20,AL
	    STI                        ;Enable interrupt nesting
	    CALL    OSIntEnter
	    CALL    OSKbdISR
	    CALL    OSIntExit
	    POPAD
	    IRET                       ;Return to interrupted task

;****************************************************************
;                        HANDLE DUMMY ISR
;                      void OSDummyISR(void)
;****************************************************************
OSDummyISR
	    PUSH    EAX
	    MOV     AL,0x20
	    OUT     0x20,AL
	    POP     EAX
	    IRET

