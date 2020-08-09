// Common/CRC.cpp

#include "StdAfx.h"

#include "../../C/7zCrc.h"

struct CCRCTableInit { CCRCTableInit() { CrcGenerateTable(); } } g_CRCTableInit;

void cherrytree_register_crc_table() {};
