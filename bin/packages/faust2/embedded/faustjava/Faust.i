/* File : Faust.i */
%module Faust

%{
#include "faust/dsp/llvm-c-dsp.h"
#include "libfaust_ext.h"
%}

/* Let's just grab the original header file here */
%include "faust/dsp/llvm-c-dsp.h"
%include "libfaust_ext.h"
