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

#অন্তর্ভাব <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#যদি defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <pthread.h>
#অন্তর্ভাব <sys/mman.h>
#অন্তর্ভাব <tramp.h>
#ifdef __linux__
#অন্তর্ভাব <linux/limits.h>
#অন্তর্ভাব <linux/types.h>
#endif
#ifdef __CYGWIN__
#অন্তর্ভাব <limits.h>
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
শূন্য __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

সংরচনা tramp;

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
সংরচনা tramp_table
{
  সংরচনা tramp_table *prev;
  সংরচনা tramp_table *next;
  শূন্য *code_table;
  শূন্য *parm_table;
  সংরচনা tramp *array;
  সংরচনা tramp *মুক্ত;
  পূর্ণাঙ্ক nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
সংরচনা tramp_parm
{
  শূন্য *data;
  শূন্য *target;
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
সংরচনা tramp
{
  সংরচনা tramp *prev;
  সংরচনা tramp *next;
  সংরচনা tramp_table *table;
  শূন্য *code;
  সংরচনা tramp_parm *parm;
};

গণনা tramp_globals_status {
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
সংরচনা tramp_globals
{
  পূর্ণাঙ্ক fd;
  off_t offset;
  শূন্য *text;
  size_t map_size;
  size_t size;
  পূর্ণাঙ্ক ntramp;
  সংরচনা tramp_table *free_tables;
  পূর্ণাঙ্ক nfree_tables;
  গণনা tramp_globals_status status;
};

স্থির সংরচনা tramp_globals tramp_globals;

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
স্থির পূর্ণাঙ্ক tramp_table_alloc (শূন্য);

#যদি defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

স্থির পূর্ণাঙ্ক
ffi_tramp_get_libffi (শূন্য)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  বর্ণ file[PATH_MAX+1], সারি[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  পূর্ণাঙ্ক nfields, found;
  পূর্ণাঙ্ক open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, পরিমাণ (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  যদি (fp == রিক্ত)
    প্রত্যাগম 0;

  found = 0;
  যাবত্ (feof (fp) == 0) {
    যদি (fgets (সারি, পরিমাণ (সারি), fp) == 0)
      ভঙ্গ;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (সারি,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    যদি (nfields != 7)
      অনুবর্ত;

    যদি (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      ভঙ্গ;
    }
  }
  fclose (fp);

  যদি (!found)
    প্রত্যাগম 0;

  tramp_globals.fd = open (file, open_flags);
  যদি (tramp_globals.fd == -1)
    প্রত্যাগম 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  যদি (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      প্রত্যাগম 0;
    }
  প্রত্যাগম 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#যদি defined (__linux__) || defined (__CYGWIN__)

স্থির পূর্ণাঙ্ক
ffi_tramp_get_temp_file (শূন্য)
{
  ssize_t গুনতি;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  গুনতি = লিখ(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  যদি (গুনতি >=0 && (size_t)গুনতি == tramp_globals.map_size && tramp_table_alloc ())
    প্রত্যাগম 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  প্রত্যাগম 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#যদি defined (__linux__) || defined (__CYGWIN__)

স্থির পূর্ণাঙ্ক
ffi_tramp_init_os (শূন্য)
{
  যদি (ffi_tramp_get_libffi ())
    প্রত্যাগম 1;
  প্রত্যাগম ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#যদি defined (__linux__) || defined (__CYGWIN__)

স্থির pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

স্থির শূন্য
ffi_tramp_lock(শূন্য)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

স্থির শূন্য
ffi_tramp_unlock(শূন্য)
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

#যদি defined (__linux__) || defined (__CYGWIN__)

স্থির পূর্ণাঙ্ক
tramp_table_map (সংরচনা tramp_table *table)
{
  বর্ণ *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (রিক্ত, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  যদি (addr == MAP_FAILED)
    প্রত্যাগম 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  যদি (table->code_table == MAP_FAILED)
    {
      (শূন্য) munmap (addr, tramp_globals.map_size * 2);
      প্রত্যাগম 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  প্রত্যাগম 1;
}

স্থির শূন্য
tramp_table_unmap (সংরচনা tramp_table *table)
{
  (শূন্য) munmap (table->code_table, tramp_globals.map_size);
  (শূন্য) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
স্থির পূর্ণাঙ্ক
ffi_tramp_init (শূন্য)
{
  long page_size;

  যদি (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    প্রত্যাগম 1;

  যদি (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    প্রত্যাগম 0;

  যদি (ffi_tramp_arch == রিক্ত)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      প্রত্যাগম 0;
    }

  tramp_globals.free_tables = রিক্ত;
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
  যদি (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      প্রত্যাগম 0;
    }

  যদি (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      প্রত্যাগম 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  প্রত্যাগম 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

স্থির শূন্য tramp_add (সংরচনা tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
স্থির পূর্ণাঙ্ক
tramp_table_alloc (শূন্য)
{
  সংরচনা tramp_table *table;
  সংরচনা tramp *tramp_array, *tramp;
  size_t size;
  বর্ণ *code, *parm;
  পূর্ণাঙ্ক i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  যদি (tramp_globals.nfree_tables > 0)
    প্রত্যাগম 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (পরিমাণ (*table));
  যদি (table == রিক্ত)
    প্রত্যাগম 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (পরিমাণ (*tramp) * tramp_globals.ntramp);
  যদি (tramp_array == রিক্ত)
    গচ্ছ free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  যদি (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      গচ্ছ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->মুক্ত = রিক্ত;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  প্রতি (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (সংরচনা tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  প্রত্যাগম 1;

/* Failure */
free_tramp_array:
  মুক্ত (tramp_array);
free_table:
  মুক্ত (table);
  প্রত্যাগম 0;
}

/*
 * Free a trampoline table.
 */
স্থির শূন্য
tramp_table_free (সংরচনা tramp_table *table)
{
  tramp_table_unmap (table);
  মুক্ত (table->array);
  মুক্ত (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
স্থির শূন্য
tramp_table_add (সংরচনা tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = রিক্ত;
  যদি (tramp_globals.free_tables != রিক্ত)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
স্থির শূন্য
tramp_table_del (সংরচনা tramp_table *table)
{
  tramp_globals.nfree_tables--;
  যদি (table->prev != রিক্ত)
    table->prev->next = table->next;
  যদি (table->next != রিক্ত)
    table->next->prev = table->prev;
  যদি (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
স্থির শূন্য
tramp_add (সংরচনা tramp *tramp)
{
  সংরচনা tramp_table *table = tramp->table;

  tramp->next = table->মুক্ত;
  tramp->prev = রিক্ত;
  যদি (table->মুক্ত != রিক্ত)
    table->মুক্ত->prev = tramp;
  table->মুক্ত = tramp;
  table->nfree++;

  যদি (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  যদি (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
স্থির শূন্য
tramp_del (সংরচনা tramp *tramp)
{
  সংরচনা tramp_table *table = tramp->table;

  table->nfree--;
  যদি (tramp->prev != রিক্ত)
    tramp->prev->next = tramp->next;
  যদি (tramp->next != রিক্ত)
    tramp->next->prev = tramp->prev;
  যদি (table->মুক্ত == tramp)
    table->মুক্ত = tramp->next;

  যদি (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

পূর্ণাঙ্ক
ffi_tramp_is_supported(শূন্য)
{
  পূর্ণাঙ্ক ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  প্রত্যাগম ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
শূন্য *
ffi_tramp_alloc (পূর্ণাঙ্ক flags)
{
  সংরচনা tramp *tramp;

  ffi_tramp_lock();

  যদি (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      প্রত্যাগম রিক্ত;
    }

  যদি (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      প্রত্যাগম রিক্ত;
    }

  tramp = tramp_globals.free_tables->মুক্ত;
  tramp_del (tramp);

  ffi_tramp_unlock();

  প্রত্যাগম tramp;
}

/*
 * Set the parameters for a trampoline.
 */
শূন্য
ffi_tramp_set_parms (শূন্য *arg, শূন্য *target, শূন্য *data)
{
  সংরচনা tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
শূন্য *
ffi_tramp_get_addr (শূন্য *arg)
{
  সংরচনা tramp *tramp = arg;
  শূন্য *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  প্রত্যাগম addr;
}

/*
 * Free a trampoline.
 */
শূন্য
ffi_tramp_free (শূন্য *arg)
{
  সংরচনা tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#অন্যথা /* !FFI_EXEC_STATIC_TRAMP */

#অন্তর্ভাব <stddef.h>

পূর্ণাঙ্ক
ffi_tramp_is_supported(শূন্য)
{
  প্রত্যাগম 0;
}

শূন্য *
ffi_tramp_alloc (পূর্ণাঙ্ক flags)
{
  প্রত্যাগম রিক্ত;
}

শূন্য
ffi_tramp_set_parms (শূন্য *arg, শূন্য *target, শূন্য *data)
{
}

শূন্য *
ffi_tramp_get_addr (শূন্য *arg)
{
  প্রত্যাগম রিক্ত;
}

শূন্য
ffi_tramp_free (শূন্য *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
