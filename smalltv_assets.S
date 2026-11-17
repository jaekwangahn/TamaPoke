#if defined(TAMAPOKE_SMALLTV_PRO)
    .section .rodata.tamapoke_smalltv_thumbs,"a",@progbits
    .balign 4
    .global smallTvThumbsBegin
    .global smallTvThumbsEnd
smallTvThumbsBegin:
    .incbin "thumbs.bin"
smallTvThumbsEnd:
#endif
