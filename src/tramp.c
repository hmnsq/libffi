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

#که_تعریف_شوی FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#إذا defined (__linux__) || defined (__CYGWIN__)
#که_تعریف_شوی __linux__
#تعریف_کړه GNU_سرچینه 1
#د_که_پای

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
#که_تعریف_شوی __linux__
#ضمن <linux/limits.h>
#ضمن <linux/types.h>
#د_که_پای
#که_تعریف_شوی __CYGWIN__
#ضمن <limits.h>
#د_که_پای
#د_که_پای

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
فراغ __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_اندازه,
  size_t *map_اندازه);

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
بنية tramp_جدول
{
  بنية tramp_جدول *prev;
  بنية tramp_جدول *بل;
  فراغ *کوډ_جدول;
  فراغ *parm_جدول;
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
  فراغ *معلومات;
  فراغ *موخه_2;
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
  بنية tramp *بل;
  بنية tramp_جدول *جدول;
  فراغ *کوډ;
  بنية tramp_parm *parm;
};

تعداد tramp_globals_حالت {
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
  فراغ *متن;
  size_t map_اندازه;
  size_t اندازه;
  صحيح ntramp;
  بنية tramp_جدول *free_tables;
  صحيح nfree_tables;
  تعداد tramp_globals_حالت status;
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
ساكن صحيح tramp_جدول_alloc (فراغ);

#إذا defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#تعریف_کړه FFI_TRAMP_STR_(x) #x
#تعریف_کړه FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ساكن صحيح
ffi_tramp_get_libffi (فراغ)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  حرف file[لار_MAX+1], کرښه[لار_MAX+100], perm[10], dev[10];
  بینښه اوږد پیل, پای_ټکی, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.متن;
  صحيح nfields, وموندل;
  صحيح open_flags = O_RDONLY;

#که_تعریف_شوی O_CLOEXEC
  open_flags |= O_CLOEXEC;
#د_که_پای

  snprintf (file, حجم (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  إذا (fp == عدم)
    أرجع 0;

  وموندل = 0;
  طالما (feof (fp) == 0) {
    إذا (fgets (کرښه, حجم (کرښه), fp) == 0)
      اكسر;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = له_مزي_ولوله (کرښه,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(لار_MAX) "s",
      &پیل, &پای_ټکی, perm, &offset, dev, &inode, file);
    إذا (nfields != 7)
      استمر;

    إذا (addr >= پیل && addr < پای_ټکی) {
      tramp_globals.offset = offset + (addr - پیل);
      وموندل = 1;
      اكسر;
    }
  }
  fclose (fp);

  إذا (!وموندل)
    أرجع 0;

  tramp_globals.fd = open (file, open_flags);
  إذا (tramp_globals.fd == -1)
    أرجع 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  إذا (!tramp_جدول_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      أرجع 0;
    }
  أرجع 1;
}

#د_که_پای /* defined (__linux__) || defined (__CYGWIN__) */

#إذا defined (__linux__) || defined (__CYGWIN__)

ساكن صحيح
ffi_tramp_get_لنډمهاله_file (فراغ)
{
  ssize_t شمېر;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_لنډمهاله_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  شمېر = اكتب(tramp_globals.fd, tramp_globals.متن, tramp_globals.map_اندازه);
  إذا (شمېر >=0 && (size_t)شمېر == tramp_globals.map_اندازه && tramp_جدول_alloc ())
    أرجع 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  أرجع 0;
}

#د_که_پای /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#إذا defined (__linux__) || defined (__CYGWIN__)

ساكن صحيح
ffi_tramp_init_os (فراغ)
{
  إذا (ffi_tramp_get_libffi ())
    أرجع 1;
  أرجع ffi_tramp_get_لنډمهاله_file ();
}

#د_که_پای /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#إذا defined (__linux__) || defined (__CYGWIN__)

ساكن pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ساكن فراغ
ffi_tramp_قلف(فراغ)
{
  pthread_mutex_قلف (&tramp_globals_mutex);
}

ساكن فراغ
ffi_tramp_unlock(فراغ)
{
  pthread_mutex_unlock (&tramp_globals_mutex);
}

#د_که_پای /* defined (__linux__) || defined (__CYGWIN__) */

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
tramp_جدول_map (بنية tramp_جدول *جدول)
{
  حرف *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (عدم, tramp_globals.map_اندازه * 2, PROT_READ | PROT_WRITE,
    MAP_شخصي | MAP_ANONYMOUS, -1, 0);
  إذا (addr == MAP_FAILED)
    أرجع 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  جدول->کوډ_جدول = mmap (addr, tramp_globals.map_اندازه, PROT_READ | PROT_EXEC,
    MAP_شخصي | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  إذا (جدول->کوډ_جدول == MAP_FAILED)
    {
      (فراغ) munmap (addr, tramp_globals.map_اندازه * 2);
      أرجع 0;
    }
  جدول->parm_جدول = جدول->کوډ_جدول + tramp_globals.map_اندازه;
  أرجع 1;
}

ساكن فراغ
tramp_جدول_unmap (بنية tramp_جدول *جدول)
{
  (فراغ) munmap (جدول->کوډ_جدول, tramp_globals.map_اندازه);
  (فراغ) munmap (جدول->parm_جدول, tramp_globals.map_اندازه);
}

#د_که_پای /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ساكن صحيح
ffi_tramp_init (فراغ)
{
  اوږد پاڼه_اندازه;

  إذا (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    أرجع 1;

  إذا (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    أرجع 0;

  إذا (ffi_tramp_arch == عدم)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      أرجع 0;
    }

  tramp_globals.free_tables = عدم;
  tramp_globals.nfree_tables = 0;

  /*
   * Get trampoline code table information from the architecture.
   */
  tramp_globals.متن = ffi_tramp_arch (&tramp_globals.اندازه,
    &tramp_globals.map_اندازه);
  tramp_globals.ntramp = tramp_globals.map_اندازه / tramp_globals.اندازه;

  /*
   * The trampoline code table is a single, fixed-size mapping.  If the
   * system page size is larger than that mapping, the static trampoline
   * mechanism cannot be used.  Both values are invariant for the life of
   * the process, so cache the FAILED verdict rather than re-running the
   * whole initialization on every allocation.
   */
  پاڼه_اندازه = sysconf (_SC_PAGESIZE);
  إذا (پاڼه_اندازه >= 0 && (size_t)پاڼه_اندازه > tramp_globals.map_اندازه)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      أرجع 0;
    }

  إذا (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      أرجع 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  أرجع 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ساكن فراغ tramp_جمع_کړه (بنية tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ساكن صحيح
tramp_جدول_alloc (فراغ)
{
  بنية tramp_جدول *جدول;
  بنية tramp *tramp_array, *tramp;
  size_t اندازه;
  حرف *کوډ, *parm;
  صحيح i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  إذا (tramp_globals.nfree_tables > 0)
    أرجع 1;

  /*
   * Allocate a new trampoline table structure.
   */
  جدول = حافظه_ورکړه (حجم (*جدول));
  إذا (جدول == عدم)
    أرجع 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = حافظه_ورکړه (حجم (*tramp) * tramp_globals.ntramp);
  إذا (tramp_array == عدم)
    اذهب free_جدول;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  إذا (!tramp_جدول_map (جدول))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      اذهب free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  جدول->array = tramp_array;
  جدول->حرر = عدم;
  جدول->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  اندازه = tramp_globals.اندازه;
  کوډ = جدول->کوډ_جدول;
  parm = جدول->parm_جدول;
  لكل (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->جدول = جدول;
      tramp->کوډ = کوډ;
      tramp->parm = (بنية tramp_parm *) parm;
      tramp_جمع_کړه (tramp);

      کوډ += اندازه;
      parm += اندازه;
    }
  /* Success */
  أرجع 1;

/* Failure */
free_tramp_array:
  حرر (tramp_array);
free_جدول:
  حرر (جدول);
  أرجع 0;
}

/*
 * Free a trampoline table.
 */
ساكن فراغ
tramp_جدول_free (بنية tramp_جدول *جدول)
{
  tramp_جدول_unmap (جدول);
  حرر (جدول->array);
  حرر (جدول);
}

/*
 * Add a new trampoline table to the global table list.
 */
ساكن فراغ
tramp_جدول_جمع_کړه (بنية tramp_جدول *جدول)
{
  جدول->بل = tramp_globals.free_tables;
  جدول->prev = عدم;
  إذا (tramp_globals.free_tables != عدم)
    tramp_globals.free_tables->prev = جدول;
  tramp_globals.free_tables = جدول;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ساكن فراغ
tramp_جدول_del (بنية tramp_جدول *جدول)
{
  tramp_globals.nfree_tables--;
  إذا (جدول->prev != عدم)
    جدول->prev->بل = جدول->بل;
  إذا (جدول->بل != عدم)
    جدول->بل->prev = جدول->prev;
  إذا (tramp_globals.free_tables == جدول)
    tramp_globals.free_tables = جدول->بل;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ساكن فراغ
tramp_جمع_کړه (بنية tramp *tramp)
{
  بنية tramp_جدول *جدول = tramp->جدول;

  tramp->بل = جدول->حرر;
  tramp->prev = عدم;
  إذا (جدول->حرر != عدم)
    جدول->حرر->prev = tramp;
  جدول->حرر = tramp;
  جدول->nfree++;

  إذا (جدول->nfree == 1)
    tramp_جدول_جمع_کړه (جدول);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  إذا (جدول->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_جدول_del (جدول);
      tramp_جدول_free (جدول);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ساكن فراغ
tramp_del (بنية tramp *tramp)
{
  بنية tramp_جدول *جدول = tramp->جدول;

  جدول->nfree--;
  إذا (tramp->prev != عدم)
    tramp->prev->بل = tramp->بل;
  إذا (tramp->بل != عدم)
    tramp->بل->prev = tramp->prev;
  إذا (جدول->حرر == tramp)
    جدول->حرر = tramp->بل;

  إذا (جدول->nfree == 0)
    tramp_جدول_del (جدول);
}

/* ------------------------ Trampoline API functions ------------------------ */

صحيح
ffi_tramp_is_supported(فراغ)
{
  صحيح ret;

  ffi_tramp_قلف();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  أرجع ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
فراغ *
ffi_tramp_alloc (صحيح flags)
{
  بنية tramp *tramp;

  ffi_tramp_قلف();

  إذا (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      أرجع عدم;
    }

  إذا (!tramp_جدول_alloc ())
    {
      ffi_tramp_unlock();
      أرجع عدم;
    }

  tramp = tramp_globals.free_tables->حرر;
  tramp_del (tramp);

  ffi_tramp_unlock();

  أرجع tramp;
}

/*
 * Set the parameters for a trampoline.
 */
فراغ
ffi_tramp_سېټ_parms (فراغ *arg, فراغ *موخه_2, فراغ *معلومات)
{
  بنية tramp *tramp = arg;

  ffi_tramp_قلف();
  tramp->parm->موخه_2 = موخه_2;
  tramp->parm->معلومات = معلومات;
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

  ffi_tramp_قلف();
  addr = tramp->کوډ;
  ffi_tramp_unlock();

  أرجع addr;
}

/*
 * Free a trampoline.
 */
فراغ
ffi_tramp_free (فراغ *arg)
{
  بنية tramp *tramp = arg;

  ffi_tramp_قلف();
  tramp_جمع_کړه (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#وإلا /* !FFI_EXEC_STATIC_TRAMP */

#ضمن <stddef.h>

صحيح
ffi_tramp_is_supported(فراغ)
{
  أرجع 0;
}

فراغ *
ffi_tramp_alloc (صحيح flags)
{
  أرجع عدم;
}

فراغ
ffi_tramp_سېټ_parms (فراغ *arg, فراغ *موخه_2, فراغ *معلومات)
{
}

فراغ *
ffi_tramp_get_addr (فراغ *arg)
{
  أرجع عدم;
}

فراغ
ffi_tramp_free (فراغ *arg)
{
}

#د_که_پای /* FFI_EXEC_STATIC_TRAMP */
