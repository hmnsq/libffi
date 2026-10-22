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

#شامل <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#اگر defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdio.h>
#شامل <unistd.h>
#شامل <stdlib.h>
#شامل <stdint.h>
#شامل <fcntl.h>
#شامل <pthread.h>
#شامل <sys/mman.h>
#شامل <tramp.h>
#ifdef __linux__
#شامل <linux/limits.h>
#شامل <linux/types.h>
#endif
#ifdef __CYGWIN__
#شامل <limits.h>
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
تهی __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

ساختار tramp;

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
ساختار tramp_table
{
  ساختار tramp_table *prev;
  ساختار tramp_table *next;
  تهی *code_table;
  تهی *parm_table;
  ساختار tramp *array;
  ساختار tramp *آزاد;
  صحیح nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
ساختار tramp_parm
{
  تهی *data;
  تهی *target;
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
ساختار tramp
{
  ساختار tramp *prev;
  ساختار tramp *next;
  ساختار tramp_table *table;
  تهی *code;
  ساختار tramp_parm *parm;
};

شمارش tramp_globals_status {
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
ساختار tramp_globals
{
  صحیح fd;
  off_t offset;
  تهی *text;
  size_t map_size;
  size_t size;
  صحیح ntramp;
  ساختار tramp_table *free_tables;
  صحیح nfree_tables;
  شمارش tramp_globals_status status;
};

ایستا ساختار tramp_globals tramp_globals;

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
ایستا صحیح tramp_table_alloc (تهی);

#اگر defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ایستا صحیح
ffi_tramp_get_libffi (تهی)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  نویسه file[PATH_MAX+1], سطر[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  صحیح nfields, found;
  صحیح open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, اندازه (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  اگر (fp == پوچ)
    بازگشت 0;

  found = 0;
  وقتی (feof (fp) == 0) {
    اگر (fgets (سطر, اندازه (سطر), fp) == 0)
      بشکن;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (سطر,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    اگر (nfields != 7)
      ادامه;

    اگر (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      بشکن;
    }
  }
  fclose (fp);

  اگر (!found)
    بازگشت 0;

  tramp_globals.fd = open (file, open_flags);
  اگر (tramp_globals.fd == -1)
    بازگشت 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  اگر (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      بازگشت 0;
    }
  بازگشت 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#اگر defined (__linux__) || defined (__CYGWIN__)

ایستا صحیح
ffi_tramp_get_temp_file (تهی)
{
  ssize_t شمار;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  شمار = اكتب(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  اگر (شمار >=0 && (size_t)شمار == tramp_globals.map_size && tramp_table_alloc ())
    بازگشت 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  بازگشت 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#اگر defined (__linux__) || defined (__CYGWIN__)

ایستا صحیح
ffi_tramp_init_os (تهی)
{
  اگر (ffi_tramp_get_libffi ())
    بازگشت 1;
  بازگشت ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#اگر defined (__linux__) || defined (__CYGWIN__)

ایستا pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ایستا تهی
ffi_tramp_lock(تهی)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

ایستا تهی
ffi_tramp_unlock(تهی)
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

#اگر defined (__linux__) || defined (__CYGWIN__)

ایستا صحیح
tramp_table_map (ساختار tramp_table *table)
{
  نویسه *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (پوچ, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  اگر (addr == MAP_FAILED)
    بازگشت 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  اگر (table->code_table == MAP_FAILED)
    {
      (تهی) munmap (addr, tramp_globals.map_size * 2);
      بازگشت 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  بازگشت 1;
}

ایستا تهی
tramp_table_unmap (ساختار tramp_table *table)
{
  (تهی) munmap (table->code_table, tramp_globals.map_size);
  (تهی) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ایستا صحیح
ffi_tramp_init (تهی)
{
  long page_size;

  اگر (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    بازگشت 1;

  اگر (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    بازگشت 0;

  اگر (ffi_tramp_arch == پوچ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      بازگشت 0;
    }

  tramp_globals.free_tables = پوچ;
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
  اگر (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      بازگشت 0;
    }

  اگر (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      بازگشت 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  بازگشت 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ایستا تهی tramp_add (ساختار tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ایستا صحیح
tramp_table_alloc (تهی)
{
  ساختار tramp_table *table;
  ساختار tramp *tramp_array, *tramp;
  size_t size;
  نویسه *code, *parm;
  صحیح i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  اگر (tramp_globals.nfree_tables > 0)
    بازگشت 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (اندازه (*table));
  اگر (table == پوچ)
    بازگشت 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (اندازه (*tramp) * tramp_globals.ntramp);
  اگر (tramp_array == پوچ)
    برو free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  اگر (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      برو free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->آزاد = پوچ;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  برای (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (ساختار tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  بازگشت 1;

/* Failure */
free_tramp_array:
  آزاد (tramp_array);
free_table:
  آزاد (table);
  بازگشت 0;
}

/*
 * Free a trampoline table.
 */
ایستا تهی
tramp_table_free (ساختار tramp_table *table)
{
  tramp_table_unmap (table);
  آزاد (table->array);
  آزاد (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
ایستا تهی
tramp_table_add (ساختار tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = پوچ;
  اگر (tramp_globals.free_tables != پوچ)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ایستا تهی
tramp_table_del (ساختار tramp_table *table)
{
  tramp_globals.nfree_tables--;
  اگر (table->prev != پوچ)
    table->prev->next = table->next;
  اگر (table->next != پوچ)
    table->next->prev = table->prev;
  اگر (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ایستا تهی
tramp_add (ساختار tramp *tramp)
{
  ساختار tramp_table *table = tramp->table;

  tramp->next = table->آزاد;
  tramp->prev = پوچ;
  اگر (table->آزاد != پوچ)
    table->آزاد->prev = tramp;
  table->آزاد = tramp;
  table->nfree++;

  اگر (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  اگر (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ایستا تهی
tramp_del (ساختار tramp *tramp)
{
  ساختار tramp_table *table = tramp->table;

  table->nfree--;
  اگر (tramp->prev != پوچ)
    tramp->prev->next = tramp->next;
  اگر (tramp->next != پوچ)
    tramp->next->prev = tramp->prev;
  اگر (table->آزاد == tramp)
    table->آزاد = tramp->next;

  اگر (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

صحیح
ffi_tramp_is_supported(تهی)
{
  صحیح ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  بازگشت ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
تهی *
ffi_tramp_alloc (صحیح flags)
{
  ساختار tramp *tramp;

  ffi_tramp_lock();

  اگر (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      بازگشت پوچ;
    }

  اگر (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      بازگشت پوچ;
    }

  tramp = tramp_globals.free_tables->آزاد;
  tramp_del (tramp);

  ffi_tramp_unlock();

  بازگشت tramp;
}

/*
 * Set the parameters for a trampoline.
 */
تهی
ffi_tramp_set_parms (تهی *arg, تهی *target, تهی *data)
{
  ساختار tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
تهی *
ffi_tramp_get_addr (تهی *arg)
{
  ساختار tramp *tramp = arg;
  تهی *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  بازگشت addr;
}

/*
 * Free a trampoline.
 */
تهی
ffi_tramp_free (تهی *arg)
{
  ساختار tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#وگرنه /* !FFI_EXEC_STATIC_TRAMP */

#شامل <stddef.h>

صحیح
ffi_tramp_is_supported(تهی)
{
  بازگشت 0;
}

تهی *
ffi_tramp_alloc (صحیح flags)
{
  بازگشت پوچ;
}

تهی
ffi_tramp_set_parms (تهی *arg, تهی *target, تهی *data)
{
}

تهی *
ffi_tramp_get_addr (تهی *arg)
{
  بازگشت پوچ;
}

تهی
ffi_tramp_free (تهی *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
