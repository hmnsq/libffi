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

#अन्तर्भाव <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#यदि defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <pthread.h>
#अन्तर्भाव <sys/mman.h>
#अन्तर्भाव <tramp.h>
#ifdef __linux__
#अन्तर्भाव <linux/limits.h>
#अन्तर्भाव <linux/types.h>
#endif
#ifdef __CYGWIN__
#अन्तर्भाव <limits.h>
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
शून्य __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

संरचना tramp;

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
संरचना tramp_table
{
  संरचना tramp_table *prev;
  संरचना tramp_table *next;
  शून्य *code_table;
  शून्य *parm_table;
  संरचना tramp *array;
  संरचना tramp *मुक्त;
  पूर्णाङ्क nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
संरचना tramp_parm
{
  शून्य *data;
  शून्य *target;
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
संरचना tramp
{
  संरचना tramp *prev;
  संरचना tramp *next;
  संरचना tramp_table *table;
  शून्य *code;
  संरचना tramp_parm *parm;
};

गणना tramp_globals_status {
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
संरचना tramp_globals
{
  पूर्णाङ्क fd;
  off_t offset;
  शून्य *text;
  size_t map_size;
  size_t size;
  पूर्णाङ्क ntramp;
  संरचना tramp_table *free_tables;
  पूर्णाङ्क nfree_tables;
  गणना tramp_globals_status status;
};

स्थिर संरचना tramp_globals tramp_globals;

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
स्थिर पूर्णाङ्क tramp_table_alloc (शून्य);

#यदि defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

स्थिर पूर्णाङ्क
ffi_tramp_get_libffi (शून्य)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  वर्ण file[PATH_MAX+1], पंक्ति[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  पूर्णाङ्क nfields, found;
  पूर्णाङ्क open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, परिमाण (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  यदि (fp == रिक्त)
    प्रत्यागम 0;

  found = 0;
  यावत् (feof (fp) == 0) {
    यदि (fgets (पंक्ति, परिमाण (पंक्ति), fp) == 0)
      भङ्ग;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (पंक्ति,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    यदि (nfields != 7)
      अनुवर्त;

    यदि (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      भङ्ग;
    }
  }
  fclose (fp);

  यदि (!found)
    प्रत्यागम 0;

  tramp_globals.fd = open (file, open_flags);
  यदि (tramp_globals.fd == -1)
    प्रत्यागम 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  यदि (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      प्रत्यागम 0;
    }
  प्रत्यागम 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#यदि defined (__linux__) || defined (__CYGWIN__)

स्थिर पूर्णाङ्क
ffi_tramp_get_temp_file (शून्य)
{
  ssize_t गिनती;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  गिनती = लिख(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  यदि (गिनती >=0 && (size_t)गिनती == tramp_globals.map_size && tramp_table_alloc ())
    प्रत्यागम 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  प्रत्यागम 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#यदि defined (__linux__) || defined (__CYGWIN__)

स्थिर पूर्णाङ्क
ffi_tramp_init_os (शून्य)
{
  यदि (ffi_tramp_get_libffi ())
    प्रत्यागम 1;
  प्रत्यागम ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#यदि defined (__linux__) || defined (__CYGWIN__)

स्थिर pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

स्थिर शून्य
ffi_tramp_lock(शून्य)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

स्थिर शून्य
ffi_tramp_unlock(शून्य)
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

#यदि defined (__linux__) || defined (__CYGWIN__)

स्थिर पूर्णाङ्क
tramp_table_map (संरचना tramp_table *table)
{
  वर्ण *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (रिक्त, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  यदि (addr == MAP_FAILED)
    प्रत्यागम 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  यदि (table->code_table == MAP_FAILED)
    {
      (शून्य) munmap (addr, tramp_globals.map_size * 2);
      प्रत्यागम 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  प्रत्यागम 1;
}

स्थिर शून्य
tramp_table_unmap (संरचना tramp_table *table)
{
  (शून्य) munmap (table->code_table, tramp_globals.map_size);
  (शून्य) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
स्थिर पूर्णाङ्क
ffi_tramp_init (शून्य)
{
  long page_size;

  यदि (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    प्रत्यागम 1;

  यदि (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    प्रत्यागम 0;

  यदि (ffi_tramp_arch == रिक्त)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      प्रत्यागम 0;
    }

  tramp_globals.free_tables = रिक्त;
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
  यदि (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      प्रत्यागम 0;
    }

  यदि (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      प्रत्यागम 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  प्रत्यागम 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

स्थिर शून्य tramp_add (संरचना tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
स्थिर पूर्णाङ्क
tramp_table_alloc (शून्य)
{
  संरचना tramp_table *table;
  संरचना tramp *tramp_array, *tramp;
  size_t size;
  वर्ण *code, *parm;
  पूर्णाङ्क i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  यदि (tramp_globals.nfree_tables > 0)
    प्रत्यागम 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (परिमाण (*table));
  यदि (table == रिक्त)
    प्रत्यागम 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (परिमाण (*tramp) * tramp_globals.ntramp);
  यदि (tramp_array == रिक्त)
    गच्छ free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  यदि (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      गच्छ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->मुक्त = रिक्त;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  प्रति (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (संरचना tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  प्रत्यागम 1;

/* Failure */
free_tramp_array:
  मुक्त (tramp_array);
free_table:
  मुक्त (table);
  प्रत्यागम 0;
}

/*
 * Free a trampoline table.
 */
स्थिर शून्य
tramp_table_free (संरचना tramp_table *table)
{
  tramp_table_unmap (table);
  मुक्त (table->array);
  मुक्त (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
स्थिर शून्य
tramp_table_add (संरचना tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = रिक्त;
  यदि (tramp_globals.free_tables != रिक्त)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
स्थिर शून्य
tramp_table_del (संरचना tramp_table *table)
{
  tramp_globals.nfree_tables--;
  यदि (table->prev != रिक्त)
    table->prev->next = table->next;
  यदि (table->next != रिक्त)
    table->next->prev = table->prev;
  यदि (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
स्थिर शून्य
tramp_add (संरचना tramp *tramp)
{
  संरचना tramp_table *table = tramp->table;

  tramp->next = table->मुक्त;
  tramp->prev = रिक्त;
  यदि (table->मुक्त != रिक्त)
    table->मुक्त->prev = tramp;
  table->मुक्त = tramp;
  table->nfree++;

  यदि (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  यदि (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
स्थिर शून्य
tramp_del (संरचना tramp *tramp)
{
  संरचना tramp_table *table = tramp->table;

  table->nfree--;
  यदि (tramp->prev != रिक्त)
    tramp->prev->next = tramp->next;
  यदि (tramp->next != रिक्त)
    tramp->next->prev = tramp->prev;
  यदि (table->मुक्त == tramp)
    table->मुक्त = tramp->next;

  यदि (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

पूर्णाङ्क
ffi_tramp_is_supported(शून्य)
{
  पूर्णाङ्क ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  प्रत्यागम ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
शून्य *
ffi_tramp_alloc (पूर्णाङ्क flags)
{
  संरचना tramp *tramp;

  ffi_tramp_lock();

  यदि (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      प्रत्यागम रिक्त;
    }

  यदि (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      प्रत्यागम रिक्त;
    }

  tramp = tramp_globals.free_tables->मुक्त;
  tramp_del (tramp);

  ffi_tramp_unlock();

  प्रत्यागम tramp;
}

/*
 * Set the parameters for a trampoline.
 */
शून्य
ffi_tramp_set_parms (शून्य *arg, शून्य *target, शून्य *data)
{
  संरचना tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
शून्य *
ffi_tramp_get_addr (शून्य *arg)
{
  संरचना tramp *tramp = arg;
  शून्य *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  प्रत्यागम addr;
}

/*
 * Free a trampoline.
 */
शून्य
ffi_tramp_free (शून्य *arg)
{
  संरचना tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#अन्यथा /* !FFI_EXEC_STATIC_TRAMP */

#अन्तर्भाव <stddef.h>

पूर्णाङ्क
ffi_tramp_is_supported(शून्य)
{
  प्रत्यागम 0;
}

शून्य *
ffi_tramp_alloc (पूर्णाङ्क flags)
{
  प्रत्यागम रिक्त;
}

शून्य
ffi_tramp_set_parms (शून्य *arg, शून्य *target, शून्य *data)
{
}

शून्य *
ffi_tramp_get_addr (शून्य *arg)
{
  प्रत्यागम रिक्त;
}

शून्य
ffi_tramp_free (शून्य *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
