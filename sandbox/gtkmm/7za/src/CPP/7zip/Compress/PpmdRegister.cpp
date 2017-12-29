// PpmdRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "PpmdDecoder.h"

#include "PpmdEncoder.h"

namespace NCompress {
namespace NPpmd {

REGISTER_CODEC_E(PPMD,
    CDecoder(),
    CEncoder(),
    0x30401,
    "PPMD")
}}
