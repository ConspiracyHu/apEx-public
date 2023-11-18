;----------------------------------------
%macro include_binary 2
global %1
%1 incbin %2
%1end:
global %1_size
%1_size dd %1end - %1
%endmacro 
;----------------------------------------

section .data

include_binary _music, "release.mus"
include_binary _demo, "release.64k"
include_binary _precalc, "precalc.64k"

