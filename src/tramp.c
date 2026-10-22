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

#అన్తర్భావ <fficonfig.h>

#నిర్వచితమైతే FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#ఒకవేళ defined (__linux__) || defined (__CYGWIN__)
#నిర్వచితమైతే __linux__
#నిర్వచించు GNU_ఆధారం 1
#అయితే_ముగింపు

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <pthread.h>
#అన్తర్భావ <sys/mman.h>
#అన్తర్భావ <tramp.h>
#నిర్వచితమైతే __linux__
#అన్తర్భావ <linux/limits.h>
#అన్తర్భావ <linux/types.h>
#అయితే_ముగింపు
#నిర్వచితమైతే __CYGWIN__
#అన్తర్భావ <limits.h>
#అయితే_ముగింపు
#అయితే_ముగింపు

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
శూన్య __attribute__((weak)) *ffi_tramp_arch (size_t *tramp_కొలత,
  size_t *map_కొలత);

/* ------------------------- Trampoline Data Structures --------------------*/

నిర్మాణం tramp;

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
నిర్మాణం tramp_పట్టిక
{
  నిర్మాణం tramp_పట్టిక *prev;
  నిర్మాణం tramp_పట్టిక *తదుపరి;
  శూన్య *సంకేతం_పట్టిక;
  శూన్య *parm_పట్టిక;
  నిర్మాణం tramp *array;
  నిర్మాణం tramp *విడుదల;
  పూర్ణాంకం nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
నిర్మాణం tramp_parm
{
  శూన్య *దత్తాంశం;
  శూన్య *లక్ష్యం_2;
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
నిర్మాణం tramp
{
  నిర్మాణం tramp *prev;
  నిర్మాణం tramp *తదుపరి;
  నిర్మాణం tramp_పట్టిక *పట్టిక;
  శూన్య *సంకేతం_2;
  నిర్మాణం tramp_parm *parm;
};

గణన tramp_globals_స్థితి {
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
నిర్మాణం tramp_globals
{
  పూర్ణాంకం fd;
  off_t offset;
  శూన్య *పాఠ్యం;
  size_t map_కొలత;
  size_t కొలత;
  పూర్ణాంకం ntramp;
  నిర్మాణం tramp_పట్టిక *free_tables;
  పూర్ణాంకం nfree_tables;
  గణన tramp_globals_స్థితి status;
};

స్థిర నిర్మాణం tramp_globals tramp_globals;

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
స్థిర పూర్ణాంకం tramp_పట్టిక_alloc (శూన్య);

#ఒకవేళ defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#నిర్వచించు FFI_TRAMP_STR_(x) #x
#నిర్వచించు FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

స్థిర పూర్ణాంకం
ffi_tramp_get_libffi (శూన్య)
{
  FILE *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  అక్షరం file[మార్గం_MAX+1], పంక్తి[మార్గం_MAX+100], perm[10], dev[10];
  గుర్తులేని దీర్ఘ ప్రారంభం, చివర, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.పాఠ్యం;
  పూర్ణాంకం nfields, దొరికింది;
  పూర్ణాంకం open_flags = O_RDONLY;

#నిర్వచితమైతే O_CLOEXEC
  open_flags |= O_CLOEXEC;
#అయితే_ముగింపు

  snprintf (file, పరిమాణం (file), "/proc/%d/maps", getpid());
  fp = fopen (file, "r");
  ఒకవేళ (fp == ఖాళీ)
    తిరిగి 0;

  దొరికింది = 0;
  అయితే (feof (fp) == 0) {
    ఒకవేళ (fgets (పంక్తి, పరిమాణం (పంక్తి), fp) == 0)
      ఆపు;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = అక్షరశ్రేణి_చదువు (పంక్తి,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(మార్గం_MAX) "s",
      &ప్రారంభం, &చివర, perm, &offset, dev, &inode, file);
    ఒకవేళ (nfields != 7)
      కొనసాగు;

    ఒకవేళ (addr >= ప్రారంభం && addr < చివర) {
      tramp_globals.offset = offset + (addr - ప్రారంభం);
      దొరికింది = 1;
      ఆపు;
    }
  }
  fclose (fp);

  ఒకవేళ (!దొరికింది)
    తిరిగి 0;

  tramp_globals.fd = open (file, open_flags);
  ఒకవేళ (tramp_globals.fd == -1)
    తిరిగి 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  ఒకవేళ (!tramp_పట్టిక_alloc ())
    {
      close (tramp_globals.fd);
      tramp_globals.fd = -1;
      తిరిగి 0;
    }
  తిరిగి 1;
}

#అయితే_ముగింపు /* defined (__linux__) || defined (__CYGWIN__) */

#ఒకవేళ defined (__linux__) || defined (__CYGWIN__)

స్థిర పూర్ణాంకం
ffi_tramp_get_తాత్కాలిక_file (శూన్య)
{
  ssize_t లెక్క;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_తాత్కాలిక_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  లెక్క = లిఖ(tramp_globals.fd, tramp_globals.పాఠ్యం, tramp_globals.map_కొలత);
  ఒకవేళ (లెక్క >=0 && (size_t)లెక్క == tramp_globals.map_కొలత && tramp_పట్టిక_alloc ())
    తిరిగి 1;

  close (tramp_globals.fd);
  tramp_globals.fd = -1;
  తిరిగి 0;
}

#అయితే_ముగింపు /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#ఒకవేళ defined (__linux__) || defined (__CYGWIN__)

స్థిర పూర్ణాంకం
ffi_tramp_init_os (శూన్య)
{
  ఒకవేళ (ffi_tramp_get_libffi ())
    తిరిగి 1;
  తిరిగి ffi_tramp_get_తాత్కాలిక_file ();
}

#అయితే_ముగింపు /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#ఒకవేళ defined (__linux__) || defined (__CYGWIN__)

స్థిర pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

స్థిర శూన్య
ffi_tramp_తాళం(శూన్య)
{
  pthread_mutex_తాళం (&tramp_globals_mutex);
}

స్థిర శూన్య
ffi_tramp_unlock(శూన్య)
{
  pthread_mutex_unlock (&tramp_globals_mutex);
}

#అయితే_ముగింపు /* defined (__linux__) || defined (__CYGWIN__) */

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

#ఒకవేళ defined (__linux__) || defined (__CYGWIN__)

స్థిర పూర్ణాంకం
tramp_పట్టిక_map (నిర్మాణం tramp_పట్టిక *పట్టిక)
{
  అక్షరం *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (ఖాళీ, tramp_globals.map_కొలత * 2, PROT_READ | PROT_WRITE,
    MAP_వ్యక్తిగత | MAP_ANONYMOUS, -1, 0);
  ఒకవేళ (addr == MAP_FAILED)
    తిరిగి 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  పట్టిక->సంకేతం_పట్టిక = mmap (addr, tramp_globals.map_కొలత, PROT_READ | PROT_EXEC,
    MAP_వ్యక్తిగత | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  ఒకవేళ (పట్టిక->సంకేతం_పట్టిక == MAP_FAILED)
    {
      (శూన్య) munmap (addr, tramp_globals.map_కొలత * 2);
      తిరిగి 0;
    }
  పట్టిక->parm_పట్టిక = పట్టిక->సంకేతం_పట్టిక + tramp_globals.map_కొలత;
  తిరిగి 1;
}

స్థిర శూన్య
tramp_పట్టిక_unmap (నిర్మాణం tramp_పట్టిక *పట్టిక)
{
  (శూన్య) munmap (పట్టిక->సంకేతం_పట్టిక, tramp_globals.map_కొలత);
  (శూన్య) munmap (పట్టిక->parm_పట్టిక, tramp_globals.map_కొలత);
}

#అయితే_ముగింపు /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
స్థిర పూర్ణాంకం
ffi_tramp_init (శూన్య)
{
  దీర్ఘ పుట_కొలత;

  ఒకవేళ (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    తిరిగి 1;

  ఒకవేళ (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    తిరిగి 0;

  ఒకవేళ (ffi_tramp_arch == ఖాళీ)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      తిరిగి 0;
    }

  tramp_globals.free_tables = ఖాళీ;
  tramp_globals.nfree_tables = 0;

  /*
   * Get trampoline code table information from the architecture.
   */
  tramp_globals.పాఠ్యం = ffi_tramp_arch (&tramp_globals.కొలత,
    &tramp_globals.map_కొలత);
  tramp_globals.ntramp = tramp_globals.map_కొలత / tramp_globals.కొలత;

  /*
   * The trampoline code table is a single, fixed-size mapping.  If the
   * system page size is larger than that mapping, the static trampoline
   * mechanism cannot be used.  Both values are invariant for the life of
   * the process, so cache the FAILED verdict rather than re-running the
   * whole initialization on every allocation.
   */
  పుట_కొలత = sysconf (_SC_PAGESIZE);
  ఒకవేళ (పుట_కొలత >= 0 && (size_t)పుట_కొలత > tramp_globals.map_కొలత)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      తిరిగి 0;
    }

  ఒకవేళ (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      తిరిగి 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  తిరిగి 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

స్థిర శూన్య tramp_కూడు (నిర్మాణం tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
స్థిర పూర్ణాంకం
tramp_పట్టిక_alloc (శూన్య)
{
  నిర్మాణం tramp_పట్టిక *పట్టిక;
  నిర్మాణం tramp *tramp_array, *tramp;
  size_t కొలత;
  అక్షరం *సంకేతం_2, *parm;
  పూర్ణాంకం i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  ఒకవేళ (tramp_globals.nfree_tables > 0)
    తిరిగి 1;

  /*
   * Allocate a new trampoline table structure.
   */
  పట్టిక = జ్ఞాపకం_ఇవ్వు (పరిమాణం (*పట్టిక));
  ఒకవేళ (పట్టిక == ఖాళీ)
    తిరిగి 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = జ్ఞాపకం_ఇవ్వు (పరిమాణం (*tramp) * tramp_globals.ntramp);
  ఒకవేళ (tramp_array == ఖాళీ)
    వెళ్ళు free_పట్టిక;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  ఒకవేళ (!tramp_పట్టిక_map (పట్టిక))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      వెళ్ళు free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  పట్టిక->array = tramp_array;
  పట్టిక->విడుదల = ఖాళీ;
  పట్టిక->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  కొలత = tramp_globals.కొలత;
  సంకేతం_2 = పట్టిక->సంకేతం_పట్టిక;
  parm = పట్టిక->parm_పట్టిక;
  కొరకు (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->పట్టిక = పట్టిక;
      tramp->సంకేతం_2 = సంకేతం_2;
      tramp->parm = (నిర్మాణం tramp_parm *) parm;
      tramp_కూడు (tramp);

      సంకేతం_2 += కొలత;
      parm += కొలత;
    }
  /* Success */
  తిరిగి 1;

/* Failure */
free_tramp_array:
  విడుదల (tramp_array);
free_పట్టిక:
  విడుదల (పట్టిక);
  తిరిగి 0;
}

/*
 * Free a trampoline table.
 */
స్థిర శూన్య
tramp_పట్టిక_free (నిర్మాణం tramp_పట్టిక *పట్టిక)
{
  tramp_పట్టిక_unmap (పట్టిక);
  విడుదల (పట్టిక->array);
  విడుదల (పట్టిక);
}

/*
 * Add a new trampoline table to the global table list.
 */
స్థిర శూన్య
tramp_పట్టిక_కూడు (నిర్మాణం tramp_పట్టిక *పట్టిక)
{
  పట్టిక->తదుపరి = tramp_globals.free_tables;
  పట్టిక->prev = ఖాళీ;
  ఒకవేళ (tramp_globals.free_tables != ఖాళీ)
    tramp_globals.free_tables->prev = పట్టిక;
  tramp_globals.free_tables = పట్టిక;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
స్థిర శూన్య
tramp_పట్టిక_del (నిర్మాణం tramp_పట్టిక *పట్టిక)
{
  tramp_globals.nfree_tables--;
  ఒకవేళ (పట్టిక->prev != ఖాళీ)
    పట్టిక->prev->తదుపరి = పట్టిక->తదుపరి;
  ఒకవేళ (పట్టిక->తదుపరి != ఖాళీ)
    పట్టిక->తదుపరి->prev = పట్టిక->prev;
  ఒకవేళ (tramp_globals.free_tables == పట్టిక)
    tramp_globals.free_tables = పట్టిక->తదుపరి;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
స్థిర శూన్య
tramp_కూడు (నిర్మాణం tramp *tramp)
{
  నిర్మాణం tramp_పట్టిక *పట్టిక = tramp->పట్టిక;

  tramp->తదుపరి = పట్టిక->విడుదల;
  tramp->prev = ఖాళీ;
  ఒకవేళ (పట్టిక->విడుదల != ఖాళీ)
    పట్టిక->విడుదల->prev = tramp;
  పట్టిక->విడుదల = tramp;
  పట్టిక->nfree++;

  ఒకవేళ (పట్టిక->nfree == 1)
    tramp_పట్టిక_కూడు (పట్టిక);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  ఒకవేళ (పట్టిక->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_పట్టిక_del (పట్టిక);
      tramp_పట్టిక_free (పట్టిక);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
స్థిర శూన్య
tramp_del (నిర్మాణం tramp *tramp)
{
  నిర్మాణం tramp_పట్టిక *పట్టిక = tramp->పట్టిక;

  పట్టిక->nfree--;
  ఒకవేళ (tramp->prev != ఖాళీ)
    tramp->prev->తదుపరి = tramp->తదుపరి;
  ఒకవేళ (tramp->తదుపరి != ఖాళీ)
    tramp->తదుపరి->prev = tramp->prev;
  ఒకవేళ (పట్టిక->విడుదల == tramp)
    పట్టిక->విడుదల = tramp->తదుపరి;

  ఒకవేళ (పట్టిక->nfree == 0)
    tramp_పట్టిక_del (పట్టిక);
}

/* ------------------------ Trampoline API functions ------------------------ */

పూర్ణాంకం
ffi_tramp_is_supported(శూన్య)
{
  పూర్ణాంకం ret;

  ffi_tramp_తాళం();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  తిరిగి ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
శూన్య *
ffi_tramp_alloc (పూర్ణాంకం flags)
{
  నిర్మాణం tramp *tramp;

  ffi_tramp_తాళం();

  ఒకవేళ (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      తిరిగి ఖాళీ;
    }

  ఒకవేళ (!tramp_పట్టిక_alloc ())
    {
      ffi_tramp_unlock();
      తిరిగి ఖాళీ;
    }

  tramp = tramp_globals.free_tables->విడుదల;
  tramp_del (tramp);

  ffi_tramp_unlock();

  తిరిగి tramp;
}

/*
 * Set the parameters for a trampoline.
 */
శూన్య
ffi_tramp_సమితి_parms (శూన్య *arg, శూన్య *లక్ష్యం_2, శూన్య *దత్తాంశం)
{
  నిర్మాణం tramp *tramp = arg;

  ffi_tramp_తాళం();
  tramp->parm->లక్ష్యం_2 = లక్ష్యం_2;
  tramp->parm->దత్తాంశం = దత్తాంశం;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
శూన్య *
ffi_tramp_get_addr (శూన్య *arg)
{
  నిర్మాణం tramp *tramp = arg;
  శూన్య *addr;

  ffi_tramp_తాళం();
  addr = tramp->సంకేతం_2;
  ffi_tramp_unlock();

  తిరిగి addr;
}

/*
 * Free a trampoline.
 */
శూన్య
ffi_tramp_free (శూన్య *arg)
{
  నిర్మాణం tramp *tramp = arg;

  ffi_tramp_తాళం();
  tramp_కూడు (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#లేకపోతే /* !FFI_EXEC_STATIC_TRAMP */

#అన్తర్భావ <stddef.h>

పూర్ణాంకం
ffi_tramp_is_supported(శూన్య)
{
  తిరిగి 0;
}

శూన్య *
ffi_tramp_alloc (పూర్ణాంకం flags)
{
  తిరిగి ఖాళీ;
}

శూన్య
ffi_tramp_సమితి_parms (శూన్య *arg, శూన్య *లక్ష్యం_2, శూన్య *దత్తాంశం)
{
}

శూన్య *
ffi_tramp_get_addr (శూన్య *arg)
{
  తిరిగి ఖాళీ;
}

శూన్య
ffi_tramp_free (శూన్య *arg)
{
}

#అయితే_ముగింపు /* FFI_EXEC_STATIC_TRAMP */
