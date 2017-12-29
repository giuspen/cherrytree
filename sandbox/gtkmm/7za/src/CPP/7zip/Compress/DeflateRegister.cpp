// DeflateRegister.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "DeflateDecoder.h"
#include "DeflateEncoder.h"

namespace NCompress {
namespace NDeflate {

REGISTER_CODEC_CREATE(CreateDec, NDecoder::CCOMCoder)

REGISTER_CODEC_CREATE(CreateEnc, NEncoder::CCOMCoder)

REGISTER_CODEC_2(Deflate, CreateDec, CreateEnc, 0x40108, "Deflate")

}}
