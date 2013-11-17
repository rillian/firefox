%ifidn __OUTPUT_FORMAT__,elf32
%include "vp8_asm_enc_offsets_x86-linux-gcc.asm"
%elifidn __OUTPUT_FORMAT__,elf64
%include "vp8_asm_enc_offsets_x86_64-linux-gcc.asm"
%elifidn __OUTPUT_FORMAT__,macho32
%include "vp8_asm_enc_offsets_x86-darwin9-gcc.asm"
%elifidn __OUTPUT_FORMAT__,macho64
%include "vp8_asm_enc_offsets_x86_64-darwin9-gcc.asm"
%elifidn __OUTPUT_FORMAT__,win32
%include "vp8_asm_enc_offsets_x86-win32-vs8.asm"
%elifidn __OUTPUT_FORMAT__,x64
%include "vp8_asm_enc_offsets_x86_64-win64-vs8.asm"
%endif
