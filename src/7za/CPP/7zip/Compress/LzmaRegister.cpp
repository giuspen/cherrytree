// LzmaRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "LzmaDecoder.h"

#include "LzmaEncoder.h"

namespace NCompress {
namespace NLzma {

REGISTER_CODEC_E(LZMA,
    CDecoder(),
    CEncoder(),
    0x30101,
    "LZMA")
}}

void cherrytree_register_lzma() {}
