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

#اگر_تعریف_شدہ FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#اگر defined (__linux__) || defined (__CYGWIN__)
#اگر_تعریف_شدہ __linux__
#تعریف_کرو GNU_ماخذ 1
#اگر_ختم

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
#اگر_تعریف_شدہ __linux__
#شامل <linux/limits.h>
#شامل <linux/types.h>
#اگر_ختم
#اگر_تعریف_شدہ __CYGWIN__
#شامل <limits.h>
#اگر_ختم
#اگر_ختم

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
خالی __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_سائز,
  size_t *map_سائز);

/* ------------------------- Trampoline Data Structures --------------------*/

ساخت tramp;

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
ساخت tramp_جدول
{
  ساخت tramp_جدول *پچھلا;
  ساخت tramp_جدول *اگلا;
  خالی *رمز_جدول;
  خالی *parm_جدول;
  ساخت tramp *array;
  ساخت tramp *آزاد;
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
ساخت tramp_parm
{
  خالی *ڈیٹا;
  خالی *ہدف_2;
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
ساخت tramp
{
  ساخت tramp *پچھلا;
  ساخت tramp *اگلا;
  ساخت tramp_جدول *جدول;
  خالی *رمز;
  ساخت tramp_parm *parm;
};

شمار tramp_globals_حالت {
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
ساخت tramp_globals
{
  صحیح fd;
  off_t offset;
  خالی *متن;
  size_t map_سائز;
  size_t سائز;
  صحیح ntramp;
  ساخت tramp_جدول *free_tables;
  صحیح nfree_tables;
  شمار tramp_globals_حالت status;
};

ساکن ساخت tramp_globals tramp_globals;

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
ساکن صحیح tramp_جدول_alloc (خالی);

#اگر defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#تعریف_کرو FFI_TRAMP_STR_(x) #x
#تعریف_کرو FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

ساکن صحیح
ffi_tramp_get_libffi (خالی)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  حرف file[راستہ_MAX+1], سطر[راستہ_MAX+100], perm[10], dev[10];
  غیرعلامتی طویل آغاز, انتہا, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.متن;
  صحیح nfields, ملا;
  صحیح open_flags = O_RDONLY;

#اگر_تعریف_شدہ O_CLOEXEC
  open_flags |= O_CLOEXEC;
#اگر_ختم

  snprintf (file, حجم (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  اگر (fp == تہی)
    واپس 0;

  ملا = 0;
  جب (feof (fp) == 0) {
    اگر (fgets (سطر, حجم (سطر), fp) == 0)
      توڑو;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = لڑی_سے_پڑھو (سطر,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(راستہ_MAX) "s",
      &آغاز, &انتہا, perm, &offset, dev, &inode, file);
    اگر (nfields != 7)
      جاری;

    اگر (addr >= آغاز && addr < انتہا) {
      tramp_globals.offset = offset + (addr - آغاز);
      ملا = 1;
      توڑو;
    }
  }
  fclose (fp);

  اگر (!ملا)
    واپس 0;

  tramp_globals.fd = open (file, open_flags);
  اگر (tramp_globals.fd == -1)
    واپس 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  اگر (!tramp_جدول_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      واپس 0;
    }
  واپس 1;
}

#اگر_ختم /* defined (__linux__) || defined (__CYGWIN__) */

#اگر defined (__linux__) || defined (__CYGWIN__)

ساکن صحیح
ffi_tramp_get_عارضی_file (خالی)
{
  ssize_t گنتی;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_عارضی_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  گنتی = اكتب(tramp_globals.fd, tramp_globals.متن, tramp_globals.map_سائز);
  اگر (گنتی >=0 && (size_t)گنتی == tramp_globals.map_سائز && tramp_جدول_alloc ())
    واپس 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  واپس 0;
}

#اگر_ختم /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#اگر defined (__linux__) || defined (__CYGWIN__)

ساکن صحیح
ffi_tramp_init_os (خالی)
{
  اگر (ffi_tramp_get_libffi ())
    واپس 1;
  واپس ffi_tramp_get_عارضی_file ();
}

#اگر_ختم /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#اگر defined (__linux__) || defined (__CYGWIN__)

ساکن pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

ساکن خالی
ffi_tramp_تالا(خالی)
{
  pthread_mutex_تالا (&tramp_globals_mutex);
}

ساکن خالی
ffi_tramp_unlock(خالی)
{
  pthread_mutex_unlock (&tramp_globals_mutex);
}

#اگر_ختم /* defined (__linux__) || defined (__CYGWIN__) */

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

ساکن صحیح
tramp_جدول_map (ساخت tramp_جدول *جدول)
{
  حرف *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (تہی, tramp_globals.map_سائز * 2, PROT_READ | PROT_WRITE,
    MAP_نجی | MAP_ANONYMOUS, -1, 0);
  اگر (addr == MAP_FAILED)
    واپس 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  جدول->رمز_جدول = mmap (addr, tramp_globals.map_سائز, PROT_READ | PROT_EXEC,
    MAP_نجی | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  اگر (جدول->رمز_جدول == MAP_FAILED)
    {
      (خالی) munmap (addr, tramp_globals.map_سائز * 2);
      واپس 0;
    }
  جدول->parm_جدول = جدول->رمز_جدول + tramp_globals.map_سائز;
  واپس 1;
}

ساکن خالی
tramp_جدول_unmap (ساخت tramp_جدول *جدول)
{
  (خالی) munmap (جدول->رمز_جدول, tramp_globals.map_سائز);
  (خالی) munmap (جدول->parm_جدول, tramp_globals.map_سائز);
}

#اگر_ختم /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
ساکن صحیح
ffi_tramp_init (خالی)
{
  طویل صفحہ_سائز;

  اگر (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    واپس 1;

  اگر (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    واپس 0;

  اگر (ffi_tramp_arch == تہی)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      واپس 0;
    }

  tramp_globals.free_tables = تہی;
  tramp_globals.nfree_tables = 0;

  /*
   * Get trampoline code table information from the architecture.
   */
  tramp_globals.متن = ffi_tramp_arch (&tramp_globals.سائز,
    &tramp_globals.map_سائز);
  tramp_globals.ntramp = tramp_globals.map_سائز / tramp_globals.سائز;

  /*
   * The trampoline code table is a single, fixed-size mapping.  If the
   * system page size is larger than that mapping, the static trampoline
   * mechanism cannot be used.  Both values are invariant for the life of
   * the process, so cache the FAILED verdict rather than re-running the
   * whole initialization on every allocation.
   */
  صفحہ_سائز = sysconf (_SC_PAGESIZE);
  اگر (صفحہ_سائز >= 0 && (size_t)صفحہ_سائز > tramp_globals.map_سائز)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      واپس 0;
    }

  اگر (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      واپس 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  واپس 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

ساکن خالی tramp_جوڑو (ساخت tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
ساکن صحیح
tramp_جدول_alloc (خالی)
{
  ساخت tramp_جدول *جدول;
  ساخت tramp *tramp_array, *tramp;
  size_t سائز;
  حرف *رمز, *parm;
  صحیح i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  اگر (tramp_globals.nfree_tables > 0)
    واپس 1;

  /*
   * Allocate a new trampoline table structure.
   */
  جدول = حافظہ_دو (حجم (*جدول));
  اگر (جدول == تہی)
    واپس 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = حافظہ_دو (حجم (*tramp) * tramp_globals.ntramp);
  اگر (tramp_array == تہی)
    جاؤ free_جدول;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  اگر (!tramp_جدول_map (جدول))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      جاؤ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  جدول->array = tramp_array;
  جدول->آزاد = تہی;
  جدول->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  سائز = tramp_globals.سائز;
  رمز = جدول->رمز_جدول;
  parm = جدول->parm_جدول;
  برائے (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->جدول = جدول;
      tramp->رمز = رمز;
      tramp->parm = (ساخت tramp_parm *) parm;
      tramp_جوڑو (tramp);

      رمز += سائز;
      parm += سائز;
    }
  /* Success */
  واپس 1;

/* Failure */
free_tramp_array:
  آزاد (tramp_array);
free_جدول:
  آزاد (جدول);
  واپس 0;
}

/*
 * Free a trampoline table.
 */
ساکن خالی
tramp_جدول_free (ساخت tramp_جدول *جدول)
{
  tramp_جدول_unmap (جدول);
  آزاد (جدول->array);
  آزاد (جدول);
}

/*
 * Add a new trampoline table to the global table list.
 */
ساکن خالی
tramp_جدول_جوڑو (ساخت tramp_جدول *جدول)
{
  جدول->اگلا = tramp_globals.free_tables;
  جدول->پچھلا = تہی;
  اگر (tramp_globals.free_tables != تہی)
    tramp_globals.free_tables->پچھلا = جدول;
  tramp_globals.free_tables = جدول;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
ساکن خالی
tramp_جدول_del (ساخت tramp_جدول *جدول)
{
  tramp_globals.nfree_tables--;
  اگر (جدول->پچھلا != تہی)
    جدول->پچھلا->اگلا = جدول->اگلا;
  اگر (جدول->اگلا != تہی)
    جدول->اگلا->پچھلا = جدول->پچھلا;
  اگر (tramp_globals.free_tables == جدول)
    tramp_globals.free_tables = جدول->اگلا;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
ساکن خالی
tramp_جوڑو (ساخت tramp *tramp)
{
  ساخت tramp_جدول *جدول = tramp->جدول;

  tramp->اگلا = جدول->آزاد;
  tramp->پچھلا = تہی;
  اگر (جدول->آزاد != تہی)
    جدول->آزاد->پچھلا = tramp;
  جدول->آزاد = tramp;
  جدول->nfree++;

  اگر (جدول->nfree == 1)
    tramp_جدول_جوڑو (جدول);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  اگر (جدول->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_جدول_del (جدول);
      tramp_جدول_free (جدول);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
ساکن خالی
tramp_del (ساخت tramp *tramp)
{
  ساخت tramp_جدول *جدول = tramp->جدول;

  جدول->nfree--;
  اگر (tramp->پچھلا != تہی)
    tramp->پچھلا->اگلا = tramp->اگلا;
  اگر (tramp->اگلا != تہی)
    tramp->اگلا->پچھلا = tramp->پچھلا;
  اگر (جدول->آزاد == tramp)
    جدول->آزاد = tramp->اگلا;

  اگر (جدول->nfree == 0)
    tramp_جدول_del (جدول);
}

/* ------------------------ Trampoline API functions ------------------------ */

صحیح
ffi_tramp_is_supported(خالی)
{
  صحیح ret;

  ffi_tramp_تالا();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  واپس ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
خالی *
ffi_tramp_alloc (صحیح flags)
{
  ساخت tramp *tramp;

  ffi_tramp_تالا();

  اگر (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      واپس تہی;
    }

  اگر (!tramp_جدول_alloc ())
    {
      ffi_tramp_unlock();
      واپس تہی;
    }

  tramp = tramp_globals.free_tables->آزاد;
  tramp_del (tramp);

  ffi_tramp_unlock();

  واپس tramp;
}

/*
 * Set the parameters for a trampoline.
 */
خالی
ffi_tramp_سیٹ_parms (خالی *arg, خالی *ہدف_2, خالی *ڈیٹا)
{
  ساخت tramp *tramp = arg;

  ffi_tramp_تالا();
  tramp->parm->ہدف_2 = ہدف_2;
  tramp->parm->ڈیٹا = ڈیٹا;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
خالی *
ffi_tramp_get_addr (خالی *arg)
{
  ساخت tramp *tramp = arg;
  خالی *addr;

  ffi_tramp_تالا();
  addr = tramp->رمز;
  ffi_tramp_unlock();

  واپس addr;
}

/*
 * Free a trampoline.
 */
خالی
ffi_tramp_free (خالی *arg)
{
  ساخت tramp *tramp = arg;

  ffi_tramp_تالا();
  tramp_جوڑو (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#ورنہ /* !FFI_EXEC_STATIC_TRAMP */

#شامل <stddef.h>

صحیح
ffi_tramp_is_supported(خالی)
{
  واپس 0;
}

خالی *
ffi_tramp_alloc (صحیح flags)
{
  واپس تہی;
}

خالی
ffi_tramp_سیٹ_parms (خالی *arg, خالی *ہدف_2, خالی *ڈیٹا)
{
}

خالی *
ffi_tramp_get_addr (خالی *arg)
{
  واپس تہی;
}

خالی
ffi_tramp_free (خالی *arg)
{
}

#اگر_ختم /* FFI_EXEC_STATIC_TRAMP */
