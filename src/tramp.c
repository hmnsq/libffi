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

#समावेश <fficonfig.h>

#अगर_घोषित FFI_EXEC_STATIC_TRAMP

/* -------------------------- Headers and Definitions ---------------------*/
/*
 * Add support for other OSes later. For now, it is just Linux and Cygwin.
 */

#अगर defined (__linux__) || defined (__CYGWIN__)
#अगर_घोषित __linux__
#घोषणा _GNU_SOURCE 1
#पूर्ण_अगर

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdio.h>
#समावेश <unistd.h>
#समावेश <stdlib.h>
#समावेश <stdint.h>
#समावेश <fcntl.h>
#समावेश <pthread.h>
#समावेश <sys/mman.h>
#समावेश <tramp.h>
#अगर_घोषित __linux__
#समावेश <linux/limits.h>
#समावेश <linux/types.h>
#पूर्ण_अगर
#अगर_घोषित __CYGWIN__
#समावेश <limits.h>
#पूर्ण_अगर
#पूर्ण_अगर

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
व्योम __attribute__((weak)) *ffi_tramp_arch (माप_प्रकार *tramp_आकार,
  माप_प्रकार *map_आकार);

/* ------------------------- Trampoline Data Structures --------------------*/

काष्ठा tramp;

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
काष्ठा tramp_table
{
  काष्ठा tramp_table *पूर्व;
  काष्ठा tramp_table *अग्र;
  व्योम *code_table;
  व्योम *parm_table;
  काष्ठा tramp *array;
  काष्ठा tramp *मुक्त;
  पूर्णांक nfree;
};

/*
 * Parameters for each trampoline.
 *
 * data
 *	Data for the target code that the trampoline jumps to.
 * target
 *	Target code that the trampoline jumps to.
 */
काष्ठा tramp_parm
{
  व्योम *तथ्य;
  व्योम *target;
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
काष्ठा tramp
{
  काष्ठा tramp *पूर्व;
  काष्ठा tramp *अग्र;
  काष्ठा tramp_table *table;
  व्योम *code;
  काष्ठा tramp_parm *parm;
};

क्रमागत tramp_globals_status {
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
काष्ठा tramp_globals
{
  पूर्णांक fd;
  off_t offset;
  व्योम *text;
  माप_प्रकार map_आकार;
  माप_प्रकार आकार;
  पूर्णांक ntramp;
  काष्ठा tramp_table *free_tables;
  पूर्णांक nfree_tables;
  क्रमागत tramp_globals_status status;
};

अटल काष्ठा tramp_globals tramp_globals;

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
अटल पूर्णांक tramp_table_alloc (व्योम);

#अगर defined (__linux__) || defined (__CYGWIN__)

/* Stringify a macro value for use in a scanf field-width specifier.  */
#घोषणा FFI_TRAMP_STR_(x) #x
#घोषणा FFI_TRAMP_STR(x)  FFI_TRAMP_STR_(x)

अटल पूर्णांक
ffi_tramp_get_libffi (व्योम)
{
  खाता *fp;
  /* `file' is sized PATH_MAX+1 so a scanf field width of PATH_MAX
     (which permits PATH_MAX characters plus the terminating NUL) cannot
     overflow.  */
  अक्षर file[PATH_MAX+1], पंक्ति_2[PATH_MAX+100], perm[10], dev[10];
  अचिन्हित दीर्घ start, end, offset, inode;
  uintptr_t addr = (uintptr_t) tramp_globals.text;
  पूर्णांक nfields, found;
  पूर्णांक open_flags = O_RDONLY;

#अगर_घोषित O_CLOEXEC
  open_flags |= O_CLOEXEC;
#पूर्ण_अगर

  snprintf (file, माप (file), "/proc/%d/maps", getpid());
  fp = ख_खोलो (file, "r");
  अगर (fp == शून्य)
    वापस 0;

  found = 0;
  जबतक (ख_पूर्ण (fp) == 0) {
    अगर (ख_माला_लो (पंक्ति_2, माप (पंक्ति_2), fp) == 0)
      अवरोध;

    /* Bound the path field width so a long mapping name cannot overflow
       the fixed-size `file' buffer.  */
    nfields = माला_पूछो (पंक्ति_2,
      "%lx-%lx %9s %lx %9s %ld %" FFI_TRAMP_STR(PATH_MAX) "s",
      &start, &end, perm, &offset, dev, &inode, file);
    अगर (nfields != 7)
      जारी;

    अगर (addr >= start && addr < end) {
      tramp_globals.offset = offset + (addr - start);
      found = 1;
      अवरोध;
    }
  }
  ख_बंद (fp);

  अगर (!found)
    वापस 0;

  tramp_globals.fd = खोलो (file, open_flags);
  अगर (tramp_globals.fd == -1)
    वापस 0;

  /*
   * Allocate a trampoline table just to make sure that the trampoline code
   * table can be mapped.
   */
  अगर (!tramp_table_alloc ())
    {
      बंद (tramp_globals.fd);
      tramp_globals.fd = -1;
      वापस 0;
    }
  वापस 1;
}

#पूर्ण_अगर /* defined (__linux__) || defined (__CYGWIN__) */

#अगर defined (__linux__) || defined (__CYGWIN__)

अटल पूर्णांक
ffi_tramp_get_अस्थायी_file (व्योम)
{
  ssize_t गिनती;

  tramp_globals.offset = 0;
  tramp_globals.fd = open_अस्थायी_exec_file ();

  /*
   * Write the trampoline code table into the temporary file and allocate a
   * trampoline table to make sure that the temporary file can be mapped.
   */
  गिनती = ग_लिखो(tramp_globals.fd, tramp_globals.text, tramp_globals.map_आकार);
  अगर (गिनती >=0 && (माप_प्रकार)गिनती == tramp_globals.map_आकार && tramp_table_alloc ())
    वापस 1;

  बंद (tramp_globals.fd);
  tramp_globals.fd = -1;
  वापस 0;
}

#पूर्ण_अगर /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ OS-specific Initialization ----------------------*/

#अगर defined (__linux__) || defined (__CYGWIN__)

अटल पूर्णांक
ffi_tramp_init_os (व्योम)
{
  अगर (ffi_tramp_get_libffi ())
    वापस 1;
  वापस ffi_tramp_get_अस्थायी_file ();
}

#पूर्ण_अगर /* defined (__linux__) || defined (__CYGWIN__) */

/* --------------------------- OS-specific Locking -------------------------*/

#अगर defined (__linux__) || defined (__CYGWIN__)

अटल pthread_mutex_t tramp_globals_mutex = PTHREAD_MUTEX_INITIALIZER;

अटल व्योम
ffi_tramp_lock(व्योम)
{
  pthread_mutex_lock (&tramp_globals_mutex);
}

अटल व्योम
ffi_tramp_unlock(व्योम)
{
  pthread_mutex_unlock (&tramp_globals_mutex);
}

#पूर्ण_अगर /* defined (__linux__) || defined (__CYGWIN__) */

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

#अगर defined (__linux__) || defined (__CYGWIN__)

अटल पूर्णांक
tramp_table_map (काष्ठा tramp_table *table)
{
  अक्षर *addr;

  /*
   * Create an anonymous mapping twice the map size. The top half will be used
   * for the code table. The bottom half will be used for the parameter table.
   */
  addr = mmap (शून्य, tramp_globals.map_आकार * 2, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  अगर (addr == MAP_FAILED)
    वापस 0;

  /*
   * Replace the top half of the anonymous mapping with the code table mapping.
   */
  table->code_table = mmap (addr, tramp_globals.map_आकार, PROT_READ | PROT_EXEC,
    MAP_PRIVATE | MAP_FIXED, tramp_globals.fd, tramp_globals.offset);
  अगर (table->code_table == MAP_FAILED)
    {
      (व्योम) munmap (addr, tramp_globals.map_आकार * 2);
      वापस 0;
    }
  table->parm_table = table->code_table + tramp_globals.map_आकार;
  वापस 1;
}

अटल व्योम
tramp_table_unmap (काष्ठा tramp_table *table)
{
  (व्योम) munmap (table->code_table, tramp_globals.map_आकार);
  (व्योम) munmap (table->parm_table, tramp_globals.map_आकार);
}

#पूर्ण_अगर /* defined (__linux__) || defined (__CYGWIN__) */

/* ------------------------ Trampoline Initialization ----------------------*/

/*
 * Initialize the static trampoline feature.
 */
अटल पूर्णांक
ffi_tramp_init (व्योम)
{
  दीर्घ page_आकार;

  अगर (tramp_globals.status == TRAMP_GLOBALS_PASSED)
    वापस 1;

  अगर (tramp_globals.status == TRAMP_GLOBALS_FAILED)
    वापस 0;

  अगर (ffi_tramp_arch == शून्य)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      वापस 0;
    }

  tramp_globals.free_tables = शून्य;
  tramp_globals.nfree_tables = 0;

  /*
   * Get trampoline code table information from the architecture.
   */
  tramp_globals.text = ffi_tramp_arch (&tramp_globals.आकार,
    &tramp_globals.map_आकार);
  tramp_globals.ntramp = tramp_globals.map_आकार / tramp_globals.आकार;

  /*
   * The trampoline code table is a single, fixed-size mapping.  If the
   * system page size is larger than that mapping, the static trampoline
   * mechanism cannot be used.  Both values are invariant for the life of
   * the process, so cache the FAILED verdict rather than re-running the
   * whole initialization on every allocation.
   */
  page_आकार = sysconf (_SC_PAGESIZE);
  अगर (page_आकार >= 0 && (माप_प्रकार)page_आकार > tramp_globals.map_आकार)
    {
      tramp_globals.status = TRAMP_GLOBALS_FAILED;
      वापस 0;
    }

  अगर (ffi_tramp_init_os ())
    {
      tramp_globals.status = TRAMP_GLOBALS_PASSED;
      वापस 1;
    }

  tramp_globals.status = TRAMP_GLOBALS_FAILED;
  वापस 0;
}

/* ---------------------- Trampoline Table functions ---------------------- */

/* This code assumes that malloc () is available on all OSes. */

अटल व्योम tramp_add (काष्ठा tramp *tramp);

/*
 * Allocate and initialize a trampoline table.
 */
अटल पूर्णांक
tramp_table_alloc (व्योम)
{
  काष्ठा tramp_table *table;
  काष्ठा tramp *tramp_array, *tramp;
  माप_प्रकार आकार;
  अक्षर *code, *parm;
  पूर्णांक i;

  /*
   * If we already have tables with free trampolines, there is no need to
   * allocate a new table.
   */
  अगर (tramp_globals.nfree_tables > 0)
    वापस 1;

  /*
   * Allocate a new trampoline table structure.
   */
  table = दो_स्मृति (माप (*table));
  अगर (table == शून्य)
    वापस 0;

  /*
   * Allocate new trampoline structures.
   */
  tramp_array = दो_स्मृति (माप (*tramp) * tramp_globals.ntramp);
  अगर (tramp_array == शून्य)
    जाओ free_table;

  /*
   * Map a code table and a parameter table into the caller's address space.
   */
  अगर (!tramp_table_map (table))
    {
      /*
       * Failed to map the code and parameter tables.
       */
      जाओ free_tramp_array;
    }

  /*
   * Initialize the trampoline table.
   */
  table->array = tramp_array;
  table->मुक्त = शून्य;
  table->nfree = 0;

  /*
   * Populate the trampoline table free list. This will also add the trampoline
   * table to the global list of trampoline tables.
   */
  आकार = tramp_globals.आकार;
  code = table->code_table;
  parm = table->parm_table;
  क्रम (i = 0; i < tramp_globals.ntramp; i++)
    {
      tramp = &tramp_array[i];
      tramp->table = table;
      tramp->code = code;
      tramp->parm = (काष्ठा tramp_parm *) parm;
      tramp_add (tramp);

      code += आकार;
      parm += आकार;
    }
  /* Success */
  वापस 1;

/* Failure */
free_tramp_array:
  मुक्त (tramp_array);
free_table:
  मुक्त (table);
  वापस 0;
}

/*
 * Free a trampoline table.
 */
अटल व्योम
tramp_table_free (काष्ठा tramp_table *table)
{
  tramp_table_unmap (table);
  मुक्त (table->array);
  मुक्त (table);
}

/*
 * Add a new trampoline table to the global table list.
 */
अटल व्योम
tramp_table_add (काष्ठा tramp_table *table)
{
  table->अग्र = tramp_globals.free_tables;
  table->पूर्व = शून्य;
  अगर (tramp_globals.free_tables != शून्य)
    tramp_globals.free_tables->पूर्व = table;
  tramp_globals.free_tables = table;
  tramp_globals.nfree_tables++;
}

/*
 * Delete a trampoline table from the global table list.
 */
अटल व्योम
tramp_table_del (काष्ठा tramp_table *table)
{
  tramp_globals.nfree_tables--;
  अगर (table->पूर्व != शून्य)
    table->पूर्व->अग्र = table->अग्र;
  अगर (table->अग्र != शून्य)
    table->अग्र->पूर्व = table->पूर्व;
  अगर (tramp_globals.free_tables == table)
    tramp_globals.free_tables = table->अग्र;
}

/* ------------------------- Trampoline functions ------------------------- */

/*
 * Add a trampoline to its trampoline table.
 */
अटल व्योम
tramp_add (काष्ठा tramp *tramp)
{
  काष्ठा tramp_table *table = tramp->table;

  tramp->अग्र = table->मुक्त;
  tramp->पूर्व = शून्य;
  अगर (table->मुक्त != शून्य)
    table->मुक्त->पूर्व = tramp;
  table->मुक्त = tramp;
  table->nfree++;

  अगर (table->nfree == 1)
    tramp_table_add (table);

  /*
   * We don't want to keep too many free trampoline tables lying around.
   */
  अगर (table->nfree == tramp_globals.ntramp &&
    tramp_globals.nfree_tables > 1)
    {
      tramp_table_del (table);
      tramp_table_free (table);
    }
}

/*
 * Remove a trampoline from its trampoline table.
 */
अटल व्योम
tramp_del (काष्ठा tramp *tramp)
{
  काष्ठा tramp_table *table = tramp->table;

  table->nfree--;
  अगर (tramp->पूर्व != शून्य)
    tramp->पूर्व->अग्र = tramp->अग्र;
  अगर (tramp->अग्र != शून्य)
    tramp->अग्र->पूर्व = tramp->पूर्व;
  अगर (table->मुक्त == tramp)
    table->मुक्त = tramp->अग्र;

  अगर (table->nfree == 0)
    tramp_table_del (table);
}

/* ------------------------ Trampoline API functions ------------------------ */

पूर्णांक
ffi_tramp_is_supported(व्योम)
{
  पूर्णांक ret;

  ffi_tramp_lock();
  ret = ffi_tramp_init ();
  ffi_tramp_unlock();
  वापस ret;
}

/*
 * Allocate a trampoline and return its opaque address.
 */
व्योम *
ffi_tramp_alloc (पूर्णांक flags)
{
  काष्ठा tramp *tramp;

  ffi_tramp_lock();

  अगर (!ffi_tramp_init () || flags != 0)
    {
      ffi_tramp_unlock();
      वापस शून्य;
    }

  अगर (!tramp_table_alloc ())
    {
      ffi_tramp_unlock();
      वापस शून्य;
    }

  tramp = tramp_globals.free_tables->मुक्त;
  tramp_del (tramp);

  ffi_tramp_unlock();

  वापस tramp;
}

/*
 * Set the parameters for a trampoline.
 */
व्योम
ffi_tramp_set_parms (व्योम *arg, व्योम *target, व्योम *तथ्य)
{
  काष्ठा tramp *tramp = arg;

  ffi_tramp_lock();
  tramp->parm->target = target;
  tramp->parm->तथ्य = तथ्य;
  ffi_tramp_unlock();
}

/*
 * Get the invocation address of a trampoline.
 */
व्योम *
ffi_tramp_get_addr (व्योम *arg)
{
  काष्ठा tramp *tramp = arg;
  व्योम *addr;

  ffi_tramp_lock();
  addr = tramp->code;
  ffi_tramp_unlock();

  वापस addr;
}

/*
 * Free a trampoline.
 */
व्योम
ffi_tramp_free (व्योम *arg)
{
  काष्ठा tramp *tramp = arg;

  ffi_tramp_lock();
  tramp_add (tramp);
  ffi_tramp_unlock();
}

/* ------------------------------------------------------------------------- */

#अन्यथा /* !FFI_EXEC_STATIC_TRAMP */

#समावेश <stddef.h>

पूर्णांक
ffi_tramp_is_supported(व्योम)
{
  वापस 0;
}

व्योम *
ffi_tramp_alloc (पूर्णांक flags)
{
  वापस शून्य;
}

व्योम
ffi_tramp_set_parms (व्योम *arg, व्योम *target, व्योम *तथ्य)
{
}

व्योम *
ffi_tramp_get_addr (व्योम *arg)
{
  वापस शून्य;
}

व्योम
ffi_tramp_free (व्योम *arg)
{
}

#पूर्ण_अगर /* FFI_EXEC_STATIC_TRAMP */
