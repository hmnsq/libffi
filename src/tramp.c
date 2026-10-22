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

#અન્તર્ભાવ <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#જો defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <pthread.h>
#અન્તર્ભાવ <sys/mman.h>
#અન્તર્ભાવ <tramp.h>
#ifdef __linux__
#અન્તર્ભાવ <linux/limits.h>
#અન્તર્ભાવ <linux/types.h>
#endif
#ifdef __CYGWIN__
#અન્તર્ભાવ <limits.h>
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
શૂન્ય __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

સંરચના tramp;

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
સંરચના tramp_table
{
  સંરચના tramp_table *prev;
  સંરચના tramp_table *next;
  શૂન્ય *code_table;
  શૂન્ય *parm_table;
  સંરચના tramp *array;
  સંરચના tramp *મુક્ત;
  પૂર્ણાંક nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
સંરચના tramp_parm
{
  શૂન્ય *data;
  શૂન્ય *target;
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
સંરચના tramp
{
  સંરચના tramp *prev;
  સંરચના tramp *next;
  સંરચના tramp_table *table;
  શૂન્ય *code;
  સંરચના tramp_parm *parm;
};

ગણના tramp_globals_status {
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
સંરચના tramp_globals
{
  પૂર્ણાંક fd;
  off_t offset;
  શૂન્ય *text;
  size_t map_size;
  size_t size;
  પૂર્ણાંક ntramp;
  સંરચના tramp_table *free_tables;
  પૂર્ણાંક nfree_tables;
  ગણના tramp_globals_status status;
};

સ્થિર સંરચના tramp_globals tramp_globals;

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
સ્થિર પૂર્ણાંક tramp_table_alloc (શૂન્ય);

#જો defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

સ્થિર પૂર્ણાંક
ffi_tramp_get_libffi (શૂન્ય)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  અક્ષર file[PATH_MAX+1], લીટી[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  પૂર્ણાંક nfields, found;
  પૂર્ણાંક open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, કદ (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  જો (fp == ખાલી)
    પાછા 0;

  found = 0;
  જ્યારે (feof (fp) == 0) {
    જો (fgets (લીટી, કદ (લીટી), fp) == 0)
      તોડો;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (લીટી,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    જો (nfields != 7)
      ચાલુ;

    જો (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      તોડો;
    }
  }
  fclose (fp);

  જો (!found)
    પાછા 0;

  tramp_globals.fd = open (file, open_flags);
  જો (tramp_globals.fd == -1)
    પાછા 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  જો (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      પાછા 0;
    }
  પાછા 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#જો defined (__linux__) || defined (__CYGWIN__)

સ્થિર પૂર્ણાંક
ffi_tramp_get_temp_file (શૂન્ય)
{
  ssize_t ગણતરી;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  ગણતરી = લિખ(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  જો (ગણતરી >=0 && (size_t)ગણતરી == tramp_globals.map_size && tramp_table_alloc ())
    પાછા 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  પાછા 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#જો defined (__linux__) || defined (__CYGWIN__)

સ્થિર પૂર્ણાંક
ffi_tramp_init_os (શૂન્ય)
{
  જો (ffi_tramp_get_libffi ())
    પાછા 1;
  પાછા ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#જો defined (__linux__) || defined (__CYGWIN__)

સ્થિર pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

સ્થિર શૂન્ય
ffi_tramp_lock(શૂન્ય)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

સ્થિર શૂન્ય
ffi_tramp_unlock(શૂન્ય)
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

#જો defined (__linux__) || defined (__CYGWIN__)

સ્થિર પૂર્ણાંક
tramp_table_map (સંરચના tramp_table *table)
{
  અક્ષર *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (ખાલી, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  જો (addr == MAP_FAILED)
    પાછા 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  જો (table->code_table == MAP_FAILED)
    {
      (શૂન્ય) munmap (addr, tramp_globals.map_size * 2);
      પાછા 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  પાછા 1;
}

સ્થિર શૂન્ય
tramp_table_unmap (સંરચના tramp_table *table)
{
  (શૂન્ય) munmap (table->code_table, tramp_globals.map_size);
  (શૂન્ય) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
સ્થિર પૂર્ણાંક
ffi_tramp_init (શૂન્ય)
{
  long page_size;

  જો (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    પાછા 1;

  જો (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    પાછા 0;

  જો (ffi_tramp_arch == ખાલી)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      પાછા 0;
    }

  tramp_globals.free_tables = ખાલી;
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
  જો (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      પાછા 0;
    }

  જો (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      પાછા 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  પાછા 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

સ્થિર શૂન્ય tramp_add (સંરચના tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
સ્થિર પૂર્ણાંક
tramp_table_alloc (શૂન્ય)
{
  સંરચના tramp_table *table;
  સંરચના tramp *tramp_array, *tramp;
  size_t size;
  અક્ષર *code, *parm;
  પૂર્ણાંક i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  જો (tramp_globals.nfree_tables > 0)
    પાછા 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (કદ (*table));
  જો (table == ખાલી)
    પાછા 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (કદ (*tramp) * tramp_globals.ntramp);
  જો (tramp_array == ખાલી)
    જાઓ free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  જો (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      જાઓ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->મુક્ત = ખાલી;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  માટે (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (સંરચના tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  પાછા 1;

/* Failure */
free_tramp_array:
  મુક્ત (tramp_array);
free_table:
  મુક્ત (table);
  પાછા 0;
}

/*
 * Free a trampoline table.
 */
સ્થિર શૂન્ય
tramp_table_free (સંરચના tramp_table *table)
{
  tramp_table_unmap (table);
  મુક્ત (table->array);
  મુક્ત (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
સ્થિર શૂન્ય
tramp_table_add (સંરચના tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = ખાલી;
  જો (tramp_globals.free_tables != ખાલી)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
સ્થિર શૂન્ય
tramp_table_del (સંરચના tramp_table *table)
{
  tramp_globals.nfree_tables--;
  જો (table->prev != ખાલી)
    table->prev->next = table->next;
  જો (table->next != ખાલી)
    table->next->prev = table->prev;
  જો (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
સ્થિર શૂન્ય
tramp_add (સંરચના tramp *tramp)
{
  સંરચના tramp_table *table = tramp->table;

  tramp->next = table->મુક્ત;
  tramp->prev = ખાલી;
  જો (table->મુક્ત != ખાલી)
    table->મુક્ત->prev = tramp;
  table->મુક્ત = tramp;
  table->nfree++;

  જો (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  જો (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
સ્થિર શૂન્ય
tramp_del (સંરચના tramp *tramp)
{
  સંરચના tramp_table *table = tramp->table;

  table->nfree--;
  જો (tramp->prev != ખાલી)
    tramp->prev->next = tramp->next;
  જો (tramp->next != ખાલી)
    tramp->next->prev = tramp->prev;
  જો (table->મુક્ત == tramp)
    table->મુક્ત = tramp->next;

  જો (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

પૂર્ણાંક
ffi_tramp_is_supported(શૂન્ય)
{
  પૂર્ણાંક ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  પાછા ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
શૂન્ય *
ffi_tramp_alloc (પૂર્ણાંક flags)
{
  સંરચના tramp *tramp;

  ffi_tramp_lock();

  જો (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      પાછા ખાલી;
    }

  જો (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      પાછા ખાલી;
    }

  tramp = tramp_globals.free_tables->મુક્ત;
  tramp_del (tramp);

  ffi_tramp_unlock();

  પાછા tramp;
}

/*
 * Set the parameters for a trampoline.
 */
શૂન્ય
ffi_tramp_set_parms (શૂન્ય *arg, શૂન્ય *target, શૂન્ય *data)
{
  સંરચના tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
શૂન્ય *
ffi_tramp_get_addr (શૂન્ય *arg)
{
  સંરચના tramp *tramp = arg;
  શૂન્ય *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  પાછા addr;
}

/*
 * Free a trampoline.
 */
શૂન્ય
ffi_tramp_free (શૂન્ય *arg)
{
  સંરચના tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#નહીંતર /* !FFI_EXEC_STATIC_TRAMP */

#અન્તર્ભાવ <stddef.h>

પૂર્ણાંક
ffi_tramp_is_supported(શૂન્ય)
{
  પાછા 0;
}

શૂન્ય *
ffi_tramp_alloc (પૂર્ણાંક flags)
{
  પાછા ખાલી;
}

શૂન્ય
ffi_tramp_set_parms (શૂન્ય *arg, શૂન્ય *target, શૂન્ય *data)
{
}

શૂન્ય *
ffi_tramp_get_addr (શૂન્ય *arg)
{
  પાછા ખાલી;
}

શૂન્ય
ffi_tramp_free (શૂન્ય *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
