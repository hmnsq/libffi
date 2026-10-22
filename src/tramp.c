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

#அந்தர்பாவ <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#எனில் defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <pthread.h>
#அந்தர்பாவ <sys/mman.h>
#அந்தர்பாவ <tramp.h>
#ifdef __linux__
#அந்தர்பாவ <linux/limits.h>
#அந்தர்பாவ <linux/types.h>
#endif
#ifdef __CYGWIN__
#அந்தர்பாவ <limits.h>
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
வெறுமை __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

கட்டமைப்பு tramp;

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
கட்டமைப்பு tramp_table
{
  கட்டமைப்பு tramp_table *prev;
  கட்டமைப்பு tramp_table *next;
  வெறுமை *code_table;
  வெறுமை *parm_table;
  கட்டமைப்பு tramp *array;
  கட்டமைப்பு tramp *விடுவி;
  முழு nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
கட்டமைப்பு tramp_parm
{
  வெறுமை *data;
  வெறுமை *target;
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
கட்டமைப்பு tramp
{
  கட்டமைப்பு tramp *prev;
  கட்டமைப்பு tramp *next;
  கட்டமைப்பு tramp_table *table;
  வெறுமை *code;
  கட்டமைப்பு tramp_parm *parm;
};

எண்ணிக்கை tramp_globals_status {
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
கட்டமைப்பு tramp_globals
{
  முழு fd;
  off_t offset;
  வெறுமை *text;
  size_t map_size;
  size_t size;
  முழு ntramp;
  கட்டமைப்பு tramp_table *free_tables;
  முழு nfree_tables;
  எண்ணிக்கை tramp_globals_status status;
};

நிலையான கட்டமைப்பு tramp_globals tramp_globals;

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
நிலையான முழு tramp_table_alloc (வெறுமை);

#எனில் defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

நிலையான முழு
ffi_tramp_get_libffi (வெறுமை)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  எழுத்து file[PATH_MAX+1], வரி[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  முழு nfields, found;
  முழு open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, அளவு (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  எனில் (fp == வெற்று)
    திரும்பு 0;

  found = 0;
  போது (feof (fp) == 0) {
    எனில் (fgets (வரி, அளவு (வரி), fp) == 0)
      நிறுத்து;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (வரி,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    எனில் (nfields != 7)
      தொடர்;

    எனில் (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      நிறுத்து;
    }
  }
  fclose (fp);

  எனில் (!found)
    திரும்பு 0;

  tramp_globals.fd = open (file, open_flags);
  எனில் (tramp_globals.fd == -1)
    திரும்பு 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  எனில் (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      திரும்பு 0;
    }
  திரும்பு 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#எனில் defined (__linux__) || defined (__CYGWIN__)

நிலையான முழு
ffi_tramp_get_temp_file (வெறுமை)
{
  ssize_t மொத்தம்;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  மொத்தம் = லிக(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  எனில் (மொத்தம் >=0 && (size_t)மொத்தம் == tramp_globals.map_size && tramp_table_alloc ())
    திரும்பு 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  திரும்பு 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#எனில் defined (__linux__) || defined (__CYGWIN__)

நிலையான முழு
ffi_tramp_init_os (வெறுமை)
{
  எனில் (ffi_tramp_get_libffi ())
    திரும்பு 1;
  திரும்பு ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#எனில் defined (__linux__) || defined (__CYGWIN__)

நிலையான pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

நிலையான வெறுமை
ffi_tramp_lock(வெறுமை)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

நிலையான வெறுமை
ffi_tramp_unlock(வெறுமை)
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

#எனில் defined (__linux__) || defined (__CYGWIN__)

நிலையான முழு
tramp_table_map (கட்டமைப்பு tramp_table *table)
{
  எழுத்து *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (வெற்று, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  எனில் (addr == MAP_FAILED)
    திரும்பு 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  எனில் (table->code_table == MAP_FAILED)
    {
      (வெறுமை) munmap (addr, tramp_globals.map_size * 2);
      திரும்பு 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  திரும்பு 1;
}

நிலையான வெறுமை
tramp_table_unmap (கட்டமைப்பு tramp_table *table)
{
  (வெறுமை) munmap (table->code_table, tramp_globals.map_size);
  (வெறுமை) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
நிலையான முழு
ffi_tramp_init (வெறுமை)
{
  long page_size;

  எனில் (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    திரும்பு 1;

  எனில் (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    திரும்பு 0;

  எனில் (ffi_tramp_arch == வெற்று)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      திரும்பு 0;
    }

  tramp_globals.free_tables = வெற்று;
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
  எனில் (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      திரும்பு 0;
    }

  எனில் (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      திரும்பு 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  திரும்பு 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

நிலையான வெறுமை tramp_add (கட்டமைப்பு tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
நிலையான முழு
tramp_table_alloc (வெறுமை)
{
  கட்டமைப்பு tramp_table *table;
  கட்டமைப்பு tramp *tramp_array, *tramp;
  size_t size;
  எழுத்து *code, *parm;
  முழு i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  எனில் (tramp_globals.nfree_tables > 0)
    திரும்பு 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (அளவு (*table));
  எனில் (table == வெற்று)
    திரும்பு 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (அளவு (*tramp) * tramp_globals.ntramp);
  எனில் (tramp_array == வெற்று)
    செல் free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  எனில் (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      செல் free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->விடுவி = வெற்று;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  க்கு (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (கட்டமைப்பு tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  திரும்பு 1;

/* Failure */
free_tramp_array:
  விடுவி (tramp_array);
free_table:
  விடுவி (table);
  திரும்பு 0;
}

/*
 * Free a trampoline table.
 */
நிலையான வெறுமை
tramp_table_free (கட்டமைப்பு tramp_table *table)
{
  tramp_table_unmap (table);
  விடுவி (table->array);
  விடுவி (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
நிலையான வெறுமை
tramp_table_add (கட்டமைப்பு tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = வெற்று;
  எனில் (tramp_globals.free_tables != வெற்று)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
நிலையான வெறுமை
tramp_table_del (கட்டமைப்பு tramp_table *table)
{
  tramp_globals.nfree_tables--;
  எனில் (table->prev != வெற்று)
    table->prev->next = table->next;
  எனில் (table->next != வெற்று)
    table->next->prev = table->prev;
  எனில் (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
நிலையான வெறுமை
tramp_add (கட்டமைப்பு tramp *tramp)
{
  கட்டமைப்பு tramp_table *table = tramp->table;

  tramp->next = table->விடுவி;
  tramp->prev = வெற்று;
  எனில் (table->விடுவி != வெற்று)
    table->விடுவி->prev = tramp;
  table->விடுவி = tramp;
  table->nfree++;

  எனில் (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  எனில் (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
நிலையான வெறுமை
tramp_del (கட்டமைப்பு tramp *tramp)
{
  கட்டமைப்பு tramp_table *table = tramp->table;

  table->nfree--;
  எனில் (tramp->prev != வெற்று)
    tramp->prev->next = tramp->next;
  எனில் (tramp->next != வெற்று)
    tramp->next->prev = tramp->prev;
  எனில் (table->விடுவி == tramp)
    table->விடுவி = tramp->next;

  எனில் (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

முழு
ffi_tramp_is_supported(வெறுமை)
{
  முழு ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  திரும்பு ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
வெறுமை *
ffi_tramp_alloc (முழு flags)
{
  கட்டமைப்பு tramp *tramp;

  ffi_tramp_lock();

  எனில் (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      திரும்பு வெற்று;
    }

  எனில் (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      திரும்பு வெற்று;
    }

  tramp = tramp_globals.free_tables->விடுவி;
  tramp_del (tramp);

  ffi_tramp_unlock();

  திரும்பு tramp;
}

/*
 * Set the parameters for a trampoline.
 */
வெறுமை
ffi_tramp_set_parms (வெறுமை *arg, வெறுமை *target, வெறுமை *data)
{
  கட்டமைப்பு tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
வெறுமை *
ffi_tramp_get_addr (வெறுமை *arg)
{
  கட்டமைப்பு tramp *tramp = arg;
  வெறுமை *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  திரும்பு addr;
}

/*
 * Free a trampoline.
 */
வெறுமை
ffi_tramp_free (வெறுமை *arg)
{
  கட்டமைப்பு tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#இல்லையெனில் /* !FFI_EXEC_STATIC_TRAMP */

#அந்தர்பாவ <stddef.h>

முழு
ffi_tramp_is_supported(வெறுமை)
{
  திரும்பு 0;
}

வெறுமை *
ffi_tramp_alloc (முழு flags)
{
  திரும்பு வெற்று;
}

வெறுமை
ffi_tramp_set_parms (வெறுமை *arg, வெறுமை *target, வெறுமை *data)
{
}

வெறுமை *
ffi_tramp_get_addr (வெறுமை *arg)
{
  திரும்பு வெற்று;
}

வெறுமை
ffi_tramp_free (வெறுமை *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
