/*
 * Routines for reading the ELF object file header information.
 *
 * Copyright (C) 1993 by Jack E. Veenstra (veenstra@cs.rochester.edu)
 * 
 * This file is part of MINT, a MIPS code interpreter and event generator
 * for parallel programs.
 * 
 * MINT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * MINT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MINT; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
//#define DEBUG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DARWIN
#include <elf.h>
#endif

#include "icode.h"
#include "globals.h"
#include "myelf.h"
#include "coff.h"
#include "symtab.h"

#include "mendian.h"

extern int32_t nm_cmp_name(const void *p1, const void *p2);

extern FILE *Fobj;              /* file descriptor for object file */
extern struct filehdr Fhdr;     /* need to fake this for COFF routines */

extern char *Stringtab;
extern namelist_ptr Nmlist;
extern int32_t Num_symbols;

static Elf32_Sym *Symtab;
static int32_t Numsyms;

/* prototype for functions used here */
extern void close_object();
extern void read_nmlist();
extern void read_linenum();

void
elf_read_hdrs(char *objfile)
{
  unsigned i;
  char *table = NULL;
  char *shstrtab = NULL;
  char *name = NULL;
  uint32_t addr, size, offset;
  uint32_t size_orig;
  uint32_t nscns, shsize, shoff;
  Elf32_Shdr shdr;
  Elf32_Ehdr Ehdr;
  int32_t have_mdebug = 0;

  close_object();
  Fobj = fopen(objfile, "r");
  if (Fobj == NULL) {
    perror(objfile);
    exit(1);
  }

  fseek(Fobj, 0, SEEK_SET);
  if (fread(&Ehdr, sizeof(Elf32_Ehdr), 1, Fobj) < 1)
    fatal("elf_read_hdrs: could not read ELF file header\n");

  EndianElf32_Ehdr(&Ehdr);

  if (Ehdr.e_ident[0] != 0x7f ||
      Ehdr.e_ident[1] != 'E' ||
      Ehdr.e_ident[2] != 'L' ||
      Ehdr.e_ident[3] != 'F')
    fatal("elf_read_hdrs: bad ELF file header\n");


  /* check that flags represents a static executable */
  if (Ehdr.e_type != ET_EXEC &&
      (Ehdr.e_flags & EF_MIPS_PIC)  &&
      (Ehdr.e_flags & EF_MIPS_CPIC)  )
    fatal("elf_read_hdrs: object file must be a statically linked executable type(%d) arch(0x%x)\n"
          ,Ehdr.e_type,Ehdr.e_flags);
#if 0
  if ((Ehdr.e_flags & EF_MIPS_ARCH) != EF_MIPS_ARCH_1)
    fatal("elf_read_hdrs: executable must be mips1\n");
#endif
 
  if (Ehdr.e_machine != EM_MIPS)
    fatal("elf_read_hdrs: object file must be for the MIPS machine\n");

  /* number of section headers, and the size of each one */
  nscns = Ehdr.e_shnum;
  shsize = Ehdr.e_shentsize;

  /* file offset of the start of the section header table */
  shoff = Ehdr.e_shoff;
    
  /* Loop through the section headers, searching for the section
   * header string table. Also look for the MDEBUG section.
   */
  for (i = 0; i < nscns; i++) {
    fseek(Fobj, shoff + i * shsize, SEEK_SET);
    if (fread(&shdr, sizeof(Elf32_Shdr), 1, Fobj) < 1)
      fatal("elf_read_hdrs: could not read section header %d\n", i);

    EndianElf32_Shdr(&shdr);

    /* If it is a string table, it might be the section header string
     * table, but we have to read it to be sure.
     */
    if (shdr.sh_type == SHT_STRTAB) {
      size = shdr.sh_size;
      if (shdr.sh_name >= size)
        continue;

      /* allocate space for the string table */
      table = (char *) malloc(size);
      fseek(Fobj, shdr.sh_offset, SEEK_SET);
      if (fread(table, size, 1, Fobj) < 1)
        fatal("elf_read_hdrs: could not read string table\n");
      if (strcmp(&table[shdr.sh_name], ".shstrtab") == 0) {
        shstrtab = table;
      } else
        free(table);
    } else if (shdr.sh_type == SHT_MDEBUG) {
      have_mdebug = 1;
    }
  }

  Text_start = 0;
  Text_entry = Ehdr.e_entry;
    
  Text_size = 0;
  Data_start = 0;
  Data_size = 0;
  Bss_start = 0;
  Bss_size = 0;

  /* loop through the section headers */
  for (i = 0; i < nscns; i++) {
    fseek(Fobj, shoff + i * shsize, SEEK_SET);
    if (fread(&shdr, sizeof(Elf32_Shdr), 1, Fobj) < 1)
      fatal("elf_read_hdrs: could not read section header %d\n", i);

    EndianElf32_Shdr(&shdr);

#ifdef DEBUG_HEADERS
    printf("name: %d (%s), type %d, flags 0x%x, addr 0x%x offset 0x%x size 0x%x (%d), link %d\n",
           shdr.sh_name, &shstrtab[shdr.sh_name],
           shdr.sh_type, shdr.sh_flags,
           shdr.sh_addr, shdr.sh_offset,
           shdr.sh_size, shdr.sh_size, shdr.sh_link);
#endif

    name = &shstrtab[shdr.sh_name];
    addr = shdr.sh_addr;
    offset = shdr.sh_offset;
    size_orig = shdr.sh_size;

    /* round the size up to a 16-byte boundary */
    /*  size = (size_orig + 0xf) & ~0xf; */
    size = size_orig;

    if (strcmp(name, ".strtab") == 0) {
        /* If the mdebug section exists, then use the string table
         * from there instead.
         */
        if (have_mdebug)
            continue;

        /* allocate space for the string table */
        table = (char *) malloc(size_orig);
        fseek(Fobj, offset, SEEK_SET);
        if (fread(table, size_orig, 1, Fobj) < 1)
            fatal("elf_read_hdrs: could not read string table\n");
        Stringtab = table;

        continue;
    } else if (strcmp(name, ".symtab") == 0) {
        int32_t i;
          
        /* If the mdebug section exists, then use the symbol table
         * from there instead.
         */
        if (have_mdebug)
            continue;
          
        /* allocate space for the symbol table */
        Symtab = (Elf32_Sym *) malloc(size_orig);
        fseek(Fobj, offset, SEEK_SET);
        if (fread(Symtab, size_orig, 1, Fobj) < 1)
            fatal("elf_read_hdrs: could not read symbol table\n");
        Numsyms = size_orig / shdr.sh_entsize;
        for(i=0;i<Numsyms;i++){
            EndianElf32_Sym(&Symtab[i]);
        }

        continue;
    } else if (strcmp(name, ".mdebug") == 0) {
        /* The .mdebug section looks the same as COFF, starting with
         * the HDDR structure. The COFF routines that read the namelist
         * and line number information seek to the file offset
         * specified in the Fhdr filehdr structure. This does not exist
         * in ELF so we have to fake it.
         */
        Fhdr.f_symptr = offset;
        read_nmlist();
        read_linenum();

        continue;
    } else {
        //fprintf(stderr,"elf: Unknown section [%s]\n",name);
    }
                
    /* This code allows the sections to appear in any order in the object
     * file. The in-memory copies of the .text, .init, and .fini
     * sections must be contiguous, but the sections can be placed
     * in any order in memory.
     *
     * The data and bss sections can also be placed in any order in
     * memory but must also be contiguous. The only exception is that
     * read-only data can be placed independently from the other
     * sections.
     */

    if (shdr.sh_type == SHT_PROGBITS) {
        // possibly text/data section

        if (shdr.sh_flags == 0x6) {
            //
            // code section
            //
            if (Text_start == 0) {
                Text_seek = offset;
                Text_start = addr;
                Text_size = size;
            } else if (addr == Text_start + Text_size) {
                Text_size += size;
            }
            else {
#ifdef DEBUG
                fprintf(stderr,
                        "%s section start address mis-match (%x vs %x)"
                        " => to adjust size\n",
                        &shstrtab[shdr.sh_name],
                        addr, (Text_start + Text_size));
#endif
                Text_size = addr + size - Text_start;
            }
        } else if (shdr.sh_flags == 0x2) {
            //
            // data section (read-only)
            //
            if (Data_start == 0) {
                /* Read-only data comes first in the list of contiguous
                 * data sections.
                 */
                Data_seek = offset;
                Data_start = addr;
                Data_size = size;

                Rdata_start = addr;
                Rdata_size = size;
            } else if (addr == Data_start + Data_size) {
                /* Read-only data is contiguous with the other
                 * data sections, but does not come first.
                 */
                Data_size += size;
                Rdata_size += size;
            } else {
                Data_size = addr + size - Data_start;
                Rdata_size =  addr + size - Data_start;
            }
        }
        else if (shdr.sh_flags == 0x3 ||
                 shdr.sh_flags == 0x10000003) {
            //
            // data section (read-write)
            //
            if (Data_start == 0) {
                Data_seek = offset;
                Data_start = addr;
                Data_size = size;
            } else if (addr == Data_start + Data_size) {
                Data_size += size;
            } else {
#ifdef DEBUG
                fprintf(stderr,
                        "%s section start address mis-match (%x vs %x)"
                        " => to adjust size\n",
                        &shstrtab[shdr.sh_name],
                        addr, (Data_start + Data_size));
#endif
                Data_size = addr + size - Data_start;
            }
        }

    }
    else if (shdr.sh_type == SHT_NOBITS) {
        // possibly bss section

        if (shdr.sh_flags == 0x3) {
            //
            // bss section (read-write)
            //
            
            if (Bss_start == 0) {
                Bss_start = addr;
                Bss_size = size;
            } else if (addr == Bss_start + Bss_size) {
                Bss_size += size;
            }
            else {
#ifdef DEBUG
                fprintf(stderr,
                        "%s section start address mis-match (%x vs %x)"
                        " => to adjust size\n",
                        &shstrtab[shdr.sh_name],
                        addr, (Bss_start + Bss_size));
#endif
                Bss_size = addr + size - Bss_start;
            }
        }
    }

  }

#ifdef  DEBUG
  printf("text: start=%08lx size=%08lx\n", Text_start, Text_size);
  printf("data: start=%08lx size=%08lx\n", Data_start, Data_size);
  printf("rdata: start=%08lx size=%08lx\n", Rdata_start, Rdata_size);
  printf("bss: start=%08lx size=%08lx\n", Bss_start, Bss_size);
#endif

  if (!have_mdebug)
      elf_read_nmlist();
  Text_size = Text_size / 4;
  Text_end = Text_start + Text_size * 4;
}

void
elf_read_nmlist()
{
    int32_t ent, bind, type, nm_next, size;

    /* allocate space for the namelist pointers and entries */
    size = Numsyms * sizeof(namelist_t);
    Nmlist = (namelist_ptr) malloc(size);

    nm_next = 0;
    for (ent = 1; ent < Numsyms; ent++) {
        type = Symtab[ent].st_info & 0xf;
        bind = Symtab[ent].st_info >> 4;
        if ((bind == STB_GLOBAL || bind == STB_WEAK) ||
            (type == STT_OBJECT || type == STT_FUNC)) {
            Nmlist[nm_next].n_value = Symtab[ent].st_value;
            Nmlist[nm_next].n_type = type;
            Nmlist[nm_next].n_name = &Stringtab[Symtab[ent].st_name];
            nm_next++;
        }
#if 0
        printf("%3d: name: %30s, value: 0x%08x, size: %6d, info: 0x%02x, section: %d\n",
               ent,
               &Stringtab[Symtab[ent].st_name],
               Symtab[ent].st_value, Symtab[ent].st_size,
               Symtab[ent].st_info, Symtab[ent].st_shndx);
#endif
    }
    Num_symbols = nm_next;

    /* sort namelist into alphabetical order for fast searching */
    qsort((char *) Nmlist, Num_symbols, sizeof(namelist_t), nm_cmp_name);
}
