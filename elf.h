/*******************************************************************************
 * Copyright (c) 2007, 2008 Wind River Systems, Inc. and others.
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
 * This module implements reading and caching of ELF files.
 */
#ifndef D_elf
#define D_elf

#include "mdep.h"
#include "context.h"

#if !defined(WIN32)
#  include <elf.h>
#endif

#if defined(WIN32)

#define EI_MAG0        0
#define EI_MAG1        1
#define EI_MAG2        2
#define EI_MAG3        3
#define EI_CLASS       4
#define EI_DATA        5
#define EI_VERSION     6
#define EI_OSABI       7
#define EI_ABIVERSION  8
#define EI_PAD         9
#define EI_NIDENT     16

#define ELFMAG0 0x7F
#define ELFMAG1  'E'
#define ELFMAG2  'L'
#define ELFMAG3  'F'
#define ELFMAG   "\177ELF"
#define SELFMAG  4

#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

#define ELFDATANONE  0
#define ELFDATA2LSB  1
#define ELFDATA2MSB  2

#define ET_NONE        0
#define ET_REL         1
#define ET_EXEC        2
#define ET_DYN         3
#define ET_CORE        4
#define ET_LOOS   0xFE00
#define ET_HIOS   0xFEFF
#define ET_LOPROC 0xFF00
#define ET_HIPROC 0xFFFF

#define EV_CURRENT     1

#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3

#define STB_LOCAL       0
#define STB_GLOBAL      1
#define STB_WEAK        2

#define STT_NOTYPE      0
#define STT_OBJECT      1
#define STT_FUNC        2
#define STT_SECTION     3
#define STT_FILE        4

typedef unsigned long  Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned long  Elf32_Off;
typedef signed   long  Elf32_Sword;
typedef unsigned long  Elf32_Word;

typedef struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct Elf32_Shdr {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off  sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct Elf32_Sym {
    Elf32_Word    st_name;
    Elf32_Addr    st_value;
    Elf32_Word    st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half    st_shndx;
} Elf32_Sym;

#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)

#endif

#if defined(_WRS_KERNEL) || defined(WIN32)

typedef uns64           Elf64_Addr;
typedef unsigned short  Elf64_Half;
typedef unsigned long   Elf64_Word;
typedef signed long     Elf64_Sword;
typedef uns64           Elf64_Xword;
typedef int64           Elf64_Sxword;
typedef uns64           Elf64_Off;
typedef unsigned short  Elf64_Section;
typedef Elf64_Half      Elf64_Versym;
typedef unsigned short  Elf64_Quarter;

typedef struct {
    Elf64_Word  st_name;
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Section st_shndx;
    Elf64_Addr  st_value;
    Elf64_Xword st_size;
} Elf64_Sym;

#define ELF64_ST_BIND(info)             ((info) >> 4)
#define ELF64_ST_TYPE(info)             ((info) & 0xf)

#endif

typedef struct Elf_Sym {
    union {
        Elf32_Sym Elf32;
        Elf64_Sym Elf64;
    } u;
} Elf_Sym;

typedef unsigned char U1_T;
typedef signed char I1_T;
typedef unsigned short U2_T;
typedef signed short I2_T;
typedef unsigned int U4_T;
typedef signed int I4_T;
typedef uns64 U8_T;
typedef int64 I8_T;

struct ELF_File;
struct ELF_Section;
typedef struct ELF_Section ELF_Section;
typedef struct ELF_File ELF_File;

struct ELF_File {
    ELF_File * next;
    U4_T ref_cnt;

    char * name;
    dev_t dev;
    ino_t ino;
    time_t mtime;
    int fd;

    int big_endian; /* 0 - least significant first, 1 - most significat first */
    int elf64;
    unsigned section_cnt;
    ELF_Section ** sections;
    char * str_pool;

    void * dwarf_io_cache;
    void * dwarf_dt_cache;

    int age;
    int listed;
};

struct ELF_Section {
    ELF_File * file;
    U4_T index;
    unsigned name_offset;
    char * name;
    void * data;
    U4_T type;
    U4_T flags;
    U8_T offset;
    U8_T size;
    U8_T addr;
    U4_T link;
    U4_T info;    
    
    void * mmap_addr;
    size_t mmap_size;
};

/*
 * Open ELF file for reading.
 * Same file can be opened mutiple times, each call to elf_open() increases reference counter.
 * File must be closed after usage by calling elf_close().
 * Returns the file descriptior on success. If error, returns NULL and sets errno.
 */
extern ELF_File * elf_open(char * file_name);

/*
 * Close ELF file.
 * Each call of elf_close() decrements reference counter.
 * The file will be kept in a cache for some time even after all references are closed.
 */
extern void elf_close(ELF_File * file);

/*
 * Iterate context ELF files that are mapped in context memory in given address range (inclusive).
 * Returns the file descriptior on success. If error, returns NULL and sets errno.
 */
extern ELF_File * elf_list_first(Context * ctx, ContextAddress addr0, ContextAddress addr1);
extern ELF_File * elf_list_next(Context * ctx);

/*
 * Finish iteration of context ELF files.
 * Clients should always call elf_list_done() after calling elf_list_first().
 */
extern void elf_list_done(Context * ctx);

/*
 * Load section data into memory.
 * section->data is set to section data address in memory.
 * Data will stay in memory at least until file is closed.
 * Returns zero on success. If error, returns -1 and sets errno.
 */
extern int elf_load(ELF_Section * section);

/*
 * Register ELF file close callback.
 * The callback is called each time an ELF file data is about to be disposed.
 * Service implementation can use the callback to deallocate
 * cached data related to the file.
 */
typedef void (*ELFCloseListener)(ELF_File *);
extern void elf_add_close_listener(ELFCloseListener listener);

#endif

