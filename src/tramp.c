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

#ଅନ୍ତର୍ଭାଵ <fficonfig.h>

#ifdef FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#ଯଦି defined (__linux__) || defined (__CYGWIN__)
#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <pthread.h>
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#ଅନ୍ତର୍ଭାଵ <tramp.h>
#ifdef __linux__
#ଅନ୍ତର୍ଭାଵ <linux/limits.h>
#ଅନ୍ତର୍ଭାଵ <linux/types.h>
#endif
#ifdef __CYGWIN__
#ଅନ୍ତର୍ଭାଵ <limits.h>
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
ଶୂନ୍ୟ __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_size,
  size_t *map_size);

/* ------------------------- Trampoline Data Structures --------------------*/

ସଂରଚନା tramp;

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
ସଂରଚନା tramp_table
{
  ସଂରଚନା tramp_table *prev;
  ସଂରଚନା tramp_table *next;
  ଶୂନ୍ୟ *code_table;
  ଶୂନ୍ୟ *parm_table;
  ସଂରଚନା tramp *array;
  ସଂରଚନା tramp *ମୁକ୍ତ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
ସଂରଚନା tramp_parm
{
  ଶୂନ୍ୟ *data;
  ଶୂନ୍ୟ *target;
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
ସଂରଚନା tramp
{
  ସଂରଚନା tramp *prev;
  ସଂରଚନା tramp *next;
  ସଂରଚନା tramp_table *table;
  ଶୂନ୍ୟ *code;
  ସଂରଚନା tramp_parm *parm;
};

ଗଣନା tramp_globals_status {
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
ସଂରଚନା tramp_globals
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
  off_t offset;
  ଶୂନ୍ୟ *text;
  size_t map_size;
  size_t size;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntramp;
  ସଂରଚନା tramp_table *free_tables;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfree_tables;
  ଗଣନା tramp_globals_status status;
};

ସ୍ଥିର ସଂରଚନା tramp_globals tramp_globals;

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tramp_table_alloc (ଶୂନ୍ୟ);

#ଯଦି defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#define FFI_TRAMP_STR_(x) #x
#define FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_get_libffi (ଶୂନ୍ୟ)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  ଅକ୍ଷର file[PATH_MAX+1], ଧାଡ଼ି[PATH_MAX+100], perm[10], dev[10];
  unsigned long start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfields, found;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_flags = O_RDONLY;

#ifdef O_CLOEXEC
  open_flags |= O_CLOEXEC;
#endif

  snprintf (file, ଆକାର (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  ଯଦି (fp == ଖାଲି)
    ଫେରନ୍ତୁ 0;

  found = 0;
  ଯେତେବେଳେ (feof (fp) == 0) {
    ଯଦି (fgets (ଧାଡ଼ି, ଆକାର (ଧାଡ଼ି), fp) == 0)
      ଭାଙ୍ଗ;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = sscanf (ଧାଡ଼ି,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    ଯଦି (nfields != 7)
      ଜାରି;

    ଯଦି (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      ଭାଙ୍ଗ;
    }
  }
  fclose (fp);

  ଯଦି (!found)
    ଫେରନ୍ତୁ 0;

  tramp_globals.fd = open (file, open_flags);
  ଯଦି (tramp_globals.fd == -1)
    ଫେରନ୍ତୁ 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  ଯଦି (!tramp_table_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      ଫେରନ୍ତୁ 0;
    }
  ଫେରନ୍ତୁ 1;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

#ଯଦି defined (__linux__) || defined (__CYGWIN__)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_get_temp_file (ଶୂନ୍ୟ)
{
  ssize_t ଗଣନା_2;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_temp_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  ଗଣନା_2 = ଲିଖ(tramp_globals.fd, tramp_globals.text, tramp_globals.map_size);
  ଯଦି (ଗଣନା_2 >=0 && (size_t)ଗଣନା_2 == tramp_globals.map_size && tramp_table_alloc ())
    ଫେରନ୍ତୁ 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  ଫେରନ୍ତୁ 0;
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#ଯଦି defined (__linux__) || defined (__CYGWIN__)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_init_os (ଶୂନ୍ୟ)
{
  ଯଦି (ffi_tramp_get_libffi ())
    ଫେରନ୍ତୁ 1;
  ଫେରନ୍ତୁ ffi_tramp_get_temp_file ();
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#ଯଦି defined (__linux__) || defined (__CYGWIN__)

ସ୍ଥିର pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ସ୍ଥିର ଶୂନ୍ୟ
ffi_tramp_lock(ଶୂନ୍ୟ)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

ସ୍ଥିର ଶୂନ୍ୟ
ffi_tramp_unlock(ଶୂନ୍ୟ)
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

#ଯଦି defined (__linux__) || defined (__CYGWIN__)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tramp_table_map (ସଂରଚନା tramp_table *table)
{
  ଅକ୍ଷର *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (ଖାଲି, tramp_globals.map_size * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  ଯଦି (addr == MAP_FAILED)
    ଫେରନ୍ତୁ 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_size, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  ଯଦି (table->code_table == MAP_FAILED)
    {
      (ଶୂନ୍ୟ) munmap (addr, tramp_globals.map_size * 2);
      ଫେରନ୍ତୁ 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_size;
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ଶୂନ୍ୟ
tramp_table_unmap (ସଂରଚନା tramp_table *table)
{
  (ଶୂନ୍ୟ) munmap (table->code_table, tramp_globals.map_size);
  (ଶୂନ୍ୟ) munmap (table->parm_table, tramp_globals.map_size);
}

#endif /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_init (ଶୂନ୍ୟ)
{
  long page_size;

  ଯଦି (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    ଫେରନ୍ତୁ 1;

  ଯଦି (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    ଫେରନ୍ତୁ 0;

  ଯଦି (ffi_tramp_arch == ଖାଲି)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ଫେରନ୍ତୁ 0;
    }

  tramp_globals.free_tables = ଖାଲି;
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
  ଯଦି (page_size >= 0 && (size_t)page_size > tramp_globals.map_size)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ଫେରନ୍ତୁ 0;
    }

  ଯଦି (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      ଫେରନ୍ତୁ 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  ଫେରନ୍ତୁ 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ସ୍ଥିର ଶୂନ୍ୟ tramp_add (ସଂରଚନା tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
tramp_table_alloc (ଶୂନ୍ୟ)
{
  ସଂରଚନା tramp_table *table;
  ସଂରଚନା tramp *tramp_array, *tramp;
  size_t size;
  ଅକ୍ଷର *code, *parm;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  ଯଦି (tramp_globals.nfree_tables > 0)
    ଫେରନ୍ତୁ 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = malloc (ଆକାର (*table));
  ଯଦି (table == ଖାଲି)
    ଫେରନ୍ତୁ 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = malloc (ଆକାର (*tramp) * tramp_globals.ntramp);
  ଯଦି (tramp_array == ଖାଲି)
    ଯାଅ free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  ଯଦି (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      ଯାଅ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->ମୁକ୍ତ = ଖାଲି;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  size = tramp_globals.size;
  code = table->code_table;
  parm = table->parm_table;
  ପାଇଁ (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (ସଂରଚନା tramp_parm *) parm;
      tramp_add (tramp);

      code += size;
      parm += size;
    }
  /* Success */
  ଫେରନ୍ତୁ 1;

/* Failure */
free_tramp_array:
  ମୁକ୍ତ (tramp_array);
free_table:
  ମୁକ୍ତ (table);
  ଫେରନ୍ତୁ 0;
}

/*
 * Free a trampoline table.
 */
ସ୍ଥିର ଶୂନ୍ୟ
tramp_table_free (ସଂରଚନା tramp_table *table)
{
  tramp_table_unmap (table);
  ମୁକ୍ତ (table->array);
  ମୁକ୍ତ (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
ସ୍ଥିର ଶୂନ୍ୟ
tramp_table_add (ସଂରଚନା tramp_table *table)
{
  table->next = tramp_globals.free_tables;
  table->prev = ଖାଲି;
  ଯଦି (tramp_globals.free_tables != ଖାଲି)
    tramp_globals.free_tables->prev = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ସ୍ଥିର ଶୂନ୍ୟ
tramp_table_del (ସଂରଚନା tramp_table *table)
{
  tramp_globals.nfree_tables--;
  ଯଦି (table->prev != ଖାଲି)
    table->prev->next = table->next;
  ଯଦି (table->next != ଖାଲି)
    table->next->prev = table->prev;
  ଯଦି (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->next;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ସ୍ଥିର ଶୂନ୍ୟ
tramp_add (ସଂରଚନା tramp *tramp)
{
  ସଂରଚନା tramp_table *table = tramp->table;

  tramp->next = table->ମୁକ୍ତ;
  tramp->prev = ଖାଲି;
  ଯଦି (table->ମୁକ୍ତ != ଖାଲି)
    table->ମୁକ୍ତ->prev = tramp;
  table->ମୁକ୍ତ = tramp;
  table->nfree++;

  ଯଦି (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  ଯଦି (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ସ୍ଥିର ଶୂନ୍ୟ
tramp_del (ସଂରଚନା tramp *tramp)
{
  ସଂରଚନା tramp_table *table = tramp->table;

  table->nfree--;
  ଯଦି (tramp->prev != ଖାଲି)
    tramp->prev->next = tramp->next;
  ଯଦି (tramp->next != ଖାଲି)
    tramp->next->prev = tramp->prev;
  ଯଦି (table->ମୁକ୍ତ == tramp)
    table->ମୁକ୍ତ = tramp->next;

  ଯଦି (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_is_supported(ଶୂନ୍ୟ)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  ଫେରନ୍ତୁ ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
ଶୂନ୍ୟ *
ffi_tramp_alloc (ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
  ସଂରଚନା tramp *tramp;

  ffi_tramp_lock();

  ଯଦି (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      ଫେରନ୍ତୁ ଖାଲି;
    }

  ଯଦି (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      ଫେରନ୍ତୁ ଖାଲି;
    }

  tramp = tramp_globals.free_tables->ମୁକ୍ତ;
  tramp_del (tramp);

  ffi_tramp_unlock();

  ଫେରନ୍ତୁ tramp;
}

/*
 * Set the parameters for a trampoline.
 */
ଶୂନ୍ୟ
ffi_tramp_set_parms (ଶୂନ୍ୟ *arg, ଶୂନ୍ୟ *target, ଶୂନ୍ୟ *data)
{
  ସଂରଚନା tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->data = data;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
ଶୂନ୍ୟ *
ffi_tramp_get_addr (ଶୂନ୍ୟ *arg)
{
  ସଂରଚନା tramp *tramp = arg;
  ଶୂନ୍ୟ *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  ଫେରନ୍ତୁ addr;
}

/*
 * Free a trampoline.
 */
ଶୂନ୍ୟ
ffi_tramp_free (ଶୂନ୍ୟ *arg)
{
  ସଂରଚନା tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#ଅନ୍ୟଥା /* !FFI_EXEC_STATIC_TRAMP */

#ଅନ୍ତର୍ଭାଵ <stddef.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_is_supported(ଶୂନ୍ୟ)
{
  ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ *
ffi_tramp_alloc (ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
  ଫେରନ୍ତୁ ଖାଲି;
}

ଶୂନ୍ୟ
ffi_tramp_set_parms (ଶୂନ୍ୟ *arg, ଶୂନ୍ୟ *target, ଶୂନ୍ୟ *data)
{
}

ଶୂନ୍ୟ *
ffi_tramp_get_addr (ଶୂନ୍ୟ *arg)
{
  ଫେରନ୍ତୁ ଖାଲି;
}

ଶୂନ୍ୟ
ffi_tramp_free (ଶୂନ୍ୟ *arg)
{
}

#endif /* FFI_EXEC_STATIC_TRAMP */
