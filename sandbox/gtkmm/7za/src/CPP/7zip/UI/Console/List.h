// List.h

#ifndef __LIST_H
#define __LIST_H

#include "../../../Common/Wildcard.h"

#include "../Common/LoadCodecs.h"

HRESULT ListArchives(CCodecs *codecs,
    const CObjectVector<COpenType> &types,
    const CIntVector &excludedFormats,
    bool stdInMode,
    UStringVector &archivePaths, UStringVector &archivePathsFull,
    bool processAltStreams, bool showAltStreams,
    const NWildcard::CCensorNode &wildcardCensor,
    bool enableHeaders, bool techMode,
    bool &passwordEnabled, UString &password,
    const CObjectVector<CProperty> *props,
    UInt64 &errors,
    UInt64 &numWarnings);

#endif
