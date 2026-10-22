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

#ضمن <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#إذا defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdio.h>
#ضمن <unistd.h>
#ضمن <stdlib.h>
#ضمن <stdint.h>
#ضمن <fcntl.h>
#ضمن <pthread.h>
#ضمن <sys/mman.h>
#ضمن <tramp.h>
#ifdef __linux__
#ضمن <linux/limits.h>
#ضمن <linux/types.h>
#endif
#ifdef __CYGWIN__
#ضمن <limits.h>
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
فراغ __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

بنية tramp;

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
بنية tramp_table
{
  بنية tramp_table *prev;
  بنية tramp_table *next;
  فراغ *code_table;
  فراغ *parm_table;
  بنية tramp *array;
  بنية tramp *حرر;
  صحيح nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
بنية tramp_parm
{
  فراغ *data;
  فراغ *target;
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
بنية tramp
{
  بنية tramp *prev;
  بنية tramp *next;
  بنية tramp_table *table;
  فراغ *code;
  بنية tramp_parm *parm;
};

تعداد tramp_globals_status {
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
بنية tramp_globals
{
  صحيح fd;
  off_t offset;
  فراغ *text;
  size_t map_size;
  size_t size;
  صحيح ntramp;
  بنية tramp_table *free_tables;
  صحيح nfree_tables;
  تعداد tramp_globals_status status;
};

ساكن بنية tramp_globals tramp_globals;

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
ساكن صحيح tramp_table_alloc (فراغ);

#إذا defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ساكن صحيح
ffi_tramp_get_libffi (فراغ)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  حرف file[PATH_MAX+1], سطر[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  صحيح nfields, found;
  صحيح open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, حجم (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  إذا (fp == عدم)
    إرجاع 0;

  found = 0;
  بينما (feof (fp) == 0) {
    إذا (fgets (سطر, حجم (سطر), fp) == 0)
      اكسر;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (سطر,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    إذا (nfields != 7)
      استمر;

    إذا (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      اكسر;
    }
  }
  fclose (fp);

  إذا (!found)
    إرجاع 0;

  tramp_globals.fd = open (file, open_flags);
  إذا (tramp_globals.fd == -1)
    إرجاع 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  إذا (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      إرجاع 0;
    }
  إرجاع 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#إذا defined (__linux__) || defined (__CYGWIN__)

ساكن صحيح
ffi_tramp_get_temp_file (فراغ)
{
  ssize_t تعداد_2;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  تعداد_2 = اكتب(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  إذا (تعداد_2 >=0 && (size_t)تعداد_2 == tramp_globals.map_size && tramp_table_alloc ())
    إرجاع 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  إرجاع 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#إذا defined (__linux__) || defined (__CYGWIN__)

ساكن صحيح
ffi_tramp_init_os (فراغ)
{
  إذا (ffi_tramp_get_libffi ())
    إرجاع 1;
  إرجاع ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#إذا defined (__linux__) || defined (__CYGWIN__)

ساكن pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ساكن فراغ
ffi_tramp_lock(فراغ)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

ساكن فراغ
ffi_tramp_unlock(فراغ)
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

#إذا defined (__linux__) || defined (__CYGWIN__)

ساكن صحيح
tramp_table_map (بنية tramp_table *table)
{
  حرف *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (عدم, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  إذا (addr == MAP_FAILED)
    إرجاع 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  إذا (table->code_table == MAP_FAILED)
    {
      (فراغ) munmap (addr, tramp_globals.map_size * 2);
      إرجاع 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  إرجاع 1;
}

ساكن فراغ
tramp_table_unmap (بنية tramp_table *table)
{
  (فراغ) munmap (table->code_table, tramp_globals.map_size);
  (فراغ) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ساكن صحيح
ffi_tramp_init (فراغ)
{
  long page_size;

  إذا (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    إرجاع 1;

  إذا (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    إرجاع 0;

  إذا (ffi_tramp_arch == عدم)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      إرجاع 0;
    }

  tramp_globals.free_tables = عدم;
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
  إذا (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      إرجاع 0;
    }

  إذا (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      إرجاع 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  إرجاع 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ساكن فراغ tramp_add (بنية tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ساكن صحيح
tramp_table_alloc (فراغ)
{
  بنية tramp_table *table;
  بنية tramp *tramp_array, *tramp;
  size_t size;
  حرف *code, *parm;
  صحيح i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  إذا (tramp_globals.nfree_tables > 0)
    إرجاع 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (حجم (*table));
  إذا (table == عدم)
    إرجاع 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (حجم (*tramp) * tramp_globals.ntramp);
  إذا (tramp_array == عدم)
    اذهب free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  إذا (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      اذهب free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->حرر = عدم;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  لكل (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (بنية tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  إرجاع 1;

/* Failure */
free_tramp_array:
  حرر (tramp_array);
free_table:
  حرر (table);
  إرجاع 0;
}

/*
 * Free a trampoline table.
 */
ساكن فراغ
tramp_table_free (بنية tramp_table *table)
{
  tramp_table_unmap (table);
  حرر (table->array);
  حرر (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
ساكن فراغ
tramp_table_add (بنية tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = عدم;
  إذا (tramp_globals.free_tables != عدم)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ساكن فراغ
tramp_table_del (بنية tramp_table *table)
{
  tramp_globals.nfree_tables--;
  إذا (table->prev != عدم)
    table->prev->next = table->next;
  إذا (table->next != عدم)
    table->next->prev = table->prev;
  إذا (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ساكن فراغ
tramp_add (بنية tramp *tramp)
{
  بنية tramp_table *table = tramp->table;

  tramp->next = table->حرر;
  tramp->prev = عدم;
  إذا (table->حرر != عدم)
    table->حرر->prev = tramp;
  table->حرر = tramp;
  table->nfree++;

  إذا (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  إذا (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ساكن فراغ
tramp_del (بنية tramp *tramp)
{
  بنية tramp_table *table = tramp->table;

  table->nfree--;
  إذا (tramp->prev != عدم)
    tramp->prev->next = tramp->next;
  إذا (tramp->next != عدم)
    tramp->next->prev = tramp->prev;
  إذا (table->حرر == tramp)
    table->حرر = tramp->next;

  إذا (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

صحيح
ffi_tramp_is_supported(فراغ)
{
  صحيح ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  إرجاع ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
فراغ *
ffi_tramp_alloc (صحيح flags)
{
  بنية tramp *tramp;

  ffi_tramp_lock();

  إذا (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      إرجاع عدم;
    }

  إذا (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      إرجاع عدم;
    }

  tramp = tramp_globals.free_tables->حرر;
  tramp_del (tramp);

  ffi_tramp_unlock();

  إرجاع tramp;
}

/*
 * Set the parameters for a trampoline.
 */
فراغ
ffi_tramp_set_parms (فراغ *arg, فراغ *target, فراغ *data)
{
  بنية tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
فراغ *
ffi_tramp_get_addr (فراغ *arg)
{
  بنية tramp *tramp = arg;
  فراغ *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  إرجاع addr;
}

/*
 * Free a trampoline.
 */
فراغ
ffi_tramp_free (فراغ *arg)
{
  بنية tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#وإلا /* !FFI_EXEC_STATIC_TRAMP */

#ضمن <stddef.h>

صحيح
ffi_tramp_is_supported(فراغ)
{
  إرجاع 0;
}

فراغ *
ffi_tramp_alloc (صحيح flags)
{
  إرجاع عدم;
}

فراغ
ffi_tramp_set_parms (فراغ *arg, فراغ *target, فراغ *data)
{
}

فراغ *
ffi_tramp_get_addr (فراغ *arg)
{
  إرجاع عدم;
}

فراغ
ffi_tramp_free (فراغ *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
