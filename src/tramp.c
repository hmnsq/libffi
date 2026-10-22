/* -----------------------------------------------------------------------
   tramp.c - Copyright (c) 2020 Madhavan T. Venkataraman
             Copyright (c) 2022, 2026 Anthony Green

   API and support functions for managing statically defined closure
   trampolines.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#അന്തര്ഭാവ <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#എങ്കിൽ defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <pthread.h>
#അന്തര്ഭാവ <sys/mman.h>
#അന്തര്ഭാവ <tramp.h>
#ifdef __linux__
#അന്തര്ഭാവ <linux/limits.h>
#അന്തര്ഭാവ <linux/types.h>
#endif
#ifdef __CYGWIN__
#അന്തര്ഭാവ <limits.h>
#endif
#endif

/*
 * Each architecture defines static code for a trampoline code table. The
 * trampoline code table is mapped into the address space of a process.
 *
 * The following architecture specific function returns:
 *
 *	- the address of the trampoline code table in the text segment
 *	- the size of each trampoline in the trampoline code table
 *	- the size of the mapping for the whole trampoline code table
 */
ശൂന്യം __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

ഘടന tramp;

/*
 * Trampoline table. Manages one trampoline code table and one trampoline
 * parameter table.
 *
 * prev, next	Links in the global trampoline table list.
 * code_table	Trampoline code table mapping.
 * parm_table	Trampoline parameter table mapping.
 * array	Array of trampolines malloced.
 * free		List of free trampolines.
 * nfree	Number of free trampolines.
 */
ഘടന tramp_table
{
  ഘടന tramp_table *prev;
  ഘടന tramp_table *next;
  ശൂന്യം *code_table;
  ശൂന്യം *parm_table;
  ഘടന tramp *array;
  ഘടന tramp *സ്വതന്ത്രം;
  പൂർണ്ണസംഖ്യ nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
ഘടന tramp_parm
{
  ശൂന്യം *data;
  ശൂന്യം *target;
};

/*
 * Trampoline structure for each trampoline.
 *
 * prev, next	Links in the trampoline free list of a trampoline table.
 * table	Trampoline table to which this trampoline belongs.
 * code		Address of this trampoline in the code table mapping.
 * parm		Address of this trampoline's parameters in the parameter
 *		table mapping.
 */
ഘടന tramp
{
  ഘടന tramp *prev;
  ഘടന tramp *next;
  ഘടന tramp_table *table;
  ശൂന്യം *code;
  ഘടന tramp_parm *parm;
};

എണ്ണൽ tramp_globals_status {
	TRAMP_GLOBALS_UNINITIALIZED = 0,
	TRAMP_GLOBALS_PASSED,
	TRAMP_GLOBALS_FAILED,
};

/*
 * Trampoline globals.
 *
 * fd
 *	File descriptor of binary file that contains the trampoline code table.
 * offset
 *	Offset of the trampoline code table in that file.
 * text
 *	Address of the trampoline code table in the text segment.
 * map_size
 *	Size of the trampoline code table mapping.
 * size
 *	Size of one trampoline in the trampoline code table.
 * ntramp
 *	Total number of trampolines in the trampoline code table.
 * free_tables
 *	List of trampoline tables that contain free trampolines.
 * nfree_tables
 *	Number of trampoline tables that contain free trampolines.
 * status
 *	Initialization status.
 */
ഘടന tramp_globals
{
  പൂർണ്ണസംഖ്യ fd;
  off_t offset;
  ശൂന്യം *text;
  size_t map_size;
  size_t size;
  പൂർണ്ണസംഖ്യ ntramp;
  ഘടന tramp_table *free_tables;
  പൂർണ്ണസംഖ്യ nfree_tables;
  എണ്ണൽ tramp_globals_status status;
};

സ്ഥിതം ഘടന tramp_globals tramp_globals;

/* --------------------- Trampoline File Initialization --------------------*/

/*
 * The trampoline file is the file used to map the trampoline code table into
 * the address space of a process. There are two ways to get this file:
 *
 * - From the OS. E.g., on Linux, /proc/<pid>/maps lists all the memory
 *   mappings for <pid>. For file-backed mappings, maps supplies the file name
 *   and the file offset. Using this, we can locate the mapping that maps
 *   libffi and get the path to the libffi binary. And, we can compute the
 *   offset of the trampoline code table within that binary.
 *
 * - Else, if we can create a temporary file, we can write the trampoline code
 *   table from the text segment into the temporary file.
 *
 * The first method is the preferred one. If the OS security subsystem
 * disallows mapping unsigned files with PROT_EXEC, then the second method
 * will fail.
 *
 * If an OS allows the trampoline code table in the text segment to be
 * directly remapped (e.g., MACH vm_remap ()), then we don't need the
 * trampoline file.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ tramp_table_alloc (ശൂന്യം);

#എങ്കിൽ defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_tramp_get_libffi (ശൂന്യം)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  അക്ഷരം file[PATH_MAX+1], വരി[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  പൂർണ്ണസംഖ്യ nfields, found;
  പൂർണ്ണസംഖ്യ open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, വലുപ്പം (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  എങ്കിൽ (fp == ശൂന്യ)
    തിരികെ 0;

  found = 0;
  ആകുമ്പോൾ (feof (fp) == 0) {
    എങ്കിൽ (fgets (വരി, വലുപ്പം (വരി), fp) == 0)
      നിർത്തുക;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (വരി,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    എങ്കിൽ (nfields != 7)
      തുടരുക;

    എങ്കിൽ (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      നിർത്തുക;
    }
  }
  fclose (fp);

  എങ്കിൽ (!found)
    തിരികെ 0;

  tramp_globals.fd = open (file, open_flags);
  എങ്കിൽ (tramp_globals.fd == -1)
    തിരികെ 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  എങ്കിൽ (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      തിരികെ 0;
    }
  തിരികെ 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#എങ്കിൽ defined (__linux__) || defined (__CYGWIN__)

സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_tramp_get_temp_file (ശൂന്യം)
{
  ssize_t എണ്ണം;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  എണ്ണം = ലിഖ(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  എങ്കിൽ (എണ്ണം >=0 && (size_t)എണ്ണം == tramp_globals.map_size && tramp_table_alloc ())
    തിരികെ 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  തിരികെ 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#എങ്കിൽ defined (__linux__) || defined (__CYGWIN__)

സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_tramp_init_os (ശൂന്യം)
{
  എങ്കിൽ (ffi_tramp_get_libffi ())
    തിരികെ 1;
  തിരികെ ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#എങ്കിൽ defined (__linux__) || defined (__CYGWIN__)

സ്ഥിതം pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

സ്ഥിതം ശൂന്യം
ffi_tramp_lock(ശൂന്യം)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

സ്ഥിതം ശൂന്യം
ffi_tramp_unlock(ശൂന്യം)
{
  pthread_mutex_unlock (&tramp_globals_mutex);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Memory Mapping ----------------------*/

/*
 * Create a trampoline code table mapping and a trampoline parameter table
 * mapping. The two mappings must be adjacent to each other for PC-relative
 * access.
 *
 * For each trampoline in the code table, there is a corresponding parameter
 * block in the parameter table. The size of the parameter block is the same
 * as the size of the trampoline. This means that the parameter block is at
 * a fixed offset from its trampoline making it easy for a trampoline to find
 * its parameters using PC-relative access.
 *
 * The parameter block will contain a struct tramp_parm. This means that
 * sizeof (struct tramp_parm) cannot exceed the size of a parameter block.
 */

#എങ്കിൽ defined (__linux__) || defined (__CYGWIN__)

സ്ഥിതം പൂർണ്ണസംഖ്യ
tramp_table_map (ഘടന tramp_table *table)
{
  അക്ഷരം *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (ശൂന്യ, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  എങ്കിൽ (addr == MAP_FAILED)
    തിരികെ 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  എങ്കിൽ (table->code_table == MAP_FAILED)
    {
      (ശൂന്യം) munmap (addr, tramp_globals.map_size * 2);
      തിരികെ 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  തിരികെ 1;
}

സ്ഥിതം ശൂന്യം
tramp_table_unmap (ഘടന tramp_table *table)
{
  (ശൂന്യം) munmap (table->code_table, tramp_globals.map_size);
  (ശൂന്യം) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_tramp_init (ശൂന്യം)
{
  long page_size;

  എങ്കിൽ (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    തിരികെ 1;

  എങ്കിൽ (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    തിരികെ 0;

  എങ്കിൽ (ffi_tramp_arch == ശൂന്യ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      തിരികെ 0;
    }

  tramp_globals.free_tables = ശൂന്യ;
  tramp_globals.nfree_tables = 0;

  /*
   * Get trampoline code table information from the architecture.
   */
  tramp_globals.text = ffi_tramp_arch (&tramp_globals.size,
    &tramp_globals.map_size);
  tramp_globals.ntramp = tramp_globals.map_size / tramp_globals.size;

  /*
   * The trampoline code table is a single, fixed-size mapping.  If the
   * system page size is larger than that mapping, the static trampoline
   * mechanism cannot be used.  Both values are invariant for the life of
   * the process, so cache the FAILED verdict rather than re-running the
   * whole initialization on every allocation.
   */
  page_size = sysconf (_SC_PAGESIZE);
  എങ്കിൽ (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      തിരികെ 0;
    }

  എങ്കിൽ (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      തിരികെ 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  തിരികെ 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

സ്ഥിതം ശൂന്യം tramp_add (ഘടന tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
tramp_table_alloc (ശൂന്യം)
{
  ഘടന tramp_table *table;
  ഘടന tramp *tramp_array, *tramp;
  size_t size;
  അക്ഷരം *code, *parm;
  പൂർണ്ണസംഖ്യ i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  എങ്കിൽ (tramp_globals.nfree_tables > 0)
    തിരികെ 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (വലുപ്പം (*table));
  എങ്കിൽ (table == ശൂന്യ)
    തിരികെ 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (വലുപ്പം (*tramp) * tramp_globals.ntramp);
  എങ്കിൽ (tramp_array == ശൂന്യ)
    പോകുക free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  എങ്കിൽ (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      പോകുക free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->സ്വതന്ത്രം = ശൂന്യ;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  വേണ്ടി (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (ഘടന tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  തിരികെ 1;

/* Failure */
free_tramp_array:
  സ്വതന്ത്രം (tramp_array);
free_table:
  സ്വതന്ത്രം (table);
  തിരികെ 0;
}

/*
 * Free a trampoline table.
 */
സ്ഥിതം ശൂന്യം
tramp_table_free (ഘടന tramp_table *table)
{
  tramp_table_unmap (table);
  സ്വതന്ത്രം (table->array);
  സ്വതന്ത്രം (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
സ്ഥിതം ശൂന്യം
tramp_table_add (ഘടന tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = ശൂന്യ;
  എങ്കിൽ (tramp_globals.free_tables != ശൂന്യ)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
സ്ഥിതം ശൂന്യം
tramp_table_del (ഘടന tramp_table *table)
{
  tramp_globals.nfree_tables--;
  എങ്കിൽ (table->prev != ശൂന്യ)
    table->prev->next = table->next;
  എങ്കിൽ (table->next != ശൂന്യ)
    table->next->prev = table->prev;
  എങ്കിൽ (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
സ്ഥിതം ശൂന്യം
tramp_add (ഘടന tramp *tramp)
{
  ഘടന tramp_table *table = tramp->table;

  tramp->next = table->സ്വതന്ത്രം;
  tramp->prev = ശൂന്യ;
  എങ്കിൽ (table->സ്വതന്ത്രം != ശൂന്യ)
    table->സ്വതന്ത്രം->prev = tramp;
  table->സ്വതന്ത്രം = tramp;
  table->nfree++;

  എങ്കിൽ (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  എങ്കിൽ (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
സ്ഥിതം ശൂന്യം
tramp_del (ഘടന tramp *tramp)
{
  ഘടന tramp_table *table = tramp->table;

  table->nfree--;
  എങ്കിൽ (tramp->prev != ശൂന്യ)
    tramp->prev->next = tramp->next;
  എങ്കിൽ (tramp->next != ശൂന്യ)
    tramp->next->prev = tramp->prev;
  എങ്കിൽ (table->സ്വതന്ത്രം == tramp)
    table->സ്വതന്ത്രം = tramp->next;

  എങ്കിൽ (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

പൂർണ്ണസംഖ്യ
ffi_tramp_is_supported(ശൂന്യം)
{
  പൂർണ്ണസംഖ്യ ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  തിരികെ ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
ശൂന്യം *
ffi_tramp_alloc (പൂർണ്ണസംഖ്യ flags)
{
  ഘടന tramp *tramp;

  ffi_tramp_lock();

  എങ്കിൽ (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      തിരികെ ശൂന്യ;
    }

  എങ്കിൽ (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      തിരികെ ശൂന്യ;
    }

  tramp = tramp_globals.free_tables->സ്വതന്ത്രം;
  tramp_del (tramp);

  ffi_tramp_unlock();

  തിരികെ tramp;
}

/*
 * Set the parameters for a trampoline.
 */
ശൂന്യം
ffi_tramp_set_parms (ശൂന്യം *arg, ശൂന്യം *target, ശൂന്യം *data)
{
  ഘടന tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
ശൂന്യം *
ffi_tramp_get_addr (ശൂന്യം *arg)
{
  ഘടന tramp *tramp = arg;
  ശൂന്യം *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  തിരികെ addr;
}

/*
 * Free a trampoline.
 */
ശൂന്യം
ffi_tramp_free (ശൂന്യം *arg)
{
  ഘടന tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#അല്ലെങ്കിൽ /* !FFI_EXEC_STATIC_TRAMP */

#അന്തര്ഭാവ <stddef.h>

പൂർണ്ണസംഖ്യ
ffi_tramp_is_supported(ശൂന്യം)
{
  തിരികെ 0;
}

ശൂന്യം *
ffi_tramp_alloc (പൂർണ്ണസംഖ്യ flags)
{
  തിരികെ ശൂന്യ;
}

ശൂന്യം
ffi_tramp_set_parms (ശൂന്യം *arg, ശൂന്യം *target, ശൂന്യം *data)
{
}

ശൂന്യം *
ffi_tramp_get_addr (ശൂന്യം *arg)
{
  തിരികെ ശൂന്യ;
}

ശൂന്യം
ffi_tramp_free (ശൂന്യം *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
