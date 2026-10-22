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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#ಆದರೆ defined (__linux__) || defined (__CYGWIN__)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
#ವ್ಯಾಖ್ಯಾನಿಸು GNU_ಮೂಲ 1
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
#ಅನ್ತರ್ಭಾವ <linux/limits.h>
#ಅನ್ತರ್ಭಾವ <linux/types.h>
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

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
ಶೂನ್ಯ __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ,
  size_t *map_ಗಾತ್ರ);

/* ------------------------- Trampoline Data Structures --------------------*/

ರಚನೆ tramp;

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
ರಚನೆ tramp_ಕೋಷ್ಟಕ
{
  ರಚನೆ tramp_ಕೋಷ್ಟಕ *prev;
  ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಮುಂದಿನ;
  ಶೂನ್ಯ *ಸಂಕೇತ_ಕೋಷ್ಟಕ;
  ಶೂನ್ಯ *parm_ಕೋಷ್ಟಕ;
  ರಚನೆ tramp *array;
  ರಚನೆ tramp *ಬಿಡುಗಡೆ;
  ಪೂರ್ಣಾಂಕ nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
ರಚನೆ tramp_parm
{
  ಶೂನ್ಯ *ದತ್ತಾಂಶ;
  ಶೂನ್ಯ *ಗುರಿ_2;
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
ರಚನೆ tramp
{
  ರಚನೆ tramp *prev;
  ರಚನೆ tramp *ಮುಂದಿನ;
  ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ;
  ಶೂನ್ಯ *ಸಂಕೇತ_2;
  ರಚನೆ tramp_parm *parm;
};

ಎಣಿಕೆ tramp_globals_ಸ್ಥಿತಿ {
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
ರಚನೆ tramp_globals
{
  ಪೂರ್ಣಾಂಕ fd;
  off_t offset;
  ಶೂನ್ಯ *ಪಠ್ಯ;
  size_t map_ಗಾತ್ರ;
  size_t ಗಾತ್ರ_2;
  ಪೂರ್ಣಾಂಕ ntramp;
  ರಚನೆ tramp_ಕೋಷ್ಟಕ *free_tables;
  ಪೂರ್ಣಾಂಕ nfree_tables;
  ಎಣಿಕೆ tramp_globals_ಸ್ಥಿತಿ status;
};

ಸ್ಥಿರ ರಚನೆ tramp_globals tramp_globals;

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tramp_ಕೋಷ್ಟಕ_alloc (ಶೂನ್ಯ);

#ಆದರೆ defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMP_STR_(x) #x
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_tramp_get_libffi (ಶೂನ್ಯ)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  ಅಕ್ಷರ file[ಮಾರ್ಗ_MAX+1], ಸಾಲು[ಮಾರ್ಗ_MAX+100], perm[10], dev[10];
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಆರಂಭ, ಕೊನೆ, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.ಪಠ್ಯ;
  ಪೂರ್ಣಾಂಕ nfields, ಸಿಕ್ಕಿತು;
  ಪೂರ್ಣಾಂಕ open_flags = O_RDONLY;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_CLOEXEC
  open_flags |= O_CLOEXEC;
#ಆದರೆ_ಅಂತ್ಯ

  snprintf (file, ಗಾತ್ರ (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  ಆದರೆ (fp == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;

  ಸಿಕ್ಕಿತು = 0;
  ಆಗ (feof (fp) == 0) {
    ಆದರೆ (fgets (ಸಾಲು, ಗಾತ್ರ (ಸಾಲು), fp) == 0)
      ನಿಲ್ಲಿಸು;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = ಅಕ್ಷರಸರಣಿ_ಓದು (ಸಾಲು,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(ಮಾರ್ಗ_MAX) "s",
      &ಆರಂಭ, &ಕೊನೆ, perm, &offset, dev, &inode, file);
    ಆದರೆ (nfields != 7)
      ಮುಂದುವರಿ;

    ಆದರೆ (addr >= ಆರಂಭ && addr < ಕೊನೆ) {
      tramp_globals.offset = offset + (addr - ಆರಂಭ);
      ಸಿಕ್ಕಿತು = 1;
      ನಿಲ್ಲಿಸು;
    }
  }
  fclose (fp);

  ಆದರೆ (!ಸಿಕ್ಕಿತು)
    ಹಿಂತಿರುಗಿ 0;

  tramp_globals.fd = open (file, open_flags);
  ಆದರೆ (tramp_globals.fd == -1)
    ಹಿಂತಿರುಗಿ 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  ಆದರೆ (!tramp_ಕೋಷ್ಟಕ_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      ಹಿಂತಿರುಗಿ 0;
    }
  ಹಿಂತಿರುಗಿ 1;
}

#ಆದರೆ_ಅಂತ್ಯ /* defined (__linux__) || defined (__CYGWIN__) */

#ಆದರೆ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_tramp_get_ತಾತ್ಕಾಲಿಕ_file (ಶೂನ್ಯ)
{
  ssize_t ಎಣಿಕೆ_2;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_ತಾತ್ಕಾಲಿಕ_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  ಎಣಿಕೆ_2 = ಲಿಖ(tramp_globals.fd, tramp_globals.ಪಠ್ಯ, tramp_globals.map_ಗಾತ್ರ);
  ಆದರೆ (ಎಣಿಕೆ_2 >=0 && (size_t)ಎಣಿಕೆ_2 == tramp_globals.map_ಗಾತ್ರ && tramp_ಕೋಷ್ಟಕ_alloc ())
    ಹಿಂತಿರುಗಿ 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#ಆದರೆ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_tramp_init_os (ಶೂನ್ಯ)
{
  ಆದರೆ (ffi_tramp_get_libffi ())
    ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ ffi_tramp_get_ತಾತ್ಕಾಲಿಕ_file ();
}

#ಆದರೆ_ಅಂತ್ಯ /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#ಆದರೆ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_tramp_ಬೀಗ(ಶೂನ್ಯ)
{
  pthread_mutex_ಬೀಗ (&tramp_globals_mutex);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_tramp_unlock(ಶೂನ್ಯ)
{
  pthread_mutex_unlock (&tramp_globals_mutex);
}

#ಆದರೆ_ಅಂತ್ಯ /* defined (__linux__) || defined (__CYGWIN__) */

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

#ಆದರೆ defined (__linux__) || defined (__CYGWIN__)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tramp_ಕೋಷ್ಟಕ_map (ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ)
{
  ಅಕ್ಷರ *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (ಖಾಲಿ, tramp_globals.map_ಗಾತ್ರ * 2, PROT_READ | PROT_WRITE,
    MAP_ಖಾಸಗಿ | MAP_ANONYMOUS, -1, 0);
  ಆದರೆ (addr == MAP_FAILED)
    ಹಿಂತಿರುಗಿ 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  ಕೋಷ್ಟಕ->ಸಂಕೇತ_ಕೋಷ್ಟಕ = mmap (addr, tramp_globals.map_ಗಾತ್ರ, PROT_READ | PROT_EXEC,
    MAP_ಖಾಸಗಿ | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  ಆದರೆ (ಕೋಷ್ಟಕ->ಸಂಕೇತ_ಕೋಷ್ಟಕ == MAP_FAILED)
    {
      (ಶೂನ್ಯ) munmap (addr, tramp_globals.map_ಗಾತ್ರ * 2);
      ಹಿಂತಿರುಗಿ 0;
    }
  ಕೋಷ್ಟಕ->parm_ಕೋಷ್ಟಕ = ಕೋಷ್ಟಕ->ಸಂಕೇತ_ಕೋಷ್ಟಕ + tramp_globals.map_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
tramp_ಕೋಷ್ಟಕ_unmap (ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ)
{
  (ಶೂನ್ಯ) munmap (ಕೋಷ್ಟಕ->ಸಂಕೇತ_ಕೋಷ್ಟಕ, tramp_globals.map_ಗಾತ್ರ);
  (ಶೂನ್ಯ) munmap (ಕೋಷ್ಟಕ->parm_ಕೋಷ್ಟಕ, tramp_globals.map_ಗಾತ್ರ);
}

#ಆದರೆ_ಅಂತ್ಯ /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_tramp_init (ಶೂನ್ಯ)
{
  ದೀರ್ಘ ಪುಟ_ಗಾತ್ರ;

  ಆದರೆ (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    ಹಿಂತಿರುಗಿ 1;

  ಆದರೆ (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    ಹಿಂತಿರುಗಿ 0;

  ಆದರೆ (ffi_tramp_arch == ಖಾಲಿ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ಹಿಂತಿರುಗಿ 0;
    }

  tramp_globals.free_tables = ಖಾಲಿ;
  tramp_globals.nfree_tables = 0;

  /*
   * Get trampoline code table information from the architecture.
   */
  tramp_globals.ಪಠ್ಯ = ffi_tramp_arch (&tramp_globals.ಗಾತ್ರ_2,
    &tramp_globals.map_ಗಾತ್ರ);
  tramp_globals.ntramp = tramp_globals.map_ಗಾತ್ರ / tramp_globals.ಗಾತ್ರ_2;

  /*
   * The trampoline code table is a single, fixed-size mapping.  If the
   * system page size is larger than that mapping, the static trampoline
   * mechanism cannot be used.  Both values are invariant for the life of
   * the process, so cache the FAILED verdict rather than re-running the
   * whole initialization on every allocation.
   */
  ಪುಟ_ಗಾತ್ರ = sysconf (_SC_PAGESIZE);
  ಆದರೆ (ಪುಟ_ಗಾತ್ರ >= 0 && (size_t)ಪುಟ_ಗಾತ್ರ > tramp_globals.map_ಗಾತ್ರ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      ಹಿಂತಿರುಗಿ 0;
    }

  ಆದರೆ (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      ಹಿಂತಿರುಗಿ 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  ಹಿಂತಿರುಗಿ 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ಸ್ಥಿರ ಶೂನ್ಯ tramp_ಕೂಡಿಸು (ರಚನೆ tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
tramp_ಕೋಷ್ಟಕ_alloc (ಶೂನ್ಯ)
{
  ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ;
  ರಚನೆ tramp *tramp_array, *tramp;
  size_t ಗಾತ್ರ_2;
  ಅಕ್ಷರ *ಸಂಕೇತ_2, *parm;
  ಪೂರ್ಣಾಂಕ i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  ಆದರೆ (tramp_globals.nfree_tables > 0)
    ಹಿಂತಿರುಗಿ 1;

  /*
   * Allocate a new trampoline table structure.
   */
  ಕೋಷ್ಟಕ = ಸ್ಮರಣೆ_ಕೊಡು (ಗಾತ್ರ (*ಕೋಷ್ಟಕ));
  ಆದರೆ (ಕೋಷ್ಟಕ == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = ಸ್ಮರಣೆ_ಕೊಡು (ಗಾತ್ರ (*tramp) * tramp_globals.ntramp);
  ಆದರೆ (tramp_array == ಖಾಲಿ)
    ಹೋಗು free_ಕೋಷ್ಟಕ;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  ಆದರೆ (!tramp_ಕೋಷ್ಟಕ_map (ಕೋಷ್ಟಕ))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      ಹೋಗು free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  ಕೋಷ್ಟಕ->array = tramp_array;
  ಕೋಷ್ಟಕ->ಬಿಡುಗಡೆ = ಖಾಲಿ;
  ಕೋಷ್ಟಕ->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  ಗಾತ್ರ_2 = tramp_globals.ಗಾತ್ರ_2;
  ಸಂಕೇತ_2 = ಕೋಷ್ಟಕ->ಸಂಕೇತ_ಕೋಷ್ಟಕ;
  parm = ಕೋಷ್ಟಕ->parm_ಕೋಷ್ಟಕ;
  ಗಾಗಿ (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->ಕೋಷ್ಟಕ = ಕೋಷ್ಟಕ;
      tramp->ಸಂಕೇತ_2 = ಸಂಕೇತ_2;
      tramp->parm = (ರಚನೆ tramp_parm *) parm;
      tramp_ಕೂಡಿಸು (tramp);

      ಸಂಕೇತ_2 += ಗಾತ್ರ_2;
      parm += ಗಾತ್ರ_2;
    }
  /* Success */
  ಹಿಂತಿರುಗಿ 1;

/* Failure */
free_tramp_array:
  ಬಿಡುಗಡೆ (tramp_array);
free_ಕೋಷ್ಟಕ:
  ಬಿಡುಗಡೆ (ಕೋಷ್ಟಕ);
  ಹಿಂತಿರುಗಿ 0;
}

/*
 * Free a trampoline table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_ಕೋಷ್ಟಕ_free (ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ)
{
  tramp_ಕೋಷ್ಟಕ_unmap (ಕೋಷ್ಟಕ);
  ಬಿಡುಗಡೆ (ಕೋಷ್ಟಕ->array);
  ಬಿಡುಗಡೆ (ಕೋಷ್ಟಕ);
}

/*
 * Add a new trampoline table to the global table list.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_ಕೋಷ್ಟಕ_ಕೂಡಿಸು (ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ)
{
  ಕೋಷ್ಟಕ->ಮುಂದಿನ = tramp_globals.free_tables;
  ಕೋಷ್ಟಕ->prev = ಖಾಲಿ;
  ಆದರೆ (tramp_globals.free_tables != ಖಾಲಿ)
    tramp_globals.free_tables->prev = ಕೋಷ್ಟಕ;
  tramp_globals.free_tables = ಕೋಷ್ಟಕ;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_ಕೋಷ್ಟಕ_del (ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ)
{
  tramp_globals.nfree_tables--;
  ಆದರೆ (ಕೋಷ್ಟಕ->prev != ಖಾಲಿ)
    ಕೋಷ್ಟಕ->prev->ಮುಂದಿನ = ಕೋಷ್ಟಕ->ಮುಂದಿನ;
  ಆದರೆ (ಕೋಷ್ಟಕ->ಮುಂದಿನ != ಖಾಲಿ)
    ಕೋಷ್ಟಕ->ಮುಂದಿನ->prev = ಕೋಷ್ಟಕ->prev;
  ಆದರೆ (tramp_globals.free_tables == ಕೋಷ್ಟಕ)
    tramp_globals.free_tables = ಕೋಷ್ಟಕ->ಮುಂದಿನ;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_ಕೂಡಿಸು (ರಚನೆ tramp *tramp)
{
  ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ = tramp->ಕೋಷ್ಟಕ;

  tramp->ಮುಂದಿನ = ಕೋಷ್ಟಕ->ಬಿಡುಗಡೆ;
  tramp->prev = ಖಾಲಿ;
  ಆದರೆ (ಕೋಷ್ಟಕ->ಬಿಡುಗಡೆ != ಖಾಲಿ)
    ಕೋಷ್ಟಕ->ಬಿಡುಗಡೆ->prev = tramp;
  ಕೋಷ್ಟಕ->ಬಿಡುಗಡೆ = tramp;
  ಕೋಷ್ಟಕ->nfree++;

  ಆದರೆ (ಕೋಷ್ಟಕ->nfree == 1)
    tramp_ಕೋಷ್ಟಕ_ಕೂಡಿಸು (ಕೋಷ್ಟಕ);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  ಆದರೆ (ಕೋಷ್ಟಕ->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_ಕೋಷ್ಟಕ_del (ಕೋಷ್ಟಕ);
      tramp_ಕೋಷ್ಟಕ_free (ಕೋಷ್ಟಕ);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ಸ್ಥಿರ ಶೂನ್ಯ
tramp_del (ರಚನೆ tramp *tramp)
{
  ರಚನೆ tramp_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ = tramp->ಕೋಷ್ಟಕ;

  ಕೋಷ್ಟಕ->nfree--;
  ಆದರೆ (tramp->prev != ಖಾಲಿ)
    tramp->prev->ಮುಂದಿನ = tramp->ಮುಂದಿನ;
  ಆದರೆ (tramp->ಮುಂದಿನ != ಖಾಲಿ)
    tramp->ಮುಂದಿನ->prev = tramp->prev;
  ಆದರೆ (ಕೋಷ್ಟಕ->ಬಿಡುಗಡೆ == tramp)
    ಕೋಷ್ಟಕ->ಬಿಡುಗಡೆ = tramp->ಮುಂದಿನ;

  ಆದರೆ (ಕೋಷ್ಟಕ->nfree == 0)
    tramp_ಕೋಷ್ಟಕ_del (ಕೋಷ್ಟಕ);
}

/* ------------------------ Trampoline API functions ------------------------ */

ಪೂರ್ಣಾಂಕ
ffi_tramp_is_supported(ಶೂನ್ಯ)
{
  ಪೂರ್ಣಾಂಕ ret;

  ffi_tramp_ಬೀಗ();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  ಹಿಂತಿರುಗಿ ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
ಶೂನ್ಯ *
ffi_tramp_alloc (ಪೂರ್ಣಾಂಕ flags)
{
  ರಚನೆ tramp *tramp;

  ffi_tramp_ಬೀಗ();

  ಆದರೆ (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

  ಆದರೆ (!tramp_ಕೋಷ್ಟಕ_alloc ())
    {
      ffi_tramp_unlock();
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

  tramp = tramp_globals.free_tables->ಬಿಡುಗಡೆ;
  tramp_del (tramp);

  ffi_tramp_unlock();

  ಹಿಂತಿರುಗಿ tramp;
}

/*
 * Set the parameters for a trampoline.
 */
ಶೂನ್ಯ
ffi_tramp_ಗಣ_parms (ಶೂನ್ಯ *arg, ಶೂನ್ಯ *ಗುರಿ_2, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
  ರಚನೆ tramp *tramp = arg;

  ffi_tramp_ಬೀಗ();
  tramp->parm->ಗುರಿ_2 = ಗುರಿ_2;
  tramp->parm->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
ಶೂನ್ಯ *
ffi_tramp_get_addr (ಶೂನ್ಯ *arg)
{
  ರಚನೆ tramp *tramp = arg;
  ಶೂನ್ಯ *addr;

  ffi_tramp_ಬೀಗ();
  addr = tramp->ಸಂಕೇತ_2;
  ffi_tramp_unlock();

  ಹಿಂತಿರುಗಿ addr;
}

/*
 * Free a trampoline.
 */
ಶೂನ್ಯ
ffi_tramp_free (ಶೂನ್ಯ *arg)
{
  ರಚನೆ tramp *tramp = arg;

  ffi_tramp_ಬೀಗ();
  tramp_ಕೂಡಿಸು (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#ಇಲ್ಲದಿದ್ದರೆ /* !FFI_EXEC_STATIC_TRAMP */

#ಅನ್ತರ್ಭಾವ <stddef.h>

ಪೂರ್ಣಾಂಕ
ffi_tramp_is_supported(ಶೂನ್ಯ)
{
  ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ *
ffi_tramp_alloc (ಪೂರ್ಣಾಂಕ flags)
{
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಶೂನ್ಯ
ffi_tramp_ಗಣ_parms (ಶೂನ್ಯ *arg, ಶೂನ್ಯ *ಗುರಿ_2, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
}

ಶೂನ್ಯ *
ffi_tramp_get_addr (ಶೂನ್ಯ *arg)
{
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಶೂನ್ಯ
ffi_tramp_free (ಶೂನ್ಯ *arg)
{
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_EXEC_STATIC_TRAMP */
