
## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CPPFLAGS = ${CHERRYTREE_CFLAGS} \
	-DCHERRYTREE_LOCALEDIR=\"${CHERRYTREE_LOCALEDIR}\" \
	-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\" \
	-I$(top_srcdir)/src/ct \
	-I$(top_srcdir)/tests \
	-I$(top_srcdir)/src/7za/CPP/myWindows \
	-I$(top_srcdir)/src/7za/CPP/myWindows \
	-I$(top_srcdir)/src/7za/CPP \
	-I$(top_srcdir)/src/7za/CPP/include_windows \
	-DUSE_LIB7Z_DLL \
	-D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE \
	-D_REENTRANT \
	-DBREAK_HANDLER \
	-DUNICODE \
	-D_UNICODE \
	-DUNIX_USE_WIN_FILE \
	-D_LIB_FOR_CHERRYTREE \
	-D_UNITTEST_DATA_DIR=\"${top_srcdir}/tests/data\" \
	-Wno-deprecated \
	-Wno-cpp

AM_CXXFLAGS = -std=c++17

bin_PROGRAMS = cherrytree

noinst_LIBRARIES = libp7za.a

check_PROGRAMS = run_tests

## Define the non executable data that needs to be installed
## and have a define tell to our software where that dir is
#appdatadir = $(datadir)/@PACKAGE@
#appdata_DATA = img/@PACKAGE@.glade
#AM_CXXFLAGS = -DDATADIR=\"$(datadir)\"

COMMON_SOURCES = \
	src/ct/ct_app.cc \
	src/ct/ct_main_win.cc \
	src/ct/ct_treestore.cc \
	src/ct/ct_codebox.cc \
	src/ct/ct_image.cc \
	src/ct/ct_table.cc \
	src/ct/ct_sqlite3_rw.cc \
	src/ct/ct_xml_rw.cc \
	src/ct/ct_config.cc \
	src/ct/ct_const.cc \
	src/ct/ct_misc_utils.cc \
	src/ct/ct_p7za_iface.cc \
	src/ct/icons.gresource.cc \
	src/ct/ct_menu.cc \
	src/ct/ct_pref_dlg.cc \
        src/ct/ct_actions_edit.cc \
        src/ct/ct_actions_find.cc \
	src/ct/ct_actions_format.cc \
	src/ct/ct_actions_tree.cc \
	src/ct/ct_actions_view.cc \
	src/ct/ct_dialogs.cc \
	src/ct/ct_list.cc \
        src/ct/ct_clipboard.cc \
        src/ct/ct_export2txt.cc \
        src/ct/ct_export2html.cc

cherrytree_SOURCES = \
	src/ct/ct_main.cc \
	${COMMON_SOURCES}

run_tests_SOURCES = \
	${COMMON_SOURCES} \
	tests/tests_misc_utils.cpp \
	tests/tests_tmp_n_p7zip.cpp

libp7za_a_SOURCES = \
	src/7za/C/7zCrc.c \
	src/7za/C/7zCrcOpt.c \
	src/7za/C/7zStream.c \
	src/7za/C/Aes.c \
	src/7za/C/Alloc.c \
	src/7za/C/Bcj2.c \
	src/7za/C/Bcj2Enc.c \
	src/7za/C/Bra.c \
	src/7za/C/Bra86.c \
	src/7za/C/BraIA64.c \
	src/7za/C/BwtSort.c \
	src/7za/C/CpuArch.c \
	src/7za/C/Delta.c \
	src/7za/C/HuffEnc.c \
	src/7za/C/LzFind.c \
	src/7za/C/LzFindMt.c \
	src/7za/C/Lzma2Dec.c \
	src/7za/C/Lzma2Enc.c \
	src/7za/C/LzmaDec.c \
	src/7za/C/LzmaEnc.c \
	src/7za/C/MtCoder.c \
	src/7za/C/Ppmd7.c \
	src/7za/C/Ppmd7Dec.c \
	src/7za/C/Ppmd7Enc.c \
	src/7za/C/Ppmd8.c \
	src/7za/C/Ppmd8Dec.c \
	src/7za/C/Ppmd8Enc.c \
	src/7za/C/Sha1.c \
	src/7za/C/Sha256.c \
	src/7za/C/Sort.c \
	src/7za/C/Threads.c \
	src/7za/C/Xz.c \
	src/7za/C/XzCrc64.c \
	src/7za/C/XzCrc64Opt.c \
	src/7za/C/XzDec.c \
	src/7za/C/XzEnc.c \
	src/7za/C/XzIn.c \
	src/7za/CPP/7zip/Archive/7z/7zDecode.cpp \
	src/7za/CPP/7zip/Archive/7z/7zEncode.cpp \
	src/7za/CPP/7zip/Archive/7z/7zExtract.cpp \
	src/7za/CPP/7zip/Archive/7z/7zFolderInStream.cpp \
	src/7za/CPP/7zip/Archive/7z/7zHandler.cpp \
	src/7za/CPP/7zip/Archive/7z/7zHandlerOut.cpp \
	src/7za/CPP/7zip/Archive/7z/7zHeader.cpp \
	src/7za/CPP/7zip/Archive/7z/7zIn.cpp \
	src/7za/CPP/7zip/Archive/7z/7zOut.cpp \
	src/7za/CPP/7zip/Archive/7z/7zProperties.cpp \
	src/7za/CPP/7zip/Archive/7z/7zRegister.cpp \
	src/7za/CPP/7zip/Archive/7z/7zSpecStream.cpp \
	src/7za/CPP/7zip/Archive/7z/7zUpdate.cpp \
	src/7za/CPP/7zip/Archive/Bz2Handler.cpp \
	src/7za/CPP/7zip/Archive/Cab/CabBlockInStream.cpp \
	src/7za/CPP/7zip/Archive/Cab/CabHandler.cpp \
	src/7za/CPP/7zip/Archive/Cab/CabHeader.cpp \
	src/7za/CPP/7zip/Archive/Cab/CabIn.cpp \
	src/7za/CPP/7zip/Archive/Cab/CabRegister.cpp \
	src/7za/CPP/7zip/Archive/Common/CoderMixer2.cpp \
	src/7za/CPP/7zip/Archive/Common/DummyOutStream.cpp \
	src/7za/CPP/7zip/Archive/Common/FindSignature.cpp \
	src/7za/CPP/7zip/Archive/Common/HandlerOut.cpp \
	src/7za/CPP/7zip/Archive/Common/InStreamWithCRC.cpp \
	src/7za/CPP/7zip/Archive/Common/ItemNameUtils.cpp \
	src/7za/CPP/7zip/Archive/Common/MultiStream.cpp \
	src/7za/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp \
	src/7za/CPP/7zip/Archive/GzHandler.cpp \
	src/7za/CPP/7zip/Archive/LzmaHandler.cpp \
	src/7za/CPP/7zip/Archive/PpmdHandler.cpp \
	src/7za/CPP/7zip/Archive/SplitHandler.cpp \
	src/7za/CPP/7zip/Archive/Tar/TarHandler.cpp \
	src/7za/CPP/7zip/Archive/Tar/TarHandlerOut.cpp \
	src/7za/CPP/7zip/Archive/Tar/TarHeader.cpp \
	src/7za/CPP/7zip/Archive/Tar/TarIn.cpp \
	src/7za/CPP/7zip/Archive/Tar/TarOut.cpp \
	src/7za/CPP/7zip/Archive/Tar/TarRegister.cpp \
	src/7za/CPP/7zip/Archive/Tar/TarUpdate.cpp \
	src/7za/CPP/7zip/Archive/XzHandler.cpp \
	src/7za/CPP/7zip/Archive/ZHandler.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipAddCommon.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipHandler.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipHandlerOut.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipIn.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipItem.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipOut.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipRegister.cpp \
	src/7za/CPP/7zip/Archive/Zip/ZipUpdate.cpp \
	src/7za/CPP/7zip/Common/CWrappers.cpp \
	src/7za/CPP/7zip/Common/CreateCoder.cpp \
	src/7za/CPP/7zip/Common/FilePathAutoRename.cpp \
	src/7za/CPP/7zip/Common/FileStreams.cpp \
	src/7za/CPP/7zip/Common/FilterCoder.cpp \
	src/7za/CPP/7zip/Common/InBuffer.cpp \
	src/7za/CPP/7zip/Common/InOutTempBuffer.cpp \
	src/7za/CPP/7zip/Common/LimitedStreams.cpp \
	src/7za/CPP/7zip/Common/MemBlocks.cpp \
	src/7za/CPP/7zip/Common/MethodProps.cpp \
	src/7za/CPP/7zip/Common/OffsetStream.cpp \
	src/7za/CPP/7zip/Common/OutBuffer.cpp \
	src/7za/CPP/7zip/Common/OutMemStream.cpp \
	src/7za/CPP/7zip/Common/ProgressMt.cpp \
	src/7za/CPP/7zip/Common/ProgressUtils.cpp \
	src/7za/CPP/7zip/Common/PropId.cpp \
	src/7za/CPP/7zip/Common/StreamBinder.cpp \
	src/7za/CPP/7zip/Common/StreamObjects.cpp \
	src/7za/CPP/7zip/Common/StreamUtils.cpp \
	src/7za/CPP/7zip/Common/UniqBlocks.cpp \
	src/7za/CPP/7zip/Common/VirtThread.cpp \
	src/7za/CPP/7zip/Compress/BZip2Crc.cpp \
	src/7za/CPP/7zip/Compress/BZip2Decoder.cpp \
	src/7za/CPP/7zip/Compress/BZip2Encoder.cpp \
	src/7za/CPP/7zip/Compress/BZip2Register.cpp \
	src/7za/CPP/7zip/Compress/Bcj2Coder.cpp \
	src/7za/CPP/7zip/Compress/Bcj2Register.cpp \
	src/7za/CPP/7zip/Compress/BcjCoder.cpp \
	src/7za/CPP/7zip/Compress/BcjRegister.cpp \
	src/7za/CPP/7zip/Compress/BitlDecoder.cpp \
	src/7za/CPP/7zip/Compress/BranchMisc.cpp \
	src/7za/CPP/7zip/Compress/BranchRegister.cpp \
	src/7za/CPP/7zip/Compress/ByteSwap.cpp \
	src/7za/CPP/7zip/Compress/CopyCoder.cpp \
	src/7za/CPP/7zip/Compress/CopyRegister.cpp \
	src/7za/CPP/7zip/Compress/Deflate64Register.cpp \
	src/7za/CPP/7zip/Compress/DeflateDecoder.cpp \
	src/7za/CPP/7zip/Compress/DeflateEncoder.cpp \
	src/7za/CPP/7zip/Compress/DeflateRegister.cpp \
	src/7za/CPP/7zip/Compress/DeltaFilter.cpp \
	src/7za/CPP/7zip/Compress/ImplodeDecoder.cpp \
	src/7za/CPP/7zip/Compress/ImplodeHuffmanDecoder.cpp \
	src/7za/CPP/7zip/Compress/LzOutWindow.cpp \
	src/7za/CPP/7zip/Compress/Lzma2Decoder.cpp \
	src/7za/CPP/7zip/Compress/Lzma2Encoder.cpp \
	src/7za/CPP/7zip/Compress/Lzma2Register.cpp \
	src/7za/CPP/7zip/Compress/LzmaDecoder.cpp \
	src/7za/CPP/7zip/Compress/LzmaEncoder.cpp \
	src/7za/CPP/7zip/Compress/LzmaRegister.cpp \
	src/7za/CPP/7zip/Compress/LzxDecoder.cpp \
	src/7za/CPP/7zip/Compress/PpmdDecoder.cpp \
	src/7za/CPP/7zip/Compress/PpmdEncoder.cpp \
	src/7za/CPP/7zip/Compress/PpmdRegister.cpp \
	src/7za/CPP/7zip/Compress/PpmdZip.cpp \
	src/7za/CPP/7zip/Compress/QuantumDecoder.cpp \
	src/7za/CPP/7zip/Compress/ShrinkDecoder.cpp \
	src/7za/CPP/7zip/Compress/ZDecoder.cpp \
	src/7za/CPP/7zip/Crypto/7zAes.cpp \
	src/7za/CPP/7zip/Crypto/7zAesRegister.cpp \
	src/7za/CPP/7zip/Crypto/HmacSha1.cpp \
	src/7za/CPP/7zip/Crypto/MyAes.cpp \
	src/7za/CPP/7zip/Crypto/MyAesReg.cpp \
	src/7za/CPP/7zip/Crypto/Pbkdf2HmacSha1.cpp \
	src/7za/CPP/7zip/Crypto/RandGen.cpp \
	src/7za/CPP/7zip/Crypto/WzAes.cpp \
	src/7za/CPP/7zip/Crypto/ZipCrypto.cpp \
	src/7za/CPP/7zip/Crypto/ZipStrong.cpp \
	src/7za/CPP/7zip/UI/Common/ArchiveCommandLine.cpp \
	src/7za/CPP/7zip/UI/Common/ArchiveExtractCallback.cpp \
	src/7za/CPP/7zip/UI/Common/ArchiveOpenCallback.cpp \
	src/7za/CPP/7zip/UI/Common/Bench.cpp \
	src/7za/CPP/7zip/UI/Common/DefaultName.cpp \
	src/7za/CPP/7zip/UI/Common/EnumDirItems.cpp \
	src/7za/CPP/7zip/UI/Common/Extract.cpp \
	src/7za/CPP/7zip/UI/Common/ExtractingFilePath.cpp \
	src/7za/CPP/7zip/UI/Common/HashCalc.cpp \
	src/7za/CPP/7zip/UI/Common/LoadCodecs.cpp \
	src/7za/CPP/7zip/UI/Common/OpenArchive.cpp \
	src/7za/CPP/7zip/UI/Common/PropIDUtils.cpp \
	src/7za/CPP/7zip/UI/Common/SetProperties.cpp \
	src/7za/CPP/7zip/UI/Common/SortUtils.cpp \
	src/7za/CPP/7zip/UI/Common/TempFiles.cpp \
	src/7za/CPP/7zip/UI/Common/Update.cpp \
	src/7za/CPP/7zip/UI/Common/UpdateAction.cpp \
	src/7za/CPP/7zip/UI/Common/UpdateCallback.cpp \
	src/7za/CPP/7zip/UI/Common/UpdatePair.cpp \
	src/7za/CPP/7zip/UI/Common/UpdateProduce.cpp \
	src/7za/CPP/7zip/UI/Console/BenchCon.cpp \
	src/7za/CPP/7zip/UI/Console/ConsoleClose.cpp \
	src/7za/CPP/7zip/UI/Console/ExtractCallbackConsole.cpp \
	src/7za/CPP/7zip/UI/Console/HashCon.cpp \
	src/7za/CPP/7zip/UI/Console/List.cpp \
	src/7za/CPP/7zip/UI/Console/Main.cpp \
	src/7za/CPP/7zip/UI/Console/MainAr.cpp \
	src/7za/CPP/7zip/UI/Console/OpenCallbackConsole.cpp \
	src/7za/CPP/7zip/UI/Console/PercentPrinter.cpp \
	src/7za/CPP/7zip/UI/Console/UpdateCallbackConsole.cpp \
	src/7za/CPP/Common/CRC.cpp \
	src/7za/CPP/Common/CommandLineParser.cpp \
	src/7za/CPP/Common/CrcReg.cpp \
	src/7za/CPP/Common/IntToString.cpp \
	src/7za/CPP/Common/ListFileUtils.cpp \
	src/7za/CPP/Common/MyString.cpp \
	src/7za/CPP/Common/MyWindows.cpp \
	src/7za/CPP/Common/Sha1Reg.cpp \
	src/7za/CPP/Common/Sha256Reg.cpp \
	src/7za/CPP/Common/StdInStream.cpp \
	src/7za/CPP/Common/StdOutStream.cpp \
	src/7za/CPP/Common/StringConvert.cpp \
	src/7za/CPP/Common/StringToInt.cpp \
	src/7za/CPP/Common/UTFConvert.cpp \
	src/7za/CPP/Common/Wildcard.cpp \
	src/7za/CPP/Common/XzCrc64Reg.cpp \
	src/7za/CPP/Windows/ErrorMsg.cpp \
	src/7za/CPP/Windows/FileDir.cpp \
	src/7za/CPP/Windows/FileFind.cpp \
	src/7za/CPP/Windows/FileIO.cpp \
	src/7za/CPP/Windows/FileName.cpp \
	src/7za/CPP/Windows/PropVariant.cpp \
	src/7za/CPP/Windows/PropVariantConv.cpp \
	src/7za/CPP/Windows/Synchronization.cpp \
	src/7za/CPP/Windows/System.cpp \
	src/7za/CPP/Windows/TimeUtils.cpp \
	src/7za/CPP/myWindows/myAddExeFlag.cpp \
	src/7za/CPP/myWindows/mySplitCommandLine.cpp \
	src/7za/CPP/myWindows/wine_date_and_time.cpp

cherrytree_LDADD = ${CHERRYTREE_LIBS} libp7za.a

cherrytree_LDFLAGS = -Wl,--whole-archive $(top_srcdir)/libp7za.a -Wl,--no-whole-archive -lpthread

run_tests_LDADD = ${CHERRYTREE_LIBS} libp7za.a

run_tests_LDFLAGS = -Wl,--whole-archive $(top_srcdir)/libp7za.a -Wl,--no-whole-archive -lpthread


dist_noinst_SCRIPTS = autogen.sh

dist_doc_DATA = README


SUBDIRS = po


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = $(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache
