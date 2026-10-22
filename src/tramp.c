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

#ಅನ್ತರ್ಭಾವ <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#ಯದಿ defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ifdef __linux__
#ಅನ್ತರ್ಭಾವ <linux/limits.h>
#ಅನ್ತರ್ಭಾವ <linux/types.h>
#endif
#ifdef __CYGWIN__
#ಅನ್ತರ್ಭಾವ <limits.h>
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
ಶೂನ್ಯ __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

ಸಂರಚನಾ tramp;

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
ಸಂರಚನಾ tramp_table
{
  ಸಂರಚನಾ tramp_table *prev;
  ಸಂರಚನಾ tramp_table *next;
  ಶೂನ್ಯ *code_table;
  ಶೂನ್ಯ *parm_table;
  ಸಂರಚನಾ tramp *array;
  ಸಂರಚನಾ tramp *ಮುಕ್ತ;
  ಪೂರ್ಣಾಙ್ಕ nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
ಸಂರಚನಾ tramp_parm
{
  ಶೂನ್ಯ *data;
  ಶೂನ್ಯ *target;
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
ಸಂರಚನಾ tramp
{
  ಸಂರಚನಾ tramp *prev;
  ಸಂರಚನಾ tramp *next;
  ಸಂರಚನಾ tramp_table *table;
  ಶೂನ್ಯ *code;
  ಸಂರಚನಾ tramp_parm *parm;
};

ಗಣನಾ tramp_globals_status {
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
ಸಂರಚನಾ tramp_globals
{
  ಪೂರ್ಣಾಙ್ಕ fd;
  off_t offset;
  ಶೂನ್ಯ *text;
  size_t map_size;
  size_t size;
  ಪೂರ್ಣಾಙ್ಕ ntramp;
  ಸಂರಚನಾ tramp_table *free_tables;
  ಪೂರ್ಣಾಙ್ಕ nfree_tables;
  ಗಣನಾ tramp_globals_status status;
};

ಸ್ಥಿರ ಸಂರಚನಾ tramp_globals tramp_globals;

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tramp_table_alloc (ಶೂನ್ಯ);

#ಯದಿ defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_get_libffi (ಶೂನ್ಯ)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  ವರ್ಣ file[PATH_MAX+1], ಸಾಲು[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  ಪೂರ್ಣಾಙ್ಕ nfields, found;
  ಪೂರ್ಣಾಙ್ಕ open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, ಪರಿಮಾಣ (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  ಯದಿ (fp == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ 0;

  found = 0;
  ಯಾವತ್ (feof (fp) == 0) {
    ಯದಿ (fgets (ಸಾಲು, ಪರಿಮಾಣ (ಸಾಲು), fp) == 0)
      ಭಙ್ಗ;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (ಸಾಲು,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    ಯದಿ (nfields != 7)
      ಅನುವರ್ತ;

    ಯದಿ (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      ಭಙ್ಗ;
    }
  }
  fclose (fp);

  ಯದಿ (!found)
    ಪ್ರತ್ಯಾಗಮ 0;

  tramp_globals.fd = open (file, open_flags);
  ಯದಿ (tramp_globals.fd == -1)
    ಪ್ರತ್ಯಾಗಮ 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  ಯದಿ (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  ಪ್ರತ್ಯಾಗಮ 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#ಯದಿ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_get_temp_file (ಶೂನ್ಯ)
{
  ssize_t ಎಣಿಕೆ;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  ಎಣಿಕೆ = ಲಿಖ(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  ಯದಿ (ಎಣಿಕೆ >=0 && (size_t)ಎಣಿಕೆ == tramp_globals.map_size && tramp_table_alloc ())
    ಪ್ರತ್ಯಾಗಮ 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  ಪ್ರತ್ಯಾಗಮ 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#ಯದಿ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_init_os (ಶೂನ್ಯ)
{
  ಯದಿ (ffi_tramp_get_libffi ())
    ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತ್ಯಾಗಮ ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#ಯದಿ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_tramp_lock(ಶೂನ್ಯ)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_tramp_unlock(ಶೂನ್ಯ)
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

#ಯದಿ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tramp_table_map (ಸಂರಚನಾ tramp_table *table)
{
  ವರ್ಣ *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (ರಿಕ್ತ, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  ಯದಿ (addr == MAP_FAILED)
    ಪ್ರತ್ಯಾಗಮ 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  ಯದಿ (table->code_table == MAP_FAILED)
    {
      (ಶೂನ್ಯ) munmap (addr, tramp_globals.map_size * 2);
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tramp_table_unmap (ಸಂರಚನಾ tramp_table *table)
{
  (ಶೂನ್ಯ) munmap (table->code_table, tramp_globals.map_size);
  (ಶೂನ್ಯ) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_init (ಶೂನ್ಯ)
{
  long page_size;

  ಯದಿ (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    ಪ್ರತ್ಯಾಗಮ 1;

  ಯದಿ (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    ಪ್ರತ್ಯಾಗಮ 0;

  ಯದಿ (ffi_tramp_arch == ರಿಕ್ತ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ಪ್ರತ್ಯಾಗಮ 0;
    }

  tramp_globals.free_tables = ರಿಕ್ತ;
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
  ಯದಿ (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ಪ್ರತ್ಯಾಗಮ 0;
    }

  ಯದಿ (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      ಪ್ರತ್ಯಾಗಮ 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ಸ್ಥಿರ ಶೂನ್ಯ tramp_add (ಸಂರಚನಾ tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
tramp_table_alloc (ಶೂನ್ಯ)
{
  ಸಂರಚನಾ tramp_table *table;
  ಸಂರಚನಾ tramp *tramp_array, *tramp;
  size_t size;
  ವರ್ಣ *code, *parm;
  ಪೂರ್ಣಾಙ್ಕ i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  ಯದಿ (tramp_globals.nfree_tables > 0)
    ಪ್ರತ್ಯಾಗಮ 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (ಪರಿಮಾಣ (*table));
  ಯದಿ (table == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (ಪರಿಮಾಣ (*tramp) * tramp_globals.ntramp);
  ಯದಿ (tramp_array == ರಿಕ್ತ)
    ಗಚ್ಛ free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  ಯದಿ (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      ಗಚ್ಛ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->ಮುಕ್ತ = ರಿಕ್ತ;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  ಪ್ರತಿ (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (ಸಂರಚನಾ tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  ಪ್ರತ್ಯಾಗಮ 1;

/* Failure */
free_tramp_array:
  ಮುಕ್ತ (tramp_array);
free_table:
  ಮುಕ್ತ (table);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Free a trampoline table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_table_free (ಸಂರಚನಾ tramp_table *table)
{
  tramp_table_unmap (table);
  ಮುಕ್ತ (table->array);
  ಮುಕ್ತ (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_table_add (ಸಂರಚನಾ tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = ರಿಕ್ತ;
  ಯದಿ (tramp_globals.free_tables != ರಿಕ್ತ)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_table_del (ಸಂರಚನಾ tramp_table *table)
{
  tramp_globals.nfree_tables--;
  ಯದಿ (table->prev != ರಿಕ್ತ)
    table->prev->next = table->next;
  ಯದಿ (table->next != ರಿಕ್ತ)
    table->next->prev = table->prev;
  ಯದಿ (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_add (ಸಂರಚನಾ tramp *tramp)
{
  ಸಂರಚನಾ tramp_table *table = tramp->table;

  tramp->next = table->ಮುಕ್ತ;
  tramp->prev = ರಿಕ್ತ;
  ಯದಿ (table->ಮುಕ್ತ != ರಿಕ್ತ)
    table->ಮುಕ್ತ->prev = tramp;
  table->ಮುಕ್ತ = tramp;
  table->nfree++;

  ಯದಿ (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  ಯದಿ (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_del (ಸಂರಚನಾ tramp *tramp)
{
  ಸಂರಚನಾ tramp_table *table = tramp->table;

  table->nfree--;
  ಯದಿ (tramp->prev != ರಿಕ್ತ)
    tramp->prev->next = tramp->next;
  ಯದಿ (tramp->next != ರಿಕ್ತ)
    tramp->next->prev = tramp->prev;
  ಯದಿ (table->ಮುಕ್ತ == tramp)
    table->ಮುಕ್ತ = tramp->next;

  ಯದಿ (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_is_supported(ಶೂನ್ಯ)
{
  ಪೂರ್ಣಾಙ್ಕ ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  ಪ್ರತ್ಯಾಗಮ ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
ಶೂನ್ಯ *
ffi_tramp_alloc (ಪೂರ್ಣಾಙ್ಕ flags)
{
  ಸಂರಚನಾ tramp *tramp;

  ffi_tramp_lock();

  ಯದಿ (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

  ಯದಿ (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

  tramp = tramp_globals.free_tables->ಮುಕ್ತ;
  tramp_del (tramp);

  ffi_tramp_unlock();

  ಪ್ರತ್ಯಾಗಮ tramp;
}

/*
 * Set the parameters for a trampoline.
 */
ಶೂನ್ಯ
ffi_tramp_set_parms (ಶೂನ್ಯ *arg, ಶೂನ್ಯ *target, ಶೂನ್ಯ *data)
{
  ಸಂರಚನಾ tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
ಶೂನ್ಯ *
ffi_tramp_get_addr (ಶೂನ್ಯ *arg)
{
  ಸಂರಚನಾ tramp *tramp = arg;
  ಶೂನ್ಯ *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  ಪ್ರತ್ಯಾಗಮ addr;
}

/*
 * Free a trampoline.
 */
ಶೂನ್ಯ
ffi_tramp_free (ಶೂನ್ಯ *arg)
{
  ಸಂರಚನಾ tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#ಅನ್ಯಥಾ /* !FFI_EXEC_STATIC_TRAMP */

#ಅನ್ತರ್ಭಾವ <stddef.h>

ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_is_supported(ಶೂನ್ಯ)
{
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ *
ffi_tramp_alloc (ಪೂರ್ಣಾಙ್ಕ flags)
{
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಶೂನ್ಯ
ffi_tramp_set_parms (ಶೂನ್ಯ *arg, ಶೂನ್ಯ *target, ಶೂನ್ಯ *data)
{
}

ಶೂನ್ಯ *
ffi_tramp_get_addr (ಶೂನ್ಯ *arg)
{
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಶೂನ್ಯ
ffi_tramp_free (ಶೂನ್ಯ *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
