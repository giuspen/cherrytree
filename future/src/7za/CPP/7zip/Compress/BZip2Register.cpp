// BZip2Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "BZip2Decoder.h"
#include "BZip2Encoder.h"

namespace NCompress {
namespace NBZip2 {

REGISTER_CODEC_CREATE(CreateDec, CDecoder)

REGISTER_CODEC_CREATE(CreateEnc, CEncoder)

REGISTER_CODEC_2(BZip2, CreateDec, CreateEnc, 0x40202, "BZip2")

}}
