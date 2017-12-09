
## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

## Set the default command-line flags for the C preprocessor to the value
## obtained from pkg-config via PKG_CHECK_MODULES in configure.ac.  These
## flags are passed to the compiler for both C and C++, in addition to the
## language-specific options.
AM_CPPFLAGS = $(P7ZR_CFLAGS) \
    -I/home/giuspen/Software/git_cherrytree/sandbox/gtkmm/7zr/src/CPP/myWindows \
    -I/home/giuspen/Software/git_cherrytree/sandbox/gtkmm/7zr/src/CPP \
    -I/home/giuspen/Software/git_cherrytree/sandbox/gtkmm/7zr/src/CPP/include_windows \
    -DUSE_LIB7Z_DLL \
    -D_FILE_OFFSET_BITS=64 \
    -D_LARGEFILE_SOURCE \
    -D_REENTRANT \
    -DENV_UNIX \
    -D_NO_CRYPTO \
    -DBREAK_HANDLER \
    -DUNICODE \
    -D_UNICODE \
    -DUNIX_USE_WIN_FILE

## Define an executable target, which will be installed into the
## directory named by the predefined variable $(bindir).
bin_PROGRAMS = p7zr

## Define the list of source files for the target. The file extension
## .c is recognized by Automake, and causes it to produce rules which invoke
## the C compiler to produce an object file (.o) from each source file. The
## header files (.h) do not result in object files by themselves, but will be
## included in distribution archives of the project.
p7zr_SOURCES = \
    src/C/7zCrc.c \
    src/C/7zCrcOpt.c \
    src/C/7zStream.c \
    src/C/Alloc.c \
    src/C/Bcj2.c \
    src/C/Bcj2Enc.c \
    src/C/Bra.c \
    src/C/Bra86.c \
    src/C/BraIA64.c \
    src/C/CpuArch.c \
    src/C/Delta.c \
    src/C/LzFind.c \
    src/C/LzFindMt.c \
    src/C/Lzma2Dec.c \
    src/C/Lzma2Enc.c \
    src/C/LzmaDec.c \
    src/C/LzmaEnc.c \
    src/C/MtCoder.c \
    src/C/Sha256.c \
    src/C/Threads.c \
    src/C/Xz.c \
    src/C/XzCrc64.c \
    src/C/XzCrc64Opt.c \
    src/C/XzDec.c \
    src/C/XzEnc.c \
    src/C/XzIn.c \
    src/CPP/7zip/Archive/7z/7zCompressionMode.cpp \
    src/CPP/7zip/Archive/7z/7zDecode.cpp \
    src/CPP/7zip/Archive/7z/7zEncode.cpp \
    src/CPP/7zip/Archive/7z/7zExtract.cpp \
    src/CPP/7zip/Archive/7z/7zFolderInStream.cpp \
    src/CPP/7zip/Archive/7z/7zHandler.cpp \
    src/CPP/7zip/Archive/7z/7zHandlerOut.cpp \
    src/CPP/7zip/Archive/7z/7zHeader.cpp \
    src/CPP/7zip/Archive/7z/7zIn.cpp \
    src/CPP/7zip/Archive/7z/7zOut.cpp \
    src/CPP/7zip/Archive/7z/7zProperties.cpp \
    src/CPP/7zip/Archive/7z/7zRegister.cpp \
    src/CPP/7zip/Archive/7z/7zSpecStream.cpp \
    src/CPP/7zip/Archive/7z/7zUpdate.cpp \
    src/CPP/7zip/Archive/Common/CoderMixer2.cpp \
    src/CPP/7zip/Archive/Common/DummyOutStream.cpp \
    src/CPP/7zip/Archive/Common/HandlerOut.cpp \
    src/CPP/7zip/Archive/Common/InStreamWithCRC.cpp \
    src/CPP/7zip/Archive/Common/ItemNameUtils.cpp \
    src/CPP/7zip/Archive/Common/MultiStream.cpp \
    src/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp \
    src/CPP/7zip/Archive/Common/ParseProperties.cpp \
    src/CPP/7zip/Archive/LzmaHandler.cpp \
    src/CPP/7zip/Archive/SplitHandler.cpp \
    src/CPP/7zip/Archive/XzHandler.cpp \
    src/CPP/7zip/Common/CWrappers.cpp \
    src/CPP/7zip/Common/CreateCoder.cpp \
    src/CPP/7zip/Common/FilePathAutoRename.cpp \
    src/CPP/7zip/Common/FileStreams.cpp \
    src/CPP/7zip/Common/FilterCoder.cpp \
    src/CPP/7zip/Common/InBuffer.cpp \
    src/CPP/7zip/Common/InOutTempBuffer.cpp \
    src/CPP/7zip/Common/LimitedStreams.cpp \
    src/CPP/7zip/Common/MethodId.cpp \
    src/CPP/7zip/Common/MethodProps.cpp \
    src/CPP/7zip/Common/OffsetStream.cpp \
    src/CPP/7zip/Common/OutBuffer.cpp \
    src/CPP/7zip/Common/ProgressUtils.cpp \
    src/CPP/7zip/Common/PropId.cpp \
    src/CPP/7zip/Common/StreamBinder.cpp \
    src/CPP/7zip/Common/StreamObjects.cpp \
    src/CPP/7zip/Common/StreamUtils.cpp \
    src/CPP/7zip/Common/UniqBlocks.cpp \
    src/CPP/7zip/Common/VirtThread.cpp \
    src/CPP/7zip/Compress/Bcj2Coder.cpp \
    src/CPP/7zip/Compress/Bcj2Register.cpp \
    src/CPP/7zip/Compress/BcjCoder.cpp \
    src/CPP/7zip/Compress/BcjRegister.cpp \
    src/CPP/7zip/Compress/BranchMisc.cpp \
    src/CPP/7zip/Compress/BranchRegister.cpp \
    src/CPP/7zip/Compress/ByteSwap.cpp \
    src/CPP/7zip/Compress/CopyCoder.cpp \
    src/CPP/7zip/Compress/CopyRegister.cpp \
    src/CPP/7zip/Compress/DeltaFilter.cpp \
    src/CPP/7zip/Compress/Lzma2Decoder.cpp \
    src/CPP/7zip/Compress/Lzma2Encoder.cpp \
    src/CPP/7zip/Compress/Lzma2Register.cpp \
    src/CPP/7zip/Compress/LzmaDecoder.cpp \
    src/CPP/7zip/Compress/LzmaEncoder.cpp \
    src/CPP/7zip/Compress/LzmaRegister.cpp \
    src/CPP/7zip/UI/Common/ArchiveCommandLine.cpp \
    src/CPP/7zip/UI/Common/ArchiveExtractCallback.cpp \
    src/CPP/7zip/UI/Common/ArchiveOpenCallback.cpp \
    src/CPP/7zip/UI/Common/Bench.cpp \
    src/CPP/7zip/UI/Common/DefaultName.cpp \
    src/CPP/7zip/UI/Common/EnumDirItems.cpp \
    src/CPP/7zip/UI/Common/Extract.cpp \
    src/CPP/7zip/UI/Common/ExtractingFilePath.cpp \
    src/CPP/7zip/UI/Common/HashCalc.cpp \
    src/CPP/7zip/UI/Common/LoadCodecs.cpp \
    src/CPP/7zip/UI/Common/OpenArchive.cpp \
    src/CPP/7zip/UI/Common/PropIDUtils.cpp \
    src/CPP/7zip/UI/Common/SetProperties.cpp \
    src/CPP/7zip/UI/Common/SortUtils.cpp \
    src/CPP/7zip/UI/Common/TempFiles.cpp \
    src/CPP/7zip/UI/Common/Update.cpp \
    src/CPP/7zip/UI/Common/UpdateAction.cpp \
    src/CPP/7zip/UI/Common/UpdateCallback.cpp \
    src/CPP/7zip/UI/Common/UpdatePair.cpp \
    src/CPP/7zip/UI/Common/UpdateProduce.cpp \
    src/CPP/7zip/UI/Console/BenchCon.cpp \
    src/CPP/7zip/UI/Console/ConsoleClose.cpp \
    src/CPP/7zip/UI/Console/ExtractCallbackConsole.cpp \
    src/CPP/7zip/UI/Console/HashCon.cpp \
    src/CPP/7zip/UI/Console/List.cpp \
    src/CPP/7zip/UI/Console/Main.cpp \
    src/CPP/7zip/UI/Console/MainAr.cpp \
    src/CPP/7zip/UI/Console/OpenCallbackConsole.cpp \
    src/CPP/7zip/UI/Console/PercentPrinter.cpp \
    src/CPP/7zip/UI/Console/UpdateCallbackConsole.cpp \
    src/CPP/7zip/UI/Console/UserInputUtils.cpp \
    src/CPP/Common/CRC.cpp \
    src/CPP/Common/CommandLineParser.cpp \
    src/CPP/Common/CrcReg.cpp \
    src/CPP/Common/IntToString.cpp \
    src/CPP/Common/ListFileUtils.cpp \
    src/CPP/Common/MyString.cpp \
    src/CPP/Common/MyVector.cpp \
    src/CPP/Common/MyWindows.cpp \
    src/CPP/Common/NewHandler.cpp \
    src/CPP/Common/Sha256Reg.cpp \
    src/CPP/Common/StdInStream.cpp \
    src/CPP/Common/StdOutStream.cpp \
    src/CPP/Common/StringConvert.cpp \
    src/CPP/Common/StringToInt.cpp \
    src/CPP/Common/UTFConvert.cpp \
    src/CPP/Common/Wildcard.cpp \
    src/CPP/Common/XzCrc64Reg.cpp \
    src/CPP/Windows/ErrorMsg.cpp \
    src/CPP/Windows/FileDir.cpp \
    src/CPP/Windows/FileFind.cpp \
    src/CPP/Windows/FileIO.cpp \
    src/CPP/Windows/FileName.cpp \
    src/CPP/Windows/PropVariant.cpp \
    src/CPP/Windows/PropVariantConv.cpp \
    src/CPP/Windows/Synchronization.cpp \
    src/CPP/Windows/System.cpp \
    src/CPP/Windows/TimeUtils.cpp \
    src/CPP/myWindows/myAddExeFlag.cpp \
    src/CPP/myWindows/mySplitCommandLine.cpp \
    src/CPP/myWindows/wine_date_and_time.cpp

## Set the library dependencies for the "example" target to the value obtained
## from pkg-config via PKG_CHECK_MODULES in configure.ac.  These libraries are
## passed to the linker in addition to the other linker flags.
p7zr_LDADD = $(P7ZR_LIBS) -lpthread

## Define an independent executable script for inclusion in the distribution
## archive. It will not be installed on an end user's system, however.
dist_noinst_SCRIPTS = autogen.sh

dist_doc_DATA = README
