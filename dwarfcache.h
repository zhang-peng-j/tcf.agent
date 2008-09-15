/*******************************************************************************
 * Copyright (c) 2006, 2008 Wind River Systems, Inc. and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 * The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *     Wind River Systems - initial API and implementation
 *******************************************************************************/

/*
 * This module implements caching of DWARF debug information.
 *
 * Cached data stays in memory at least until end of the current event dispatch cycle.
 * To lock data for longer period of time clients can use ELF_File.ref_cnt.
 *
 * Functions in this module use exceptions to report errors, see exceptions.h
 */
#ifndef D_dwarfcache
#define D_dwarfcache

#include "elf.h"

typedef struct FileInfo FileInfo;
typedef struct LocationInfo LocationInfo;
typedef struct ObjectInfo ObjectInfo;
typedef struct LineNumbersState LineNumbersState;
typedef struct CompUnit CompUnit;
typedef struct SymbolSection SymbolSection;
typedef struct DWARFCache DWARFCache;


struct FileInfo {
    char * mName;
    char * mDir;
    U4_T mModTime;
    U4_T mSize;
};

#define SYM_HASH_SIZE 1023

struct SymbolSection {
    ELF_File * mFile;
    unsigned mIndex;
    char * mStrPool;
    size_t mStrPoolSize;
    unsigned sym_cnt;
    Elf_Sym * mSymPool;    /* pointer to ELF section data: array of Elf32_Sym or Elf64_Sym */
    size_t mSymPoolSize;
    unsigned mSymbolHash[SYM_HASH_SIZE];
    unsigned * mHashNext;
};

struct LocationInfo {
    U1_T * mAddr;
    size_t mSize;
    U1_T mList;
};

struct ObjectInfo {
    ObjectInfo * mHashNext;
    ObjectInfo * mListNext;
    ObjectInfo * mSibling;
    ObjectInfo * mChildren;
    ObjectInfo * mParent;

    U2_T mTag;
    U8_T mID;
    U8_T mLowPC;
    U8_T mHighPC;
    U8_T mConstValue;
    U1_T * mConstValueAddr;
    size_t mConstValueSize;
    LocationInfo mLocation;
    LocationInfo mFrameBase;
    SymbolSection * mSymbolSection;
    U4_T mSymbol;
    U1_T mDeclaration;
    U1_T mPrototyped;
    U1_T mExternal;
    U1_T mBitStride;
    U1_T mOrdering;
    U2_T mEncoding;
    U8_T mSize;
    U4_T mLowBound;
    U4_T mLength;
    ObjectInfo * mType;
    ObjectInfo * mSpecification;
    CompUnit * mCompUnit;
    char * mName;
    U4_T mDeclFile;
    U4_T mDeclLine;
};

struct LineNumbersState {
    unsigned mFile;
    unsigned mLine;
    unsigned mColumn;
    ContextAddress mAddress;
    U1_T mISA;
    U1_T mIsStmt;
    U1_T mBasicBlock;
    U1_T mPrologueEnd;
    U1_T mEpilogueBegin;
    U1_T mEndSequence;
};

struct CompUnit {
    ELF_File * mFile;
    ELF_Section * mSection;

    U8_T mID;
    ContextAddress mLowPC;
    ContextAddress mHighPC;

    U2_T mVersion;
    U1_T mAddressSize;
    U8_T mDebugRangesOffs;
    U8_T mLineInfoOffs;
    char * mName;
    char * mDir;

    U4_T mFilesCnt;
    U4_T mFilesMax;
    FileInfo * mFiles;

    U4_T mDirsCnt;
    U4_T mDirsMax;
    char ** mDirs;

    U4_T mStatesCnt;
    U4_T mStatesMax;
    LineNumbersState * mStates;

    CompUnit * mBaseTypes;
    ObjectInfo * mChildren;
};

#define SYM_CACHE_MAGIC         0x84625490

struct DWARFCache {
    int magic;
    ELF_File * mFile;
    CompUnit ** mCompUnits;
    unsigned mCompUnitsCnt;
    ELF_Section * mDebugRanges;
    ELF_Section * mDebugARanges;
    ELF_Section * mDebugLine;
    ELF_Section * mDebugLoc;
    SymbolSection ** sym_sections;
    unsigned sym_sections_cnt;
    unsigned sym_sections_len;
    ObjectInfo ** mObjectHash;
    ObjectInfo * mObjectList;
    Elf_Sym ** mSymbolHash;
    unsigned mSymbolTableLen;
    DWARFCache * mLineInfoNext;
};

extern DWARFCache * get_dwarf_cache(ELF_File * file);
extern unsigned calc_symbol_name_hash(char * s);
extern void load_line_numbers(DWARFCache * cache, CompUnit * unit);
extern ObjectInfo * find_object(DWARFCache * cache, U8_T ID);

#endif
