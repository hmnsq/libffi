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

#ਅਨ੍ਤਰ੍ਭਾਵ <fficonfig.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#ਜੇ defined (__linux__) || defined (__CYGWIN__)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GNU_ਸਰੋਤ 1
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/types.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

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
ਖਾਲੀ __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_ਮਾਪ,
  size_t *map_ਮਾਪ);

/* ------------------------- Trampoline Data Structures --------------------*/

ਬਣਤਰ tramp;

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
ਬਣਤਰ tramp_ਸਾਰਣੀ
{
  ਬਣਤਰ tramp_ਸਾਰਣੀ *prev;
  ਬਣਤਰ tramp_ਸਾਰਣੀ *ਅਗਲਾ;
  ਖਾਲੀ *ਕੋਡ_ਸਾਰਣੀ;
  ਖਾਲੀ *parm_ਸਾਰਣੀ;
  ਬਣਤਰ tramp *array;
  ਬਣਤਰ tramp *ਮੁਕਤ;
  ਪੂਰਨ nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
ਬਣਤਰ tramp_parm
{
  ਖਾਲੀ *ਅੰਕੜੇ;
  ਖਾਲੀ *ਨਿਸ਼ਾਨਾ_2;
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
ਬਣਤਰ tramp
{
  ਬਣਤਰ tramp *prev;
  ਬਣਤਰ tramp *ਅਗਲਾ;
  ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ;
  ਖਾਲੀ *ਕੋਡ;
  ਬਣਤਰ tramp_parm *parm;
};

ਗਿਣਤੀ tramp_globals_ਸਥਿਤੀ {
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
ਬਣਤਰ tramp_globals
{
  ਪੂਰਨ fd;
  off_t offset;
  ਖਾਲੀ *ਪਾਠ;
  size_t map_ਮਾਪ;
  size_t ਮਾਪ;
  ਪੂਰਨ ntramp;
  ਬਣਤਰ tramp_ਸਾਰਣੀ *free_tables;
  ਪੂਰਨ nfree_tables;
  ਗਿਣਤੀ tramp_globals_ਸਥਿਤੀ status;
};

ਸਥਿਰ ਬਣਤਰ tramp_globals tramp_globals;

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
ਸਥਿਰ ਪੂਰਨ tramp_ਸਾਰਣੀ_alloc (ਖਾਲੀ);

#ਜੇ defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMP_STR_(x) #x
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ਸਥਿਰ ਪੂਰਨ
ffi_tramp_get_libffi (ਖਾਲੀ)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  ਅੱਖਰ file[ਰਸਤਾ_MAX+1], ਸਤਰ[ਰਸਤਾ_MAX+100], perm[10], dev[10];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸ਼ੁਰੂ, ਸਿਰਾ, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.ਪਾਠ;
  ਪੂਰਨ nfields, ਮਿਲਿਆ;
  ਪੂਰਨ open_flags = O_RDONLY;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_CLOEXEC
  open_flags |= O_CLOEXEC;
#ਜੇ_ਅੰਤ

  snprintf (file, ਆਕਾਰ (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  ਜੇ (fp == ਨਿਰਾ)
    ਵਾਪਸ 0;

  ਮਿਲਿਆ = 0;
  ਜਦੋਂ (feof (fp) == 0) {
    ਜੇ (fgets (ਸਤਰ, ਆਕਾਰ (ਸਤਰ), fp) == 0)
      ਰੋਕੋ;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ (ਸਤਰ,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(ਰਸਤਾ_MAX) "s",
      &ਸ਼ੁਰੂ, &ਸਿਰਾ, perm, &offset, dev, &inode, file);
    ਜੇ (nfields != 7)
      ਜਾਰੀ;

    ਜੇ (addr >= ਸ਼ੁਰੂ && addr < ਸਿਰਾ) {
      tramp_globals.offset = offset + (addr - ਸ਼ੁਰੂ);
      ਮਿਲਿਆ = 1;
      ਰੋਕੋ;
    }
  }
  fclose (fp);

  ਜੇ (!ਮਿਲਿਆ)
    ਵਾਪਸ 0;

  tramp_globals.fd = open (file, open_flags);
  ਜੇ (tramp_globals.fd == -1)
    ਵਾਪਸ 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  ਜੇ (!tramp_ਸਾਰਣੀ_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      ਵਾਪਸ 0;
    }
  ਵਾਪਸ 1;
}

#ਜੇ_ਅੰਤ /* defined (__linux__) || defined (__CYGWIN__) */

#ਜੇ defined (__linux__) || defined (__CYGWIN__)

ਸਥਿਰ ਪੂਰਨ
ffi_tramp_get_ਆਰਜ਼ੀ_file (ਖਾਲੀ)
{
  ssize_t ਗਿਣਤੀ_2;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_ਆਰਜ਼ੀ_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  ਗਿਣਤੀ_2 = ਲਿਖ(tramp_globals.fd, tramp_globals.ਪਾਠ, tramp_globals.map_ਮਾਪ);
  ਜੇ (ਗਿਣਤੀ_2 >=0 && (size_t)ਗਿਣਤੀ_2 == tramp_globals.map_ਮਾਪ && tramp_ਸਾਰਣੀ_alloc ())
    ਵਾਪਸ 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#ਜੇ defined (__linux__) || defined (__CYGWIN__)

ਸਥਿਰ ਪੂਰਨ
ffi_tramp_init_os (ਖਾਲੀ)
{
  ਜੇ (ffi_tramp_get_libffi ())
    ਵਾਪਸ 1;
  ਵਾਪਸ ffi_tramp_get_ਆਰਜ਼ੀ_file ();
}

#ਜੇ_ਅੰਤ /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#ਜੇ defined (__linux__) || defined (__CYGWIN__)

ਸਥਿਰ pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ਸਥਿਰ ਖਾਲੀ
ffi_tramp_ਤਾਲਾ(ਖਾਲੀ)
{
  pthread_mutex_ਤਾਲਾ (&tramp_globals_mutex);
}

ਸਥਿਰ ਖਾਲੀ
ffi_tramp_unlock(ਖਾਲੀ)
{
  pthread_mutex_unlock (&tramp_globals_mutex);
}

#ਜੇ_ਅੰਤ /* defined (__linux__) || defined (__CYGWIN__) */

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

#ਜੇ defined (__linux__) || defined (__CYGWIN__)

ਸਥਿਰ ਪੂਰਨ
tramp_ਸਾਰਣੀ_map (ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ)
{
  ਅੱਖਰ *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (ਨਿਰਾ, tramp_globals.map_ਮਾਪ * 2, PROT_READ | PROT_WRITE,
    MAP_ਨਿੱਜੀ | MAP_ANONYMOUS, -1, 0);
  ਜੇ (addr == MAP_FAILED)
    ਵਾਪਸ 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  ਸਾਰਣੀ->ਕੋਡ_ਸਾਰਣੀ = mmap (addr, tramp_globals.map_ਮਾਪ, PROT_READ | PROT_EXEC,
    MAP_ਨਿੱਜੀ | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  ਜੇ (ਸਾਰਣੀ->ਕੋਡ_ਸਾਰਣੀ == MAP_FAILED)
    {
      (ਖਾਲੀ) munmap (addr, tramp_globals.map_ਮਾਪ * 2);
      ਵਾਪਸ 0;
    }
  ਸਾਰਣੀ->parm_ਸਾਰਣੀ = ਸਾਰਣੀ->ਕੋਡ_ਸਾਰਣੀ + tramp_globals.map_ਮਾਪ;
  ਵਾਪਸ 1;
}

ਸਥਿਰ ਖਾਲੀ
tramp_ਸਾਰਣੀ_unmap (ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ)
{
  (ਖਾਲੀ) munmap (ਸਾਰਣੀ->ਕੋਡ_ਸਾਰਣੀ, tramp_globals.map_ਮਾਪ);
  (ਖਾਲੀ) munmap (ਸਾਰਣੀ->parm_ਸਾਰਣੀ, tramp_globals.map_ਮਾਪ);
}

#ਜੇ_ਅੰਤ /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ਸਥਿਰ ਪੂਰਨ
ffi_tramp_init (ਖਾਲੀ)
{
  ਲੰਮਾ ਪੰਨਾ_ਮਾਪ;

  ਜੇ (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    ਵਾਪਸ 1;

  ਜੇ (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    ਵਾਪਸ 0;

  ਜੇ (ffi_tramp_arch == ਨਿਰਾ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ਵਾਪਸ 0;
    }

  tramp_globals.free_tables = ਨਿਰਾ;
  tramp_globals.nfree_tables = 0;

  /*
   * Get trampoline code table information from the architecture.
   */
  tramp_globals.ਪਾਠ = ffi_tramp_arch (&tramp_globals.ਮਾਪ,
    &tramp_globals.map_ਮਾਪ);
  tramp_globals.ntramp = tramp_globals.map_ਮਾਪ / tramp_globals.ਮਾਪ;

  /*
   * The trampoline code table is a single, fixed-size mapping.  If the
   * system page size is larger than that mapping, the static trampoline
   * mechanism cannot be used.  Both values are invariant for the life of
   * the process, so cache the FAILED verdict rather than re-running the
   * whole initialization on every allocation.
   */
  ਪੰਨਾ_ਮਾਪ = sysconf (_SC_PAGESIZE);
  ਜੇ (ਪੰਨਾ_ਮਾਪ >= 0 && (size_t)ਪੰਨਾ_ਮਾਪ > tramp_globals.map_ਮਾਪ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ਵਾਪਸ 0;
    }

  ਜੇ (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      ਵਾਪਸ 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  ਵਾਪਸ 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ਸਥਿਰ ਖਾਲੀ tramp_ਜੋੜੋ (ਬਣਤਰ tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ਸਥਿਰ ਪੂਰਨ
tramp_ਸਾਰਣੀ_alloc (ਖਾਲੀ)
{
  ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ;
  ਬਣਤਰ tramp *tramp_array, *tramp;
  size_t ਮਾਪ;
  ਅੱਖਰ *ਕੋਡ, *parm;
  ਪੂਰਨ i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  ਜੇ (tramp_globals.nfree_tables > 0)
    ਵਾਪਸ 1;

  /*
   * Allocate a new trampoline table structure.
   */
  ਸਾਰਣੀ = ਸਮ੍ਰਿਤੀ_ਦਿਓ (ਆਕਾਰ (*ਸਾਰਣੀ));
  ਜੇ (ਸਾਰਣੀ == ਨਿਰਾ)
    ਵਾਪਸ 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = ਸਮ੍ਰਿਤੀ_ਦਿਓ (ਆਕਾਰ (*tramp) * tramp_globals.ntramp);
  ਜੇ (tramp_array == ਨਿਰਾ)
    ਜਾਓ free_ਸਾਰਣੀ;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  ਜੇ (!tramp_ਸਾਰਣੀ_map (ਸਾਰਣੀ))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      ਜਾਓ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  ਸਾਰਣੀ->array = tramp_array;
  ਸਾਰਣੀ->ਮੁਕਤ = ਨਿਰਾ;
  ਸਾਰਣੀ->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  ਮਾਪ = tramp_globals.ਮਾਪ;
  ਕੋਡ = ਸਾਰਣੀ->ਕੋਡ_ਸਾਰਣੀ;
  parm = ਸਾਰਣੀ->parm_ਸਾਰਣੀ;
  ਲਈ (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->ਸਾਰਣੀ = ਸਾਰਣੀ;
      tramp->ਕੋਡ = ਕੋਡ;
      tramp->parm = (ਬਣਤਰ tramp_parm *) parm;
      tramp_ਜੋੜੋ (tramp);

      ਕੋਡ += ਮਾਪ;
      parm += ਮਾਪ;
    }
  /* Success */
  ਵਾਪਸ 1;

/* Failure */
free_tramp_array:
  ਮੁਕਤ (tramp_array);
free_ਸਾਰਣੀ:
  ਮੁਕਤ (ਸਾਰਣੀ);
  ਵਾਪਸ 0;
}

/*
 * Free a trampoline table.
 */
ਸਥਿਰ ਖਾਲੀ
tramp_ਸਾਰਣੀ_free (ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ)
{
  tramp_ਸਾਰਣੀ_unmap (ਸਾਰਣੀ);
  ਮੁਕਤ (ਸਾਰਣੀ->array);
  ਮੁਕਤ (ਸਾਰਣੀ);
}

/*
 * Add a new trampoline table to the global table list.
 */
ਸਥਿਰ ਖਾਲੀ
tramp_ਸਾਰਣੀ_ਜੋੜੋ (ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ)
{
  ਸਾਰਣੀ->ਅਗਲਾ = tramp_globals.free_tables;
  ਸਾਰਣੀ->prev = ਨਿਰਾ;
  ਜੇ (tramp_globals.free_tables != ਨਿਰਾ)
    tramp_globals.free_tables->prev = ਸਾਰਣੀ;
  tramp_globals.free_tables = ਸਾਰਣੀ;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ਸਥਿਰ ਖਾਲੀ
tramp_ਸਾਰਣੀ_del (ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ)
{
  tramp_globals.nfree_tables--;
  ਜੇ (ਸਾਰਣੀ->prev != ਨਿਰਾ)
    ਸਾਰਣੀ->prev->ਅਗਲਾ = ਸਾਰਣੀ->ਅਗਲਾ;
  ਜੇ (ਸਾਰਣੀ->ਅਗਲਾ != ਨਿਰਾ)
    ਸਾਰਣੀ->ਅਗਲਾ->prev = ਸਾਰਣੀ->prev;
  ਜੇ (tramp_globals.free_tables == ਸਾਰਣੀ)
    tramp_globals.free_tables = ਸਾਰਣੀ->ਅਗਲਾ;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ਸਥਿਰ ਖਾਲੀ
tramp_ਜੋੜੋ (ਬਣਤਰ tramp *tramp)
{
  ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ = tramp->ਸਾਰਣੀ;

  tramp->ਅਗਲਾ = ਸਾਰਣੀ->ਮੁਕਤ;
  tramp->prev = ਨਿਰਾ;
  ਜੇ (ਸਾਰਣੀ->ਮੁਕਤ != ਨਿਰਾ)
    ਸਾਰਣੀ->ਮੁਕਤ->prev = tramp;
  ਸਾਰਣੀ->ਮੁਕਤ = tramp;
  ਸਾਰਣੀ->nfree++;

  ਜੇ (ਸਾਰਣੀ->nfree == 1)
    tramp_ਸਾਰਣੀ_ਜੋੜੋ (ਸਾਰਣੀ);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  ਜੇ (ਸਾਰਣੀ->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_ਸਾਰਣੀ_del (ਸਾਰਣੀ);
      tramp_ਸਾਰਣੀ_free (ਸਾਰਣੀ);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ਸਥਿਰ ਖਾਲੀ
tramp_del (ਬਣਤਰ tramp *tramp)
{
  ਬਣਤਰ tramp_ਸਾਰਣੀ *ਸਾਰਣੀ = tramp->ਸਾਰਣੀ;

  ਸਾਰਣੀ->nfree--;
  ਜੇ (tramp->prev != ਨਿਰਾ)
    tramp->prev->ਅਗਲਾ = tramp->ਅਗਲਾ;
  ਜੇ (tramp->ਅਗਲਾ != ਨਿਰਾ)
    tramp->ਅਗਲਾ->prev = tramp->prev;
  ਜੇ (ਸਾਰਣੀ->ਮੁਕਤ == tramp)
    ਸਾਰਣੀ->ਮੁਕਤ = tramp->ਅਗਲਾ;

  ਜੇ (ਸਾਰਣੀ->nfree == 0)
    tramp_ਸਾਰਣੀ_del (ਸਾਰਣੀ);
}

/* ------------------------ Trampoline API functions ------------------------ */

ਪੂਰਨ
ffi_tramp_is_supported(ਖਾਲੀ)
{
  ਪੂਰਨ ret;

  ffi_tramp_ਤਾਲਾ();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  ਵਾਪਸ ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
ਖਾਲੀ *
ffi_tramp_alloc (ਪੂਰਨ flags)
{
  ਬਣਤਰ tramp *tramp;

  ffi_tramp_ਤਾਲਾ();

  ਜੇ (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      ਵਾਪਸ ਨਿਰਾ;
    }

  ਜੇ (!tramp_ਸਾਰਣੀ_alloc ())
    {
      ffi_tramp_unlock();
      ਵਾਪਸ ਨਿਰਾ;
    }

  tramp = tramp_globals.free_tables->ਮੁਕਤ;
  tramp_del (tramp);

  ffi_tramp_unlock();

  ਵਾਪਸ tramp;
}

/*
 * Set the parameters for a trampoline.
 */
ਖਾਲੀ
ffi_tramp_ਸੈੱਟ_parms (ਖਾਲੀ *arg, ਖਾਲੀ *ਨਿਸ਼ਾਨਾ_2, ਖਾਲੀ *ਅੰਕੜੇ)
{
  ਬਣਤਰ tramp *tramp = arg;

  ffi_tramp_ਤਾਲਾ();
  tramp->parm->ਨਿਸ਼ਾਨਾ_2 = ਨਿਸ਼ਾਨਾ_2;
  tramp->parm->ਅੰਕੜੇ = ਅੰਕੜੇ;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
ਖਾਲੀ *
ffi_tramp_get_addr (ਖਾਲੀ *arg)
{
  ਬਣਤਰ tramp *tramp = arg;
  ਖਾਲੀ *addr;

  ffi_tramp_ਤਾਲਾ();
  addr = tramp->ਕੋਡ;
  ffi_tramp_unlock();

  ਵਾਪਸ addr;
}

/*
 * Free a trampoline.
 */
ਖਾਲੀ
ffi_tramp_free (ਖਾਲੀ *arg)
{
  ਬਣਤਰ tramp *tramp = arg;

  ffi_tramp_ਤਾਲਾ();
  tramp_ਜੋੜੋ (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#ਨਹੀਂਤਾਂ /* !FFI_EXEC_STATIC_TRAMP */

#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

ਪੂਰਨ
ffi_tramp_is_supported(ਖਾਲੀ)
{
  ਵਾਪਸ 0;
}

ਖਾਲੀ *
ffi_tramp_alloc (ਪੂਰਨ flags)
{
  ਵਾਪਸ ਨਿਰਾ;
}

ਖਾਲੀ
ffi_tramp_ਸੈੱਟ_parms (ਖਾਲੀ *arg, ਖਾਲੀ *ਨਿਸ਼ਾਨਾ_2, ਖਾਲੀ *ਅੰਕੜੇ)
{
}

ਖਾਲੀ *
ffi_tramp_get_addr (ਖਾਲੀ *arg)
{
  ਵਾਪਸ ਨਿਰਾ;
}

ਖਾਲੀ
ffi_tramp_free (ਖਾਲੀ *arg)
{
}

#ਜੇ_ਅੰਤ /* FFI_EXEC_STATIC_TRAMP */
