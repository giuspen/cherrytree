// Deflate64Register.cpp

#include "StdAfx.h"

#include "../Common/RegisterCodec.h"

#include "DeflateDecoder.h"

#include "DeflateEncoder.h"

namespace NCompress {
namespace NDeflate {

REGISTER_CODEC_CREATE(CreateDec, NDecoder::CCOMCoder64())

REGISTER_CODEC_CREATE(CreateEnc, NEncoder::CCOMCoder64())

REGISTER_CODEC_2(Deflate64, CreateDec, CreateEnc, 0x40109, "Deflate64")

}}
