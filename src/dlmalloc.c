/*
  This is a version (aka dlmalloc) of malloc/free/realloc written by
  Doug Lea and released to the public domain, as explained at
  http://creativecommons.org/publicdomain/zero/1.0/ Send questions,
  comments, complaints, performance data, etc to dl@cs.oswego.edu

* Version 2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea
   Note: There may be an updated version of this malloc obtainable at
           ftp://gee.cs.oswego.edu/pub/misc/malloc.c
         Check before installing!

* Quickstart

  This library is all in one file to simplify the most common usage:
  ftp it, compile it (-O3), and link it into another program. All of
  the compile-time options default to reasonable values for use on
  most platforms.  You might later want to step through various
  compile-time and dynamic tuning options.

  For convenience, an include file for code using this malloc is at:
     ftp://gee.cs.oswego.edu/pub/misc/malloc-2.8.6.h
  You don't really need this .h file unless you call functions not
  defined in your system include files.  The .h file contains only the
  excerpts from this file needed for using this malloc on ANSI C/C++
  systems, so long as you haven't changed compile-time options about
  naming and tuning parameters.  If you do, then you can create your
  own malloc.h that does include all settings by cutting at the point
  indicated below. Note that you may already by default be using a C
  library containing a malloc that is based on some version of this
  malloc (for example in linux). You might still want to use the one
  in this file to customize settings or to avoid overheads associated
  with library versions.

* Vital statistics:

  Supported pointer/size_t representation:       4 or 8 bytes
       size_t MUST be an unsigned type of the same width as
       pointers. (If you are using an ancient system that declares
       size_t as a signed type, or need it to be a different width
       than pointers, you can use a previous release of this malloc
       (e.g. 2.7.2) supporting these.)

  Alignment:                                     8 bytes (minimum)
       This suffices for nearly all current machines and C compilers.
       However, you can define MALLOC_ALIGNMENT to be wider than this
       if necessary (up to 128bytes), at the expense of using more space.

  Minimum overhead per allocated chunk:   4 or  8 bytes (if 4byte sizes)
                                          8 or 16 bytes (if 8byte sizes)
       Each malloced chunk has a hidden word of overhead holding size
       and status information, and additional cross-check word
       if FOOTERS is defined.

  Minimum allocated size: 4-byte ptrs:  16 bytes    (including overhead)
                          8-byte ptrs:  32 bytes    (including overhead)

       Even a request for zero bytes (i.e., malloc(0)) returns a
       pointer to something of the minimum allocatable size.
       The maximum overhead wastage (i.e., number of extra bytes
       allocated than were requested in malloc) is less than or equal
       to the minimum size, except for requests >= mmap_threshold that
       are serviced via mmap(), where the worst case wastage is about
       32 bytes plus the remainder from a system page (the minimal
       mmap unit); typically 4096 or 8192 bytes.

  Security: static-safe; optionally more or less
       The "security" of malloc refers to the ability of malicious
       code to accentuate the effects of errors (for example, freeing
       space that is not currently malloc'ed or overwriting past the
       ends of chunks) in code that calls malloc.  This malloc
       guarantees not to modify any memory locations below the base of
       heap, i.e., static variables, even in the presence of usage
       errors.  The routines additionally detect most improper frees
       and reallocs.  All this holds as long as the static bookkeeping
       for malloc itself is not corrupted by some other means.  This
       is only one aspect of security -- these checks do not, and
       cannot, detect all possible programming errors.

       If FOOTERS is defined nonzero, then each allocated chunk
       carries an additional check word to verify that it was malloced
       from its space.  These check words are the same within each
       execution of a program using malloc, but differ across
       executions, so externally crafted fake chunks cannot be
       freed. This improves security by rejecting frees/reallocs that
       could corrupt heap memory, in addition to the checks preventing
       writes to statics that are always on.  This may further improve
       security at the expense of time and space overhead.  (Note that
       FOOTERS may also be worth using with MSPACES.)

       By default detected errors cause the program to abort (calling
       "abort()"). You can override this to instead proceed past
       errors by defining PROCEED_ON_ERROR.  In this case, a bad free
       has no effect, and a malloc that encounters a bad address
       caused by user overwrites will ignore the bad address by
       dropping pointers and indices to all known memory. This may
       be appropriate for programs that should continue if at all
       possible in the face of programming errors, although they may
       run out of memory because dropped memory is never reclaimed.

       If you don't like either of these options, you can define
       CORRUPTION_ERROR_ACTION and USAGE_ERROR_ACTION to do anything
       else. And if if you are sure that your program using malloc has
       no errors or vulnerabilities, you can define INSECURE to 1,
       which might (or might not) provide a small performance improvement.

       It is also possible to limit the maximum total allocatable
       space, using malloc_set_footprint_limit. This is not
       designed as a security feature in itself (calls to set limits
       are not screened or privileged), but may be useful as one
       aspect of a secure implementation.

  Thread-safety: NOT thread-safe unless USE_LOCKS defined non-zero
       When USE_LOCKS is defined, each public call to malloc, free,
       etc is surrounded with a lock. By default, this uses a plain
       pthread mutex, win32 critical section, or a spin-lock if if
       available for the platform and not disabled by setting
       USE_SPIN_LOCKS=0.  However, if USE_RECURSIVE_LOCKS is defined,
       recursive versions are used instead (which are not required for
       base functionality but may be needed in layered extensions).
       Using a global lock is not especially fast, and can be a major
       bottleneck.  It is designed only to provide minimal protection
       in concurrent environments, and to provide a basis for
       extensions.  If you are using malloc in a concurrent program,
       consider instead using nedmalloc
       (http://www.nedprod.com/programs/portable/nedmalloc/) or
       ptmalloc (See http://www.malloc.de), which are derived from
       versions of this malloc.

  System requirements: Any combination of MORECORE and/or MMAP/MUNMAP
       This malloc can use unix sbrk or any emulation (invoked using
       the CALL_MORECORE macro) and/or mmap/munmap or any emulation
       (invoked using CALL_MMAP/CALL_MUNMAP) to get and release system
       memory.  On most unix systems, it tends to work best if both
       MORECORE and MMAP are enabled.  On Win32, it uses emulations
       based on VirtualAlloc. It also uses common C library functions
       like memset.

  Compliance: I believe it is compliant with the Single Unix Specification
       (See http://www.unix.org). Also SVID/XPG, ANSI C, and probably
       others as well.

* Overview of algorithms

  This is not the fastest, most space-conserving, most portable, or
  most tunable malloc ever written. However it is among the fastest
  while also being among the most space-conserving, portable and
  tunable.  Consistent balance across these factors results in a good
  general-purpose allocator for malloc-intensive programs.

  In most ways, this malloc is a best-fit allocator. Generally, it
  chooses the best-fitting existing chunk for a request, with ties
  broken in approximately least-recently-used order. (This strategy
  normally maintains low fragmentation.) However, for requests less
  than 256bytes, it deviates from best-fit when there is not an
  exactly fitting available chunk by preferring to use space adjacent
  to that used for the previous small request, as well as by breaking
  ties in approximately most-recently-used order. (These enhance
  locality of series of small allocations.)  And for very large requests
  (>= 256Kb by default), it relies on system memory mapping
  facilities, if supported.  (This helps avoid carrying around and
  possibly fragmenting memory used only for large chunks.)

  All operations (except malloc_stats and mallinfo) have execution
  times that are bounded by a constant factor of the number of bits in
  a size_t, not counting any clearing in calloc or copying in realloc,
  or actions surrounding MORECORE and MMAP that have times
  proportional to the number of non-contiguous regions returned by
  system allocation routines, which is often just 1. In real-time
  applications, you can optionally suppress segment traversals using
  NO_SEGMENT_TRAVERSAL, which assures bounded execution even when
  system allocators return non-contiguous spaces, at the typical
  expense of carrying around more memory and increased fragmentation.

  The implementation is not very modular and seriously overuses
  macros. Perhaps someday all C compilers will do as good a job
  inlining modular code as can now be done by brute-force expansion,
  but now, enough of them seem not to.

  Some compilers issue a lot of warnings about code that is
  dead/unreachable only on some platforms, and also about intentional
  uses of negation on unsigned types. All known cases of each can be
  ignored.

  For a longer but out of date high-level description, see
     http://gee.cs.oswego.edu/dl/html/malloc.html

* MSPACES
  If MSPACES is defined, then in addition to malloc, free, etc.,
  this file also defines mspace_malloc, mspace_free, etc. These
  are versions of malloc routines that take an "mspace" argument
  obtained using create_mspace, to control all internal bookkeeping.
  If ONLY_MSPACES is defined, only these versions are compiled.
  So if you would like to use this allocator for only some allocations,
  and your system malloc for others, you can compile with
  ONLY_MSPACES and then do something like...
    static mspace mymspace = create_mspace(0,0); // for example
    #define mymalloc(bytes)  mspace_malloc(mymspace, bytes)

  (Note: If you only need one instance of an mspace, you can instead
  use "USE_DL_PREFIX" to relabel the global malloc.)

  You can similarly create thread-local allocators by storing
  mspaces as thread-locals. For example:
    static __thread mspace tlms = 0;
    void*  tlmalloc(size_t bytes) {
      if (tlms == 0) tlms = create_mspace(0, 0);
      return mspace_malloc(tlms, bytes);
    }
    void  tlfree(void* mem) { mspace_free(tlms, mem); }

  Unless FOOTERS is defined, each mspace is completely independent.
  You cannot allocate from one and free to another (although
  conformance is only weakly checked, so usage errors are not always
  caught). If FOOTERS is defined, then each chunk carries around a tag
  indicating its originating mspace, and frees are directed to their
  originating spaces. Normally, this requires use of locks.

 -------------------------  Compile-time options ---------------------------

Be careful in setting #define values for numerical constants of type
size_t. On some systems, literal values are not automatically extended
to size_t precision unless they are explicitly casted. You can also
use the symbolic values MAX_SIZE_T, SIZE_T_ONE, etc below.

WIN32                    default: defined if _WIN32 defined
  Defining WIN32 sets up defaults for MS environment and compilers.
  Otherwise defaults are for unix. Beware that there seem to be some
  cases where this malloc might not be a pure drop-in replacement for
  Win32 malloc: Random-looking failures from Win32 GDI API's (eg;
  SetDIBits()) may be due to bugs in some video driver implementations
  when pixel buffers are malloc()ed, and the region spans more than
  one VirtualAlloc()ed region. Because dlmalloc uses a small (64Kb)
  default granularity, pixel buffers may straddle virtual allocation
  regions more often than when using the Microsoft allocator.  You can
  avoid this by using VirtualAlloc() and VirtualFree() for all pixel
  buffers rather than using malloc().  If this is not possible,
  recompile this malloc with a larger DEFAULT_GRANULARITY. Note:
  in cases where MSC and gcc (cygwin) are known to differ on WIN32,
  conditions use _MSC_VER to distinguish them.

DLMALLOC_EXPORT       default: extern
  Defines how public APIs are declared. If you want to export via a
  Windows DLL, you might define this as
    #define DLMALLOC_EXPORT extern  __declspec(dllexport)
  If you want a POSIX ELF shared object, you might use
    #define DLMALLOC_EXPORT extern __attribute__((visibility("default")))

MALLOC_ALIGNMENT         default: (size_t)(2 * sizeof(void *))
  Controls the minimum alignment for malloc'ed chunks.  It must be a
  power of two and at least 8, even on machines for which smaller
  alignments would suffice. It may be defined as larger than this
  though. Note however that code and data structures are optimized for
  the case of 8-byte alignment.

MSPACES                  default: 0 (false)
  If true, compile in support for independent allocation spaces.
  This is only supported if HAVE_MMAP is true.

ONLY_MSPACES             default: 0 (false)
  If true, only compile in mspace versions, not regular versions.

USE_LOCKS                default: 0 (false)
  Causes each call to each public routine to be surrounded with
  pthread or WIN32 mutex lock/unlock. (If set true, this can be
  overridden on a per-mspace basis for mspace versions.) If set to a
  non-zero value other than 1, locks are used, but their
  implementation is left out, so lock functions must be supplied manually,
  as described below.

USE_SPIN_LOCKS           default: 1 iff USE_LOCKS and spin locks available
  If true, uses custom spin locks for locking. This is currently
  supported only gcc >= 4.1, older gccs on x86 platforms, and recent
  MS compilers.  Otherwise, posix locks or win32 critical sections are
  used.

USE_RECURSIVE_LOCKS      default: not defined
  If defined nonzero, uses recursive (aka reentrant) locks, otherwise
  uses plain mutexes. This is not required for malloc proper, but may
  be needed for layered allocators such as nedmalloc.

LOCK_AT_FORK            default: not defined
  If defined nonzero, performs pthread_atfork upon initialization
  to initialize child lock while holding parent lock. The implementation
  assumes that pthread locks (not custom locks) are being used. In other
  cases, you may need to customize the implementation.

FOOTERS                  default: 0
  If true, provide extra checking and dispatching by placing
  information in the footers of allocated chunks. This adds
  space and time overhead.

INSECURE                 default: 0
  If true, omit checks for usage errors and heap space overwrites.

USE_DL_PREFIX            default: NOT defined
  Causes compiler to prefix all public routines with the string 'dl'.
  This can be useful when you only want to use this malloc in one part
  of a program, using your regular system malloc elsewhere.

MALLOC_INSPECT_ALL       default: NOT defined
  If defined, compiles malloc_inspect_all and mspace_inspect_all, that
  perform traversal of all heap space.  Unless access to these
  functions is otherwise restricted, you probably do not want to
  include them in secure implementations.

ABORT                    default: defined as abort()
  Defines how to abort on failed checks.  On most systems, a failed
  check cannot die with an "assert" or even print an informative
  message, because the underlying print routines in turn call malloc,
  which will fail again.  Generally, the best policy is to simply call
  abort(). It's not very useful to do more than this because many
  errors due to overwriting will show up as address faults (null, odd
  addresses etc) rather than malloc-triggered checks, so will also
  abort.  Also, most compilers know that abort() does not return, so
  can better optimize code conditionally calling it.

PROCEED_ON_ERROR           default: defined as 0 (false)
  Controls whether detected bad addresses cause them to bypassed
  rather than aborting. If set, detected bad arguments to free and
  realloc are ignored. And all bookkeeping information is zeroed out
  upon a detected overwrite of freed heap space, thus losing the
  ability to ever return it from malloc again, but enabling the
  application to proceed. If PROCEED_ON_ERROR is defined, the
  static variable malloc_corruption_error_count is compiled in
  and can be examined to see if errors have occurred. This option
  generates slower code than the default abort policy.

DEBUG                    default: NOT defined
  The DEBUG setting is mainly intended for people trying to modify
  this code or diagnose problems when porting to new platforms.
  However, it may also be able to better isolate user errors than just
  using runtime checks.  The assertions in the check routines spell
  out in more detail the assumptions and invariants underlying the
  algorithms.  The checking is fairly extensive, and will slow down
  execution noticeably. Calling malloc_stats or mallinfo with DEBUG
  set will attempt to check every non-mmapped allocated and free chunk
  in the course of computing the summaries.

ABORT_ON_ASSERT_FAILURE   default: defined as 1 (true)
  Debugging assertion failures can be nearly impossible if your
  version of the assert macro causes malloc to be called, which will
  lead to a cascade of further failures, blowing the runtime stack.
  ABORT_ON_ASSERT_FAILURE cause assertions failures to call abort(),
  which will usually make debugging easier.

MALLOC_FAILURE_ACTION     default: sets errno to ENOMEM, or no-op on win32
  The action to take before "return 0" when malloc fails to be able to
  return memory because there is none available.

HAVE_MORECORE             default: 1 (true) unless win32 or ONLY_MSPACES
  True if this system supports sbrk or an emulation of it.

MORECORE                  default: sbrk
  The name of the sbrk-style system routine to call to obtain more
  memory.  See below for guidance on writing custom MORECORE
  functions. The type of the argument to sbrk/MORECORE varies across
  systems.  It cannot be size_t, because it supports negative
  arguments, so it is normally the signed type of the same width as
  size_t (sometimes declared as "intptr_t").  It doesn't much matter
  though. Internally, we only call it with arguments less than half
  the max value of a size_t, which should work across all reasonable
  possibilities, although sometimes generating compiler warnings.

MORECORE_CONTIGUOUS       default: 1 (true) if HAVE_MORECORE
  If true, take advantage of fact that consecutive calls to MORECORE
  with positive arguments always return contiguous increasing
  addresses.  This is true of unix sbrk. It does not hurt too much to
  set it true anyway, since malloc copes with non-contiguities.
  Setting it false when definitely non-contiguous saves time
  and possibly wasted space it would take to discover this though.

MORECORE_CANNOT_TRIM      default: NOT defined
  True if MORECORE cannot release space back to the system when given
  negative arguments. This is generally necessary only if you are
  using a hand-crafted MORECORE function that cannot handle negative
  arguments.

NO_SEGMENT_TRAVERSAL       default: 0
  If non-zero, suppresses traversals of memory segments
  returned by either MORECORE or CALL_MMAP. This disables
  merging of segments that are contiguous, and selectively
  releasing them to the OS if unused, but bounds execution times.

HAVE_MMAP                 default: 1 (true)
  True if this system supports mmap or an emulation of it.  If so, and
  HAVE_MORECORE is not true, MMAP is used for all system
  allocation. If set and HAVE_MORECORE is true as well, MMAP is
  primarily used to directly allocate very large blocks. It is also
  used as a backup strategy in cases where MORECORE fails to provide
  space from system. Note: A single call to MUNMAP is assumed to be
  able to unmap memory that may have be allocated using multiple calls
  to MMAP, so long as they are adjacent.

HAVE_MREMAP               default: 1 on linux, else 0
  If true realloc() uses mremap() to re-allocate large blocks and
  extend or shrink allocation spaces.

MMAP_CLEARS               default: 1 except on WINCE.
  True if mmap clears memory so calloc doesn't need to. This is true
  for standard unix mmap using /dev/zero and on WIN32 except for WINCE.

USE_BUILTIN_FFS            default: 0 (i.e., not used)
  Causes malloc to use the builtin ffs() function to compute indices.
  Some compilers may recognize and intrinsify ffs to be faster than the
  supplied C version. Also, the case of x86 using gcc is special-cased
  to an asm instruction, so is already as fast as it can be, and so
  this setting has no effect. Similarly for Win32 under recent MS compilers.
  (On most x86s, the asm version is only slightly faster than the C version.)

malloc_getpagesize         default: derive from system includes, or 4096.
  The system page size. To the extent possible, this malloc manages
  memory from the system in page-size units.  This may be (and
  usually is) a function rather than a constant. This is ignored
  if WIN32, where page size is determined using getSystemInfo during
  initialization.

USE_DEV_RANDOM             default: 0 (i.e., not used)
  Causes malloc to use /dev/random to initialize secure magic seed for
  stamping footers. Otherwise, the current time is used.

NO_MALLINFO                default: 0
  If defined, don't compile "mallinfo". This can be a simple way
  of dealing with mismatches between system declarations and
  those in this file.

MALLINFO_FIELD_TYPE        default: size_t
  The type of the fields in the mallinfo struct. This was originally
  defined as "int" in SVID etc, but is more usefully defined as
  size_t. The value is used only if  HAVE_USR_INCLUDE_MALLOC_H is not set

NO_MALLOC_STATS            default: 0
  If defined, don't compile "malloc_stats". This avoids calls to
  fprintf and bringing in stdio dependencies you might not want.

REALLOC_ZERO_BYTES_FREES    default: not defined
  This should be set if a call to realloc with zero bytes should
  be the same as a call to free. Some people think it should. Otherwise,
  since this malloc returns a unique pointer for malloc(0), so does
  realloc(p, 0).

LACKS_UNISTD_H, LACKS_FCNTL_H, LACKS_SYS_PARAM_H, LACKS_SYS_MMAN_H
LACKS_STRINGS_H, LACKS_STRING_H, LACKS_SYS_TYPES_H,  LACKS_ERRNO_H
LACKS_STDLIB_H LACKS_SCHED_H LACKS_TIME_H  default: NOT defined unless on WIN32
  Define these if your system does not have these header files.
  You might need to manually insert some of the declarations they provide.

DEFAULT_GRANULARITY        default: page size if MORECORE_CONTIGUOUS,
                                system_info.dwAllocationGranularity in WIN32,
                                otherwise 64K.
      Also settable using mallopt(M_GRANULARITY, x)
  The unit for allocating and deallocating memory from the system.  On
  most systems with contiguous MORECORE, there is no reason to
  make this more than a page. However, systems with MMAP tend to
  either require or encourage larger granularities.  You can increase
  this value to prevent system allocation functions to be called so
  often, especially if they are slow.  The value must be at least one
  page and must be a power of two.  Setting to 0 causes initialization
  to either page size or win32 region size.  (Note: In previous
  versions of malloc, the equivalent of this option was called
  "TOP_PAD")

DEFAULT_TRIM_THRESHOLD    default: 2MB
      Also settable using mallopt(M_TRIM_THRESHOLD, x)
  The maximum amount of unused top-most memory to keep before
  releasing via malloc_trim in free().  Automatic trimming is mainly
  useful in long-lived programs using contiguous MORECORE.  Because
  trimming via sbrk can be slow on some systems, and can sometimes be
  wasteful (in cases where programs immediately afterward allocate
  more large chunks) the value should be high enough so that your
  overall system performance would improve by releasing this much
  memory.  As a rough guide, you might set to a value close to the
  average size of a process (program) running on your system.
  Releasing this much memory would allow such a process to run in
  memory.  Generally, it is worth tuning trim thresholds when a
  program undergoes phases where several large chunks are allocated
  and released in ways that can reuse each other's storage, perhaps
  mixed with phases where there are no such chunks at all. The trim
  value must be greater than page size to have any useful effect.  To
  disable trimming completely, you can set to MAX_SIZE_T. Note that the trick
  some people use of mallocing a huge space and then freeing it at
  program startup, in an attempt to reserve system memory, doesn't
  have the intended effect under automatic trimming, since that memory
  will immediately be returned to the system.

DEFAULT_MMAP_THRESHOLD       default: 256K
      Also settable using mallopt(M_MMAP_THRESHOLD, x)
  The request size threshold for using MMAP to directly service a
  request. Requests of at least this size that cannot be allocated
  using already-existing space will be serviced via mmap.  (If enough
  normal freed space already exists it is used instead.)  Using mmap
  segregates relatively large chunks of memory so that they can be
  individually obtained and released from the host system. A request
  serviced through mmap is never reused by any other request (at least
  not directly; the system may just so happen to remap successive
  requests to the same locations).  Segregating space in this way has
  the benefits that: Mmapped space can always be individually released
  back to the system, which helps keep the system level memory demands
  of a long-lived program low.  Also, mapped memory doesn't become
  `locked' between other chunks, as can happen with normally allocated
  chunks, which means that even trimming via malloc_trim would not
  release them.  However, it has the disadvantage that the space
  cannot be reclaimed, consolidated, and then used to service later
  requests, as happens with normal chunks.  The advantages of mmap
  nearly always outweigh disadvantages for "large" chunks, but the
  value of "large" may vary across systems.  The default is an
  empirically derived value that works well in most systems. You can
  disable mmap by setting to MAX_SIZE_T.

MAX_RELEASE_CHECK_RATE   default: 4095 unless not HAVE_MMAP
  The number of consolidated frees between checks to release
  unused segments when freeing. When using non-contiguous segments,
  especially with multiple mspaces, checking only for topmost space
  doesn't always suffice to trigger trimming. To compensate for this,
  free() will, with a period of MAX_RELEASE_CHECK_RATE (or the
  current number of segments, if greater) try to release unused
  segments to the OS when freeing chunks that result in
  consolidation. The best value for this parameter is a compromise
  between slowing down frees with relatively costly checks that
  rarely trigger versus holding on to unused memory. To effectively
  disable, set to MAX_SIZE_T. This may lead to a very slight speed
  improvement at the expense of carrying around more memory.
*/

/* Version identifier to allow people to support multiple versions */
#నిర్వచితం_కాకపోతే DLMALLOC_సంస్కరణ
#నిర్వచించు DLMALLOC_సంస్కరణ 20806
#అయితే_ముగింపు /* DLMALLOC_VERSION */

#నిర్వచితం_కాకపోతే DLMALLOC_EXPORT
#నిర్వచించు DLMALLOC_EXPORT బాహ్య
#అయితే_ముగింపు

#ఒకవేళ defined __linux__ && !defined GNU_ఆధారం
/* mremap() on Linux requires this via sys/mman.h */
#నిర్వచించు GNU_ఆధారం 1
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే WIN32
#నిర్వచితమైతే _WIN32
#నిర్వచించు WIN32 1
#అయితే_ముగింపు  /* _WIN32 */
#నిర్వచితమైతే _WIN32_WCE
#నిర్వచించు LACKS_FCNTL_H
#నిర్వచించు WIN32 1
#అయితే_ముగింపు /* _WIN32_WCE */
#అయితే_ముగింపు  /* WIN32 */
#నిర్వచితమైతే WIN32
#నిర్వచించు WIN32_LEAN_AND_సగటు
#అన్తర్భావ <windows.h>
#అన్తర్భావ <tchar.h>
#నిర్వచించు HAVE_MMAP 1
#నిర్వచించు HAVE_MORECORE 0
#నిర్వచించు LACKS_UNISTD_H
#నిర్వచించు LACKS_SYS_PARAM_H
#నిర్వచించు LACKS_SYS_MMAN_H
#నిర్వచించు LACKS_STRING_H
#నిర్వచించు LACKS_అక్షరశ్రేణులు_H
#నిర్వచించు LACKS_SYS_TYPES_H
#నిర్వచించు LACKS_ERRNO_H
#నిర్వచించు LACKS_SCHED_H
#నిర్వచితం_కాకపోతే MALLOC_FAILURE_ACTION
#నిర్వచించు MALLOC_FAILURE_ACTION
#అయితే_ముగింపు /* MALLOC_FAILURE_ACTION */
#నిర్వచితం_కాకపోతే MMAP_CLEARS
#నిర్వచితమైతే _WIN32_WCE /* WINCE reportedly does not clear */
#నిర్వచించు MMAP_CLEARS 0
#లేకపోతే
#నిర్వచించు MMAP_CLEARS 1
#అయితే_ముగింపు /* _WIN32_WCE */
#అయితే_ముగింపు /*MMAP_CLEARS */
#అయితే_ముగింపు  /* WIN32 */

#నిర్వచితమైతే __OS2__
#నిర్వచించు INCL_DOS
#అన్తర్భావ <os2.h>
#నిర్వచించు HAVE_MMAP 1
#నిర్వచించు HAVE_MORECORE 0
#నిర్వచించు LACKS_SYS_MMAN_H
#అయితే_ముగింపు  /* __OS2__ */

#ఒకవేళ defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#నిర్వచితం_కాకపోతే HAVE_MORECORE
#నిర్వచించు HAVE_MORECORE 0
#నిర్వచించు HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#నిర్వచితం_కాకపోతే MALLOC_ALIGNMENT
#నిర్వచించు MALLOC_ALIGNMENT ((size_t)16U)
#అయితే_ముగింపు
#అయితే_ముగింపు  /* HAVE_MORECORE */
#అయితే_ముగింపు  /* DARWIN */

#నిర్వచితం_కాకపోతే LACKS_SYS_TYPES_H
#అన్తర్భావ <sys/types.h>  /* For size_t */
#అయితే_ముగింపు  /* LACKS_SYS_TYPES_H */

/* The maximum possible size_t value has all bits set */
#నిర్వచించు MAX_కొలత_T           (~(size_t)0)

#నిర్వచితం_కాకపోతే USE_LOCKS /* ensure true if spin or recursive locks set */
#నిర్వచించు USE_LOCKS  ((defined(USE_SPIN_LOCKS) && USE_SPIN_LOCKS != 0) || \
                    (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0))
#అయితే_ముగింపు /* USE_LOCKS */

#ఒకవేళ USE_LOCKS /* Spin locks for gcc >= 4.1, older gcc on x86, MSC >= 1310 */
#ఒకవేళ ((defined(__GNUC__) &&                                              \
      ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) ||      \
       defined(__i386__) || defined(__x86_64__))) ||                    \
     (defined(_MSC_VER) && _MSC_VER>=1310))
#నిర్వచితం_కాకపోతే USE_SPIN_LOCKS
#నిర్వచించు USE_SPIN_LOCKS 1
#అయితే_ముగింపు /* USE_SPIN_LOCKS */
#లేదావేళ USE_SPIN_LOCKS
#దోషం "USE_SPIN_LOCKS defined without implementation"
#అయితే_ముగింపు /* ... locks available... */
#లేదావేళ !defined(USE_SPIN_LOCKS)
#నిర్వచించు USE_SPIN_LOCKS 0
#అయితే_ముగింపు /* USE_LOCKS */

#నిర్వచితం_కాకపోతే ONLY_MSPACES
#నిర్వచించు ONLY_MSPACES 0
#అయితే_ముగింపు  /* ONLY_MSPACES */
#నిర్వచితం_కాకపోతే MSPACES
#ఒకవేళ ONLY_MSPACES
#నిర్వచించు MSPACES 1
#లేకపోతే   /* ONLY_MSPACES */
#నిర్వచించు MSPACES 0
#అయితే_ముగింపు  /* ONLY_MSPACES */
#అయితే_ముగింపు  /* MSPACES */
#నిర్వచితం_కాకపోతే MALLOC_ALIGNMENT
#నిర్వచించు MALLOC_ALIGNMENT ((size_t)(2 * పరిమాణం(శూన్య *)))
#అయితే_ముగింపు  /* MALLOC_ALIGNMENT */
#నిర్వచితం_కాకపోతే FOOTERS
#నిర్వచించు FOOTERS 0
#అయితే_ముగింపు  /* FOOTERS */
#నిర్వచితం_కాకపోతే ABORT
#నిర్వచించు ABORT  abort()
#అయితే_ముగింపు  /* ABORT */
#నిర్వచితం_కాకపోతే ABORT_ON_ASSERT_FAILURE
#నిర్వచించు ABORT_ON_ASSERT_FAILURE 1
#అయితే_ముగింపు  /* ABORT_ON_ASSERT_FAILURE */
#నిర్వచితం_కాకపోతే PROCEED_ON_ERROR
#నిర్వచించు PROCEED_ON_ERROR 0
#అయితే_ముగింపు  /* PROCEED_ON_ERROR */

#నిర్వచితం_కాకపోతే INSECURE
#నిర్వచించు INSECURE 0
#అయితే_ముగింపు  /* INSECURE */
#నిర్వచితం_కాకపోతే MALLOC_INSPECT_ALL
#నిర్వచించు MALLOC_INSPECT_ALL 0
#అయితే_ముగింపు  /* MALLOC_INSPECT_ALL */
#నిర్వచితం_కాకపోతే HAVE_MMAP
#నిర్వచించు HAVE_MMAP 1
#అయితే_ముగింపు  /* HAVE_MMAP */
#నిర్వచితం_కాకపోతే MMAP_CLEARS
#నిర్వచించు MMAP_CLEARS 1
#అయితే_ముగింపు  /* MMAP_CLEARS */
#నిర్వచితం_కాకపోతే HAVE_MREMAP
#నిర్వచితమైతే linux
#నిర్వచించు HAVE_MREMAP 1
#నిర్వచించు GNU_ఆధారం /* Turns on mremap() definition */
#లేకపోతే   /* linux */
#నిర్వచించు HAVE_MREMAP 0
#అయితే_ముగింపు  /* linux */
#అయితే_ముగింపు  /* HAVE_MREMAP */
#నిర్వచితం_కాకపోతే MALLOC_FAILURE_ACTION
#నిర్వచించు MALLOC_FAILURE_ACTION  errno = ENOMEM;
#అయితే_ముగింపు  /* MALLOC_FAILURE_ACTION */
#నిర్వచితం_కాకపోతే HAVE_MORECORE
#ఒకవేళ ONLY_MSPACES
#నిర్వచించు HAVE_MORECORE 0
#లేకపోతే   /* ONLY_MSPACES */
#నిర్వచించు HAVE_MORECORE 1
#అయితే_ముగింపు  /* ONLY_MSPACES */
#అయితే_ముగింపు  /* HAVE_MORECORE */
#ఒకవేళ !HAVE_MORECORE
#నిర్వచించు MORECORE_CONTIGUOUS 0
#లేకపోతే   /* !HAVE_MORECORE */
#నిర్వచించు MORECORE_DEFAULT sbrk
#నిర్వచితం_కాకపోతే MORECORE_CONTIGUOUS
#నిర్వచించు MORECORE_CONTIGUOUS 1
#అయితే_ముగింపు  /* MORECORE_CONTIGUOUS */
#అయితే_ముగింపు  /* HAVE_MORECORE */
#నిర్వచితం_కాకపోతే DEFAULT_GRANULARITY
#ఒకవేళ (MORECORE_CONTIGUOUS || defined(WIN32))
#నిర్వచించు DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#లేకపోతే   /* MORECORE_CONTIGUOUS */
#నిర్వచించు DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#అయితే_ముగింపు  /* MORECORE_CONTIGUOUS */
#అయితే_ముగింపు  /* DEFAULT_GRANULARITY */
#నిర్వచితం_కాకపోతే DEFAULT_TRIM_పరిమితి
#నిర్వచితం_కాకపోతే MORECORE_CANNOT_TRIM
#నిర్వచించు DEFAULT_TRIM_పరిమితి ((size_t)2U * (size_t)1024U * (size_t)1024U)
#లేకపోతే   /* MORECORE_CANNOT_TRIM */
#నిర్వచించు DEFAULT_TRIM_పరిమితి MAX_కొలత_T
#అయితే_ముగింపు  /* MORECORE_CANNOT_TRIM */
#అయితే_ముగింపు  /* DEFAULT_TRIM_THRESHOLD */
#నిర్వచితం_కాకపోతే DEFAULT_MMAP_పరిమితి
#ఒకవేళ HAVE_MMAP
#నిర్వచించు DEFAULT_MMAP_పరిమితి ((size_t)256U * (size_t)1024U)
#లేకపోతే   /* HAVE_MMAP */
#నిర్వచించు DEFAULT_MMAP_పరిమితి MAX_కొలత_T
#అయితే_ముగింపు  /* HAVE_MMAP */
#అయితే_ముగింపు  /* DEFAULT_MMAP_THRESHOLD */
#నిర్వచితం_కాకపోతే MAX_RELEASE_CHECK_రేటు
#ఒకవేళ HAVE_MMAP
#నిర్వచించు MAX_RELEASE_CHECK_రేటు 4095
#లేకపోతే
#నిర్వచించు MAX_RELEASE_CHECK_రేటు MAX_కొలత_T
#అయితే_ముగింపు /* HAVE_MMAP */
#అయితే_ముగింపు /* MAX_RELEASE_CHECK_RATE */
#నిర్వచితం_కాకపోతే USE_BUILTIN_FFS
#నిర్వచించు USE_BUILTIN_FFS 0
#అయితే_ముగింపు  /* USE_BUILTIN_FFS */
#నిర్వచితం_కాకపోతే USE_DEV_RANDOM
#నిర్వచించు USE_DEV_RANDOM 0
#అయితే_ముగింపు  /* USE_DEV_RANDOM */
#నిర్వచితం_కాకపోతే NO_MALLINFO
#నిర్వచించు NO_MALLINFO 0
#అయితే_ముగింపు  /* NO_MALLINFO */
#నిర్వచితం_కాకపోతే MALLINFO_క్షేత్రం_TYPE
#నిర్వచించు MALLINFO_క్షేత్రం_TYPE size_t
#అయితే_ముగింపు  /* MALLINFO_FIELD_TYPE */
#నిర్వచితం_కాకపోతే NO_MALLOC_STATS
#నిర్వచించు NO_MALLOC_STATS 0
#అయితే_ముగింపు  /* NO_MALLOC_STATS */
#నిర్వచితం_కాకపోతే NO_SEGMENT_TRAVERSAL
#నిర్వచించు NO_SEGMENT_TRAVERSAL 0
#అయితే_ముగింపు /* NO_SEGMENT_TRAVERSAL */

/*
  mallopt tuning options.  SVID/XPG defines four standard parameter
  numbers for mallopt, normally defined in malloc.h.  None of these
  are used in this malloc, so setting them has no effect. But this
  malloc does support the following options.
*/

/* The system's malloc.h may have conflicting defines. */
#నిర్వచనం_రద్దు M_TRIM_పరిమితి
#నిర్వచనం_రద్దు M_GRANULARITY
#నిర్వచనం_రద్దు M_MMAP_పరిమితి

#నిర్వచించు M_TRIM_పరిమితి     (-1)
#నిర్వచించు M_GRANULARITY        (-2)
#నిర్వచించు M_MMAP_పరిమితి     (-3)

/* ------------------------ Mallinfo declarations ------------------------ */

#ఒకవేళ !NO_MALLINFO
/*
  This version of malloc supports the standard SVID/XPG mallinfo
  routine that returns a struct containing usage properties and
  statistics. It should work on any system that has a
  /usr/include/malloc.h defining struct mallinfo.  The main
  declaration needed is the mallinfo struct that is returned (by-copy)
  by mallinfo().  The malloinfo struct contains a bunch of fields that
  are not even meaningful in this version of malloc.  These fields are
  are instead filled by mallinfo() with other numbers that might be of
  interest.

  HAVE_USR_INCLUDE_MALLOC_H should be set if you have a
  /usr/include/malloc.h file that includes a declaration of struct
  mallinfo.  If so, it is included; else a compliant version is
  declared below.  These must be precisely the same for mallinfo() to
  work.  The original SVID version of this struct, defined on most
  systems with mallinfo, declares all fields as ints. But some others
  define as unsigned long. If your system defines the fields using a
  type of different width than listed here, you MUST #include your
  system version and #define HAVE_USR_INCLUDE_MALLOC_H.
*/

/* #define HAVE_USR_INCLUDE_MALLOC_H */

#నిర్వచితమైతే HAVE_USR_INCLUDE_MALLOC_H
#అన్తర్భావ "/usr/include/malloc.h"
#లేకపోతే /* HAVE_USR_INCLUDE_MALLOC_H */
#నిర్వచితం_కాకపోతే STRUCT_MALLINFO_DECLARED
/* HP-UX (and others?) redefines mallinfo unless _STRUCT_MALLINFO is defined */
#నిర్వచించు _STRUCT_MALLINFO
#నిర్వచించు STRUCT_MALLINFO_DECLARED 1
నిర్మాణం mallinfo {
  MALLINFO_క్షేత్రం_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_క్షేత్రం_TYPE ordblks;  /* number of free chunks */
  MALLINFO_క్షేత్రం_TYPE smblks;   /* always 0 */
  MALLINFO_క్షేత్రం_TYPE hblks;    /* always 0 */
  MALLINFO_క్షేత్రం_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_క్షేత్రం_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_క్షేత్రం_TYPE fsmblks;  /* always 0 */
  MALLINFO_క్షేత్రం_TYPE uordblks; /* total allocated space */
  MALLINFO_క్షేత్రం_TYPE fordblks; /* total free space */
  MALLINFO_క్షేత్రం_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#అయితే_ముగింపు /* STRUCT_MALLINFO_DECLARED */
#అయితే_ముగింపు /* HAVE_USR_INCLUDE_MALLOC_H */
#అయితే_ముగింపు /* NO_MALLINFO */

/*
  Try to persuade compilers to inline. The most critical functions for
  inlining are defined as macros, so these aren't used for them.
*/

#నిర్వచితం_కాకపోతే FORCEINLINE
  #ఒకవేళ defined(__GNUC__)
#నిర్వచించు FORCEINLINE __inline __attribute__ ((always_inline))
  #లేదావేళ defined(_MSC_VER)
    #నిర్వచించు FORCEINLINE __forceinline
  #అయితే_ముగింపు
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే NOINLINE
  #ఒకవేళ defined(__GNUC__)
    #నిర్వచించు NOINLINE __attribute__ ((noinline))
  #లేదావేళ defined(_MSC_VER)
    #నిర్వచించు NOINLINE __declspec(noinline)
  #లేకపోతే
    #నిర్వచించు NOINLINE
  #అయితే_ముగింపు
#అయితే_ముగింపు

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#నిర్వచితం_కాకపోతే FORCEINLINE
 #నిర్వచించు FORCEINLINE అంతఃవరుస
#అయితే_ముగింపు
#అయితే_ముగింపు /* __cplusplus */
#నిర్వచితం_కాకపోతే FORCEINLINE
 #నిర్వచించు FORCEINLINE
#అయితే_ముగింపు

#ఒకవేళ !ONLY_MSPACES

/* ------------------- Declarations of public routines ------------------- */

#నిర్వచితం_కాకపోతే USE_DL_ఉపసర్గ
#నిర్వచించు dlcalloc               సున్నా_జ్ఞాపకం_ఇవ్వు
#నిర్వచించు dlfree                 విడుదల
#నిర్వచించు dlmalloc               జ్ఞాపకం_ఇవ్వు
#నిర్వచించు dlmemalign             memalign
#నిర్వచించు dlposix_memalign       posix_memalign
#నిర్వచించు dlrealloc              realloc
#నిర్వచించు dlrealloc_లోపల_ఉంచు     realloc_లోపల_ఉంచు
#నిర్వచించు dlvalloc               valloc
#నిర్వచించు dlpvalloc              pvalloc
#నిర్వచించు dlmallinfo             mallinfo
#నిర్వచించు dlmallopt              mallopt
#నిర్వచించు dlmalloc_trim          malloc_trim
#నిర్వచించు dlmalloc_stats         malloc_stats
#నిర్వచించు dlmalloc_usable_కొలత   malloc_usable_కొలత
#నిర్వచించు dlmalloc_footprint     malloc_footprint
#నిర్వచించు dlmalloc_max_footprint malloc_max_footprint
#నిర్వచించు dlmalloc_footprint_హద్దు malloc_footprint_హద్దు
#నిర్వచించు dlmalloc_సమితి_footprint_హద్దు malloc_సమితి_footprint_హద్దు
#నిర్వచించు dlmalloc_inspect_all   malloc_inspect_all
#నిర్వచించు dlindependent_calloc   independent_calloc
#నిర్వచించు dlindependent_comalloc independent_comalloc
#నిర్వచించు dlbulk_free            bulk_free
#అయితే_ముగింపు /* USE_DL_PREFIX */

/*
  malloc(size_t n)
  Returns a pointer to a newly allocated chunk of at least n bytes, or
  null if no space is available, in which case errno is set to ENOMEM
  on ANSI C systems.

  If n is zero, malloc returns a minimum-sized chunk. (The minimum
  size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
  systems.)  Note that size_t is an unsigned type, so calls with
  arguments that would be negative if signed are interpreted as
  requests for huge amounts of space, which will often fail. The
  maximum supported value of n differs across systems, but is in all
  cases less than the maximum representable value of a size_t.
*/
DLMALLOC_EXPORT శూన్య* dlmalloc(size_t);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
DLMALLOC_EXPORT శూన్య  dlfree(శూన్య*);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
DLMALLOC_EXPORT శూన్య* dlcalloc(size_t, size_t);

/*
  realloc(void* p, size_t n)
  Returns a pointer to a chunk of size n that contains the same data
  as does chunk p up to the minimum of (n, p's size) bytes, or null
  if no space is available.

  The returned pointer may or may not be the same as p. The algorithm
  prefers extending p in most cases when possible, otherwise it
  employs the equivalent of a malloc-copy-free sequence.

  If p is null, realloc is equivalent to malloc.

  If space is not available, realloc returns null, errno is set (if on
  ANSI) and p is NOT freed.

  if n is for fewer bytes than already held by p, the newly unused
  space is lopped off and freed if possible.  realloc with a size
  argument of zero (re)allocates a minimum-sized chunk.

  The old unix realloc convention of allowing the last-free'd chunk
  to be used as an argument to realloc is not supported.
*/
DLMALLOC_EXPORT శూన్య* dlrealloc(శూన్య*, size_t);

/*
  realloc_in_place(void* p, size_t n)
  Resizes the space allocated for p to size n, only if this can be
  done without moving p (i.e., only if there is adjacent space
  available if n is greater than p's current allocated size, or n is
  less than or equal to p's size). This may be used instead of plain
  realloc if an alternative allocation strategy is needed upon failure
  to expand space; for example, reallocation of a buffer that must be
  memory-aligned or cleared. You can use realloc_in_place to trigger
  these alternatives only when needed.

  Returns p if successful; otherwise null.
*/
DLMALLOC_EXPORT శూన్య* dlrealloc_లోపల_ఉంచు(శూన్య*, size_t);

/*
  memalign(size_t alignment, size_t n);
  Returns a pointer to a newly allocated chunk of n bytes, aligned
  in accord with the alignment argument.

  The alignment argument should be a power of two. If the argument is
  not a power of two, the nearest greater power is used.
  8-byte alignment is guaranteed by normal malloc calls, so don't
  bother calling memalign with an argument of 8 or less.

  Overreliance on memalign is a sure way to fragment space.
*/
DLMALLOC_EXPORT శూన్య* dlmemalign(size_t, size_t);

/*
  int posix_memalign(void** pp, size_t alignment, size_t n);
  Allocates a chunk of n bytes, aligned in accord with the alignment
  argument. Differs from memalign only in that it (1) assigns the
  allocated memory to *pp rather than returning it, (2) fails and
  returns EINVAL if the alignment is not a power of two (3) fails and
  returns ENOMEM if memory cannot be allocated.
*/
DLMALLOC_EXPORT పూర్ణాంకం dlposix_memalign(శూన్య**, size_t, size_t);

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
DLMALLOC_EXPORT శూన్య* dlvalloc(size_t);

/*
  mallopt(int parameter_number, int parameter_value)
  Sets tunable parameters The format is to provide a
  (parameter-number, parameter-value) pair.  mallopt then sets the
  corresponding parameter to the argument value if it can (i.e., so
  long as the value is meaningful), and returns 1 if successful else
  0.  To workaround the fact that mallopt is specified to use int,
  not size_t parameters, the value -1 is specially treated as the
  maximum unsigned size_t value.

  SVID/XPG/ANSI defines four standard param numbers for mallopt,
  normally defined in malloc.h.  None of these are use in this malloc,
  so setting them has no effect. But this malloc also supports other
  options in mallopt. See below for details.  Briefly, supported
  parameters are as follows (listed defaults are for "typical"
  configurations).

  Symbol            param #  default    allowed param values
  M_TRIM_THRESHOLD     -1   2*1024*1024   any   (-1 disables)
  M_GRANULARITY        -2     page size   any power of 2 >= page size
  M_MMAP_THRESHOLD     -3      256*1024   any   (or 0 if no MMAP support)
*/
DLMALLOC_EXPORT పూర్ణాంకం dlmallopt(పూర్ణాంకం, పూర్ణాంకం);

/*
  malloc_footprint();
  Returns the number of bytes obtained from the system.  The total
  number of bytes allocated by malloc, realloc etc., is less than this
  value. Unlike mallinfo, this function returns only a precomputed
  result, so can be called frequently to monitor memory consumption.
  Even if locks are otherwise defined, this function does not use them,
  so results might not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint(శూన్య);

/*
  malloc_max_footprint();
  Returns the maximum number of bytes obtained from the system. This
  value will be greater than current footprint if deallocated space
  has been reclaimed by the system. The peak number of bytes allocated
  by malloc, realloc etc., is less than this value. Unlike mallinfo,
  this function returns only a precomputed result, so can be called
  frequently to monitor memory consumption.  Even if locks are
  otherwise defined, this function does not use them, so results might
  not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_max_footprint(శూన్య);

/*
  malloc_footprint_limit();
  Returns the number of bytes that the heap is allowed to obtain from
  the system, returning the last value returned by
  malloc_set_footprint_limit, or the maximum size_t value if
  never set. The returned value reflects a permission. There is no
  guarantee that this number of bytes can actually be obtained from
  the system.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint_హద్దు();

/*
  malloc_set_footprint_limit();
  Sets the maximum number of bytes to obtain from the system, causing
  failure returns from malloc and related functions upon attempts to
  exceed this value. The argument value may be subject to page
  rounding to an enforceable limit; this actual value is returned.
  Using an argument of the maximum possible size_t effectively
  disables checks. If the argument is less than or equal to the
  current malloc_footprint, then all future allocations that require
  additional system memory will fail. However, invocation cannot
  retroactively deallocate existing used memory.
*/
DLMALLOC_EXPORT size_t dlmalloc_సమితి_footprint_హద్దు(size_t బైట్లు);

#ఒకవేళ MALLOC_INSPECT_ALL
/*
  malloc_inspect_all(void(*handler)(void *start,
                                    void *end,
                                    size_t used_bytes,
                                    void* callback_arg),
                      void* arg);
  Traverses the heap and calls the given handler for each managed
  region, skipping all bytes that are (or may be) used for bookkeeping
  purposes.  Traversal does not include include chunks that have been
  directly memory mapped. Each reported region begins at the start
  address, and continues up to but not including the end address.  The
  first used_bytes of the region contain allocated data. If
  used_bytes is zero, the region is unallocated. The handler is
  invoked with the given callback argument. If locks are defined, they
  are held during the entire traversal. It is a bad idea to invoke
  other malloc functions from within the handler.

  For example, to count the number of in-use chunks with size greater
  than 1000, you could write:
  static int count = 0;
  void count_chunks(void* start, void* end, size_t used, void* arg) {
    if (used >= 1000) ++count;
  }
  then:
    malloc_inspect_all(count_chunks, NULL);

  malloc_inspect_all is compiled only if MALLOC_INSPECT_ALL is defined.
*/
DLMALLOC_EXPORT శూన్య dlmalloc_inspect_all(శూన్య(*handler)(శూన్య*, శూన్య *, size_t, శూన్య*),
                           శూన్య* arg);

#అయితే_ముగింపు /* MALLOC_INSPECT_ALL */

#ఒకవేళ !NO_MALLINFO
/*
  mallinfo()
  Returns (by copy) a struct containing various summary statistics:

  arena:     current total non-mmapped bytes allocated from system
  ordblks:   the number of free chunks
  smblks:    always zero.
  hblks:     current number of mmapped regions
  hblkhd:    total bytes held in mmapped regions
  usmblks:   the maximum total allocated space. This will be greater
                than current total if trimming has occurred.
  fsmblks:   always zero
  uordblks:  current total allocated space (normal or mmapped)
  fordblks:  total free space
  keepcost:  the maximum number of bytes that could ideally be released
               back to system via malloc_trim. ("ideally" means that
               it ignores page restrictions etc.)

  Because these fields are ints, but internal bookkeeping may
  be kept as longs, the reported values may wrap around zero and
  thus be inaccurate.
*/
DLMALLOC_EXPORT నిర్మాణం mallinfo dlmallinfo(శూన్య);
#అయితే_ముగింపు /* NO_MALLINFO */

/*
  independent_calloc(size_t n_elements, size_t element_size, void* chunks[]);

  independent_calloc is similar to calloc, but instead of returning a
  single cleared space, it returns an array of pointers to n_elements
  independent elements that can hold contents of size elem_size, each
  of which starts out cleared, and can be independently freed,
  realloc'ed etc. The elements are guaranteed to be adjacently
  allocated (this is not guaranteed to occur with multiple callocs or
  mallocs), which may also improve cache locality in some
  applications.

  The "chunks" argument is optional (i.e., may be null, which is
  probably the most typical usage). If it is null, the returned array
  is itself dynamically allocated and should also be freed when it is
  no longer needed. Otherwise, the chunks array must be of at least
  n_elements in length. It is filled in with the pointers to the
  chunks.

  In either case, independent_calloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and "chunks"
  is null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be freed when it is no longer needed. This can be
  done all at once using bulk_free.

  independent_calloc simplifies and speeds up implementations of many
  kinds of pools.  It may also be useful when constructing large data
  structures that initially have a fixed number of fixed-sized nodes,
  but the number is not known at compile time, and some of the nodes
  may later need to be freed. For example:

  struct Node { int item; struct Node* next; };

  struct Node* build_list() {
    struct Node** pool;
    int n = read_number_of_nodes_needed();
    if (n <= 0) return 0;
    pool = (struct Node**)(independent_calloc(n, sizeof(struct Node), 0);
    if (pool == 0) die();
    // organize into a linked list...
    struct Node* first = pool[0];
    for (i = 0; i < n-1; ++i)
      pool[i]->next = pool[i+1];
    free(pool);     // Can now free the array (or not, if it is needed later)
    return first;
  }
*/
DLMALLOC_EXPORT శూన్య** dlindependent_calloc(size_t, size_t, శూన్య**);

/*
  independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);

  independent_comalloc allocates, all at once, a set of n_elements
  chunks with sizes indicated in the "sizes" array.    It returns
  an array of pointers to these elements, each of which can be
  independently freed, realloc'ed etc. The elements are guaranteed to
  be adjacently allocated (this is not guaranteed to occur with
  multiple callocs or mallocs), which may also improve cache locality
  in some applications.

  The "chunks" argument is optional (i.e., may be null). If it is null
  the returned array is itself dynamically allocated and should also
  be freed when it is no longer needed. Otherwise, the chunks array
  must be of at least n_elements in length. It is filled in with the
  pointers to the chunks.

  In either case, independent_comalloc returns this pointer array, or
  null if the allocation failed.  If n_elements is zero and chunks is
  null, it returns a chunk representing an array with zero elements
  (which should be freed if not wanted).

  Each element must be freed when it is no longer needed. This can be
  done all at once using bulk_free.

  independent_comallac differs from independent_calloc in that each
  element may have a different size, and also that it does not
  automatically clear elements.

  independent_comalloc can be used to speed up allocation in cases
  where several structs or objects must always be allocated at the
  same time.  For example:

  struct Head { ... }
  struct Foot { ... }

  void send_message(char* msg) {
    int msglen = strlen(msg);
    size_t sizes[3] = { sizeof(struct Head), msglen, sizeof(struct Foot) };
    void* chunks[3];
    if (independent_comalloc(3, sizes, chunks) == 0)
      die();
    struct Head* head = (struct Head*)(chunks[0]);
    char*        body = (char*)(chunks[1]);
    struct Foot* foot = (struct Foot*)(chunks[2]);
    // ...
  }

  In general though, independent_comalloc is worth using only for
  larger values of n_elements. For small values, you probably won't
  detect enough difference from series of malloc calls to bother.

  Overuse of independent_comalloc can increase overall memory usage,
  since it cannot reuse existing noncontiguous small chunks that
  might be available for some of the elements.
*/
DLMALLOC_EXPORT శూన్య** dlindependent_comalloc(size_t, size_t*, శూన్య**);

/*
  bulk_free(void* array[], size_t n_elements)
  Frees and clears (sets to null) each non-null pointer in the given
  array.  This is likely to be faster than freeing them one-by-one.
  If footers are used, pointers that have been allocated in different
  mspaces are not freed or cleared, and the count of all such pointers
  is returned.  For large arrays of pointers with poor locality, it
  may be worthwhile to sort this array before calling bulk_free.
*/
DLMALLOC_EXPORT size_t  dlbulk_free(శూన్య**, size_t n_elements);

/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
DLMALLOC_EXPORT శూన్య*  dlpvalloc(size_t);

/*
  malloc_trim(size_t pad);

  If possible, gives memory back to the system (via negative arguments
  to sbrk) if there is unused memory at the `high' end of the malloc
  pool or in unused MMAP segments. You can call this after freeing
  large blocks of memory to potentially reduce the system-level memory
  requirements of a program. However, it cannot guarantee to reduce
  memory. Under some allocation patterns, some large free blocks of
  memory will be locked between two used chunks, so they cannot be
  given back to the system.

  The `pad' argument to malloc_trim represents the amount of free
  trailing space to leave untrimmed. If this argument is zero, only
  the minimum amount of memory to maintain internal data structures
  will be left. Non-zero arguments can be supplied to maintain enough
  trailing space to service future expected allocations without having
  to re-obtain memory from the system.

  Malloc_trim returns 1 if it actually released any memory, else 0.
*/
DLMALLOC_EXPORT పూర్ణాంకం  dlmalloc_trim(size_t);

/*
  malloc_stats();
  Prints on stderr the amount of space obtained from the system (both
  via sbrk and mmap), the maximum amount (which may be more than
  current if malloc_trim and/or munmap got called), and the current
  number of bytes allocated via malloc (or realloc, etc) but not yet
  freed. Note that this is the number of bytes allocated, not the
  number requested. It will be larger than the number requested
  because of alignment and bookkeeping overhead. Because it includes
  alignment wastage as being in use, this figure may be greater than
  zero even when no user-level chunks are allocated.

  The reported current and maximum system memory can be inaccurate if
  a program makes other calls to system memory allocation functions
  (normally sbrk) outside of malloc.

  malloc_stats prints only the most commonly interesting statistics.
  More information can be obtained by calling mallinfo.
*/
DLMALLOC_EXPORT శూన్య  dlmalloc_stats(శూన్య);

/*
  malloc_usable_size(void* p);

  Returns the number of bytes you can actually use in
  an allocated chunk, which may be more than you requested (although
  often not) due to alignment and minimum size constraints.
  You can use this many bytes without worrying about
  overwriting other allocated objects. This is not a particularly great
  programming practice. malloc_usable_size can be more useful in
  debugging and assertions, for example:

  p = malloc(n);
  assert(malloc_usable_size(p) >= 256);
*/
size_t dlmalloc_usable_కొలత(శూన్య*);

#అయితే_ముగింపు /* ONLY_MSPACES */

#ఒకవేళ MSPACES

/*
  mspace is an opaque type representing an independent
  region of space that supports mspace_malloc, etc.
*/
రకనిర్వచనం శూన్య* mspace;

/*
  create_mspace creates and returns a new independent space with the
  given initial capacity, or, if 0, the default granularity size.  It
  returns null if there is no system memory available to create the
  space.  If argument locked is non-zero, the space uses a separate
  lock to control access. The capacity of the space will grow
  dynamically as needed to service mspace_malloc requests.  You can
  control the sizes of incremental increases of this space by
  compiling with a different DEFAULT_GRANULARITY or dynamically
  setting with mallopt(M_GRANULARITY, value).
*/
DLMALLOC_EXPORT mspace create_mspace(size_t సామర్థ్యం, పూర్ణాంకం locked);

/*
  destroy_mspace destroys the given space, and attempts to return all
  of its memory back to the system, returning the total number of
  bytes freed. After destruction, the results of access to all memory
  used by the space become undefined.
*/
DLMALLOC_EXPORT size_t destroy_mspace(mspace msp);

/*
  create_mspace_with_base uses the memory supplied as the initial base
  of a new mspace. Part (less than 128*sizeof(size_t) bytes) of this
  space is used for bookkeeping, so the capacity must be at least this
  large. (Otherwise 0 is returned.) When this initial space is
  exhausted, additional memory will be obtained from the system.
  Destroying this space will deallocate all additionally allocated
  space (if possible) but not the initial base.
*/
DLMALLOC_EXPORT mspace create_mspace_with_పునాది(శూన్య* పునాది, size_t సామర్థ్యం, పూర్ణాంకం locked);

/*
  mspace_track_large_chunks controls whether requests for large chunks
  are allocated in their own untracked mmapped regions, separate from
  others in this mspace. By default large chunks are not tracked,
  which reduces fragmentation. However, such chunks are not
  necessarily released to the system upon destroy_mspace.  Enabling
  tracking by setting to true may increase fragmentation, but avoids
  leakage when relying on destroy_mspace to release all memory
  allocated using this space.  The function returns the previous
  setting.
*/
DLMALLOC_EXPORT పూర్ణాంకం mspace_track_large_chunks(mspace msp, పూర్ణాంకం enable);


/*
  mspace_malloc behaves as malloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT శూన్య* mspace_malloc(mspace msp, size_t బైట్లు);

/*
  mspace_free behaves as free, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_free is not actually needed.
  free may be called instead of mspace_free because freed chunks from
  any space are handled by their originating spaces.
*/
DLMALLOC_EXPORT శూన్య mspace_free(mspace msp, శూన్య* mem);

/*
  mspace_realloc behaves as realloc, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_realloc is not actually
  needed.  realloc may be called instead of mspace_realloc because
  realloced chunks from any space are handled by their originating
  spaces.
*/
DLMALLOC_EXPORT శూన్య* mspace_realloc(mspace msp, శూన్య* mem, size_t newsize);

/*
  mspace_calloc behaves as calloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT శూన్య* mspace_calloc(mspace msp, size_t n_elements, size_t elem_కొలత);

/*
  mspace_memalign behaves as memalign, but operates within
  the given space.
*/
DLMALLOC_EXPORT శూన్య* mspace_memalign(mspace msp, size_t alignment, size_t బైట్లు);

/*
  mspace_independent_calloc behaves as independent_calloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT శూన్య** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_కొలత, శూన్య* chunks[]);

/*
  mspace_independent_comalloc behaves as independent_comalloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT శూన్య** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], శూన్య* chunks[]);

/*
  mspace_footprint() returns the number of bytes obtained from the
  system for this space.
*/
DLMALLOC_EXPORT size_t mspace_footprint(mspace msp);

/*
  mspace_max_footprint() returns the peak number of bytes obtained from the
  system for this space.
*/
DLMALLOC_EXPORT size_t mspace_max_footprint(mspace msp);


#ఒకవేళ !NO_MALLINFO
/*
  mspace_mallinfo behaves as mallinfo, but reports properties of
  the given space.
*/
DLMALLOC_EXPORT నిర్మాణం mallinfo mspace_mallinfo(mspace msp);
#అయితే_ముగింపు /* NO_MALLINFO */

/*
  malloc_usable_size(void* p) behaves the same as malloc_usable_size;
*/
DLMALLOC_EXPORT size_t mspace_usable_కొలత(స్థిరరాశి శూన్య* mem);

/*
  mspace_malloc_stats behaves as malloc_stats, but reports
  properties of the given space.
*/
DLMALLOC_EXPORT శూన్య mspace_malloc_stats(mspace msp);

/*
  mspace_trim behaves as malloc_trim, but
  operates within the given space.
*/
DLMALLOC_EXPORT పూర్ణాంకం mspace_trim(mspace msp, size_t pad);

/*
  An alias for mallopt.
*/
DLMALLOC_EXPORT పూర్ణాంకం mspace_mallopt(పూర్ణాంకం, పూర్ణాంకం);

#అయితే_ముగింపు /* MSPACES */

#నిర్వచితమైతే __cplusplus
}  /* end of extern "C" */
#అయితే_ముగింపు /* __cplusplus */

/*
  ========================================================================
  To make a fully customizable malloc.h header file, cut everything
  above this line, put into file malloc.h, edit to suit, and #include it
  on the next line, as well as in programs that use this malloc.
  ========================================================================
*/

/* #include "malloc.h" */

/*------------------------------ internal #includes ---------------------- */

#నిర్వచితమైతే _MSC_VER
#సూచన warning( disable : 4146 ) /* no "unsigned" warnings */
#అయితే_ముగింపు /* _MSC_VER */
#ఒకవేళ !NO_MALLOC_STATS
#అన్తర్భావ <stdio.h>       /* for printing in malloc_stats */
#అయితే_ముగింపు /* NO_MALLOC_STATS */
#నిర్వచితం_కాకపోతే LACKS_ERRNO_H
#అన్తర్భావ <errno.h>       /* for MALLOC_FAILURE_ACTION */
#అయితే_ముగింపు /* LACKS_ERRNO_H */
#నిర్వచితమైతే DEBUG
#ఒకవేళ ABORT_ON_ASSERT_FAILURE
#నిర్వచనం_రద్దు ప్రతిజ్ఞా
#నిర్వచించు ప్రతిజ్ఞా(x) ఒకవేళ(!(x)) ABORT
#లేకపోతే /* ABORT_ON_ASSERT_FAILURE */
#అన్తర్భావ <assert.h>
#అయితే_ముగింపు /* ABORT_ON_ASSERT_FAILURE */
#లేకపోతే  /* DEBUG */
#నిర్వచితం_కాకపోతే ప్రతిజ్ఞా
#నిర్వచించు ప్రతిజ్ఞా(x)
#అయితే_ముగింపు
#నిర్వచించు DEBUG 0
#అయితే_ముగింపు /* DEBUG */
#ఒకవేళ !defined(WIN32) && !defined(LACKS_సమయం_H)
#అన్తర్భావ <time.h>        /* for magic initialization */
#అయితే_ముగింపు /* WIN32 */
#నిర్వచితం_కాకపోతే LACKS_STDLIB_H
#అన్తర్భావ <stdlib.h>      /* for abort() */
#అయితే_ముగింపు /* LACKS_STDLIB_H */
#నిర్వచితం_కాకపోతే LACKS_STRING_H
#అన్తర్భావ <string.h>      /* for memset etc */
#అయితే_ముగింపు  /* LACKS_STRING_H */
#ఒకవేళ USE_BUILTIN_FFS
#నిర్వచితం_కాకపోతే LACKS_అక్షరశ్రేణులు_H
#అన్తర్భావ <strings.h>     /* for ffs */
#అయితే_ముగింపు /* LACKS_STRINGS_H */
#అయితే_ముగింపు /* USE_BUILTIN_FFS */
#ఒకవేళ HAVE_MMAP
#నిర్వచితం_కాకపోతే LACKS_SYS_MMAN_H
/* On some versions of linux, mremap decl in mman.h needs __USE_GNU set */
#ఒకవేళ (defined(linux) && !defined(__USE_GNU))
#నిర్వచించు __USE_GNU 1
#అన్తర్భావ <sys/mman.h>    /* for mmap */
#నిర్వచనం_రద్దు __USE_GNU
#లేకపోతే
#అన్తర్భావ <sys/mman.h>    /* for mmap */
#అయితే_ముగింపు /* linux */
#అయితే_ముగింపు /* LACKS_SYS_MMAN_H */
#నిర్వచితం_కాకపోతే LACKS_FCNTL_H
#అన్తర్భావ <fcntl.h>
#అయితే_ముగింపు /* LACKS_FCNTL_H */
#అయితే_ముగింపు /* HAVE_MMAP */
#నిర్వచితం_కాకపోతే LACKS_UNISTD_H
#అన్తర్భావ <unistd.h>     /* for sbrk, sysconf */
#లేకపోతే /* LACKS_UNISTD_H */
#ఒకవేళ !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
బాహ్య శూన్య*     sbrk(ptrdiff_t);
#అయితే_ముగింపు /* FreeBSD etc */
#అయితే_ముగింపు /* LACKS_UNISTD_H */

/* Declarations for locking */
#ఒకవేళ USE_LOCKS
#నిర్వచితం_కాకపోతే WIN32
#ఒకవేళ defined (__SVR4) && defined (__sun)  /* solaris */
#అన్తర్భావ <thread.h>
#లేదావేళ !defined(LACKS_SCHED_H)
#అన్తర్భావ <sched.h>
#అయితే_ముగింపు /* solaris or LACKS_SCHED_H */
#ఒకవేళ (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0) || !USE_SPIN_LOCKS
#అన్తర్భావ <pthread.h>
#అయితే_ముగింపు /* USE_RECURSIVE_LOCKS ... */
#లేదావేళ defined(_MSC_VER)
#నిర్వచితం_కాకపోతే _M_AMD64
/* These are already defined on AMD64 builds */
#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు /* __cplusplus */
LONG __cdecl _InterlockedCompareExchange(LONG అస్థిర *Dest, LONG Exchange, LONG Comp);
LONG __cdecl _InterlockedExchange(LONG అస్థిర *లక్ష్యం, LONG విలువ_2);
#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు /* __cplusplus */
#అయితే_ముగింపు /* _M_AMD64 */
#సూచన intrinsic (_InterlockedCompareExchange)
#సూచన intrinsic (_InterlockedExchange)
#నిర్వచించు interlockedcompareexchange _InterlockedCompareExchange
#నిర్వచించు interlockedexchange _InterlockedExchange
#లేదావేళ defined(WIN32) && defined(__GNUC__)
#నిర్వచించు interlockedcompareexchange(a, b, c) __sync_val_compare_and_swap(a, c, b)
#నిర్వచించు interlockedexchange __sync_lock_test_and_set
#అయితే_ముగింపు /* Win32 */
#లేకపోతే /* USE_LOCKS */
#అయితే_ముగింపు /* USE_LOCKS */

#నిర్వచితం_కాకపోతే తాళం_AT_FORK
#నిర్వచించు తాళం_AT_FORK 0
#అయితే_ముగింపు

/* Declarations for bit scanning on win32 */
#ఒకవేళ defined(_MSC_VER) && _MSC_VER>=1300
#నిర్వచితం_కాకపోతే బిట్టు_పరిశీలన_Forward /* Try to avoid pulling in WinNT.h */
#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు /* __cplusplus */
గుర్తులేని అక్షరం బిట్టు_పరిశీలన_Forward_2(గుర్తులేని దీర్ఘ *సూచీ, గుర్తులేని దీర్ఘ ముసుగు);
గుర్తులేని అక్షరం బిట్టు_పరిశీలన_తిరగేయి(గుర్తులేని దీర్ఘ *సూచీ, గుర్తులేని దీర్ఘ ముసుగు);
#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు /* __cplusplus */

#నిర్వచించు బిట్టు_పరిశీలన_Forward బిట్టు_పరిశీలన_Forward_2
#నిర్వచించు బిట్టు_పరిశీలన_తిరగేయి_2 బిట్టు_పరిశీలన_తిరగేయి
#సూచన intrinsic(బిట్టు_పరిశీలన_Forward_2)
#సూచన intrinsic(బిట్టు_పరిశీలన_తిరగేయి)
#అయితే_ముగింపు /* BitScanForward */
#అయితే_ముగింపు /* defined(_MSC_VER) && _MSC_VER>=1300 */

#నిర్వచితం_కాకపోతే WIN32
#నిర్వచితం_కాకపోతే malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef SC_పుట_కొలత
#      define SC_పుట_కొలత _SC_PAGESIZE
#    endif
#  endif
#  ifdef SC_పుట_కొలత
#    define malloc_getpagesize sysconf(SC_పుట_కొలత)
#  లేకపోతే
#    ఒకవేళ defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       బాహ్య size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    లేకపోతే
#      ifdef WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize()
#      లేకపోతే
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        లేకపోతే
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            లేకపోతే
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          లేకపోతే
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            లేకపోతే
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              లేకపోతే /* just guess */
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#అయితే_ముగింపు
#అయితే_ముగింపు

/* libffi: the upstream malloc_getpagesize definition above is gated on
   !WIN32 (WIN32 builds use GetSystemInfo in init_mparams instead).  Guarantee
   the symbol is always defined so the !WIN32 init_mparams branch still compiles
   on toolchains where it is not preprocessed out as expected (e.g. MSVC).  */
#నిర్వచితం_కాకపోతే malloc_getpagesize
#నిర్వచించు malloc_getpagesize ((size_t)4096U)
#అయితే_ముగింపు

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#నిర్వచించు కొలత_T_కొలత         (పరిమాణం(size_t))
#నిర్వచించు కొలత_T_BITSIZE      (పరిమాణం(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#నిర్వచించు కొలత_T_ZERO         ((size_t)0)
#నిర్వచించు కొలత_T_ONE          ((size_t)1)
#నిర్వచించు కొలత_T_TWO          ((size_t)2)
#నిర్వచించు కొలత_T_FOUR         ((size_t)4)
#నిర్వచించు TWO_కొలత_T_SIZES    (కొలత_T_కొలత<<1)
#నిర్వచించు FOUR_కొలత_T_SIZES   (కొలత_T_కొలత<<2)
#నిర్వచించు SIX_కొలత_T_SIZES    (FOUR_కొలత_T_SIZES+TWO_కొలత_T_SIZES)
#నిర్వచించు HALF_MAX_కొలత_T     (MAX_కొలత_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#నిర్వచించు CHUNK_ALIGN_ముసుగు    (MALLOC_ALIGNMENT - కొలత_T_ONE)

/* True if address a has acceptable alignment */
#నిర్వచించు is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_ముసుగు)) == 0)

/* the number of bytes to offset an address to align it */
#నిర్వచించు align_స్థానభ్రంశం(A)\
 ((((size_t)(A) & CHUNK_ALIGN_ముసుగు) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_ముసుగు)) & CHUNK_ALIGN_ముసుగు))

/* -------------------------- MMAP preliminaries ------------------------- */

/*
   If HAVE_MORECORE or HAVE_MMAP are false, we just define calls and
   checks to fail so compiler optimizer can delete code rather than
   using so many "#if"s.
*/


/* MORECORE and MMAP must return MFAIL on failure */
#నిర్వచించు MFAIL                ((శూన్య*)(MAX_కొలత_T))
#నిర్వచించు CMFAIL               ((అక్షరం*)(MFAIL)) /* defined for convenience */

#ఒకవేళ HAVE_MMAP

#ఒకవేళ !defined(WIN32) && !defined (__OS2__)
#నిర్వచించు MUNMAP_DEFAULT(a, s)  munmap((a), (s))
#నిర్వచించు MMAP_PROT            (PROT_READ|PROT_WRITE)
#ఒకవేళ !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#నిర్వచించు MAP_ANONYMOUS        MAP_ANON
#అయితే_ముగింపు /* MAP_ANON */
#నిర్వచితమైతే MAP_ANONYMOUS
#నిర్వచించు MMAP_FLAGS           (MAP_వ్యక్తిగత|MAP_ANONYMOUS)
#నిర్వచించు MMAP_DEFAULT(s)       mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#లేకపోతే /* MAP_ANONYMOUS */
/*
   Nearly all versions of mmap support MAP_ANONYMOUS, so the following
   is unlikely to be needed, but is supplied just in case.
*/
#నిర్వచించు MMAP_FLAGS           (MAP_వ్యక్తిగత)
స్థిర పూర్ణాంకం dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#నిర్వచించు MMAP_DEFAULT(s) ((dev_zero_fd < 0) ? \
           (dev_zero_fd = open("/dev/zero", O_RDWR), \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#అయితే_ముగింపు /* MAP_ANONYMOUS */

#నిర్వచించు ప్రత్యక్ష_MMAP_DEFAULT(s) MMAP_DEFAULT(s)

#లేదావేళ defined(__OS2__)

/* OS/2 MMAP via DosAllocMem */
స్థిర శూన్య* os2mmap(size_t కొలత) {
  శూన్య* ptr;
  ఒకవేళ (DosAllocMem(&ptr, కొలత, OBJ_ANY|PAG_COMMIT|PAG_READ|PAG_WRITE) &&
      DosAllocMem(&ptr, కొలత, PAG_COMMIT|PAG_READ|PAG_WRITE))
    తిరిగి MFAIL;
  తిరిగి ptr;
}

#నిర్వచించు os2direct_mmap(n)     os2mmap(n)

/* This function supports releasing coalesed segments */
స్థిర పూర్ణాంకం os2munmap(శూన్య* ptr, size_t కొలత) {
  అయితే (కొలత) {
    ULONG ul_కొలత = కొలత;
    ULONG ulFlags = 0;
    ఒకవేళ (DosQueryMem(ptr, &ul_కొలత, &ulFlags) != 0)
      తిరిగి -1;
    ఒకవేళ ((ulFlags & PAG_పునాది) == 0 ||(ulFlags & PAG_COMMIT) == 0 ||
        ul_కొలత > కొలత)
      తిరిగి -1;
    ఒకవేళ (DosFreeMem(ptr) != 0)
      తిరిగి -1;
    ptr = ( శూన్య * ) ( ( అక్షరం * ) ptr + ul_కొలత );
    కొలత -= ul_కొలత;
  }
  తిరిగి 0;
}

#నిర్వచించు MMAP_DEFAULT(s)         os2mmap(s)
#నిర్వచించు MUNMAP_DEFAULT(a, s)    os2munmap((a), (s))
#నిర్వచించు ప్రత్యక్ష_MMAP_DEFAULT(s)  os2direct_mmap(s)

#లేకపోతే /* WIN32 */

/* Win32 MMAP via VirtualAlloc */
/* libffi: plain static (not FORCEINLINE) -- mingw's <windows.h> defines
   FORCEINLINE with an 'extern' storage class, which clashes with 'static'.  */
స్థిర శూన్య* win32mmap(size_t కొలత) {
  శూన్య* ptr = VirtualAlloc(0, కొలత, MEM_RESERVE|MEM_COMMIT, పుట_EXECUTE_READWRITE);
  తిరిగి (ptr != 0)? ptr: MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
స్థిర శూన్య* win32direct_mmap(size_t కొలత) {
  శూన్య* ptr = VirtualAlloc(0, కొలత, MEM_RESERVE|MEM_COMMIT|MEM_శిఖరం_కిందికి,
                           పుట_EXECUTE_READWRITE);
  తిరిగి (ptr != 0)? ptr: MFAIL;
}

/* This function supports releasing coalesed segments */
స్థిర పూర్ణాంకం win32munmap(శూన్య* ptr, size_t కొలత) {
  జ్ఞాపకం_BASIC_INFORMATION minfo;
  అక్షరం* cptr = (అక్షరం*)ptr;
  అయితే (కొలత) {
    ఒకవేళ (VirtualQuery(cptr, &minfo, పరిమాణం(minfo)) == 0)
      తిరిగి -1;
    ఒకవేళ (minfo.పునాది_చిరునామా != cptr || minfo.కేటాయింపు_పునాది != cptr ||
        minfo.State != MEM_COMMIT || minfo.Region_కొలత > కొలత)
      తిరిగి -1;
    ఒకవేళ (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
      తిరిగి -1;
    cptr += minfo.Region_కొలత;
    కొలత -= minfo.Region_కొలత;
  }
  తిరిగి 0;
}

#నిర్వచించు MMAP_DEFAULT(s)             win32mmap(s)
#నిర్వచించు MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
#నిర్వచించు ప్రత్యక్ష_MMAP_DEFAULT(s)      win32direct_mmap(s)
#అయితే_ముగింపు /* WIN32 */
#అయితే_ముగింపు /* HAVE_MMAP */

#ఒకవేళ HAVE_MREMAP
#నిర్వచితం_కాకపోతే WIN32
#నిర్వచించు MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#అయితే_ముగింపు /* WIN32 */
#అయితే_ముగింపు /* HAVE_MREMAP */

/**
 * Define CALL_MORECORE
 */
#ఒకవేళ HAVE_MORECORE
    #నిర్వచితమైతే MORECORE
        #నిర్వచించు CALL_MORECORE(S)    MORECORE(S)
    #లేకపోతే  /* MORECORE */
        #నిర్వచించు CALL_MORECORE(S)    MORECORE_DEFAULT(S)
    #అయితే_ముగింపు /* MORECORE */
#లేకపోతే  /* HAVE_MORECORE */
    #నిర్వచించు CALL_MORECORE(S)        MFAIL
#అయితే_ముగింపు /* HAVE_MORECORE */

/**
 * Define CALL_MMAP/CALL_MUNMAP/CALL_DIRECT_MMAP
 */
#ఒకవేళ HAVE_MMAP
    #నిర్వచించు USE_MMAP_బిట్టు            (కొలత_T_ONE)

    #నిర్వచితమైతే MMAP
        #నిర్వచించు CALL_MMAP(s)        MMAP(s)
    #లేకపోతే /* MMAP */
        #నిర్వచించు CALL_MMAP(s)        MMAP_DEFAULT(s)
    #అయితే_ముగింపు /* MMAP */
    #నిర్వచితమైతే MUNMAP
        #నిర్వచించు CALL_MUNMAP(a, s)   MUNMAP((a), (s))
    #లేకపోతే /* MUNMAP */
        #నిర్వచించు CALL_MUNMAP(a, s)   MUNMAP_DEFAULT((a), (s))
    #అయితే_ముగింపు /* MUNMAP */
    #నిర్వచితమైతే ప్రత్యక్ష_MMAP
        #నిర్వచించు CALL_ప్రత్యక్ష_MMAP(s) ప్రత్యక్ష_MMAP(s)
    #లేకపోతే /* DIRECT_MMAP */
        #నిర్వచించు CALL_ప్రత్యక్ష_MMAP(s) ప్రత్యక్ష_MMAP_DEFAULT(s)
    #అయితే_ముగింపు /* DIRECT_MMAP */
#లేకపోతే  /* HAVE_MMAP */
    #నిర్వచించు USE_MMAP_బిట్టు            (కొలత_T_ZERO)

    #నిర్వచించు MMAP(s)                 MFAIL
    #నిర్వచించు MUNMAP(a, s)            (-1)
    #నిర్వచించు ప్రత్యక్ష_MMAP(s)          MFAIL
    #నిర్వచించు CALL_ప్రత్యక్ష_MMAP(s)     ప్రత్యక్ష_MMAP(s)
    #నిర్వచించు CALL_MMAP(s)            MMAP(s)
    #నిర్వచించు CALL_MUNMAP(a, s)       MUNMAP((a), (s))
#అయితే_ముగింపు /* HAVE_MMAP */

/**
 * Define CALL_MREMAP
 */
#ఒకవేళ HAVE_MMAP && HAVE_MREMAP
    #నిర్వచితమైతే MREMAP
        #నిర్వచించు CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
    #లేకపోతే /* MREMAP */
        #నిర్వచించు CALL_MREMAP(addr, osz, nsz, mv) MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
    #అయితే_ముగింపు /* MREMAP */
#లేకపోతే  /* HAVE_MMAP && HAVE_MREMAP */
    #నిర్వచించు CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#అయితే_ముగింపు /* HAVE_MMAP && HAVE_MREMAP */

/* mstate bit set if contiguous morecore disabled or failed */
#నిర్వచించు USE_NONCONTIGUOUS_బిట్టు (4U)

/* segment bit set in create_mspace_with_base */
#నిర్వచించు EXTERN_బిట్టు            (8U)


/* --------------------------- Lock preliminaries ------------------------ */

/*
  When locks are defined, there is one global lock, plus
  one per-mspace lock.

  The global lock_ensures that mparams.magic and other unique
  mparams values are initialized only once. It also protects
  sequences of calls to MORECORE.  In many cases sys_alloc requires
  two calls, that should not be interleaved with calls by other
  threads.  This does not protect against direct calls to MORECORE
  by other threads not using this lock, so there is still code to
  cope the best we can on interference.

  Per-mspace locks surround calls to malloc, free, etc.
  By default, locks are simple non-reentrant mutexes.

  Because lock-protected regions generally have bounded times, it is
  OK to use the supplied simple spinlocks. Spinlocks are likely to
  improve performance for lightly contended applications, but worsen
  performance under heavy contention.

  If USE_LOCKS is > 1, the definitions of lock routines here are
  bypassed, in which case you will need to define the type MLOCK_T,
  and at least INITIAL_LOCK, DESTROY_LOCK, ACQUIRE_LOCK, RELEASE_LOCK
  and TRY_LOCK.  You must also declare a
    static MLOCK_T malloc_global_mutex = { initialization values };.

*/

#ఒకవేళ !USE_LOCKS
#నిర్వచించు USE_తాళం_బిట్టు               (0U)
#నిర్వచించు INITIAL_తాళం(l)            (0)
#నిర్వచించు DESTROY_తాళం(l)            (0)
#నిర్వచించు ACQUIRE_MALLOC_GLOBAL_తాళం()
#నిర్వచించు RELEASE_MALLOC_GLOBAL_తాళం()

#లేకపోతే
#ఒకవేళ USE_LOCKS > 1
/* -----------------------  User-defined locks ------------------------ */
/* Define your own lock implementation here */
/* #define INITIAL_LOCK(lk)  ... */
/* #define DESTROY_LOCK(lk)  ... */
/* #define ACQUIRE_LOCK(lk)  ... */
/* #define RELEASE_LOCK(lk)  ... */
/* #define TRY_LOCK(lk) ... */
/* static MLOCK_T malloc_global_mutex = ... */

#లేదావేళ USE_SPIN_LOCKS

/* First, define CAS_LOCK and CLEAR_LOCK on ints */
/* Note CAS_LOCK defined to return 0 on success */

#ఒకవేళ defined(__GNUC__)&& (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#నిర్వచించు CAS_తాళం(sl)     __sync_lock_test_and_set(sl, 1)
#నిర్వచించు CLEAR_తాళం(sl)   __sync_lock_release(sl)

#లేదావేళ (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
/* Custom spin locks for older gcc on x86 */
స్థిర FORCEINLINE పూర్ణాంకం x86_cas_తాళం(పూర్ణాంకం *sl) {
  పూర్ణాంకం ret;
  పూర్ణాంకం val = 1;
  పూర్ణాంకం cmp = 0;
  __asm__ __volatile__  ("lock; cmpxchgl %1, %2"
                         : "=a" (ret)
                         : "r" (val), "m" (*(sl)), "0"(cmp)
                         : "memory", "cc");
  తిరిగి ret;
}

స్థిర FORCEINLINE శూన్య x86_clear_తాళం(పూర్ణాంకం* sl) {
  ప్రతిజ్ఞా(*sl != 0);
  పూర్ణాంకం prev = 0;
  పూర్ణాంకం ret;
  __asm__ __volatile__ ("lock; xchgl %0, %1"
                        : "=r" (ret)
                        : "m" (*(sl)), "0"(prev)
                        : "memory");
}

#నిర్వచించు CAS_తాళం(sl)     x86_cas_తాళం(sl)
#నిర్వచించు CLEAR_తాళం(sl)   x86_clear_తాళం(sl)

#లేకపోతే /* Win32 MSC */
#నిర్వచించు CAS_తాళం(sl)     interlockedexchange((LONG అస్థిర *)(sl), (LONG)1)
#నిర్వచించు CLEAR_తాళం(sl)   interlockedexchange ((LONG అస్థిర *)(sl), (LONG)0)

#అయితే_ముగింపు /* ... gcc spins locks ... */

/* How to yield for a spin lock */
#నిర్వచించు SPINS_చొప్పున_YIELD       63
#ఒకవేళ defined(_MSC_VER)
#నిర్వచించు SLEEP_EX_DURATION     50 /* delay for yield/sleep */
#నిర్వచించు SPIN_తాళం_YIELD  SleepEx(SLEEP_EX_DURATION, FALSE)
#లేదావేళ defined (__SVR4) && defined (__sun) /* solaris */
#నిర్వచించు SPIN_తాళం_YIELD   thr_yield();
#లేదావేళ !defined(LACKS_SCHED_H)
#నిర్వచించు SPIN_తాళం_YIELD   sched_yield();
#లేకపోతే
#నిర్వచించు SPIN_తాళం_YIELD
#అయితే_ముగింపు /* ... yield ... */

/* libffi: read the spin-lock word with a relaxed atomic load for the lock-free
   "is it held?" peek, so ThreadSanitizer does not flag it as racing with the
   atomic CLEAR_LOCK release on unlock.  Relaxed ordering is sufficient: the
   peek is only a hint to avoid a costly CAS, and CAS_LOCK still provides the
   acquire ordering on success.  Falls back to the original volatile read when
   atomic builtins are unavailable.  */
#ఒకవేళ defined(__ATOMIC_RELAXED)
# define ffi_spin_peek(sl) __atomic_load_n((sl), __ATOMIC_RELAXED)
#లేదావేళ defined(_MSC_VER)
# define ffi_spin_peek(sl) \
  ((పూర్ణాంకం)_InterlockedCompareExchange((LONG అస్థిర *)(sl), (LONG)0, (LONG)0))
#లేకపోతే
# define ffi_spin_peek(sl) (*(అస్థిర పూర్ణాంకం *)(sl))
#అయితే_ముగింపు

#ఒకవేళ !defined(USE_RECURSIVE_LOCKS) || USE_RECURSIVE_LOCKS == 0
/* Plain spin locks use single word (embedded in malloc_states) */
స్థిర పూర్ణాంకం spin_acquire_తాళం(పూర్ణాంకం *sl) {
  పూర్ణాంకం spins = 0;
  అయితే (ffi_spin_peek(sl) != 0 || CAS_తాళం(sl)) {
    ఒకవేళ ((++spins & SPINS_చొప్పున_YIELD) == 0) {
      SPIN_తాళం_YIELD;
    }
  }
  తిరిగి 0;
}

#నిర్వచించు MLOCK_T               పూర్ణాంకం
#నిర్వచించు TRY_తాళం(sl)          !CAS_తాళం(sl)
#నిర్వచించు RELEASE_తాళం(sl)      CLEAR_తాళం(sl)
#నిర్వచించు ACQUIRE_తాళం(sl)      (CAS_తాళం(sl)? spin_acquire_తాళం(sl) : 0)
#నిర్వచించు INITIAL_తాళం(sl)      (*sl = 0)
#నిర్వచించు DESTROY_తాళం(sl)      (0)
స్థిర MLOCK_T malloc_global_mutex = 0;

#లేకపోతే /* USE_RECURSIVE_LOCKS */
/* types for lock owners */
#నిర్వచితమైతే WIN32
#నిర్వచించు THREAD_ID_T           DWORD
#నిర్వచించు CURRENT_THREAD        GetCurrentThreadId()
#నిర్వచించు EQ_OWNER(X,Y)         ((X) == (Y))
#లేకపోతే
/*
  Note: the following assume that pthread_t is a type that can be
  initialized to (casted) zero. If this is not the case, you will need to
  somehow redefine these or not use spin locks.
*/
#నిర్వచించు THREAD_ID_T           pthread_t
#నిర్వచించు CURRENT_THREAD        pthread_self()
#నిర్వచించు EQ_OWNER(X,Y)         pthread_equal(X, Y)
#అయితే_ముగింపు

నిర్మాణం malloc_recursive_తాళం {
  పూర్ణాంకం sl;
  గుర్తులేని పూర్ణాంకం c;
  THREAD_ID_T threadid;
};

#నిర్వచించు MLOCK_T  నిర్మాణం malloc_recursive_తాళం
స్థిర MLOCK_T malloc_global_mutex = { 0, 0, (THREAD_ID_T)0};

స్థిర FORCEINLINE శూన్య recursive_release_తాళం(MLOCK_T *lk) {
  ప్రతిజ్ఞా(lk->sl != 0);
  ఒకవేళ (--lk->c == 0) {
    CLEAR_తాళం(&lk->sl);
  }
}

స్థిర FORCEINLINE పూర్ణాంకం recursive_acquire_తాళం(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  పూర్ణాంకం spins = 0;
  కొరకు (;;) {
    ఒకవేళ (ffi_spin_peek(&lk->sl) == 0) {
      ఒకవేళ (!CAS_తాళం(&lk->sl)) {
        lk->threadid = mythreadid;
        lk->c = 1;
        తిరిగి 0;
      }
    }
    లేకపోతే ఒకవేళ (EQ_OWNER(lk->threadid, mythreadid)) {
      ++lk->c;
      తిరిగి 0;
    }
    ఒకవేళ ((++spins & SPINS_చొప్పున_YIELD) == 0) {
      SPIN_తాళం_YIELD;
    }
  }
}

స్థిర FORCEINLINE పూర్ణాంకం recursive_try_తాళం(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  ఒకవేళ (ffi_spin_peek(&lk->sl) == 0) {
    ఒకవేళ (!CAS_తాళం(&lk->sl)) {
      lk->threadid = mythreadid;
      lk->c = 1;
      తిరిగి 1;
    }
  }
  లేకపోతే ఒకవేళ (EQ_OWNER(lk->threadid, mythreadid)) {
    ++lk->c;
    తిరిగి 1;
  }
  తిరిగి 0;
}

#నిర్వచించు RELEASE_తాళం(lk)      recursive_release_తాళం(lk)
#నిర్వచించు TRY_తాళం(lk)          recursive_try_తాళం(lk)
#నిర్వచించు ACQUIRE_తాళం(lk)      recursive_acquire_తాళం(lk)
#నిర్వచించు INITIAL_తాళం(lk)      ((lk)->threadid = (THREAD_ID_T)0, (lk)->sl = 0, (lk)->c = 0)
#నిర్వచించు DESTROY_తాళం(lk)      (0)
#అయితే_ముగింపు /* USE_RECURSIVE_LOCKS */

#లేదావేళ defined(WIN32) /* Win32 critical sections */
#నిర్వచించు MLOCK_T               CRITICAL_SECTION
#నిర్వచించు ACQUIRE_తాళం(lk)      (EnterCriticalSection(lk), 0)
#నిర్వచించు RELEASE_తాళం(lk)      LeaveCriticalSection(lk)
#నిర్వచించు TRY_తాళం(lk)          TryEnterCriticalSection(lk)
#నిర్వచించు INITIAL_తాళం(lk)      (!Initialize_Critical_Section_And_Spin_లెక్క((lk), 0x80000000|4000))
#నిర్వచించు DESTROY_తాళం(lk)      (DeleteCriticalSection(lk), 0)
#నిర్వచించు NEED_GLOBAL_తాళం_INIT

స్థిర MLOCK_T malloc_global_mutex;
స్థిర అస్థిర LONG malloc_global_mutex_స్థితి;

/* Use spin loop to initialize global lock */
స్థిర శూన్య init_malloc_global_mutex() {
  కొరకు (;;) {
    దీర్ఘ stat = malloc_global_mutex_స్థితి;
    ఒకవేళ (stat > 0)
      తిరిగి;
    /* transition to < 0 while initializing, then to > 0) */
    ఒకవేళ (stat == 0 &&
        interlockedcompareexchange(&malloc_global_mutex_స్థితి, (LONG)-1, (LONG)0) == 0) {
      InitializeCriticalSection(&malloc_global_mutex);
      interlockedexchange(&malloc_global_mutex_స్థితి, (LONG)1);
      తిరిగి;
    }
    SleepEx(0, FALSE);
  }
}

#లేకపోతే /* pthreads-based locks */
#నిర్వచించు MLOCK_T               pthread_mutex_t
#నిర్వచించు ACQUIRE_తాళం(lk)      pthread_mutex_తాళం(lk)
#నిర్వచించు RELEASE_తాళం(lk)      pthread_mutex_unlock(lk)
#నిర్వచించు TRY_తాళం(lk)          (!pthread_mutex_trylock(lk))
#నిర్వచించు INITIAL_తాళం(lk)      pthread_init_తాళం(lk)
#నిర్వచించు DESTROY_తాళం(lk)      pthread_mutex_destroy(lk)

#ఒకవేళ defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0 && defined(linux) && !defined(PTHREAD_MUTEX_RECURSIVE)
/* Cope with old-style linux recursive lock initialization by adding */
/* skipped internal declaration from pthread.h */
బాహ్య పూర్ణాంకం pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
                                              పూర్ణాంకం __kind));
#నిర్వచించు PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#నిర్వచించు pthread_mutexattr_settype(x,y) pthread_mutexattr_setkind_np(x,y)
#అయితే_ముగింపు /* USE_RECURSIVE_LOCKS ... */

స్థిర MLOCK_T malloc_global_mutex = PTHREAD_MUTEX_INITIALIZER;

స్థిర పూర్ణాంకం pthread_init_తాళం (MLOCK_T *lk) {
  pthread_mutexattr_t attr;
  ఒకవేళ (pthread_mutexattr_init(&attr)) తిరిగి 1;
#ఒకవేళ defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0
  ఒకవేళ (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) తిరిగి 1;
#అయితే_ముగింపు
  ఒకవేళ (pthread_mutex_init(lk, &attr)) తిరిగి 1;
  ఒకవేళ (pthread_mutexattr_destroy(&attr)) తిరిగి 1;
  తిరిగి 0;
}

#అయితే_ముగింపు /* ... lock types ... */

/* Common code for all lock types */
#నిర్వచించు USE_తాళం_బిట్టు               (2U)

#నిర్వచితం_కాకపోతే ACQUIRE_MALLOC_GLOBAL_తాళం
#నిర్వచించు ACQUIRE_MALLOC_GLOBAL_తాళం()  ACQUIRE_తాళం(&malloc_global_mutex);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే RELEASE_MALLOC_GLOBAL_తాళం
#నిర్వచించు RELEASE_MALLOC_GLOBAL_తాళం()  RELEASE_తాళం(&malloc_global_mutex);
#అయితే_ముగింపు

#అయితే_ముగింపు /* USE_LOCKS */

/* -----------------------  Chunk representations ------------------------ */

/*
  (The following includes lightly edited explanations by Colin Plumb.)

  The malloc_chunk declaration below is misleading (but accurate and
  necessary).  It declares a "view" into memory allowing access to
  necessary fields at known offsets from a given base.

  Chunks of memory are maintained using a `boundary tag' method as
  originally described by Knuth.  (See the paper by Paul Wilson
  ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a survey of such
  techniques.)  Sizes of free chunks are stored both in the front of
  each chunk and at the end.  This makes consolidating fragmented
  chunks into bigger chunks fast.  The head fields also hold bits
  representing whether chunks are free or in use.

  Here are some pictures to make it clearer.  They are "exploded" to
  show that the state of a chunk can be thought of as extending from
  the high 31 bits of the head field of its header through the
  prev_foot and PINUSE_BIT bit of the following chunk header.

  A chunk that's in use looks like:

   chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
           | Size of previous chunk (if P = 0)                             |
           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |P|
         | Size of this chunk                                         1| +-+
   mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |                                                               |
         +-                                                             -+
         |                                                               |
         +-                                                             -+
         |                                                               :
         +-      size - sizeof(size_t) available payload bytes          -+
         :                                                               |
 chunk-> +-                                                             -+
         |                                                               |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |1|
       | Size of next chunk (may or may not be in use)               | +-+
 mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    And if it's free, it looks like this:

   chunk-> +-                                                             -+
           | User payload (must be in use, or we would have merged!)       |
           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |P|
         | Size of this chunk                                         0| +-+
   mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         | Next pointer                                                  |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         | Prev pointer                                                  |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         |                                                               :
         +-      size - sizeof(struct chunk) unused bytes               -+
         :                                                               |
 chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         | Size of this chunk                                            |
         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |0|
       | Size of next chunk (must be in use, or we would have merged)| +-+
 mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |                                                               :
       +- User payload                                                -+
       :                                                               |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                                                     |0|
                                                                     +-+
  Note that since we always merge adjacent free chunks, the chunks
  adjacent to a free chunk must be in use.

  Given a pointer to a chunk (which can be derived trivially from the
  payload pointer) we can, in O(1) time, find out whether the adjacent
  chunks are free, and if so, unlink them from the lists that they
  are on and merge them with the current chunk.

  Chunks always begin on even word boundaries, so the mem portion
  (which is returned to the user) is also on an even word boundary, and
  thus at least double-word aligned.

  The P (PINUSE_BIT) bit, stored in the unused low-order bit of the
  chunk size (which is always a multiple of two words), is an in-use
  bit for the *previous* chunk.  If that bit is *clear*, then the
  word before the current chunk size contains the previous chunk
  size, and can be used to find the front of the previous chunk.
  The very first chunk allocated always has this bit set, preventing
  access to non-existent (or non-owned) memory. If pinuse is set for
  any given chunk, then you CANNOT determine the size of the
  previous chunk, and might even get a memory addressing fault when
  trying to do so.

  The C (CINUSE_BIT) bit, stored in the unused second-lowest bit of
  the chunk size redundantly records whether the current chunk is
  inuse (unless the chunk is mmapped). This redundancy enables usage
  checks within free and realloc, and reduces indirection when freeing
  and consolidating chunks.

  Each freshly allocated chunk must have both cinuse and pinuse set.
  That is, each allocated chunk borders either a previously allocated
  and still in-use chunk, or the base of its memory arena. This is
  ensured by making all allocations from the `lowest' part of any
  found chunk.  Further, no free chunk physically borders another one,
  so each free chunk is known to be preceded and followed by either
  inuse chunks or the ends of memory.

  Note that the `foot' of the current chunk is actually represented
  as the prev_foot of the NEXT chunk. This makes it easier to
  deal with alignments etc but can be very confusing when trying
  to extend or adapt this code.

  The exceptions to all this are

     1. The special chunk `top' is the top-most available chunk (i.e.,
        the one bordering the end of available memory). It is treated
        specially.  Top is never included in any bin, is used only if
        no other chunk is available, and is released back to the
        system if it is very large (see M_TRIM_THRESHOLD).  In effect,
        the top chunk is treated as larger (and thus less well
        fitting) than any other available chunk.  The top chunk
        doesn't update its trailing size field since there is no next
        contiguous chunk that would have to index off it. However,
        space is still allocated for it (TOP_FOOT_SIZE) to enable
        separation or merging when space is extended.

     3. Chunks allocated via mmap, have both cinuse and pinuse bits
        cleared in their head fields.  Because they are allocated
        one-by-one, each must carry its own prev_foot field, which is
        also used to hold the offset this chunk has within its mmapped
        region, which is needed to preserve alignment. Each mmapped
        chunk is trailed by the first two fields of a fake next-chunk
        for sake of usage checks.

*/

నిర్మాణం malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               తల;       /* Size and inuse bits. */
  నిర్మాణం malloc_chunk* fd;         /* double links -- used only if free. */
  నిర్మాణం malloc_chunk* bk;
};

రకనిర్వచనం నిర్మాణం malloc_chunk  mchunk;
రకనిర్వచనం నిర్మాణం malloc_chunk* mchunkptr;
రకనిర్వచనం నిర్మాణం malloc_chunk* sbinptr;  /* The type of bins of chunks */
రకనిర్వచనం గుర్తులేని పూర్ణాంకం bindex_t;         /* Described below */
రకనిర్వచనం గుర్తులేని పూర్ణాంకం binmap_t;         /* Described below */
రకనిర్వచనం గుర్తులేని పూర్ణాంకం flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#నిర్వచించు MCHUNK_కొలత         (పరిమాణం(mchunk))

#ఒకవేళ FOOTERS
#నిర్వచించు CHUNK_OVERHEAD      (TWO_కొలత_T_SIZES)
#లేకపోతే /* FOOTERS */
#నిర్వచించు CHUNK_OVERHEAD      (కొలత_T_కొలత)
#అయితే_ముగింపు /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#నిర్వచించు MMAP_CHUNK_OVERHEAD (TWO_కొలత_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#నిర్వచించు MMAP_FOOT_PAD       (FOUR_కొలత_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#నిర్వచించు MIN_CHUNK_కొలత\
  ((MCHUNK_కొలత + CHUNK_ALIGN_ముసుగు) & ~CHUNK_ALIGN_ముసుగు)

/* conversion from malloc headers to user pointers, and back */
#నిర్వచించు chunk2mem(p)        ((శూన్య*)((అక్షరం*)(p)       + TWO_కొలత_T_SIZES))
#నిర్వచించు mem2chunk(mem)      ((mchunkptr)((అక్షరం*)(mem) - TWO_కొలత_T_SIZES))
/* chunk associated with aligned address A */
#నిర్వచించు align_as_chunk(A)   (mchunkptr)((A) + align_స్థానభ్రంశం(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#నిర్వచించు MAX_REQUEST         ((-MIN_CHUNK_కొలత) << 2)
#నిర్వచించు MIN_REQUEST         (MIN_CHUNK_కొలత - CHUNK_OVERHEAD - కొలత_T_ONE)

/* pad request bytes into a usable size */
#నిర్వచించు pad_request(req) \
   (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_ముసుగు) & ~CHUNK_ALIGN_ముసుగు)

/* pad request, checking for minimum (but not maximum) */
#నిర్వచించు request2size(req) \
  (((req) < MIN_REQUEST)? MIN_CHUNK_కొలత : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#నిర్వచించు PINUSE_బిట్టు          (కొలత_T_ONE)
#నిర్వచించు CINUSE_బిట్టు          (కొలత_T_TWO)
#నిర్వచించు FLAG4_బిట్టు           (కొలత_T_FOUR)
#నిర్వచించు INUSE_బిట్లు          (PINUSE_బిట్టు|CINUSE_బిట్టు)
#నిర్వచించు FLAG_బిట్లు           (PINUSE_బిట్టు|CINUSE_బిట్టు|FLAG4_బిట్టు)

/* Head value for fenceposts */
#నిర్వచించు FENCEPOST_తల      (INUSE_బిట్లు|కొలత_T_కొలత)

/* extraction of fields from head words */
#నిర్వచించు cinuse(p)           ((p)->తల & CINUSE_బిట్టు)
#నిర్వచించు pinuse(p)           ((p)->తల & PINUSE_బిట్టు)
#నిర్వచించు flag4inuse(p)       ((p)->తల & FLAG4_బిట్టు)
#నిర్వచించు is_inuse(p)         (((p)->తల & INUSE_బిట్లు) != PINUSE_బిట్టు)
#నిర్వచించు is_mmapped(p)       (((p)->తల & INUSE_బిట్లు) == 0)

#నిర్వచించు chunksize(p)        ((p)->తల & ~(FLAG_బిట్లు))

#నిర్వచించు clear_pinuse(p)     ((p)->తల &= ~PINUSE_బిట్టు)
#నిర్వచించు సమితి_flag4(p)        ((p)->తల |= FLAG4_బిట్టు)
#నిర్వచించు clear_flag4(p)      ((p)->తల &= ~FLAG4_బిట్టు)

/* Treat space at ptr +/- offset as a chunk */
#నిర్వచించు chunk_plus_స్థానభ్రంశం(p, s)  ((mchunkptr)(((అక్షరం*)(p)) + (s)))
#నిర్వచించు chunk_minus_స్థానభ్రంశం(p, s) ((mchunkptr)(((అక్షరం*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#నిర్వచించు తదుపరి_chunk(p) ((mchunkptr)( ((అక్షరం*)(p)) + ((p)->తల & ~FLAG_బిట్లు)))
#నిర్వచించు prev_chunk(p) ((mchunkptr)( ((అక్షరం*)(p)) - ((p)->prev_foot) ))

/* extract next chunk's pinuse bit */
#నిర్వచించు తదుపరి_pinuse(p)  ((తదుపరి_chunk(p)->తల) & PINUSE_బిట్టు)

/* Get/set size at footer */
#నిర్వచించు get_foot(p, s)  (((mchunkptr)((అక్షరం*)(p) + (s)))->prev_foot)
#నిర్వచించు సమితి_foot(p, s)  (((mchunkptr)((అక్షరం*)(p) + (s)))->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#నిర్వచించు సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, s)\
  ((p)->తల = (s|PINUSE_బిట్టు), సమితి_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#నిర్వచించు సమితి_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#నిర్వచించు overhead_for(p)\
 (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)

/* Return true if malloced space is not necessarily cleared */
#ఒకవేళ MMAP_CLEARS
#నిర్వచించు calloc_must_clear(p) (!is_mmapped(p))
#లేకపోతే /* MMAP_CLEARS */
#నిర్వచించు calloc_must_clear(p) (1)
#అయితే_ముగింపు /* MMAP_CLEARS */

/* ---------------------- Overlaid data structures ----------------------- */

/*
  When chunks are not in use, they are treated as nodes of either
  lists or trees.

  "Small"  chunks are stored in circular doubly-linked lists, and look
  like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk in list             |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk in list            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space (may be 0 bytes long)                .
            .                                                               .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Larger chunks are kept in a form of bitwise digital trees (aka
  tries) keyed on chunksizes.  Because malloc_tree_chunks are only for
  free chunks greater than 256 bytes, their size doesn't impose any
  constraints on user chunk sizes.  Each node looks like:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk of same size        |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk of same size       |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Pointer to left child (child[0])                  |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Pointer to right child (child[1])                 |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Pointer to parent                                 |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             bin index of this chunk                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space                                      .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Each tree holding treenodes is a tree of unique chunk sizes.  Chunks
  of the same size are arranged in a circularly-linked list, with only
  the oldest chunk (the next to be used, in our FIFO ordering)
  actually in the tree.  (Tree members are distinguished by a non-null
  parent pointer.)  If a chunk with the same size an an existing node
  is inserted, it is linked off the existing node using pointers that
  work in the same way as fd/bk pointers of small chunks.

  Each tree contains a power of 2 sized range of chunk sizes (the
  smallest is 0x100 <= x < 0x180), which is is divided in half at each
  tree level, with the chunks in the smaller half of the range (0x100
  <= x < 0x140 for the top nose) in the left subtree and the larger
  half (0x140 <= x < 0x180) in the right subtree.  This is, of course,
  done by inspecting individual bits.

  Using these rules, each node's left subtree contains all smaller
  sizes than its right subtree.  However, the node at the root of each
  subtree has no particular ordering relationship to either.  (The
  dividing line between the subtree sizes is based on trie relation.)
  If we remove the last chunk of a given size from the interior of the
  tree, we need to replace it with a leaf node.  The tree ordering
  rules permit a node to be replaced by any leaf below it.

  The smallest chunk in a tree (a common operation in a best-fit
  allocator) can be found by walking a path to the leftmost leaf in
  the tree.  Unlike a usual binary tree, where we follow left child
  pointers until we reach a null, here we follow the right child
  pointer any time the left one is null, until we reach a leaf with
  both child pointers null. The smallest chunk in the tree will be
  somewhere along that path.

  The worst case number of steps to add, find, or remove a node is
  bounded by the number of bits differentiating chunks within
  bins. Under current bin calculations, this ranges from 6 up to 21
  (for 32 bit sizes) or up to 53 (for 64 bit sizes). The typical case
  is of course much better.
*/

నిర్మాణం malloc_వృక్షం_chunk {
  /* The first four fields must be compatible with malloc_chunk */
  size_t                    prev_foot;
  size_t                    తల;
  నిర్మాణం malloc_వృక్షం_chunk* fd;
  నిర్మాణం malloc_వృక్షం_chunk* bk;

  నిర్మాణం malloc_వృక్షం_chunk* బిడ్డ[2];
  నిర్మాణం malloc_వృక్షం_chunk* మాతృ;
  bindex_t                  సూచీ;
};

రకనిర్వచనం నిర్మాణం malloc_వృక్షం_chunk  tchunk;
రకనిర్వచనం నిర్మాణం malloc_వృక్షం_chunk* tchunkptr;
రకనిర్వచనం నిర్మాణం malloc_వృక్షం_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#నిర్వచించు leftmost_బిడ్డ(t) ((t)->బిడ్డ[0] != 0? (t)->బిడ్డ[0] : (t)->బిడ్డ[1])

/* ----------------------------- Segments -------------------------------- */

/*
  Each malloc space may include non-contiguous segments, held in a
  list headed by an embedded malloc_segment record representing the
  top-most space. Segments also include flags holding properties of
  the space. Large chunks that are directly allocated by mmap are not
  included in this list. They are instead independently created and
  destroyed without otherwise keeping track of them.

  Segment management mainly comes into play for spaces allocated by
  MMAP.  Any call to MMAP might or might not return memory that is
  adjacent to an existing segment.  MORECORE normally contiguously
  extends the current space, so this space is almost always adjacent,
  which is simpler and faster to deal with. (This is why MORECORE is
  used preferentially to MMAP when both are available -- see
  sys_alloc.)  When allocating using MMAP, we don't use any of the
  hinting mechanisms (inconsistently) supported in various
  implementations of unix mmap, or distinguish reserving from
  committing memory. Instead, we just ask for space, and exploit
  contiguity when we get it.  It is probably possible to do
  better than this on some systems, but no general scheme seems
  to be significantly better.

  Management entails a simpler variant of the consolidation scheme
  used for chunks to reduce fragmentation -- new adjacent memory is
  normally prepended or appended to an existing segment. However,
  there are limitations compared to chunk consolidation that mostly
  reflect the fact that segment processing is relatively infrequent
  (occurring only when getting memory from system) and that we
  don't expect to have huge numbers of segments:

  * Segments are not indexed, so traversal requires linear scans.  (It
    would be possible to index these, but is not worth the extra
    overhead and complexity for most programs on most platforms.)
  * New segments are only appended to old ones when holding top-most
    memory; if they cannot be prepended to others, they are held in
    different segments.

  Except for the top-most segment of an mstate, each segment record
  is kept at the tail of its segment. Segments are added by pushing
  segment records onto the list headed by &mstate.seg for the
  containing mstate.

  Segment flags control allocation/merge/deallocation policies:
  * If EXTERN_BIT set, then we did not allocate this segment,
    and so should not try to deallocate or merge with others.
    (This currently holds only for the initial segment passed
    into create_mspace_with_base.)
  * If USE_MMAP_BIT set, the segment may be merged with
    other surrounding mmapped segments and trimmed/de-allocated
    using munmap.
  * If neither bit is set, then the segment was obtained using
    MORECORE so can be merged with surrounding MORECORE'd segments
    and deallocated/trimmed using MORECORE with negative arguments.
*/

నిర్మాణం malloc_segment {
  అక్షరం*        పునాది;             /* base address */
  size_t       కొలత;             /* allocated size */
  నిర్మాణం malloc_segment* తదుపరి;   /* ptr to next segment */
#ఒకవేళ FFI_MMAP_EXEC_WRIT
  /* The mmap magic is supposed to store the address of the executable
     segment at the very end of the requested block.  */

# define mmap_exec_స్థానభ్రంశం(b,s) (*(ptrdiff_t*)((b)+(s)-పరిమాణం(ptrdiff_t)))

  /* We can only merge segments if their corresponding executable
     segments are at identical offsets.  */
# define check_segment_విలీనం(S,b,s) \
  (mmap_exec_స్థానభ్రంశం((b),(s)) == (S)->exec_స్థానభ్రంశం)

# define కూడు_segment_exec_స్థానభ్రంశం(p,S) ((అక్షరం*)(p) + (S)->exec_స్థానభ్రంశం)
# define sub_segment_exec_స్థానభ్రంశం(p,S) ((అక్షరం*)(p) - (S)->exec_స్థానభ్రంశం)

  /* The removal of sflags only works with HAVE_MORECORE == 0.  */

# define get_segment_flags(S)   (USE_MMAP_బిట్టు)
# define సమితి_segment_flags(S,v) \
  (((v) != USE_MMAP_బిట్టు) ? (ABORT, (v)) :				\
   (((S)->exec_స్థానభ్రంశం =							\
     mmap_exec_స్థానభ్రంశం((S)->పునాది, (S)->కొలత)),				\
    (mmap_exec_స్థానభ్రంశం((S)->పునాది + (S)->exec_స్థానభ్రంశం, (S)->కొలత) !=	\
     (S)->exec_స్థానభ్రంశం) ? (ABORT, (v)) :					\
   (mmap_exec_స్థానభ్రంశం((S)->పునాది, (S)->కొలత) = 0), (v)))

  /* We use an offset here, instead of a pointer, because then, when
     base changes, we don't have to modify this.  On architectures
     with segmented addresses, this might not work.  */
  ptrdiff_t    exec_స్థానభ్రంశం;
#లేకపోతే

# define get_segment_flags(S)   ((S)->sflags)
# define సమితి_segment_flags(S,v) ((S)->sflags = (v))
# define check_segment_విలీనం(S,b,s) (1)

  flag_t       sflags;           /* mmap and extern flag */
#అయితే_ముగింపు
};

#నిర్వచించు is_mmapped_segment(S)  (get_segment_flags(S) & USE_MMAP_బిట్టు)
#నిర్వచించు is_extern_segment(S)   (get_segment_flags(S) & EXTERN_బిట్టు)

రకనిర్వచనం నిర్మాణం malloc_segment  msegment;
రకనిర్వచనం నిర్మాణం malloc_segment* msegmentptr;

/* ---------------------------- malloc_state ----------------------------- */

/*
   A malloc_state holds all of the bookkeeping for a space.
   The main fields are:

శిఖరం
    The topmost chunk of the currently active segment. Its size is
    cached in topsize.  The actual size of topmost space is
    topsize+TOP_FOOT_SIZE, which includes space reserved for adding
    fenceposts and segment records if necessary when getting more
    space from the system.  The size at which to autotrim top is
    cached from mparams in trim_check, except that it is disabled if
    an autotrim fails.

  Designated victim (dv)
    This is the preferred chunk for servicing small requests that
    don't have exact fits.  It is normally the chunk split off most
    recently to service another small request.  Its size is cached in
    dvsize. The link fields of this chunk are not maintained since it
    is not kept in a bin.

  SmallBins
    An array of bin headers for free chunks.  These bins hold chunks
    with sizes less than MIN_LARGE_SIZE bytes. Each bin contains
    chunks of all the same size, spaced 8 bytes apart.  To simplify
    use in double-linked lists, each bin header acts as a malloc_chunk
    pointing to the real first node, if it exists (else pointing to
    itself).  This avoids special-casing for headers.  But to avoid
    waste, we allocate only the fd/bk pointers of bins, and then use
    repositioning tricks to treat these as the fields of a chunk.

  TreeBins
    Treebins are pointers to the roots of trees holding a range of
    sizes. There are 2 equally spaced treebins for each power of two
    from TREE_SHIFT to TREE_SHIFT+16. The last bin holds anything
    larger.

  Bin maps
    There is one bit map for small bins ("smallmap") and one for
    treebins ("treemap).  Each bin sets its bit when non-empty, and
    clears the bit when empty.  Bit operations are then used to avoid
    bin-by-bin searching -- nearly all "search" is done without ever
    looking at bins that won't be selected.  The bit maps
    conservatively use 32 bits per map word, even if on 64bit system.
    For a good description of some of the bit-based techniques used
    here, see Henry S. Warren Jr's book "Hacker's Delight" (and
    supplement at http://hackersdelight.org/). Many of these are
    intended to reduce the branchiness of paths through malloc etc, as
    well as to reduce the number of memory locations read or written.

  Segments
    A list of segments headed by an embedded malloc_segment record
    representing the initial space.

  Address check support
    The least_addr field is the least address ever obtained from
    MORECORE or MMAP. Attempted frees and reallocs of any address less
    than this are trapped (unless INSECURE is defined).

  Magic tag
    A cross-check field that should always hold same value as mparams.magic.

  Max allowed footprint
    The maximum allowed bytes to allocate from system (zero means no limit)

  Flags
    Bits recording whether to use MMAP, locks, or contiguous MORECORE

  Statistics
    Each space keeps track of current and maximum system memory
    obtained via MORECORE or MMAP.

  Trim support
    Fields holding the amount of unused topmost memory that should trigger
    trimming, and a counter to force periodic scanning to release unused
    non-topmost segments.

  Locking
    If USE_LOCKS is defined, the "mutex" lock is acquired and released
    around every public call using this mspace.

  Extension support
    A void* pointer and a size_t field that can be used to help implement
    extensions to this malloc.
*/

/* Bin types, widths and sizes */
#నిర్వచించు NSMALLBINS        (32U)
#నిర్వచించు NTREEBINS         (32U)
#నిర్వచించు SMALLBIN_జరుపు    (3U)
#నిర్వచించు SMALLBIN_వెడల్పు    (కొలత_T_ONE << SMALLBIN_జరుపు)
#నిర్వచించు TREEBIN_జరుపు     (8U)
#నిర్వచించు MIN_LARGE_కొలత    (కొలత_T_ONE << TREEBIN_జరుపు)
#నిర్వచించు MAX_SMALL_కొలత    (MIN_LARGE_కొలత - కొలత_T_ONE)
#నిర్వచించు MAX_SMALL_REQUEST (MAX_SMALL_కొలత - CHUNK_ALIGN_ముసుగు - CHUNK_OVERHEAD)

నిర్మాణం malloc_state {
  binmap_t   smallmap;
  binmap_t   treemap;
  size_t     dvsize;
  size_t     topsize;
  అక్షరం*      కనిష్ఠ_addr;
  mchunkptr  dv;
  mchunkptr  శిఖరం;
  size_t     trim_check;
  size_t     release_checks;
  size_t     magic;
  mchunkptr  smallbins[(NSMALLBINS+1)*2];
  tbinptr    treebins[NTREEBINS];
  size_t     footprint;
  size_t     max_footprint;
  size_t     footprint_హద్దు; /* zero means no limit */
  flag_t     mflags;
#ఒకవేళ USE_LOCKS
  MLOCK_T    mutex;     /* locate lock among fields that rarely change */
#అయితే_ముగింపు /* USE_LOCKS */
  msegment   seg;
  శూన్య*      extp;      /* Unused but available for extensions */
  size_t     exts;
};

రకనిర్వచనం నిర్మాణం malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

నిర్మాణం malloc_params {
  size_t magic;
  size_t పుట_కొలత;
  size_t granularity;
  size_t mmap_పరిమితి;
  size_t trim_పరిమితి;
  flag_t default_mflags;
};

స్థిర నిర్మాణం malloc_params mparams;

/* libffi: mparams is initialized exactly once (under the malloc global lock),
   but mparams.magic is read on every allocation's fast path WITHOUT a lock to
   decide whether initialization already happened.  Upstream writes magic
   through a `volatile` cast, which is not a synchronizing operation, so
   ThreadSanitizer reports a data race (libffi issue #873) between that one-time
   write and the lock-free readers.  Access magic with acquire/release ordering
   instead: a thread that observes magic != 0 via the acquire load is then
   guaranteed to see all the other initialization writes (page_size, mflags,
   ...) performed before the release store.  Fall back to the original volatile
   access when no supported compiler atomic operations are available.  */
#ఒకవేళ USE_LOCKS && defined(__ATOMIC_ACQUIRE)
# define ffi_mparams_magic_ఎక్కించు()   __atomic_load_n(&mparams.magic, __ATOMIC_ACQUIRE)
# define ffi_mparams_magic_store(v) __atomic_store_n(&mparams.magic, (size_t)(v), __ATOMIC_RELEASE)
#లేదావేళ USE_LOCKS && defined(_MSC_VER)
# ఒకవేళ defined(_WIN64)
#  define ffi_mparams_magic_ఎక్కించు() \
  ((size_t)_InterlockedCompareExchange64((__int64 అస్థిర *)&mparams.magic, \
                                         (__int64)0, (__int64)0))
#  define ffi_mparams_magic_store(v) \
  ((శూన్య)_InterlockedExchange64((__int64 అస్థిర *)&mparams.magic, \
                                (__int64)(v)))
# లేకపోతే
#  define ffi_mparams_magic_ఎక్కించు() \
  ((size_t)_InterlockedCompareExchange((LONG అస్థిర *)&mparams.magic, \
                                       (LONG)0, (LONG)0))
#  define ffi_mparams_magic_store(v) \
  ((శూన్య)_InterlockedExchange((LONG అస్థిర *)&mparams.magic, (LONG)(v)))
# endif
#లేకపోతే
# define ffi_mparams_magic_ఎక్కించు()   (mparams.magic)
# define ffi_mparams_magic_store(v) (*(అస్థిర size_t *)(&(mparams.magic)) = (size_t)(v))
#అయితే_ముగింపు

/* Ensure mparams initialized */
#నిర్వచించు ensure_initialization() (శూన్య)(ffi_mparams_magic_ఎక్కించు() != 0 || init_mparams())

#ఒకవేళ !ONLY_MSPACES

/* The global malloc_state used for all non-"mspace" calls */
స్థిర నిర్మాణం malloc_state _gm_;
#నిర్వచించు gm                 (&_gm_)
#నిర్వచించు is_global(M)       ((M) == &_gm_)

#అయితే_ముగింపు /* !ONLY_MSPACES */

#నిర్వచించు is_initialized(M)  ((M)->శిఖరం != 0)

/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#నిర్వచించు use_తాళం(M)           ((M)->mflags &   USE_తాళం_బిట్టు)
#నిర్వచించు enable_తాళం(M)        ((M)->mflags |=  USE_తాళం_బిట్టు)
#ఒకవేళ USE_LOCKS
#నిర్వచించు disable_తాళం(M)       ((M)->mflags &= ~USE_తాళం_బిట్టు)
#లేకపోతే
#నిర్వచించు disable_తాళం(M)
#అయితే_ముగింపు

#నిర్వచించు use_mmap(M)           ((M)->mflags &   USE_MMAP_బిట్టు)
#నిర్వచించు enable_mmap(M)        ((M)->mflags |=  USE_MMAP_బిట్టు)
#ఒకవేళ HAVE_MMAP
#నిర్వచించు disable_mmap(M)       ((M)->mflags &= ~USE_MMAP_బిట్టు)
#లేకపోతే
#నిర్వచించు disable_mmap(M)
#అయితే_ముగింపు

#నిర్వచించు use_noncontiguous(M)  ((M)->mflags &   USE_NONCONTIGUOUS_బిట్టు)
#నిర్వచించు disable_contiguous(M) ((M)->mflags |=  USE_NONCONTIGUOUS_బిట్టు)

#నిర్వచించు సమితి_తాళం(M,L)\
 ((M)->mflags = (L)?\
  ((M)->mflags | USE_తాళం_బిట్టు) :\
  ((M)->mflags & ~USE_తాళం_బిట్టు))

/* page-align a size */
#నిర్వచించు పుట_align(S)\
 (((S) + (mparams.పుట_కొలత - కొలత_T_ONE)) & ~(mparams.పుట_కొలత - కొలత_T_ONE))

/* granularity-align a size */
#నిర్వచించు granularity_align(S)\
  (((S) + (mparams.granularity - కొలత_T_ONE))\
   & ~(mparams.granularity - కొలత_T_ONE))


/* For mmap, use granularity alignment on windows, else page-align */
#నిర్వచితమైతే WIN32
#నిర్వచించు mmap_align(S) granularity_align(S)
#లేకపోతే
#నిర్వచించు mmap_align(S) పుట_align(S)
#అయితే_ముగింపు

/* For sys_alloc, enough padding to ensure can malloc request on success */
#నిర్వచించు SYS_ALLOC_PADDING (శిఖరం_FOOT_కొలత + MALLOC_ALIGNMENT)

#నిర్వచించు is_పుట_aligned(S)\
   (((size_t)(S) & (mparams.పుట_కొలత - కొలత_T_ONE)) == 0)
#నిర్వచించు is_granularity_aligned(S)\
   (((size_t)(S) & (mparams.granularity - కొలత_T_ONE)) == 0)

/*  True if segment S holds address A */
#నిర్వచించు segment_holds(S, A)\
  ((అక్షరం*)(A) >= S->పునాది && (అక్షరం*)(A) < S->పునాది + S->కొలత)

/* Return segment holding given address */
స్థిర msegmentptr segment_holding(mstate m, అక్షరం* addr) {
  msegmentptr sp = &m->seg;
  కొరకు (;;) {
    ఒకవేళ (addr >= sp->పునాది && addr < sp->పునాది + sp->కొలత)
      తిరిగి sp;
    ఒకవేళ ((sp = sp->తదుపరి) == 0)
      తిరిగి 0;
  }
}

/* Return true if segment contains a segment link */
స్థిర పూర్ణాంకం has_segment_link(mstate m, msegmentptr ss) {
  msegmentptr sp = &m->seg;
  కొరకు (;;) {
    ఒకవేళ ((అక్షరం*)sp >= ss->పునాది && (అక్షరం*)sp < ss->పునాది + ss->కొలత)
      తిరిగి 1;
    ఒకవేళ ((sp = sp->తదుపరి) == 0)
      తిరిగి 0;
  }
}

#నిర్వచితం_కాకపోతే MORECORE_CANNOT_TRIM
#నిర్వచించు should_trim(M,s)  ((s) > (M)->trim_check)
#లేకపోతే  /* MORECORE_CANNOT_TRIM */
#నిర్వచించు should_trim(M,s)  (0)
#అయితే_ముగింపు /* MORECORE_CANNOT_TRIM */

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#నిర్వచించు శిఖరం_FOOT_కొలత\
  (align_స్థానభ్రంశం(chunk2mem(0))+pad_request(పరిమాణం(నిర్మాణం malloc_segment))+MIN_CHUNK_కొలత)


/* -------------------------------  Hooks -------------------------------- */

/*
  PREACTION should be defined to return 0 on success, and nonzero on
  failure. If you are not using locking, you can redefine these to do
  anything you like.
*/

#ఒకవేళ USE_LOCKS
#నిర్వచించు PREACTION(M)  ((use_తాళం(M))? ACQUIRE_తాళం(&(M)->mutex) : 0)
#నిర్వచించు POSTACTION(M) { ఒకవేళ (use_తాళం(M)) RELEASE_తాళం(&(M)->mutex); }
#లేకపోతే /* USE_LOCKS */

#నిర్వచితం_కాకపోతే PREACTION
#నిర్వచించు PREACTION(M) (0)
#అయితే_ముగింపు  /* PREACTION */

#నిర్వచితం_కాకపోతే POSTACTION
#నిర్వచించు POSTACTION(M)
#అయితే_ముగింపు  /* POSTACTION */

#అయితే_ముగింపు /* USE_LOCKS */

/*
  CORRUPTION_ERROR_ACTION is triggered upon detected bad addresses.
  USAGE_ERROR_ACTION is triggered on detected bad frees and
  reallocs. The argument p is an address that might have triggered the
  fault. It is ignored by the two predefined actions, but might be
  useful in custom actions that try to help diagnose errors.
*/

#ఒకవేళ PROCEED_ON_ERROR

/* A count of the number of corruption errors causing resets */
పూర్ణాంకం malloc_corruption_error_లెక్క;

/* default corruption action */
స్థిర శూన్య reset_on_error(mstate m);

#నిర్వచించు CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#నిర్వచించు USAGE_ERROR_ACTION(m, p)

#లేకపోతే /* PROCEED_ON_ERROR */

#నిర్వచితం_కాకపోతే CORRUPTION_ERROR_ACTION
#నిర్వచించు CORRUPTION_ERROR_ACTION(m) ABORT
#అయితే_ముగింపు /* CORRUPTION_ERROR_ACTION */

#నిర్వచితం_కాకపోతే USAGE_ERROR_ACTION
#నిర్వచించు USAGE_ERROR_ACTION(m,p) ABORT
#అయితే_ముగింపు /* USAGE_ERROR_ACTION */

#అయితే_ముగింపు /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#ఒకవేళ ! DEBUG

#నిర్వచించు check_free_chunk(M,P)
#నిర్వచించు check_inuse_chunk(M,P)
#నిర్వచించు check_malloced_chunk(M,P,N)
#నిర్వచించు check_mmapped_chunk(M,P)
#నిర్వచించు check_malloc_state(M)
#నిర్వచించు check_శిఖరం_chunk(M,P)

#లేకపోతే /* DEBUG */
#నిర్వచించు check_free_chunk(M,P)       do_check_free_chunk(M,P)
#నిర్వచించు check_inuse_chunk(M,P)      do_check_inuse_chunk(M,P)
#నిర్వచించు check_శిఖరం_chunk(M,P)        do_check_శిఖరం_chunk(M,P)
#నిర్వచించు check_malloced_chunk(M,P,N) do_check_malloced_chunk(M,P,N)
#నిర్వచించు check_mmapped_chunk(M,P)    do_check_mmapped_chunk(M,P)
#నిర్వచించు check_malloc_state(M)       do_check_malloc_state(M)

స్థిర శూన్య   do_check_any_chunk(mstate m, mchunkptr p);
స్థిర శూన్య   do_check_శిఖరం_chunk(mstate m, mchunkptr p);
స్థిర శూన్య   do_check_mmapped_chunk(mstate m, mchunkptr p);
స్థిర శూన్య   do_check_inuse_chunk(mstate m, mchunkptr p);
స్థిర శూన్య   do_check_free_chunk(mstate m, mchunkptr p);
స్థిర శూన్య   do_check_malloced_chunk(mstate m, శూన్య* mem, size_t s);
స్థిర శూన్య   do_check_వృక్షం(mstate m, tchunkptr t);
స్థిర శూన్య   do_check_treebin(mstate m, bindex_t i);
స్థిర శూన్య   do_check_smallbin(mstate m, bindex_t i);
స్థిర శూన్య   do_check_malloc_state(mstate m);
స్థిర పూర్ణాంకం    bin_కనుగొను(mstate m, mchunkptr x);
స్థిర size_t traverse_and_check(mstate m);
#అయితే_ముగింపు /* DEBUG */

/* ---------------------------- Indexing Bins ---------------------------- */

#నిర్వచించు is_small(s)         (((s) >> SMALLBIN_జరుపు) < NSMALLBINS)
#నిర్వచించు small_సూచీ(s)      (bindex_t)((s)  >> SMALLBIN_జరుపు)
#నిర్వచించు small_index2size(i) ((i)  << SMALLBIN_జరుపు)
#నిర్వచించు MIN_SMALL_సూచీ     (small_సూచీ(MIN_CHUNK_కొలత))

/* addressing by index. See above about smallbin repositioning */
#నిర్వచించు smallbin_at(M, i)   ((sbinptr)((అక్షరం*)&((M)->smallbins[(i)<<1])))
#నిర్వచించు treebin_at(M,i)     (&((M)->treebins[i]))

/* assign tree index for size S to variable I.
   libffi: enable the __builtin_clz path on any GNU-compatible compiler, not
   only x86, so the log2 is an intrinsic rather than the generic C idiom that
   the compiler can't always recognize (libffi #754).  */
#ఒకవేళ defined(__GNUC__)
#నిర్వచించు compute_వృక్షం_సూచీ(S, I)\
{\
  గుర్తులేని పూర్ణాంకం X = S >> TREEBIN_జరుపు;\
  ఒకవేళ (X == 0)\
    I = 0;\
  లేకపోతే ఒకవేళ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  లేకపోతే {\
    గుర్తులేని పూర్ణాంకం K = (గుర్తులేని) పరిమాణం(X)*__CHAR_BIT__ - 1 - (గుర్తులేని) __builtin_clz(X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_జరుపు-1)) & 1)));\
  }\
}

#లేదావేళ defined (__INTEL_COMPILER)
#నిర్వచించు compute_వృక్షం_సూచీ(S, I)\
{\
  size_t X = S >> TREEBIN_జరుపు;\
  ఒకవేళ (X == 0)\
    I = 0;\
  లేకపోతే ఒకవేళ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  లేకపోతే {\
    గుర్తులేని పూర్ణాంకం K = బిట్టు_పరిశీలన_తిరగేయి_3 (X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_జరుపు-1)) & 1)));\
  }\
}

#లేదావేళ defined(_MSC_VER) && _MSC_VER>=1300
#నిర్వచించు compute_వృక్షం_సూచీ(S, I)\
{\
  size_t X = S >> TREEBIN_జరుపు;\
  ఒకవేళ (X == 0)\
    I = 0;\
  లేకపోతే ఒకవేళ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  లేకపోతే {\
    గుర్తులేని పూర్ణాంకం K;\
    బిట్టు_పరిశీలన_తిరగేయి((DWORD *) &K, (DWORD) X);\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_జరుపు-1)) & 1)));\
  }\
}

#లేకపోతే /* GNUC */
#నిర్వచించు compute_వృక్షం_సూచీ(S, I)\
{\
  size_t X = S >> TREEBIN_జరుపు;\
  ఒకవేళ (X == 0)\
    I = 0;\
  లేకపోతే ఒకవేళ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  లేకపోతే {\
    గుర్తులేని పూర్ణాంకం Y = (గుర్తులేని పూర్ణాంకం)X;\
    గుర్తులేని పూర్ణాంకం N = ((Y - 0x100) >> 16) & 8;\
    గుర్తులేని పూర్ణాంకం K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_జరుపు-1)) & 1));\
  }\
}
#అయితే_ముగింపు /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#నిర్వచించు బిట్టు_for_వృక్షం_సూచీ(i) \
   (i == NTREEBINS-1)? (కొలత_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_జరుపు - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#నిర్వచించు leftshift_for_వృక్షం_సూచీ(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((కొలత_T_BITSIZE-కొలత_T_ONE) - (((i) >> 1) + TREEBIN_జరుపు - 2)))

/* The size of the smallest chunk held in bin with index i */
#నిర్వచించు minsize_for_వృక్షం_సూచీ(i) \
   ((కొలత_T_ONE << (((i) >> 1) + TREEBIN_జరుపు)) |  \
   (((size_t)((i) & కొలత_T_ONE)) << (((i) >> 1) + TREEBIN_జరుపు - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#నిర్వచించు idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#నిర్వచించు mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#నిర్వచించు clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#నిర్వచించు smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#నిర్వచించు mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#నిర్వచించు clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#నిర్వచించు treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#నిర్వచించు కనిష్ఠ_బిట్టు(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#నిర్వచించు ఎడమ_బిట్లు(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#నిర్వచించు same_or_ఎడమ_బిట్లు(x) ((x) | -(x))

/* index corresponding to given bit.
   libffi: use __builtin_ctz on any GNU-compatible compiler, not only x86
   (libffi #754).  */

#ఒకవేళ defined(__GNUC__)
#నిర్వచించు compute_bit2idx(X, I)\
{\
  గుర్తులేని పూర్ణాంకం J;\
  J = __builtin_ctz(X); \
  I = (bindex_t)J;\
}

#లేదావేళ defined (__INTEL_COMPILER)
#నిర్వచించు compute_bit2idx(X, I)\
{\
  గుర్తులేని పూర్ణాంకం J;\
  J = బిట్టు_పరిశీలన_forward (X); \
  I = (bindex_t)J;\
}

#లేదావేళ defined(_MSC_VER) && _MSC_VER>=1300
#నిర్వచించు compute_bit2idx(X, I)\
{\
  గుర్తులేని పూర్ణాంకం J;\
  బిట్టు_పరిశీలన_Forward_2((DWORD *) &J, X);\
  I = (bindex_t)J;\
}

#లేదావేళ USE_BUILTIN_FFS
#నిర్వచించు compute_bit2idx(X, I) I = ffs(X)-1

#లేకపోతే
#నిర్వచించు compute_bit2idx(X, I)\
{\
  గుర్తులేని పూర్ణాంకం Y = X - 1;\
  గుర్తులేని పూర్ణాంకం K = Y >> (16-4) & 16;\
  గుర్తులేని పూర్ణాంకం N = K;        Y >>= K;\
  N += K = Y >> (8-3) &  8;  Y >>= K;\
  N += K = Y >> (4-2) &  4;  Y >>= K;\
  N += K = Y >> (2-1) &  2;  Y >>= K;\
  N += K = Y >> (1-0) &  1;  Y >>= K;\
  I = (bindex_t)(N + Y);\
}
#అయితే_ముగింపు /* GNUC */


/* ----------------------- Runtime Check Support ------------------------- */

/*
  For security, the main invariant is that malloc/free/etc never
  writes to a static address other than malloc_state, unless static
  malloc_state itself has been corrupted, which cannot occur via
  malloc (because of these checks). In essence this means that we
  believe all pointers, sizes, maps etc held in malloc_state, but
  check all of those linked or offsetted from other embedded data
  structures.  These checks are interspersed with main code in a way
  that tends to minimize their run-time cost.

  When FOOTERS is defined, in addition to range checking, we also
  verify footer fields of inuse chunks, which can be used guarantee
  that the mstate controlling malloc/free is intact.  This is a
  streamlined version of the approach described by William Robertson
  et al in "Run-time Detection of Heap-based Overflows" LISA'03
  http://www.usenix.org/events/lisa03/tech/robertson.html The footer
  of an inuse chunk holds the xor of its mstate and a random seed,
  that is checked upon calls to free() and realloc().  This is
  (probabalistically) unguessable from outside the program, but can be
  computed by any code successfully malloc'ing any chunk, so does not
  itself provide protection against code that has already broken
  security through some other means.  Unlike Robertson et al, we
  always dynamically check addresses of all offset chunks (previous,
  next, etc). This turns out to be cheaper than relying on hashes.
*/

#ఒకవేళ !INSECURE
/* Check if address a is at least as high as any from MORECORE or MMAP */
#నిర్వచించు ok_చిరునామా(M, a) ((అక్షరం*)(a) >= (M)->కనిష్ఠ_addr)
/* Check if address of next chunk n is higher than base chunk p */
#నిర్వచించు ok_తదుపరి(p, n)    ((అక్షరం*)(p) < (అక్షరం*)(n))
/* Check if p has inuse status */
#నిర్వచించు ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#నిర్వచించు ok_pinuse(p)     pinuse(p)

#లేకపోతే /* !INSECURE */
#నిర్వచించు ok_చిరునామా(M, a) (1)
#నిర్వచించు ok_తదుపరి(b, n)    (1)
#నిర్వచించు ok_inuse(p)      (1)
#నిర్వచించు ok_pinuse(p)     (1)
#అయితే_ముగింపు /* !INSECURE */

#ఒకవేళ (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#నిర్వచించు ok_magic(M)      ((M)->magic == mparams.magic)
#లేకపోతే  /* (FOOTERS && !INSECURE) */
#నిర్వచించు ok_magic(M)      (1)
#అయితే_ముగింపు /* (FOOTERS && !INSECURE) */

/* In gcc, use __builtin_expect to minimize impact of checks */
#ఒకవేళ !INSECURE
#ఒకవేళ defined(__GNUC__) && __GNUC__ >= 3
#నిర్వచించు RTCHECK(e)  __builtin_expect(e, 1)
#లేకపోతే /* GNUC */
#నిర్వచించు RTCHECK(e)  (e)
#అయితే_ముగింపు /* GNUC */
#లేకపోతే /* !INSECURE */
#నిర్వచించు RTCHECK(e)  (1)
#అయితే_ముగింపు /* !INSECURE */

/* macros to set up inuse chunks with or without footers */

#ఒకవేళ !FOOTERS

#నిర్వచించు mark_inuse_foot(M,p,s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#నిర్వచించు సమితి_inuse(M,p,s)\
  ((p)->తల = (((p)->తల & PINUSE_బిట్టు)|s|CINUSE_బిట్టు),\
  ((mchunkptr)(((అక్షరం*)(p)) + (s)))->తల |= PINUSE_బిట్టు)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#నిర్వచించు సమితి_inuse_and_pinuse(M,p,s)\
  ((p)->తల = (s|PINUSE_బిట్టు|CINUSE_బిట్టు),\
  ((mchunkptr)(((అక్షరం*)(p)) + (s)))->తల |= PINUSE_బిట్టు)

/* Set size, cinuse and pinuse bit of this chunk */
#నిర్వచించు సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(M, p, s)\
  ((p)->తల = (s|PINUSE_బిట్టు|CINUSE_బిట్టు))

#లేకపోతే /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#నిర్వచించు mark_inuse_foot(M,p,s)\
  (((mchunkptr)((అక్షరం*)(p) + (s)))->prev_foot = ((size_t)(M) ^ mparams.magic))

#నిర్వచించు get_mstate_for(p)\
  ((mstate)(((mchunkptr)((అక్షరం*)(p) +\
    (chunksize(p))))->prev_foot ^ mparams.magic))

#నిర్వచించు సమితి_inuse(M,p,s)\
  ((p)->తల = (((p)->తల & PINUSE_బిట్టు)|s|CINUSE_బిట్టు),\
  (((mchunkptr)(((అక్షరం*)(p)) + (s)))->తల |= PINUSE_బిట్టు), \
  mark_inuse_foot(M,p,s))

#నిర్వచించు సమితి_inuse_and_pinuse(M,p,s)\
  ((p)->తల = (s|PINUSE_బిట్టు|CINUSE_బిట్టు),\
  (((mchunkptr)(((అక్షరం*)(p)) + (s)))->తల |= PINUSE_బిట్టు),\
 mark_inuse_foot(M,p,s))

#నిర్వచించు సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(M, p, s)\
  ((p)->తల = (s|PINUSE_బిట్టు|CINUSE_బిట్టు),\
  mark_inuse_foot(M, p, s))

#అయితే_ముగింపు /* !FOOTERS */

/* ---------------------------- setting mparams -------------------------- */

#ఒకవేళ తాళం_AT_FORK
స్థిర శూన్య pre_fork(శూన్య)         { ACQUIRE_తాళం(&(gm)->mutex); }
స్థిర శూన్య post_fork_మాతృ(శూన్య) { RELEASE_తాళం(&(gm)->mutex); }
స్థిర శూన్య post_fork_బిడ్డ(శూన్య)  { INITIAL_తాళం(&(gm)->mutex); }
#అయితే_ముగింపు /* LOCK_AT_FORK */

/* Initialize mparams */
స్థిర పూర్ణాంకం init_mparams(శూన్య) {
#నిర్వచితమైతే NEED_GLOBAL_తాళం_INIT
  ఒకవేళ (malloc_global_mutex_స్థితి <= 0)
    init_malloc_global_mutex();
#అయితే_ముగింపు

  ACQUIRE_MALLOC_GLOBAL_తాళం();
  ఒకవేళ (mparams.magic == 0) {
    size_t magic;
    size_t psize;
    size_t gsize;

#ఒకవేళ !defined(WIN32) && !defined(__OS2__)
    psize = malloc_getpagesize;
    gsize = ((DEFAULT_GRANULARITY != 0)? DEFAULT_GRANULARITY : psize);
#లేదావేళ defined (__OS2__)
    /* if low-memory is used, os2munmap() would break
       if it were anything other than 64k */
    psize = 4096u;
    gsize = 65536u;
#లేకపోతే /* WIN32 */
    {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      psize = system_info.dw_పుట_కొలత;
      gsize = ((DEFAULT_GRANULARITY != 0)?
               DEFAULT_GRANULARITY : system_info.dw_కేటాయింపు_Granularity);
    }
#అయితే_ముగింపు /* WIN32 */

    /* Sanity-check configuration:
       size_t must be unsigned and as wide as pointer type.
       ints must be at least 4 bytes.
       alignment must be at least 8.
       Alignment, min chunk size, and page size must all be powers of 2.
    */
    ఒకవేళ ((పరిమాణం(size_t) != పరిమాణం(అక్షరం*)) ||
        (MAX_కొలత_T < MIN_CHUNK_కొలత)  ||
        (పరిమాణం(పూర్ణాంకం) < 4)  ||
        (MALLOC_ALIGNMENT < (size_t)8U) ||
        ((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-కొలత_T_ONE)) != 0) ||
        ((MCHUNK_కొలత      & (MCHUNK_కొలత-కొలత_T_ONE))      != 0) ||
        ((gsize            & (gsize-కొలత_T_ONE))            != 0) ||
        ((psize            & (psize-కొలత_T_ONE))            != 0))
      ABORT;
    mparams.granularity = gsize;
    mparams.పుట_కొలత = psize;
    mparams.mmap_పరిమితి = DEFAULT_MMAP_పరిమితి;
    mparams.trim_పరిమితి = DEFAULT_TRIM_పరిమితి;
#ఒకవేళ MORECORE_CONTIGUOUS
    mparams.default_mflags = USE_తాళం_బిట్టు|USE_MMAP_బిట్టు;
#లేకపోతే  /* MORECORE_CONTIGUOUS */
    mparams.default_mflags = USE_తాళం_బిట్టు|USE_MMAP_బిట్టు|USE_NONCONTIGUOUS_బిట్టు;
#అయితే_ముగింపు /* MORECORE_CONTIGUOUS */

#ఒకవేళ !ONLY_MSPACES
    /* Set up lock for main malloc area */
    gm->mflags = mparams.default_mflags;
    (శూన్య)INITIAL_తాళం(&gm->mutex);
#అయితే_ముగింపు
#ఒకవేళ తాళం_AT_FORK
    pthread_atfork(&pre_fork, &post_fork_మాతృ, &post_fork_బిడ్డ);
#అయితే_ముగింపు

    {
#ఒకవేళ USE_DEV_RANDOM
      పూర్ణాంకం fd;
      గుర్తులేని అక్షరం buf[పరిమాణం(size_t)];
      /* Try to use /dev/urandom, else fall back on using time */
      ఒకవేళ ((fd = open("/dev/urandom", O_RDONLY)) >= 0 &&
          పఠ(fd, buf, పరిమాణం(buf)) == పరిమాణం(buf)) {
        magic = *((size_t *) buf);
        close(fd);
      }
      లేకపోతే
#అయితే_ముగింపు /* USE_DEV_RANDOM */
#నిర్వచితమైతే WIN32
      magic = (size_t)(Get_Tick_లెక్క() ^ (size_t)0x55555555U);
#లేదావేళ defined(LACKS_సమయం_H)
      magic = (size_t)&magic ^ (size_t)0x55555555U;
#లేకపోతే
      magic = (size_t)(time(0) ^ (size_t)0x55555555U);
#అయితే_ముగింపు
      magic |= (size_t)8U;    /* ensure nonzero */
      magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
      /* libffi: release-store so any thread that observes magic via the
         acquire load in ensure_initialization() sees the writes above (#873).  */
      ffi_mparams_magic_store(magic);
    }
  }

  RELEASE_MALLOC_GLOBAL_తాళం();
  తిరిగి 1;
}

/* support for mallopt */
స్థిర పూర్ణాంకం change_mparam(పూర్ణాంకం param_సంఖ్య, పూర్ణాంకం విలువ) {
  size_t val;
  ensure_initialization();
  val = (విలువ == -1)? MAX_కొలత_T : (size_t)విలువ;
  ఎంపిక(param_సంఖ్య) {
  స్థితి M_TRIM_పరిమితి:
    mparams.trim_పరిమితి = val;
    తిరిగి 1;
  స్థితి M_GRANULARITY:
    ఒకవేళ (val >= mparams.పుట_కొలత && ((val & (val-1)) == 0)) {
      mparams.granularity = val;
      తిరిగి 1;
    }
    లేకపోతే
      తిరిగి 0;
  స్థితి M_MMAP_పరిమితి:
    mparams.mmap_పరిమితి = val;
    తిరిగి 1;
  అప్రమేయ:
    తిరిగి 0;
  }
}

#ఒకవేళ DEBUG
/* ------------------------- Debugging Support --------------------------- */

/* Check properties of any chunk, whether free, inuse, mmapped etc  */
స్థిర శూన్య do_check_any_chunk(mstate m, mchunkptr p) {
  ప్రతిజ్ఞా((is_aligned(chunk2mem(p))) || (p->తల == FENCEPOST_తల));
  ప్రతిజ్ఞా(ok_చిరునామా(m, p));
}

/* Check properties of top chunk */
స్థిర శూన్య do_check_శిఖరం_chunk(mstate m, mchunkptr p) {
  msegmentptr sp = segment_holding(m, (అక్షరం*)p);
  size_t  sz = p->తల & ~INUSE_బిట్లు; /* third-lowest bit can be set! */
  ప్రతిజ్ఞా(sp != 0);
  ప్రతిజ్ఞా((is_aligned(chunk2mem(p))) || (p->తల == FENCEPOST_తల));
  ప్రతిజ్ఞా(ok_చిరునామా(m, p));
  ప్రతిజ్ఞా(sz == m->topsize);
  ప్రతిజ్ఞా(sz > 0);
  ప్రతిజ్ఞా(sz == ((sp->పునాది + sp->కొలత) - (అక్షరం*)p) - శిఖరం_FOOT_కొలత);
  ప్రతిజ్ఞా(pinuse(p));
  ప్రతిజ్ఞా(!pinuse(chunk_plus_స్థానభ్రంశం(p, sz)));
}

/* Check properties of (inuse) mmapped chunks */
స్థిర శూన్య do_check_mmapped_chunk(mstate m, mchunkptr p) {
  size_t  sz = chunksize(p);
  size_t len = (sz + (p->prev_foot) + MMAP_FOOT_PAD);
  ప్రతిజ్ఞా(is_mmapped(p));
  ప్రతిజ్ఞా(use_mmap(m));
  ప్రతిజ్ఞా((is_aligned(chunk2mem(p))) || (p->తల == FENCEPOST_తల));
  ప్రతిజ్ఞా(ok_చిరునామా(m, p));
  ప్రతిజ్ఞా(!is_small(sz));
  ప్రతిజ్ఞా((len & (mparams.పుట_కొలత-కొలత_T_ONE)) == 0);
  ప్రతిజ్ఞా(chunk_plus_స్థానభ్రంశం(p, sz)->తల == FENCEPOST_తల);
  ప్రతిజ్ఞా(chunk_plus_స్థానభ్రంశం(p, sz+కొలత_T_కొలత)->తల == 0);
}

/* Check properties of inuse chunks */
స్థిర శూన్య do_check_inuse_chunk(mstate m, mchunkptr p) {
  do_check_any_chunk(m, p);
  ప్రతిజ్ఞా(is_inuse(p));
  ప్రతిజ్ఞా(తదుపరి_pinuse(p));
  /* If not pinuse and not mmapped, previous chunk has OK offset */
  ప్రతిజ్ఞా(is_mmapped(p) || pinuse(p) || తదుపరి_chunk(prev_chunk(p)) == p);
  ఒకవేళ (is_mmapped(p))
    do_check_mmapped_chunk(m, p);
}

/* Check properties of free chunks */
స్థిర శూన్య do_check_free_chunk(mstate m, mchunkptr p) {
  size_t sz = chunksize(p);
  mchunkptr తదుపరి = chunk_plus_స్థానభ్రంశం(p, sz);
  do_check_any_chunk(m, p);
  ప్రతిజ్ఞా(!is_inuse(p));
  ప్రతిజ్ఞా(!తదుపరి_pinuse(p));
  ప్రతిజ్ఞా (!is_mmapped(p));
  ఒకవేళ (p != m->dv && p != m->శిఖరం) {
    ఒకవేళ (sz >= MIN_CHUNK_కొలత) {
      ప్రతిజ్ఞా((sz & CHUNK_ALIGN_ముసుగు) == 0);
      ప్రతిజ్ఞా(is_aligned(chunk2mem(p)));
      ప్రతిజ్ఞా(తదుపరి->prev_foot == sz);
      ప్రతిజ్ఞా(pinuse(p));
      ప్రతిజ్ఞా (తదుపరి == m->శిఖరం || is_inuse(తదుపరి));
      ప్రతిజ్ఞా(p->fd->bk == p);
      ప్రతిజ్ఞా(p->bk->fd == p);
    }
    లేకపోతే  /* markers are always of size SIZE_T_SIZE */
      ప్రతిజ్ఞా(sz == కొలత_T_కొలత);
  }
}

/* Check properties of malloced chunks at the point they are malloced */
స్థిర శూన్య do_check_malloced_chunk(mstate m, శూన్య* mem, size_t s) {
  ఒకవేళ (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    size_t sz = p->తల & ~INUSE_బిట్లు;
    do_check_inuse_chunk(m, p);
    ప్రతిజ్ఞా((sz & CHUNK_ALIGN_ముసుగు) == 0);
    ప్రతిజ్ఞా(sz >= MIN_CHUNK_కొలత);
    ప్రతిజ్ఞా(sz >= s);
    /* unless mmapped, size is less than MIN_CHUNK_SIZE more than request */
    ప్రతిజ్ఞా(is_mmapped(p) || sz < (s + MIN_CHUNK_కొలత));
  }
}

/* Check a tree and its subtrees.  */
స్థిర శూన్య do_check_వృక్షం(mstate m, tchunkptr t) {
  tchunkptr తల = 0;
  tchunkptr u = t;
  bindex_t tindex = t->సూచీ;
  size_t tsize = chunksize(t);
  bindex_t idx;
  compute_వృక్షం_సూచీ(tsize, idx);
  ప్రతిజ్ఞా(tindex == idx);
  ప్రతిజ్ఞా(tsize >= MIN_LARGE_కొలత);
  ప్రతిజ్ఞా(tsize >= minsize_for_వృక్షం_సూచీ(idx));
  ప్రతిజ్ఞా((idx == NTREEBINS-1) || (tsize < minsize_for_వృక్షం_సూచీ((idx+1))));

  చేయి { /* traverse through chain of same-sized nodes */
    do_check_any_chunk(m, ((mchunkptr)u));
    ప్రతిజ్ఞా(u->సూచీ == tindex);
    ప్రతిజ్ఞా(chunksize(u) == tsize);
    ప్రతిజ్ఞా(!is_inuse(u));
    ప్రతిజ్ఞా(!తదుపరి_pinuse(u));
    ప్రతిజ్ఞా(u->fd->bk == u);
    ప్రతిజ్ఞా(u->bk->fd == u);
    ఒకవేళ (u->మాతృ == 0) {
      ప్రతిజ్ఞా(u->బిడ్డ[0] == 0);
      ప్రతిజ్ఞా(u->బిడ్డ[1] == 0);
    }
    లేకపోతే {
      ప్రతిజ్ఞా(తల == 0); /* only one node on chain has parent */
      తల = u;
      ప్రతిజ్ఞా(u->మాతృ != u);
      ప్రతిజ్ఞా (u->మాతృ->బిడ్డ[0] == u ||
              u->మాతృ->బిడ్డ[1] == u ||
              *((tbinptr*)(u->మాతృ)) == u);
      ఒకవేళ (u->బిడ్డ[0] != 0) {
        ప్రతిజ్ఞా(u->బిడ్డ[0]->మాతృ == u);
        ప్రతిజ్ఞా(u->బిడ్డ[0] != u);
        do_check_వృక్షం(m, u->బిడ్డ[0]);
      }
      ఒకవేళ (u->బిడ్డ[1] != 0) {
        ప్రతిజ్ఞా(u->బిడ్డ[1]->మాతృ == u);
        ప్రతిజ్ఞా(u->బిడ్డ[1] != u);
        do_check_వృక్షం(m, u->బిడ్డ[1]);
      }
      ఒకవేళ (u->బిడ్డ[0] != 0 && u->బిడ్డ[1] != 0) {
        ప్రతిజ్ఞా(chunksize(u->బిడ్డ[0]) < chunksize(u->బిడ్డ[1]));
      }
    }
    u = u->fd;
  } అయితే (u != t);
  ప్రతిజ్ఞా(తల != 0);
}

/*  Check all the chunks in a treebin.  */
స్థిర శూన్య do_check_treebin(mstate m, bindex_t i) {
  tbinptr* tb = treebin_at(m, i);
  tchunkptr t = *tb;
  పూర్ణాంకం ఖాళీలు = (m->treemap & (1U << i)) == 0;
  ఒకవేళ (t == 0)
    ప్రతిజ్ఞా(ఖాళీలు);
  ఒకవేళ (!ఖాళీలు)
    do_check_వృక్షం(m, t);
}

/*  Check all the chunks in a smallbin.  */
స్థిర శూన్య do_check_smallbin(mstate m, bindex_t i) {
  sbinptr b = smallbin_at(m, i);
  mchunkptr p = b->bk;
  గుర్తులేని పూర్ణాంకం ఖాళీలు = (m->smallmap & (1U << i)) == 0;
  ఒకవేళ (p == b)
    ప్రతిజ్ఞా(ఖాళీలు);
  ఒకవేళ (!ఖాళీలు) {
    కొరకు (; p != b; p = p->bk) {
      size_t కొలత = chunksize(p);
      mchunkptr q;
      /* each chunk claims to be free */
      do_check_free_chunk(m, p);
      /* chunk belongs in bin */
      ప్రతిజ్ఞా(small_సూచీ(కొలత) == i);
      ప్రతిజ్ఞా(p->bk == b || chunksize(p->bk) == chunksize(p));
      /* chunk is followed by an inuse chunk */
      q = తదుపరి_chunk(p);
      ఒకవేళ (q->తల != FENCEPOST_తల)
        do_check_inuse_chunk(m, q);
    }
  }
}

/* Find x in a bin. Used in other check functions. */
స్థిర పూర్ణాంకం bin_కనుగొను(mstate m, mchunkptr x) {
  size_t కొలత = chunksize(x);
  ఒకవేళ (is_small(కొలత)) {
    bindex_t sidx = small_సూచీ(కొలత);
    sbinptr b = smallbin_at(m, sidx);
    ఒకవేళ (smallmap_is_marked(m, sidx)) {
      mchunkptr p = b;
      చేయి {
        ఒకవేళ (p == x)
          తిరిగి 1;
      } అయితే ((p = p->fd) != b);
    }
  }
  లేకపోతే {
    bindex_t tidx;
    compute_వృక్షం_సూచీ(కొలత, tidx);
    ఒకవేళ (treemap_is_marked(m, tidx)) {
      tchunkptr t = *treebin_at(m, tidx);
      size_t sizebits = కొలత << leftshift_for_వృక్షం_సూచీ(tidx);
      అయితే (t != 0 && chunksize(t) != కొలత) {
        t = t->బిడ్డ[(sizebits >> (కొలత_T_BITSIZE-కొలత_T_ONE)) & 1];
        sizebits <<= 1;
      }
      ఒకవేళ (t != 0) {
        tchunkptr u = t;
        చేయి {
          ఒకవేళ (u == (tchunkptr)x)
            తిరిగి 1;
        } అయితే ((u = u->fd) != t);
      }
    }
  }
  తిరిగి 0;
}

/* Traverse each chunk and check it; return total */
స్థిర size_t traverse_and_check(mstate m) {
  size_t కూడిక = 0;
  ఒకవేళ (is_initialized(m)) {
    msegmentptr s = &m->seg;
    కూడిక += m->topsize + శిఖరం_FOOT_కొలత;
    అయితే (s != 0) {
      mchunkptr q = align_as_chunk(s->పునాది);
      mchunkptr lastq = 0;
      ప్రతిజ్ఞా(pinuse(q));
      అయితే (segment_holds(s, q) &&
             q != m->శిఖరం && q->తల != FENCEPOST_తల) {
        కూడిక += chunksize(q);
        ఒకవేళ (is_inuse(q)) {
          ప్రతిజ్ఞా(!bin_కనుగొను(m, q));
          do_check_inuse_chunk(m, q);
        }
        లేకపోతే {
          ప్రతిజ్ఞా(q == m->dv || bin_కనుగొను(m, q));
          ప్రతిజ్ఞా(lastq == 0 || is_inuse(lastq)); /* Not 2 consecutive free */
          do_check_free_chunk(m, q);
        }
        lastq = q;
        q = తదుపరి_chunk(q);
      }
      s = s->తదుపరి;
    }
  }
  తిరిగి కూడిక;
}


/* Check all properties of malloc_state. */
స్థిర శూన్య do_check_malloc_state(mstate m) {
  bindex_t i;
  size_t మొత్తం;
  /* check bins */
  కొరకు (i = 0; i < NSMALLBINS; ++i)
    do_check_smallbin(m, i);
  కొరకు (i = 0; i < NTREEBINS; ++i)
    do_check_treebin(m, i);

  ఒకవేళ (m->dvsize != 0) { /* check dv chunk */
    do_check_any_chunk(m, m->dv);
    ప్రతిజ్ఞా(m->dvsize == chunksize(m->dv));
    ప్రతిజ్ఞా(m->dvsize >= MIN_CHUNK_కొలత);
    ప్రతిజ్ఞా(bin_కనుగొను(m, m->dv) == 0);
  }

  ఒకవేళ (m->శిఖరం != 0) {   /* check top chunk */
    do_check_శిఖరం_chunk(m, m->శిఖరం);
    /*assert(m->topsize == chunksize(m->top)); redundant */
    ప్రతిజ్ఞా(m->topsize > 0);
    ప్రతిజ్ఞా(bin_కనుగొను(m, m->శిఖరం) == 0);
  }

  మొత్తం = traverse_and_check(m);
  ప్రతిజ్ఞా(మొత్తం <= m->footprint);
  ప్రతిజ్ఞా(m->footprint <= m->max_footprint);
}
#అయితే_ముగింపు /* DEBUG */

/* ----------------------------- statistics ------------------------------ */

#ఒకవేళ !NO_MALLINFO
స్థిర నిర్మాణం mallinfo అంతర్గత_mallinfo(mstate m) {
  నిర్మాణం mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ensure_initialization();
  ఒకవేళ (!PREACTION(m)) {
    check_malloc_state(m);
    ఒకవేళ (is_initialized(m)) {
      size_t nfree = కొలత_T_ONE; /* top always free */
      size_t mfree = m->topsize + శిఖరం_FOOT_కొలత;
      size_t కూడిక = mfree;
      msegmentptr s = &m->seg;
      అయితే (s != 0) {
        mchunkptr q = align_as_chunk(s->పునాది);
        అయితే (segment_holds(s, q) &&
               q != m->శిఖరం && q->తల != FENCEPOST_తల) {
          size_t sz = chunksize(q);
          కూడిక += sz;
          ఒకవేళ (!is_inuse(q)) {
            mfree += sz;
            ++nfree;
          }
          q = తదుపరి_chunk(q);
        }
        s = s->తదుపరి;
      }

      nm.arena    = కూడిక;
      nm.ordblks  = nfree;
      nm.hblkhd   = m->footprint - కూడిక;
      nm.usmblks  = m->max_footprint;
      nm.uordblks = m->footprint - mfree;
      nm.fordblks = mfree;
      nm.keepcost = m->topsize;
    }

    POSTACTION(m);
  }
  తిరిగి nm;
}
#అయితే_ముగింపు /* !NO_MALLINFO */

#ఒకవేళ !NO_MALLOC_STATS
స్థిర శూన్య అంతర్గత_malloc_stats(mstate m) {
  ensure_initialization();
  ఒకవేళ (!PREACTION(m)) {
    size_t maxfp = 0;
    size_t fp = 0;
    size_t వాడిన = 0;
    check_malloc_state(m);
    ఒకవేళ (is_initialized(m)) {
      msegmentptr s = &m->seg;
      maxfp = m->max_footprint;
      fp = m->footprint;
      వాడిన = fp - (m->topsize + శిఖరం_FOOT_కొలత);

      అయితే (s != 0) {
        mchunkptr q = align_as_chunk(s->పునాది);
        అయితే (segment_holds(s, q) &&
               q != m->శిఖరం && q->తల != FENCEPOST_తల) {
          ఒకవేళ (!is_inuse(q))
            వాడిన -= chunksize(q);
          q = తదుపరి_chunk(q);
        }
        s = s->తదుపరి;
      }
    }
    POSTACTION(m); /* drop lock */
    fprintf(stderr, "max system bytes = %10lu\n", (గుర్తులేని దీర్ఘ)(maxfp));
    fprintf(stderr, "system bytes     = %10lu\n", (గుర్తులేని దీర్ఘ)(fp));
    fprintf(stderr, "in use bytes     = %10lu\n", (గుర్తులేని దీర్ఘ)(వాడిన));
  }
}
#అయితే_ముగింపు /* NO_MALLOC_STATS */

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#నిర్వచించు చొప్పించు_small_chunk(M, P, S) {\
  bindex_t I  = small_సూచీ(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  ప్రతిజ్ఞా(S >= MIN_CHUNK_కొలత);\
  ఒకవేళ (!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  లేకపోతే ఒకవేళ (RTCHECK(ok_చిరునామా(M, B->fd)))\
    F = B->fd;\
  లేకపోతే {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#నిర్వచించు unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  bindex_t I = small_సూచీ(S);\
  ప్రతిజ్ఞా(P != B);\
  ప్రతిజ్ఞా(P != F);\
  ప్రతిజ్ఞా(chunksize(P) == small_index2size(I));\
  ఒకవేళ (RTCHECK(F == smallbin_at(M,I) || (ok_చిరునామా(M, F) && F->bk == P))) { \
    ఒకవేళ (B == F) {\
      clear_smallmap(M, I);\
    }\
    లేకపోతే ఒకవేళ (RTCHECK(B == smallbin_at(M,I) ||\
                     (ok_చిరునామా(M, B) && B->fd == P))) {\
      F->bk = B;\
      B->fd = F;\
    }\
    లేకపోతే {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  లేకపోతే {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Unlink the first chunk from a smallbin */
#నిర్వచించు unlink_మొదటి_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  ప్రతిజ్ఞా(P != B);\
  ప్రతిజ్ఞా(P != F);\
  ప్రతిజ్ఞా(chunksize(P) == small_index2size(I));\
  ఒకవేళ (B == F) {\
    clear_smallmap(M, I);\
  }\
  లేకపోతే ఒకవేళ (RTCHECK(ok_చిరునామా(M, F) && F->bk == P)) {\
    F->bk = B;\
    B->fd = F;\
  }\
  లేకపోతే {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#నిర్వచించు replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  ప్రతిజ్ఞా(is_small(DVS));\
  ఒకవేళ (DVS != 0) {\
    mchunkptr DV = M->dv;\
    చొప్పించు_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#నిర్వచించు చొప్పించు_large_chunk(M, X, S) {\
  tbinptr* H;\
  bindex_t I;\
  compute_వృక్షం_సూచీ(S, I);\
  H = treebin_at(M, I);\
  X->సూచీ = I;\
  X->బిడ్డ[0] = X->బిడ్డ[1] = 0;\
  ఒకవేళ (!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->మాతృ = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  లేకపోతే {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_వృక్షం_సూచీ(I);\
    కొరకు (;;) {\
      ఒకవేళ (chunksize(T) != S) {\
        tchunkptr* C = &(T->బిడ్డ[(K >> (కొలత_T_BITSIZE-కొలత_T_ONE)) & 1]);\
        K <<= 1;\
        ఒకవేళ (*C != 0)\
          T = *C;\
        లేకపోతే ఒకవేళ (RTCHECK(ok_చిరునామా(M, C))) {\
          *C = X;\
          X->మాతృ = T;\
          X->fd = X->bk = X;\
          ఆపు;\
        }\
        లేకపోతే {\
          CORRUPTION_ERROR_ACTION(M);\
          ఆపు;\
        }\
      }\
      లేకపోతే {\
        tchunkptr F = T->fd;\
        ఒకవేళ (RTCHECK(ok_చిరునామా(M, T) && ok_చిరునామా(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->మాతృ = 0;\
          ఆపు;\
        }\
        లేకపోతే {\
          CORRUPTION_ERROR_ACTION(M);\
          ఆపు;\
        }\
      }\
    }\
  }\
}

/*
  Unlink steps:

  1. If x is a chained node, unlink it from its same-sized fd/bk links
     and choose its bk node as its replacement.
  2. If x was the last node of its size, but not a leaf node, it must
     be replaced with a leaf node (not merely one with an open left or
     right), to make sure that lefts and rights of descendants
     correspond properly to bit masks.  We use the rightmost descendant
     of x.  We could use any other leaf, but this is easy to locate and
     tends to counteract removal of leftmosts elsewhere, and so keeps
     paths shorter than minimally guaranteed.  This doesn't loop much
     because on average a node in a tree is near the bottom.
  3. If x is the base of a chain (i.e., has parent links) relink
     x's parent and children to x's replacement (or null if none).
*/

#నిర్వచించు unlink_large_chunk(M, X) {\
  tchunkptr XP = X->మాతృ;\
  tchunkptr R;\
  ఒకవేళ (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    ఒకవేళ (RTCHECK(ok_చిరునామా(M, F) && F->bk == X && R->fd == X)) {\
      F->bk = R;\
      R->fd = F;\
    }\
    లేకపోతే {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  లేకపోతే {\
    tchunkptr* RP;\
    ఒకవేళ (((R = *(RP = &(X->బిడ్డ[1]))) != 0) ||\
        ((R = *(RP = &(X->బిడ్డ[0]))) != 0)) {\
      tchunkptr* CP;\
      అయితే ((*(CP = &(R->బిడ్డ[1])) != 0) ||\
             (*(CP = &(R->బిడ్డ[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      ఒకవేళ (RTCHECK(ok_చిరునామా(M, RP)))\
        *RP = 0;\
      లేకపోతే {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  ఒకవేళ (XP != 0) {\
    tbinptr* H = treebin_at(M, X->సూచీ);\
    ఒకవేళ (X == *H) {\
      ఒకవేళ ((*H = R) == 0) \
        clear_treemap(M, X->సూచీ);\
    }\
    లేకపోతే ఒకవేళ (RTCHECK(ok_చిరునామా(M, XP))) {\
      ఒకవేళ (XP->బిడ్డ[0] == X) \
        XP->బిడ్డ[0] = R;\
      లేకపోతే \
        XP->బిడ్డ[1] = R;\
    }\
    లేకపోతే\
      CORRUPTION_ERROR_ACTION(M);\
    ఒకవేళ (R != 0) {\
      ఒకవేళ (RTCHECK(ok_చిరునామా(M, R))) {\
        tchunkptr C0, C1;\
        R->మాతృ = XP;\
        ఒకవేళ ((C0 = X->బిడ్డ[0]) != 0) {\
          ఒకవేళ (RTCHECK(ok_చిరునామా(M, C0))) {\
            R->బిడ్డ[0] = C0;\
            C0->మాతృ = R;\
          }\
          లేకపోతే\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        ఒకవేళ ((C1 = X->బిడ్డ[1]) != 0) {\
          ఒకవేళ (RTCHECK(ok_చిరునామా(M, C1))) {\
            R->బిడ్డ[1] = C1;\
            C1->మాతృ = R;\
          }\
          లేకపోతే\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      లేకపోతే\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}

/* Relays to large vs small bin operations */

#నిర్వచించు చొప్పించు_chunk(M, P, S)\
  ఒకవేళ (is_small(S)) చొప్పించు_small_chunk(M, P, S)\
  లేకపోతే { tchunkptr TP = (tchunkptr)(P); చొప్పించు_large_chunk(M, TP, S); }

#నిర్వచించు unlink_chunk(M, P, S)\
  ఒకవేళ (is_small(S)) unlink_small_chunk(M, P, S)\
  లేకపోతే { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#ఒకవేళ ONLY_MSPACES
#నిర్వచించు అంతర్గత_malloc(m, b) mspace_malloc(m, b)
#నిర్వచించు అంతర్గత_free(m, mem) mspace_free(m,mem);
#లేకపోతే /* ONLY_MSPACES */
#ఒకవేళ MSPACES
#నిర్వచించు అంతర్గత_malloc(m, b)\
  ((m == gm)? dlmalloc(b) : mspace_malloc(m, b))
#నిర్వచించు అంతర్గత_free(m, mem)\
   ఒకవేళ (m == gm) dlfree(mem); లేకపోతే mspace_free(m,mem);
#లేకపోతే /* MSPACES */
#నిర్వచించు అంతర్గత_malloc(m, b) dlmalloc(b)
#నిర్వచించు అంతర్గత_free(m, mem) dlfree(mem)
#అయితే_ముగింపు /* MSPACES */
#అయితే_ముగింపు /* ONLY_MSPACES */

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
స్థిర శూన్య* mmap_alloc(mstate m, size_t nb) {
  size_t mmsize = mmap_align(nb + SIX_కొలత_T_SIZES + CHUNK_ALIGN_ముసుగు);
  ఒకవేళ (m->footprint_హద్దు != 0) {
    size_t fp = m->footprint + mmsize;
    ఒకవేళ (fp <= m->footprint || fp > m->footprint_హద్దు)
      తిరిగి 0;
  }
  ఒకవేళ (mmsize > nb) {     /* Check for wrap around 0 */
    అక్షరం* mm = (అక్షరం*)(CALL_ప్రత్యక్ష_MMAP(mmsize));
    ఒకవేళ (mm != CMFAIL) {
      size_t offset = align_స్థానభ్రంశం(chunk2mem(mm));
      size_t psize = mmsize - offset - MMAP_FOOT_PAD;
      mchunkptr p = (mchunkptr)(mm + offset);
      p->prev_foot = offset;
      p->తల = psize;
      mark_inuse_foot(m, p, psize);
      chunk_plus_స్థానభ్రంశం(p, psize)->తల = FENCEPOST_తల;
      chunk_plus_స్థానభ్రంశం(p, psize+కొలత_T_కొలత)->తల = 0;

      ఒకవేళ (m->కనిష్ఠ_addr == 0 || mm < m->కనిష్ఠ_addr)
        m->కనిష్ఠ_addr = mm;
      ఒకవేళ ((m->footprint += mmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      ప్రతిజ్ఞా(is_aligned(chunk2mem(p)));
      check_mmapped_chunk(m, p);
      తిరిగి chunk2mem(p);
    }
  }
  తిరిగి 0;
}

/* Realloc using mmap */
స్థిర mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb, పూర్ణాంకం flags) {
  size_t oldsize = chunksize(oldp);
  (శూన్య)flags; /* placate people compiling -Wunused */
  ఒకవేళ (is_small(nb)) /* Can't shrink mmap regions below small size */
    తిరిగి 0;
  /* Keep old chunk if big enough but not too big */
  ఒకవేళ (oldsize >= nb + కొలత_T_కొలత &&
      (oldsize - nb) <= (mparams.granularity << 1))
    తిరిగి oldp;
  లేకపోతే {
    size_t offset = oldp->prev_foot;
    size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
    size_t newmmsize = mmap_align(nb + SIX_కొలత_T_SIZES + CHUNK_ALIGN_ముసుగు);
    అక్షరం* cp = (అక్షరం*)CALL_MREMAP((అక్షరం*)oldp - offset,
                                  oldmmsize, newmmsize, flags);
    ఒకవేళ (cp != CMFAIL) {
      mchunkptr newp = (mchunkptr)(cp + offset);
      size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
      newp->తల = psize;
      mark_inuse_foot(m, newp, psize);
      chunk_plus_స్థానభ్రంశం(newp, psize)->తల = FENCEPOST_తల;
      chunk_plus_స్థానభ్రంశం(newp, psize+కొలత_T_కొలత)->తల = 0;

      ఒకవేళ (cp < m->కనిష్ఠ_addr)
        m->కనిష్ఠ_addr = cp;
      ఒకవేళ ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      check_mmapped_chunk(m, newp);
      తిరిగి newp;
    }
  }
  తిరిగి 0;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
స్థిర శూన్య init_శిఖరం(mstate m, mchunkptr p, size_t psize) {
  /* Ensure alignment */
  size_t offset = align_స్థానభ్రంశం(chunk2mem(p));
  p = (mchunkptr)((అక్షరం*)p + offset);
  psize -= offset;

  m->శిఖరం = p;
  m->topsize = psize;
  p->తల = psize | PINUSE_బిట్టు;
  /* set size of fake trailing chunk holding overhead space only once */
  chunk_plus_స్థానభ్రంశం(p, psize)->తల = శిఖరం_FOOT_కొలత;
  m->trim_check = mparams.trim_పరిమితి; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
స్థిర శూన్య init_bins(mstate m) {
  /* Establish circular links for smallbins */
  bindex_t i;
  కొరకు (i = 0; i < NSMALLBINS; ++i) {
    sbinptr bin = smallbin_at(m,i);
    bin->fd = bin->bk = bin;
  }
}

#ఒకవేళ PROCEED_ON_ERROR

/* default corruption action */
స్థిర శూన్య reset_on_error(mstate m) {
  పూర్ణాంకం i;
  ++malloc_corruption_error_లెక్క;
  /* Reinitialize fields to forget about all memory */
  m->smallmap = m->treemap = 0;
  m->dvsize = m->topsize = 0;
  m->seg.పునాది = 0;
  m->seg.కొలత = 0;
  m->seg.తదుపరి = 0;
  m->శిఖరం = m->dv = 0;
  కొరకు (i = 0; i < NTREEBINS; ++i)
    *treebin_at(m, i) = 0;
  init_bins(m);
}
#అయితే_ముగింపు /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
స్థిర శూన్య* prepend_alloc(mstate m, అక్షరం* newbase, అక్షరం* oldbase,
                           size_t nb) {
  mchunkptr p = align_as_chunk(newbase);
  mchunkptr oldfirst = align_as_chunk(oldbase);
  size_t psize = (అక్షరం*)oldfirst - (అక్షరం*)p;
  mchunkptr q = chunk_plus_స్థానభ్రంశం(p, nb);
  size_t qsize = psize - nb;
  సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, p, nb);

  ప్రతిజ్ఞా((అక్షరం*)oldfirst > (అక్షరం*)q);
  ప్రతిజ్ఞా(pinuse(oldfirst));
  ప్రతిజ్ఞా(qsize >= MIN_CHUNK_కొలత);

  /* consolidate remainder with first chunk of old base */
  ఒకవేళ (oldfirst == m->శిఖరం) {
    size_t tsize = m->topsize += qsize;
    m->శిఖరం = q;
    q->తల = tsize | PINUSE_బిట్టు;
    check_శిఖరం_chunk(m, q);
  }
  లేకపోతే ఒకవేళ (oldfirst == m->dv) {
    size_t dsize = m->dvsize += qsize;
    m->dv = q;
    సమితి_కొలత_and_pinuse_యొక్క_free_chunk(q, dsize);
  }
  లేకపోతే {
    ఒకవేళ (!is_inuse(oldfirst)) {
      size_t nsize = chunksize(oldfirst);
      unlink_chunk(m, oldfirst, nsize);
      oldfirst = chunk_plus_స్థానభ్రంశం(oldfirst, nsize);
      qsize += nsize;
    }
    సమితి_free_with_pinuse(q, qsize, oldfirst);
    చొప్పించు_chunk(m, q, qsize);
    check_free_chunk(m, q);
  }

  check_malloced_chunk(m, chunk2mem(p), nb);
  తిరిగి chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
స్థిర శూన్య కూడు_segment(mstate m, అక్షరం* tbase, size_t tsize, flag_t mmapped) {
  /* Determine locations and sizes of segment, fenceposts, old top */
  అక్షరం* old_శిఖరం = (అక్షరం*)m->శిఖరం;
  msegmentptr oldsp = segment_holding(m, old_శిఖరం);
  అక్షరం* old_చివర = oldsp->పునాది + oldsp->కొలత;
  size_t ssize = pad_request(పరిమాణం(నిర్మాణం malloc_segment));
  అక్షరం* rawsp = old_చివర - (ssize + FOUR_కొలత_T_SIZES + CHUNK_ALIGN_ముసుగు);
  size_t offset = align_స్థానభ్రంశం(chunk2mem(rawsp));
  అక్షరం* asp = rawsp + offset;
  అక్షరం* csp = (asp < (old_శిఖరం + MIN_CHUNK_కొలత))? old_శిఖరం : asp;
  mchunkptr sp = (mchunkptr)csp;
  msegmentptr ss = (msegmentptr)(chunk2mem(sp));
  mchunkptr tnext = chunk_plus_స్థానభ్రంశం(sp, ssize);
  mchunkptr p = tnext;
  పూర్ణాంకం nfences = 0;
  (శూన్య)nfences; /* Suppress unused variable warning */

  /* reset top to new space */
  init_శిఖరం(m, (mchunkptr)tbase, tsize - శిఖరం_FOOT_కొలత);

  /* Set up segment record */
  ప్రతిజ్ఞా(is_aligned(ss));
  సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, sp, ssize);
  *ss = m->seg; /* Push current record */
  m->seg.పునాది = tbase;
  m->seg.కొలత = tsize;
  (శూన్య)సమితి_segment_flags(&m->seg, mmapped);
  m->seg.తదుపరి = ss;

  /* Insert trailing fenceposts */
  కొరకు (;;) {
    mchunkptr nextp = chunk_plus_స్థానభ్రంశం(p, కొలత_T_కొలత);
    p->తల = FENCEPOST_తల;
    ++nfences;
    ఒకవేళ ((అక్షరం*)(&(nextp->తల)) < old_చివర)
      p = nextp;
    లేకపోతే
      ఆపు;
  }
  ప్రతిజ్ఞా(nfences >= 2);

  /* Insert the rest of old top into a bin as an ordinary free chunk */
  ఒకవేళ (csp != old_శిఖరం) {
    mchunkptr q = (mchunkptr)old_శిఖరం;
    size_t psize = csp - old_శిఖరం;
    mchunkptr tn = chunk_plus_స్థానభ్రంశం(q, psize);
    సమితి_free_with_pinuse(q, psize, tn);
    చొప్పించు_chunk(m, q, psize);
  }

  check_శిఖరం_chunk(m, m->శిఖరం);
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
స్థిర శూన్య* sys_alloc(mstate m, size_t nb) {
  అక్షరం* tbase = CMFAIL;
  size_t tsize = 0;
  flag_t mmap_flag = 0;
  size_t asize; /* allocation size */

  ensure_initialization();

  /* Directly map large chunks, but only if already initialized */
  ఒకవేళ (use_mmap(m) && nb >= mparams.mmap_పరిమితి && m->topsize != 0) {
    శూన్య* mem = mmap_alloc(m, nb);
    ఒకవేళ (mem != 0)
      తిరిగి mem;
  }

  asize = granularity_align(nb + SYS_ALLOC_PADDING);
  ఒకవేళ (asize <= nb)
    తిరిగి 0; /* wraparound */
  ఒకవేళ (m->footprint_హద్దు != 0) {
    size_t fp = m->footprint + asize;
    ఒకవేళ (fp <= m->footprint || fp > m->footprint_హద్దు)
      తిరిగి 0;
  }

  /*
    Try getting memory in any of three ways (in most-preferred to
    least-preferred order):
    1. A call to MORECORE that can normally contiguously extend memory.
       (disabled if not MORECORE_CONTIGUOUS or not HAVE_MORECORE or
       or main space is mmapped or a previous contiguous call failed)
    2. A call to MMAP new space (disabled if not HAVE_MMAP).
       Note that under the default settings, if MORECORE is unable to
       fulfill a request, and HAVE_MMAP is true, then mmap is
       used as a noncontiguous system allocator. This is a useful backup
       strategy for systems with holes in address spaces -- in this case
       sbrk cannot contiguously expand the heap, but mmap may be able to
       find space.
    3. A call to MORECORE that cannot usually contiguously extend memory.
       (disabled if not HAVE_MORECORE)

   In all cases, we need to request enough bytes from system to ensure
   we can malloc nb bytes upon success, so pad with enough space for
   top_foot, plus alignment-pad to make sure we don't lose bytes if
   not on boundary, and round this up to a granularity unit.
  */

  ఒకవేళ (MORECORE_CONTIGUOUS && !use_noncontiguous(m)) {
    అక్షరం* br = CMFAIL;
    size_t ssize = asize; /* sbrk call size */
    msegmentptr ss = (m->శిఖరం == 0)? 0 : segment_holding(m, (అక్షరం*)m->శిఖరం);
    ACQUIRE_MALLOC_GLOBAL_తాళం();

    ఒకవేళ (ss == 0) {  /* First time through or recovery */
      అక్షరం* పునాది = (అక్షరం*)CALL_MORECORE(0);
      ఒకవేళ (పునాది != CMFAIL) {
        size_t fp;
        /* Adjust to end on a page boundary */
        ఒకవేళ (!is_పుట_aligned(పునాది))
          ssize += (పుట_align((size_t)పునాది) - (size_t)పునాది);
        fp = m->footprint + ssize; /* recheck limits */
        ఒకవేళ (ssize > nb && ssize < HALF_MAX_కొలత_T &&
            (m->footprint_హద్దు == 0 ||
             (fp > m->footprint && fp <= m->footprint_హద్దు)) &&
            (br = (అక్షరం*)(CALL_MORECORE(ssize))) == పునాది) {
          tbase = పునాది;
          tsize = ssize;
        }
      }
    }
    లేకపోతే {
      /* Subtract out existing available top space from MORECORE request. */
      ssize = granularity_align(nb - m->topsize + SYS_ALLOC_PADDING);
      /* Use mem here only if it did continuously extend old space */
      ఒకవేళ (ssize < HALF_MAX_కొలత_T &&
          (br = (అక్షరం*)(CALL_MORECORE(ssize))) == ss->పునాది+ss->కొలత) {
        tbase = br;
        tsize = ssize;
      }
    }

    ఒకవేళ (tbase == CMFAIL) {    /* Cope with partial failure */
      ఒకవేళ (br != CMFAIL) {    /* Try to use/extend the space we did get */
        ఒకవేళ (ssize < HALF_MAX_కొలత_T &&
            ssize < nb + SYS_ALLOC_PADDING) {
          size_t esize = granularity_align(nb + SYS_ALLOC_PADDING - ssize);
          ఒకవేళ (esize < HALF_MAX_కొలత_T) {
            అక్షరం* చివర = (అక్షరం*)CALL_MORECORE(esize);
            ఒకవేళ (చివర != CMFAIL)
              ssize += esize;
            లేకపోతే {            /* Can't use; try to release */
              (శూన్య) CALL_MORECORE(-ssize);
              br = CMFAIL;
            }
          }
        }
      }
      ఒకవేళ (br != CMFAIL) {    /* Use the space we did get */
        tbase = br;
        tsize = ssize;
      }
      లేకపోతే
        disable_contiguous(m); /* Don't try contiguous path in the future */
    }

    RELEASE_MALLOC_GLOBAL_తాళం();
  }

  ఒకవేళ (HAVE_MMAP && tbase == CMFAIL) {  /* Try MMAP */
    అక్షరం* mp = (అక్షరం*)(CALL_MMAP(asize));
    ఒకవేళ (mp != CMFAIL) {
      tbase = mp;
      tsize = asize;
      mmap_flag = USE_MMAP_బిట్టు;
    }
  }

  ఒకవేళ (HAVE_MORECORE && tbase == CMFAIL) { /* Try noncontiguous MORECORE */
    ఒకవేళ (asize < HALF_MAX_కొలత_T) {
      అక్షరం* br = CMFAIL;
      అక్షరం* చివర = CMFAIL;
      ACQUIRE_MALLOC_GLOBAL_తాళం();
      br = (అక్షరం*)(CALL_MORECORE(asize));
      చివర = (అక్షరం*)(CALL_MORECORE(0));
      RELEASE_MALLOC_GLOBAL_తాళం();
      ఒకవేళ (br != CMFAIL && చివర != CMFAIL && br < చివర) {
        size_t ssize = చివర - br;
        ఒకవేళ (ssize > nb + శిఖరం_FOOT_కొలత) {
          tbase = br;
          tsize = ssize;
        }
      }
    }
  }

  ఒకవేళ (tbase != CMFAIL) {

    ఒకవేళ ((m->footprint += tsize) > m->max_footprint)
      m->max_footprint = m->footprint;

    ఒకవేళ (!is_initialized(m)) { /* first-time initialization */
      ఒకవేళ (m->కనిష్ఠ_addr == 0 || tbase < m->కనిష్ఠ_addr)
        m->కనిష్ఠ_addr = tbase;
      m->seg.పునాది = tbase;
      m->seg.కొలత = tsize;
      (శూన్య)సమితి_segment_flags(&m->seg, mmap_flag);
      m->magic = mparams.magic;
      m->release_checks = MAX_RELEASE_CHECK_రేటు;
      init_bins(m);
#ఒకవేళ !ONLY_MSPACES
      ఒకవేళ (is_global(m))
        init_శిఖరం(m, (mchunkptr)tbase, tsize - శిఖరం_FOOT_కొలత);
      లేకపోతే
#అయితే_ముగింపు
      {
        /* Offset top by embedded malloc_state */
        mchunkptr mn = తదుపరి_chunk(mem2chunk(m));
        init_శిఖరం(m, mn, (size_t)((tbase + tsize) - (అక్షరం*)mn) -శిఖరం_FOOT_కొలత);
      }
    }

    లేకపోతే {
      /* Try to merge with an existing segment */
      msegmentptr sp = &m->seg;
      /* Only consider most recent segment if traversal suppressed */
      అయితే (sp != 0 && tbase != sp->పునాది + sp->కొలత)
        sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->తదుపరి;
      ఒకవేళ (sp != 0 &&
          !is_extern_segment(sp) &&
          check_segment_విలీనం(sp, tbase, tsize) &&
          (get_segment_flags(sp) & USE_MMAP_బిట్టు) == mmap_flag &&
          segment_holds(sp, m->శిఖరం)) { /* append */
        sp->కొలత += tsize;
        init_శిఖరం(m, m->శిఖరం, m->topsize + tsize);
      }
      లేకపోతే {
        ఒకవేళ (tbase < m->కనిష్ఠ_addr)
          m->కనిష్ఠ_addr = tbase;
        sp = &m->seg;
        అయితే (sp != 0 && sp->పునాది != tbase + tsize)
          sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->తదుపరి;
        ఒకవేళ (sp != 0 &&
            !is_extern_segment(sp) &&
            check_segment_విలీనం(sp, tbase, tsize) &&
            (get_segment_flags(sp) & USE_MMAP_బిట్టు) == mmap_flag) {
          అక్షరం* oldbase = sp->పునాది;
          sp->పునాది = tbase;
          sp->కొలత += tsize;
          తిరిగి prepend_alloc(m, tbase, oldbase, nb);
        }
        లేకపోతే
          కూడు_segment(m, tbase, tsize, mmap_flag);
      }
    }

    ఒకవేళ (nb < m->topsize) { /* Allocate from new or extended top space */
      size_t rsize = m->topsize -= nb;
      mchunkptr p = m->శిఖరం;
      mchunkptr r = m->శిఖరం = chunk_plus_స్థానభ్రంశం(p, nb);
      r->తల = rsize | PINUSE_బిట్టు;
      సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, p, nb);
      check_శిఖరం_chunk(m, m->శిఖరం);
      check_malloced_chunk(m, chunk2mem(p), nb);
      తిరిగి chunk2mem(p);
    }
  }

  MALLOC_FAILURE_ACTION;
  తిరిగి 0;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
స్థిర size_t release_వాడని_segments(mstate m) {
  size_t released = 0;
  పూర్ణాంకం nsegs = 0;
  msegmentptr pred = &m->seg;
  msegmentptr sp = pred->తదుపరి;
  అయితే (sp != 0) {
    అక్షరం* పునాది = sp->పునాది;
    size_t కొలత = sp->కొలత;
    msegmentptr తదుపరి = sp->తదుపరి;
    ++nsegs;
    ఒకవేళ (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
      mchunkptr p = align_as_chunk(పునాది);
      size_t psize = chunksize(p);
      /* Can unmap if first chunk holds entire segment and not pinned */
      ఒకవేళ (!is_inuse(p) && (అక్షరం*)p + psize >= పునాది + కొలత - శిఖరం_FOOT_కొలత) {
        tchunkptr tp = (tchunkptr)p;
        ప్రతిజ్ఞా(segment_holds(sp, (అక్షరం*)sp));
        ఒకవేళ (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        లేకపోతే {
          unlink_large_chunk(m, tp);
        }
        ఒకవేళ (CALL_MUNMAP(పునాది, కొలత) == 0) {
          released += కొలత;
          m->footprint -= కొలత;
          /* unlink obsoleted record */
          sp = pred;
          sp->తదుపరి = తదుపరి;
        }
        లేకపోతే { /* back out if cannot unmap */
          చొప్పించు_large_chunk(m, tp, psize);
        }
      }
    }
    ఒకవేళ (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
      ఆపు;
    pred = sp;
    sp = తదుపరి;
  }
  /* Reset check counter */
  m->release_checks = (((size_t) nsegs > (size_t) MAX_RELEASE_CHECK_రేటు)?
                       (size_t) nsegs : (size_t) MAX_RELEASE_CHECK_రేటు);
  తిరిగి released;
}

స్థిర పూర్ణాంకం sys_trim(mstate m, size_t pad) {
  size_t released = 0;
  ensure_initialization();
  ఒకవేళ (pad < MAX_REQUEST && is_initialized(m)) {
    pad += శిఖరం_FOOT_కొలత; /* ensure enough room for segment overhead */

    ఒకవేళ (m->topsize > pad) {
      /* Shrink top space in granularity-size units, keeping at least one */
      size_t unit = mparams.granularity;
      size_t extra = ((m->topsize - pad + (unit - కొలత_T_ONE)) / unit -
                      కొలత_T_ONE) * unit;
      msegmentptr sp = segment_holding(m, (అక్షరం*)m->శిఖరం);

      ఒకవేళ (sp == 0)
        CORRUPTION_ERROR_ACTION(m);
      ఒకవేళ (!is_extern_segment(sp)) {
        ఒకవేళ (is_mmapped_segment(sp)) {
          ఒకవేళ (HAVE_MMAP &&
              sp->కొలత >= extra &&
              !has_segment_link(m, sp)) { /* can't shrink if pinned */
            size_t newsize = sp->కొలత - extra;
            (శూన్య)newsize; /* placate people compiling -Wunused-variable */
            /* Prefer mremap, fall back to munmap */
            ఒకవేళ ((CALL_MREMAP(sp->పునాది, sp->కొలత, newsize, 0) != MFAIL) ||
                (CALL_MUNMAP(sp->పునాది + newsize, extra) == 0)) {
              released = extra;
            }
          }
        }
        లేకపోతే ఒకవేళ (HAVE_MORECORE) {
          ఒకవేళ (extra >= HALF_MAX_కొలత_T) /* Avoid wrapping negative */
            extra = (HALF_MAX_కొలత_T) + కొలత_T_ONE - unit;
          ACQUIRE_MALLOC_GLOBAL_తాళం();
          {
            /* Make sure end of memory is where we last set it. */
            అక్షరం* old_br = (అక్షరం*)(CALL_MORECORE(0));
            ఒకవేళ (old_br == sp->పునాది + sp->కొలత) {
              అక్షరం* rel_br = (అక్షరం*)(CALL_MORECORE(-extra));
              అక్షరం* new_br = (అక్షరం*)(CALL_MORECORE(0));
              ఒకవేళ (rel_br != CMFAIL && new_br < old_br)
                released = old_br - new_br;
            }
          }
          RELEASE_MALLOC_GLOBAL_తాళం();
        }
      }

      ఒకవేళ (released != 0) {
        sp->కొలత -= released;
        m->footprint -= released;
        init_శిఖరం(m, m->శిఖరం, m->topsize - released);
        check_శిఖరం_chunk(m, m->శిఖరం);
      }
    }

    /* Unmap any unused mmapped segments */
    ఒకవేళ (HAVE_MMAP)
      released += release_వాడని_segments(m);

    /* On failure, disable autotrim to avoid repeated failed future calls */
    ఒకవేళ (released == 0 && m->topsize > m->trim_check)
      m->trim_check = MAX_కొలత_T;
  }

  తిరిగి (released != 0)? 1 : 0;
}

/* Consolidate and bin a chunk. Differs from exported versions
   of free mainly in that the chunk need not be marked as inuse.
*/
స్థిర శూన్య dispose_chunk(mstate m, mchunkptr p, size_t psize) {
  mchunkptr తదుపరి = chunk_plus_స్థానభ్రంశం(p, psize);
  ఒకవేళ (!pinuse(p)) {
    mchunkptr prev;
    size_t prevsize = p->prev_foot;
    ఒకవేళ (is_mmapped(p)) {
      psize += prevsize + MMAP_FOOT_PAD;
      ఒకవేళ (CALL_MUNMAP((అక్షరం*)p - prevsize, psize) == 0)
        m->footprint -= psize;
      తిరిగి;
    }
    prev = chunk_minus_స్థానభ్రంశం(p, prevsize);
    psize += prevsize;
    p = prev;
    ఒకవేళ (RTCHECK(ok_చిరునామా(m, prev))) { /* consolidate backward */
      ఒకవేళ (p != m->dv) {
        unlink_chunk(m, p, prevsize);
      }
      లేకపోతే ఒకవేళ ((తదుపరి->తల & INUSE_బిట్లు) == INUSE_బిట్లు) {
        m->dvsize = psize;
        సమితి_free_with_pinuse(p, psize, తదుపరి);
        తిరిగి;
      }
    }
    లేకపోతే {
      CORRUPTION_ERROR_ACTION(m);
      తిరిగి;
    }
  }
  ఒకవేళ (RTCHECK(ok_చిరునామా(m, తదుపరి))) {
    ఒకవేళ (!cinuse(తదుపరి)) {  /* consolidate forward */
      ఒకవేళ (తదుపరి == m->శిఖరం) {
        size_t tsize = m->topsize += psize;
        m->శిఖరం = p;
        p->తల = tsize | PINUSE_బిట్టు;
        ఒకవేళ (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        తిరిగి;
      }
      లేకపోతే ఒకవేళ (తదుపరి == m->dv) {
        size_t dsize = m->dvsize += psize;
        m->dv = p;
        సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, dsize);
        తిరిగి;
      }
      లేకపోతే {
        size_t nsize = chunksize(తదుపరి);
        psize += nsize;
        unlink_chunk(m, తదుపరి, nsize);
        సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, psize);
        ఒకవేళ (p == m->dv) {
          m->dvsize = psize;
          తిరిగి;
        }
      }
    }
    లేకపోతే {
      సమితి_free_with_pinuse(p, psize, తదుపరి);
    }
    చొప్పించు_chunk(m, p, psize);
  }
  లేకపోతే {
    CORRUPTION_ERROR_ACTION(m);
  }
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
స్థిర శూన్య* tmalloc_large(mstate m, size_t nb) {
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  bindex_t idx;
  compute_వృక్షం_సూచీ(nb, idx);
  ఒకవేళ ((t = *treebin_at(m, idx)) != 0) {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_వృక్షం_సూచీ(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    కొరకు (;;) {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      ఒకవేళ (trem < rsize) {
        v = t;
        ఒకవేళ ((rsize = trem) == 0)
          ఆపు;
      }
      rt = t->బిడ్డ[1];
      t = t->బిడ్డ[(sizebits >> (కొలత_T_BITSIZE-కొలత_T_ONE)) & 1];
      ఒకవేళ (rt != 0 && rt != t)
        rst = rt;
      ఒకవేళ (t == 0) {
        t = rst; /* set t to least subtree holding sizes > nb */
        ఆపు;
      }
      sizebits <<= 1;
    }
  }
  ఒకవేళ (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
    binmap_t leftbits = ఎడమ_బిట్లు(idx2bit(idx)) & m->treemap;
    ఒకవేళ (leftbits != 0) {
      bindex_t i;
      binmap_t leastbit = కనిష్ఠ_బిట్టు(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  అయితే (t != 0) { /* find smallest of tree or subtree */
    size_t trem = chunksize(t) - nb;
    ఒకవేళ (trem < rsize) {
      rsize = trem;
      v = t;
    }
    t = leftmost_బిడ్డ(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  ఒకవేళ (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
    ఒకవేళ (RTCHECK(ok_చిరునామా(m, v))) { /*విభజించు*/
      mchunkptr r = chunk_plus_స్థానభ్రంశం(v, nb);
      ప్రతిజ్ఞా(chunksize(v) == rsize + nb);
      ఒకవేళ (RTCHECK(ok_తదుపరి(v, r))) {
        unlink_large_chunk(m, v);
        ఒకవేళ (rsize < MIN_CHUNK_కొలత)
          సమితి_inuse_and_pinuse(m, v, (rsize + nb));
        లేకపోతే {
          సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, v, nb);
          సమితి_కొలత_and_pinuse_యొక్క_free_chunk(r, rsize);
          చొప్పించు_chunk(m, r, rsize);
        }
        తిరిగి chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  తిరిగి 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
స్థిర శూన్య* tmalloc_small(mstate m, size_t nb) {
  tchunkptr t, v;
  size_t rsize;
  bindex_t i;
  binmap_t leastbit = కనిష్ఠ_బిట్టు(m->treemap);
  compute_bit2idx(leastbit, i);
  v = t = *treebin_at(m, i);
  rsize = chunksize(t) - nb;

  అయితే ((t = leftmost_బిడ్డ(t)) != 0) {
    size_t trem = chunksize(t) - nb;
    ఒకవేళ (trem < rsize) {
      rsize = trem;
      v = t;
    }
  }

  ఒకవేళ (RTCHECK(ok_చిరునామా(m, v))) {
    mchunkptr r = chunk_plus_స్థానభ్రంశం(v, nb);
    ప్రతిజ్ఞా(chunksize(v) == rsize + nb);
    ఒకవేళ (RTCHECK(ok_తదుపరి(v, r))) {
      unlink_large_chunk(m, v);
      ఒకవేళ (rsize < MIN_CHUNK_కొలత)
        సమితి_inuse_and_pinuse(m, v, (rsize + nb));
      లేకపోతే {
        సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, v, nb);
        సమితి_కొలత_and_pinuse_యొక్క_free_chunk(r, rsize);
        replace_dv(m, r, rsize);
      }
      తిరిగి chunk2mem(v);
    }
  }

  CORRUPTION_ERROR_ACTION(m);
  తిరిగి 0;
}

#ఒకవేళ !ONLY_MSPACES

శూన్య* dlmalloc(size_t బైట్లు) {
  /*
     Basic algorithm:
     If a small request (< 256 bytes minus per-chunk overhead):
       1. If one exists, use a remainderless chunk in associated smallbin.
          (Remainderless means that there are too few excess bytes to
          represent as a chunk.)
       2. If it is big enough, use the dv chunk, which is normally the
          chunk adjacent to the one used for the most recent small request.
       3. If one exists, split the smallest available chunk in a bin,
          saving remainder in dv.
       4. If it is big enough, use the top chunk.
       5. If available, get memory from system and use it
     Otherwise, for a large request:
       1. Find the smallest available binned chunk that fits, and use it
          if it is better fitting than dv chunk, splitting if necessary.
       2. If better fitting than any binned chunk, use the dv chunk.
       3. If it is big enough, use the top chunk.
       4. If request size >= mmap threshold, try to directly mmap this chunk.
       5. If available, get memory from system and use it

     The ugly goto's here ensure that postaction occurs along all paths.
  */

#ఒకవేళ USE_LOCKS
  ensure_initialization(); /* initialize in sys_alloc if not using locks */
#అయితే_ముగింపు

  ఒకవేళ (!PREACTION(gm)) {
    శూన్య* mem;
    size_t nb;
    ఒకవేళ (బైట్లు <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (బైట్లు < MIN_REQUEST)? MIN_CHUNK_కొలత : pad_request(బైట్లు);
      idx = small_సూచీ(nb);
      smallbits = gm->smallmap >> idx;

      ఒకవేళ ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(gm, idx);
        p = b->fd;
        ప్రతిజ్ఞా(chunksize(p) == small_index2size(idx));
        unlink_మొదటి_small_chunk(gm, b, p, idx);
        సమితి_inuse_and_pinuse(gm, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(gm, mem, nb);
        వెళ్ళు postaction;
      }

      లేకపోతే ఒకవేళ (nb > gm->dvsize) {
        ఒకవేళ (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & ఎడమ_బిట్లు(idx2bit(idx));
          binmap_t leastbit = కనిష్ఠ_బిట్టు(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(gm, i);
          p = b->fd;
          ప్రతిజ్ఞా(chunksize(p) == small_index2size(i));
          unlink_మొదటి_small_chunk(gm, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          ఒకవేళ (కొలత_T_కొలత != 4 && rsize < MIN_CHUNK_కొలత)
            సమితి_inuse_and_pinuse(gm, p, small_index2size(i));
          లేకపోతే {
            సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(gm, p, nb);
            r = chunk_plus_స్థానభ్రంశం(p, nb);
            సమితి_కొలత_and_pinuse_యొక్క_free_chunk(r, rsize);
            replace_dv(gm, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(gm, mem, nb);
          వెళ్ళు postaction;
        }

        లేకపోతే ఒకవేళ (gm->treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0) {
          check_malloced_chunk(gm, mem, nb);
          వెళ్ళు postaction;
        }
      }
    }
    లేకపోతే ఒకవేళ (బైట్లు >= MAX_REQUEST)
      nb = MAX_కొలత_T; /* Too big to allocate. Force failure (in sys alloc) */
    లేకపోతే {
      nb = pad_request(బైట్లు);
      ఒకవేళ (gm->treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0) {
        check_malloced_chunk(gm, mem, nb);
        వెళ్ళు postaction;
      }
    }

    ఒకవేళ (nb <= gm->dvsize) {
      size_t rsize = gm->dvsize - nb;
      mchunkptr p = gm->dv;
      ఒకవేళ (rsize >= MIN_CHUNK_కొలత) { /* split dv */
        mchunkptr r = gm->dv = chunk_plus_స్థానభ్రంశం(p, nb);
        gm->dvsize = rsize;
        సమితి_కొలత_and_pinuse_యొక్క_free_chunk(r, rsize);
        సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(gm, p, nb);
      }
      లేకపోతే { /* exhaust dv */
        size_t dvs = gm->dvsize;
        gm->dvsize = 0;
        gm->dv = 0;
        సమితి_inuse_and_pinuse(gm, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(gm, mem, nb);
      వెళ్ళు postaction;
    }

    లేకపోతే ఒకవేళ (nb < gm->topsize) { /* Split top */
      size_t rsize = gm->topsize -= nb;
      mchunkptr p = gm->శిఖరం;
      mchunkptr r = gm->శిఖరం = chunk_plus_స్థానభ్రంశం(p, nb);
      r->తల = rsize | PINUSE_బిట్టు;
      సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(gm, p, nb);
      mem = chunk2mem(p);
      check_శిఖరం_chunk(gm, gm->శిఖరం);
      check_malloced_chunk(gm, mem, nb);
      వెళ్ళు postaction;
    }

    mem = sys_alloc(gm, nb);

  postaction:
    POSTACTION(gm);
    తిరిగి mem;
  }

  తిరిగి 0;
}

/* ---------------------------- free --------------------------- */

శూన్య dlfree(శూన్య* mem) {
  /*
     Consolidate freed chunks with preceding or succeeding bordering
     free chunks, if they exist, and then place in a bin.  Intermixed
     with special cases for top, dv, mmapped chunks, and usage errors.
  */

  ఒకవేళ (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#ఒకవేళ FOOTERS
    mstate fm = get_mstate_for(p);
    ఒకవేళ (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      తిరిగి;
    }
#లేకపోతే /* FOOTERS */
#నిర్వచించు fm gm
#అయితే_ముగింపు /* FOOTERS */
    ఒకవేళ (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      ఒకవేళ (RTCHECK(ok_చిరునామా(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr తదుపరి = chunk_plus_స్థానభ్రంశం(p, psize);
        ఒకవేళ (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          ఒకవేళ (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            ఒకవేళ (CALL_MUNMAP((అక్షరం*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            వెళ్ళు postaction;
          }
          లేకపోతే {
            mchunkptr prev = chunk_minus_స్థానభ్రంశం(p, prevsize);
            psize += prevsize;
            p = prev;
            ఒకవేళ (RTCHECK(ok_చిరునామా(fm, prev))) { /* consolidate backward */
              ఒకవేళ (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              లేకపోతే ఒకవేళ ((తదుపరి->తల & INUSE_బిట్లు) == INUSE_బిట్లు) {
                fm->dvsize = psize;
                సమితి_free_with_pinuse(p, psize, తదుపరి);
                వెళ్ళు postaction;
              }
            }
            లేకపోతే
              వెళ్ళు erroraction;
          }
        }

        ఒకవేళ (RTCHECK(ok_తదుపరి(p, తదుపరి) && ok_pinuse(తదుపరి))) {
          ఒకవేళ (!cinuse(తదుపరి)) {  /* consolidate forward */
            ఒకవేళ (తదుపరి == fm->శిఖరం) {
              size_t tsize = fm->topsize += psize;
              fm->శిఖరం = p;
              p->తల = tsize | PINUSE_బిట్టు;
              ఒకవేళ (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              ఒకవేళ (should_trim(fm, tsize))
                sys_trim(fm, 0);
              వెళ్ళు postaction;
            }
            లేకపోతే ఒకవేళ (తదుపరి == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, dsize);
              వెళ్ళు postaction;
            }
            లేకపోతే {
              size_t nsize = chunksize(తదుపరి);
              psize += nsize;
              unlink_chunk(fm, తదుపరి, nsize);
              సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, psize);
              ఒకవేళ (p == fm->dv) {
                fm->dvsize = psize;
                వెళ్ళు postaction;
              }
            }
          }
          లేకపోతే
            సమితి_free_with_pinuse(p, psize, తదుపరి);

          ఒకవేళ (is_small(psize)) {
            చొప్పించు_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          లేకపోతే {
            tchunkptr tp = (tchunkptr)p;
            చొప్పించు_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            ఒకవేళ (--fm->release_checks == 0)
              release_వాడని_segments(fm);
          }
          వెళ్ళు postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
#ఒకవేళ !FOOTERS
#నిర్వచనం_రద్దు fm
#అయితే_ముగింపు /* FOOTERS */
}

శూన్య* dlcalloc(size_t n_elements, size_t elem_కొలత) {
  శూన్య* mem;
  size_t req = 0;
  ఒకవేళ (n_elements != 0) {
    req = n_elements * elem_కొలత;
    ఒకవేళ (((n_elements | elem_కొలత) & ~(size_t)0xffff) &&
        (req / n_elements != elem_కొలత))
      req = MAX_కొలత_T; /* force downstream failure on overflow */
  }
  mem = dlmalloc(req);
  ఒకవేళ (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  తిరిగి mem;
}

#అయితే_ముగింపు /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
స్థిర mchunkptr try_realloc_chunk(mstate m, mchunkptr p, size_t nb,
                                   పూర్ణాంకం can_move) {
  mchunkptr newp = 0;
  size_t oldsize = chunksize(p);
  mchunkptr తదుపరి = chunk_plus_స్థానభ్రంశం(p, oldsize);
  ఒకవేళ (RTCHECK(ok_చిరునామా(m, p) && ok_inuse(p) &&
              ok_తదుపరి(p, తదుపరి) && ok_pinuse(తదుపరి))) {
    ఒకవేళ (is_mmapped(p)) {
      newp = mmap_resize(m, p, nb, can_move);
    }
    లేకపోతే ఒకవేళ (oldsize >= nb) {             /* already big enough */
      size_t rsize = oldsize - nb;
      ఒకవేళ (rsize >= MIN_CHUNK_కొలత) {      /* split off remainder */
        mchunkptr r = chunk_plus_స్థానభ్రంశం(p, nb);
        సమితి_inuse(m, p, nb);
        సమితి_inuse(m, r, rsize);
        dispose_chunk(m, r, rsize);
      }
      newp = p;
    }
    లేకపోతే ఒకవేళ (తదుపరి == m->శిఖరం) {  /* extend into top */
      ఒకవేళ (oldsize + m->topsize > nb) {
        size_t newsize = oldsize + m->topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_స్థానభ్రంశం(p, nb);
        సమితి_inuse(m, p, nb);
        newtop->తల = newtopsize |PINUSE_బిట్టు;
        m->శిఖరం = newtop;
        m->topsize = newtopsize;
        newp = p;
      }
    }
    లేకపోతే ఒకవేళ (తదుపరి == m->dv) { /* extend into dv */
      size_t dvs = m->dvsize;
      ఒకవేళ (oldsize + dvs >= nb) {
        size_t dsize = oldsize + dvs - nb;
        ఒకవేళ (dsize >= MIN_CHUNK_కొలత) {
          mchunkptr r = chunk_plus_స్థానభ్రంశం(p, nb);
          mchunkptr n = chunk_plus_స్థానభ్రంశం(r, dsize);
          సమితి_inuse(m, p, nb);
          సమితి_కొలత_and_pinuse_యొక్క_free_chunk(r, dsize);
          clear_pinuse(n);
          m->dvsize = dsize;
          m->dv = r;
        }
        లేకపోతే { /* exhaust dv */
          size_t newsize = oldsize + dvs;
          సమితి_inuse(m, p, newsize);
          m->dvsize = 0;
          m->dv = 0;
        }
        newp = p;
      }
    }
    లేకపోతే ఒకవేళ (!cinuse(తదుపరి)) { /* extend into next free chunk */
      size_t nextsize = chunksize(తదుపరి);
      ఒకవేళ (oldsize + nextsize >= nb) {
        size_t rsize = oldsize + nextsize - nb;
        unlink_chunk(m, తదుపరి, nextsize);
        ఒకవేళ (rsize < MIN_CHUNK_కొలత) {
          size_t newsize = oldsize + nextsize;
          సమితి_inuse(m, p, newsize);
        }
        లేకపోతే {
          mchunkptr r = chunk_plus_స్థానభ్రంశం(p, nb);
          సమితి_inuse(m, p, nb);
          సమితి_inuse(m, r, rsize);
          dispose_chunk(m, r, rsize);
        }
        newp = p;
      }
    }
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(m, chunk2mem(p));
  }
  తిరిగి newp;
}

స్థిర శూన్య* అంతర్గత_memalign(mstate m, size_t alignment, size_t బైట్లు) {
  శూన్య* mem = 0;
  ఒకవేళ (alignment <  MIN_CHUNK_కొలత) /* must be at least a minimum chunk size */
    alignment = MIN_CHUNK_కొలత;
  ఒకవేళ ((alignment & (alignment-కొలత_T_ONE)) != 0) {/* Ensure a power of 2 */
    size_t a = MALLOC_ALIGNMENT << 1;
    అయితే (a < alignment) a <<= 1;
    alignment = a;
  }
  ఒకవేళ (బైట్లు >= MAX_REQUEST - alignment) {
    ఒకవేళ (m != 0)  { /* Test isn't needed but avoids compiler warning */
      MALLOC_FAILURE_ACTION;
    }
  }
  లేకపోతే {
    size_t nb = request2size(బైట్లు);
    size_t req = nb + alignment + MIN_CHUNK_కొలత - CHUNK_OVERHEAD;
    mem = అంతర్గత_malloc(m, req);
    ఒకవేళ (mem != 0) {
      mchunkptr p = mem2chunk(mem);
      ఒకవేళ (PREACTION(m))
        తిరిగి 0;
      ఒకవేళ ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
        అక్షరం* br = (అక్షరం*)mem2chunk((size_t)(((size_t)((అక్షరం*)mem + alignment -
                                                       కొలత_T_ONE)) &
                                             -alignment));
        అక్షరం* pos = ((size_t)(br - (అక్షరం*)(p)) >= MIN_CHUNK_కొలత)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (అక్షరం*)(p);
        size_t newsize = chunksize(p) - leadsize;

        ఒకవేళ (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
          newp->prev_foot = p->prev_foot + leadsize;
          newp->తల = newsize;
        }
        లేకపోతే { /* Otherwise, give back leader, use the rest */
          సమితి_inuse(m, newp, newsize);
          సమితి_inuse(m, p, leadsize);
          dispose_chunk(m, p, leadsize);
        }
        p = newp;
      }

      /* Give back spare room at the end */
      ఒకవేళ (!is_mmapped(p)) {
        size_t కొలత = chunksize(p);
        ఒకవేళ (కొలత > nb + MIN_CHUNK_కొలత) {
          size_t remainder_కొలత = కొలత - nb;
          mchunkptr remainder = chunk_plus_స్థానభ్రంశం(p, nb);
          సమితి_inuse(m, p, nb);
          సమితి_inuse(m, remainder, remainder_కొలత);
          dispose_chunk(m, remainder, remainder_కొలత);
        }
      }

      mem = chunk2mem(p);
      ప్రతిజ్ఞా (chunksize(p) >= nb);
      ప్రతిజ్ఞా(((size_t)mem & (alignment - 1)) == 0);
      check_inuse_chunk(m, p);
      POSTACTION(m);
    }
  }
  తిరిగి mem;
}

/*
  Common support for independent_X routines, handling
    all of the combinations that can result.
  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/
స్థిర శూన్య** ialloc(mstate m,
                     size_t n_elements,
                     size_t* sizes,
                     పూర్ణాంకం opts,
                     శూన్య* chunks[]) {

  size_t    element_కొలత;   /* chunksize of each element, if all same */
  size_t    contents_కొలత;  /* total size of elements */
  size_t    array_కొలత;     /* request size of pointer array */
  శూన్య*     mem;            /* malloced aggregate space */
  mchunkptr p;              /* corresponding chunk */
  size_t    remainder_కొలత; /* remaining bytes while splitting */
  శూన్య**    marray;         /* either "chunks" or malloced ptr array */
  mchunkptr array_chunk;    /* chunk for malloced ptr array */
  flag_t    was_enabled;    /* to disable mmap */
  size_t    కొలత;
  size_t    i;

  ensure_initialization();
  /* compute array length, if needed */
  ఒకవేళ (chunks != 0) {
    ఒకవేళ (n_elements == 0)
      తిరిగి chunks; /* nothing to do */
    marray = chunks;
    array_కొలత = 0;
  }
  లేకపోతే {
    /* if empty req, must still return chunk representing empty array */
    ఒకవేళ (n_elements == 0)
      తిరిగి (శూన్య**)అంతర్గత_malloc(m, 0);
    marray = 0;
    array_కొలత = request2size(n_elements * (పరిమాణం(శూన్య*)));
  }

  /* compute total element size */
  ఒకవేళ (opts & 0x1) { /* all-same-size */
    element_కొలత = request2size(*sizes);
    contents_కొలత = n_elements * element_కొలత;
  }
  లేకపోతే { /* add up all the sizes */
    element_కొలత = 0;
    contents_కొలత = 0;
    కొరకు (i = 0; i != n_elements; ++i)
      contents_కొలత += request2size(sizes[i]);
  }

  కొలత = contents_కొలత + array_కొలత;

  /*
     Allocate the aggregate chunk.  First disable direct-mmapping so
     malloc won't use it, since we would not be able to later
     free/realloc space internal to a segregated mmap region.
  */
  was_enabled = use_mmap(m);
  disable_mmap(m);
  mem = అంతర్గత_malloc(m, కొలత - CHUNK_OVERHEAD);
  ఒకవేళ (was_enabled)
    enable_mmap(m);
  ఒకవేళ (mem == 0)
    తిరిగి 0;

  ఒకవేళ (PREACTION(m)) తిరిగి 0;
  p = mem2chunk(mem);
  remainder_కొలత = chunksize(p);

  ప్రతిజ్ఞా(!is_mmapped(p));

  ఒకవేళ (opts & 0x2) {       /* optionally clear the elements */
    memset((size_t*)mem, 0, remainder_కొలత - కొలత_T_కొలత - array_కొలత);
  }

  /* If not provided, allocate the pointer array as final part of chunk */
  ఒకవేళ (marray == 0) {
    size_t  array_chunk_కొలత;
    array_chunk = chunk_plus_స్థానభ్రంశం(p, contents_కొలత);
    array_chunk_కొలత = remainder_కొలత - contents_కొలత;
    marray = (శూన్య**) (chunk2mem(array_chunk));
    సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, array_chunk, array_chunk_కొలత);
    remainder_కొలత = contents_కొలత;
  }

  /* split out elements */
  కొరకు (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    ఒకవేళ (i != n_elements-1) {
      ఒకవేళ (element_కొలత != 0)
        కొలత = element_కొలత;
      లేకపోతే
        కొలత = request2size(sizes[i]);
      remainder_కొలత -= కొలత;
      సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, p, కొలత);
      p = chunk_plus_స్థానభ్రంశం(p, కొలత);
    }
    లేకపోతే { /* the final element absorbs any overallocation slop */
      సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(m, p, remainder_కొలత);
      ఆపు;
    }
  }

#ఒకవేళ DEBUG
  ఒకవేళ (marray != chunks) {
    /* final element must have exactly exhausted chunk */
    ఒకవేళ (element_కొలత != 0) {
      ప్రతిజ్ఞా(remainder_కొలత == element_కొలత);
    }
    లేకపోతే {
      ప్రతిజ్ఞా(remainder_కొలత == request2size(sizes[i]));
    }
    check_inuse_chunk(m, mem2chunk(marray));
  }
  కొరకు (i = 0; i != n_elements; ++i)
    check_inuse_chunk(m, mem2chunk(marray[i]));

#అయితే_ముగింపు /* DEBUG */

  POSTACTION(m);
  తిరిగి marray;
}

/* Try to free all pointers in the given array.
   Note: this could be made faster, by delaying consolidation,
   at the price of disabling some user integrity checks, We
   still optimize some consolidations by combining adjacent
   chunks before freeing, which will occur often if allocated
   with ialloc or the array is sorted.
*/
స్థిర size_t అంతర్గత_bulk_free(mstate m, శూన్య* array[], size_t nelem) {
  size_t unfreed = 0;
  ఒకవేళ (!PREACTION(m)) {
    శూన్య** a;
    శూన్య** fence = &(array[nelem]);
    కొరకు (a = array; a != fence; ++a) {
      శూన్య* mem = *a;
      ఒకవేళ (mem != 0) {
        mchunkptr p = mem2chunk(mem);
        size_t psize = chunksize(p);
#ఒకవేళ FOOTERS
        ఒకవేళ (get_mstate_for(p) != m) {
          ++unfreed;
          కొనసాగు;
        }
#అయితే_ముగింపు
        check_inuse_chunk(m, p);
        *a = 0;
        ఒకవేళ (RTCHECK(ok_చిరునామా(m, p) && ok_inuse(p))) {
          శూన్య ** b = a + 1; /* try to merge with next chunk */
          mchunkptr తదుపరి = తదుపరి_chunk(p);
          ఒకవేళ (b != fence && *b == chunk2mem(తదుపరి)) {
            size_t newsize = chunksize(తదుపరి) + psize;
            సమితి_inuse(m, p, newsize);
            *b = chunk2mem(p);
          }
          లేకపోతే
            dispose_chunk(m, p, psize);
        }
        లేకపోతే {
          CORRUPTION_ERROR_ACTION(m);
          ఆపు;
        }
      }
    }
    ఒకవేళ (should_trim(m, m->topsize))
      sys_trim(m, 0);
    POSTACTION(m);
  }
  తిరిగి unfreed;
}

/* Traversal */
#ఒకవేళ MALLOC_INSPECT_ALL
స్థిర శూన్య అంతర్గత_inspect_all(mstate m,
                                 శూన్య(*handler)(శూన్య *ప్రారంభం,
                                                శూన్య *చివర,
                                                size_t వాడిన_బైట్లు,
                                                శూన్య* callback_arg),
                                 శూన్య* arg) {
  ఒకవేళ (is_initialized(m)) {
    mchunkptr శిఖరం = m->శిఖరం;
    msegmentptr s;
    కొరకు (s = &m->seg; s != 0; s = s->తదుపరి) {
      mchunkptr q = align_as_chunk(s->పునాది);
      అయితే (segment_holds(s, q) && q->తల != FENCEPOST_తల) {
        mchunkptr తదుపరి = తదుపరి_chunk(q);
        size_t sz = chunksize(q);
        size_t వాడిన;
        శూన్య* ప్రారంభం;
        ఒకవేళ (is_inuse(q)) {
          వాడిన = sz - CHUNK_OVERHEAD; /* must not be mmapped */
          ప్రారంభం = chunk2mem(q);
        }
        లేకపోతే {
          వాడిన = 0;
          ఒకవేళ (is_small(sz)) {     /* offset by possible bookkeeping */
            ప్రారంభం = (శూన్య*)((అక్షరం*)q + పరిమాణం(నిర్మాణం malloc_chunk));
          }
          లేకపోతే {
            ప్రారంభం = (శూన్య*)((అక్షరం*)q + పరిమాణం(నిర్మాణం malloc_వృక్షం_chunk));
          }
        }
        ఒకవేళ (ప్రారంభం < (శూన్య*)తదుపరి)  /* skip if all space is bookkeeping */
          handler(ప్రారంభం, తదుపరి, వాడిన, arg);
        ఒకవేళ (q == శిఖరం)
          ఆపు;
        q = తదుపరి;
      }
    }
  }
}
#అయితే_ముగింపు /* MALLOC_INSPECT_ALL */

/* ------------------ Exported realloc, memalign, etc -------------------- */

#ఒకవేళ !ONLY_MSPACES

శూన్య* dlrealloc(శూన్య* oldmem, size_t బైట్లు) {
  శూన్య* mem = 0;
  ఒకవేళ (oldmem == 0) {
    mem = dlmalloc(బైట్లు);
  }
  లేకపోతే ఒకవేళ (బైట్లు >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#నిర్వచితమైతే REALLOC_ZERO_బైట్లు_FREES
  లేకపోతే ఒకవేళ (బైట్లు == 0) {
    dlfree(oldmem);
  }
#అయితే_ముగింపు /* REALLOC_ZERO_BYTES_FREES */
  లేకపోతే {
    size_t nb = request2size(బైట్లు);
    mchunkptr oldp = mem2chunk(oldmem);
#ఒకవేళ ! FOOTERS
    mstate m = gm;
#లేకపోతే /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    ఒకవేళ (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      తిరిగి 0;
    }
#అయితే_ముగింపు /* FOOTERS */
    ఒకవేళ (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      ఒకవేళ (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      లేకపోతే {
        mem = అంతర్గత_malloc(m, బైట్లు);
        ఒకవేళ (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          జ్ఞాపకం_నకలు(mem, oldmem, (oc < బైట్లు)? oc : బైట్లు);
          అంతర్గత_free(m, oldmem);
        }
      }
    }
  }
  తిరిగి mem;
}

శూన్య* dlrealloc_లోపల_ఉంచు(శూన్య* oldmem, size_t బైట్లు) {
  శూన్య* mem = 0;
  ఒకవేళ (oldmem != 0) {
    ఒకవేళ (బైట్లు >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    లేకపోతే {
      size_t nb = request2size(బైట్లు);
      mchunkptr oldp = mem2chunk(oldmem);
#ఒకవేళ ! FOOTERS
      mstate m = gm;
#లేకపోతే /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      ఒకవేళ (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        తిరిగి 0;
      }
#అయితే_ముగింపు /* FOOTERS */
      ఒకవేళ (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        ఒకవేళ (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  తిరిగి mem;
}

శూన్య* dlmemalign(size_t alignment, size_t బైట్లు) {
  ఒకవేళ (alignment <= MALLOC_ALIGNMENT) {
    తిరిగి dlmalloc(బైట్లు);
  }
  తిరిగి అంతర్గత_memalign(gm, alignment, బైట్లు);
}

పూర్ణాంకం dlposix_memalign(శూన్య** pp, size_t alignment, size_t బైట్లు) {
  శూన్య* mem = 0;
  ఒకవేళ (alignment == MALLOC_ALIGNMENT)
    mem = dlmalloc(బైట్లు);
  లేకపోతే {
    size_t d = alignment / పరిమాణం(శూన్య*);
    size_t r = alignment % పరిమాణం(శూన్య*);
    ఒకవేళ (r != 0 || d == 0 || (d & (d-కొలత_T_ONE)) != 0)
      తిరిగి EINVAL;
    లేకపోతే ఒకవేళ (బైట్లు <= MAX_REQUEST - alignment) {
      ఒకవేళ (alignment <  MIN_CHUNK_కొలత)
        alignment = MIN_CHUNK_కొలత;
      mem = అంతర్గత_memalign(gm, alignment, బైట్లు);
    }
  }
  ఒకవేళ (mem == 0)
    తిరిగి ENOMEM;
  లేకపోతే {
    *pp = mem;
    తిరిగి 0;
  }
}

శూన్య* dlvalloc(size_t బైట్లు) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.పుట_కొలత;
  తిరిగి dlmemalign(pagesz, బైట్లు);
}

శూన్య* dlpvalloc(size_t బైట్లు) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.పుట_కొలత;
  తిరిగి dlmemalign(pagesz, (బైట్లు + pagesz - కొలత_T_ONE) & ~(pagesz - కొలత_T_ONE));
}

శూన్య** dlindependent_calloc(size_t n_elements, size_t elem_కొలత,
                            శూన్య* chunks[]) {
  size_t sz = elem_కొలత; /* serves as 1-element array */
  తిరిగి ialloc(gm, n_elements, &sz, 3, chunks);
}

శూన్య** dlindependent_comalloc(size_t n_elements, size_t sizes[],
                              శూన్య* chunks[]) {
  తిరిగి ialloc(gm, n_elements, sizes, 0, chunks);
}

size_t dlbulk_free(శూన్య* array[], size_t nelem) {
  తిరిగి అంతర్గత_bulk_free(gm, array, nelem);
}

#ఒకవేళ MALLOC_INSPECT_ALL
శూన్య dlmalloc_inspect_all(శూన్య(*handler)(శూన్య *ప్రారంభం,
                                         శూన్య *చివర,
                                         size_t వాడిన_బైట్లు,
                                         శూన్య* callback_arg),
                          శూన్య* arg) {
  ensure_initialization();
  ఒకవేళ (!PREACTION(gm)) {
    అంతర్గత_inspect_all(gm, handler, arg);
    POSTACTION(gm);
  }
}
#అయితే_ముగింపు /* MALLOC_INSPECT_ALL */

పూర్ణాంకం dlmalloc_trim(size_t pad) {
  పూర్ణాంకం ఫలితం = 0;
  ensure_initialization();
  ఒకవేళ (!PREACTION(gm)) {
    ఫలితం = sys_trim(gm, pad);
    POSTACTION(gm);
  }
  తిరిగి ఫలితం;
}

size_t dlmalloc_footprint(శూన్య) {
  తిరిగి gm->footprint;
}

size_t dlmalloc_max_footprint(శూన్య) {
  తిరిగి gm->max_footprint;
}

size_t dlmalloc_footprint_హద్దు(శూన్య) {
  size_t maf = gm->footprint_హద్దు;
  తిరిగి maf == 0 ? MAX_కొలత_T : maf;
}

size_t dlmalloc_సమితి_footprint_హద్దు(size_t బైట్లు) {
  size_t ఫలితం;  /* invert sense of 0 */
  ఒకవేళ (బైట్లు == 0)
    ఫలితం = granularity_align(1); /* Use minimal size */
  ఒకవేళ (బైట్లు == MAX_కొలత_T)
    ఫలితం = 0;                    /* disable */
  లేకపోతే
    ఫలితం = granularity_align(బైట్లు);
  తిరిగి gm->footprint_హద్దు = ఫలితం;
}

#ఒకవేళ !NO_MALLINFO
నిర్మాణం mallinfo dlmallinfo(శూన్య) {
  తిరిగి అంతర్గత_mallinfo(gm);
}
#అయితే_ముగింపు /* NO_MALLINFO */

#ఒకవేళ !NO_MALLOC_STATS
శూన్య dlmalloc_stats(శూన్య) {
  అంతర్గత_malloc_stats(gm);
}
#అయితే_ముగింపు /* NO_MALLOC_STATS */

పూర్ణాంకం dlmallopt(పూర్ణాంకం param_సంఖ్య, పూర్ణాంకం విలువ) {
  తిరిగి change_mparam(param_సంఖ్య, విలువ);
}

size_t dlmalloc_usable_కొలత(శూన్య* mem) {
  ఒకవేళ (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    ఒకవేళ (is_inuse(p))
      తిరిగి chunksize(p) - overhead_for(p);
  }
  తిరిగి 0;
}

#అయితే_ముగింపు /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#ఒకవేళ MSPACES

స్థిర mstate init_user_mstate(అక్షరం* tbase, size_t tsize) {
  size_t msize = pad_request(పరిమాణం(నిర్మాణం malloc_state));
  mchunkptr mn;
  mchunkptr msp = align_as_chunk(tbase);
  mstate m = (mstate)(chunk2mem(msp));
  memset(m, 0, msize);
  (శూన్య)INITIAL_తాళం(&m->mutex);
  msp->తల = (msize|INUSE_బిట్లు);
  m->seg.పునాది = m->కనిష్ఠ_addr = tbase;
  m->seg.కొలత = m->footprint = m->max_footprint = tsize;
  m->magic = mparams.magic;
  m->release_checks = MAX_RELEASE_CHECK_రేటు;
  m->mflags = mparams.default_mflags;
  m->extp = 0;
  m->exts = 0;
  disable_contiguous(m);
  init_bins(m);
  mn = తదుపరి_chunk(mem2chunk(m));
  init_శిఖరం(m, mn, (size_t)((tbase + tsize) - (అక్షరం*)mn) - శిఖరం_FOOT_కొలత);
  check_శిఖరం_chunk(m, m->శిఖరం);
  తిరిగి m;
}

mspace create_mspace(size_t సామర్థ్యం, పూర్ణాంకం locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(పరిమాణం(నిర్మాణం malloc_state));
  ఒకవేళ (సామర్థ్యం < (size_t) -(msize + శిఖరం_FOOT_కొలత + mparams.పుట_కొలత)) {
    size_t rs = ((సామర్థ్యం == 0)? mparams.granularity :
                 (సామర్థ్యం + శిఖరం_FOOT_కొలత + msize));
    size_t tsize = granularity_align(rs);
    అక్షరం* tbase = (అక్షరం*)(CALL_MMAP(tsize));
    ఒకవేళ (tbase != CMFAIL) {
      m = init_user_mstate(tbase, tsize);
      సమితి_segment_flags(&m->seg, USE_MMAP_బిట్టు);
      సమితి_తాళం(m, locked);
    }
  }
  తిరిగి (mspace)m;
}

mspace create_mspace_with_పునాది(శూన్య* పునాది, size_t సామర్థ్యం, పూర్ణాంకం locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(పరిమాణం(నిర్మాణం malloc_state));
  ఒకవేళ (సామర్థ్యం > msize + శిఖరం_FOOT_కొలత &&
      సామర్థ్యం < (size_t) -(msize + శిఖరం_FOOT_కొలత + mparams.పుట_కొలత)) {
    m = init_user_mstate((అక్షరం*)పునాది, సామర్థ్యం);
    సమితి_segment_flags(&m->seg, EXTERN_బిట్టు);
    సమితి_తాళం(m, locked);
  }
  తిరిగి (mspace)m;
}

పూర్ణాంకం mspace_track_large_chunks(mspace msp, పూర్ణాంకం enable) {
  పూర్ణాంకం ret = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (!PREACTION(ms)) {
    ఒకవేళ (!use_mmap(ms)) {
      ret = 1;
    }
    ఒకవేళ (!enable) {
      enable_mmap(ms);
    } లేకపోతే {
      disable_mmap(ms);
    }
    POSTACTION(ms);
  }
  తిరిగి ret;
}

size_t destroy_mspace(mspace msp) {
  size_t freed = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    msegmentptr sp = &ms->seg;
    (శూన్య)DESTROY_తాళం(&ms->mutex); /* destroy before unmapped */
    అయితే (sp != 0) {
      అక్షరం* పునాది = sp->పునాది;
      size_t కొలత = sp->కొలత;
      flag_t flag = get_segment_flags(sp);
      (శూన్య)పునాది; /* placate people compiling -Wunused-variable */
      sp = sp->తదుపరి;
      ఒకవేళ ((flag & USE_MMAP_బిట్టు) && !(flag & EXTERN_బిట్టు) &&
          CALL_MUNMAP(పునాది, కొలత) == 0)
        freed += కొలత;
    }
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
  తిరిగి freed;
}

/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

శూన్య* mspace_malloc(mspace msp, size_t బైట్లు) {
  mstate ms = (mstate)msp;
  ఒకవేళ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    తిరిగి 0;
  }
  ఒకవేళ (!PREACTION(ms)) {
    శూన్య* mem;
    size_t nb;
    ఒకవేళ (బైట్లు <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (బైట్లు < MIN_REQUEST)? MIN_CHUNK_కొలత : pad_request(బైట్లు);
      idx = small_సూచీ(nb);
      smallbits = ms->smallmap >> idx;

      ఒకవేళ ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(ms, idx);
        p = b->fd;
        ప్రతిజ్ఞా(chunksize(p) == small_index2size(idx));
        unlink_మొదటి_small_chunk(ms, b, p, idx);
        సమితి_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        వెళ్ళు postaction;
      }

      లేకపోతే ఒకవేళ (nb > ms->dvsize) {
        ఒకవేళ (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & ఎడమ_బిట్లు(idx2bit(idx));
          binmap_t leastbit = కనిష్ఠ_బిట్టు(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b->fd;
          ప్రతిజ్ఞా(chunksize(p) == small_index2size(i));
          unlink_మొదటి_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          ఒకవేళ (కొలత_T_కొలత != 4 && rsize < MIN_CHUNK_కొలత)
            సమితి_inuse_and_pinuse(ms, p, small_index2size(i));
          లేకపోతే {
            సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(ms, p, nb);
            r = chunk_plus_స్థానభ్రంశం(p, nb);
            సమితి_కొలత_and_pinuse_యొక్క_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          వెళ్ళు postaction;
        }

        లేకపోతే ఒకవేళ (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
          check_malloced_chunk(ms, mem, nb);
          వెళ్ళు postaction;
        }
      }
    }
    లేకపోతే ఒకవేళ (బైట్లు >= MAX_REQUEST)
      nb = MAX_కొలత_T; /* Too big to allocate. Force failure (in sys alloc) */
    లేకపోతే {
      nb = pad_request(బైట్లు);
      ఒకవేళ (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
        check_malloced_chunk(ms, mem, nb);
        వెళ్ళు postaction;
      }
    }

    ఒకవేళ (nb <= ms->dvsize) {
      size_t rsize = ms->dvsize - nb;
      mchunkptr p = ms->dv;
      ఒకవేళ (rsize >= MIN_CHUNK_కొలత) { /* split dv */
        mchunkptr r = ms->dv = chunk_plus_స్థానభ్రంశం(p, nb);
        ms->dvsize = rsize;
        సమితి_కొలత_and_pinuse_యొక్క_free_chunk(r, rsize);
        సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(ms, p, nb);
      }
      లేకపోతే { /* exhaust dv */
        size_t dvs = ms->dvsize;
        ms->dvsize = 0;
        ms->dv = 0;
        సమితి_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      వెళ్ళు postaction;
    }

    లేకపోతే ఒకవేళ (nb < ms->topsize) { /* Split top */
      size_t rsize = ms->topsize -= nb;
      mchunkptr p = ms->శిఖరం;
      mchunkptr r = ms->శిఖరం = chunk_plus_స్థానభ్రంశం(p, nb);
      r->తల = rsize | PINUSE_బిట్టు;
      సమితి_కొలత_and_pinuse_యొక్క_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_శిఖరం_chunk(ms, ms->శిఖరం);
      check_malloced_chunk(ms, mem, nb);
      వెళ్ళు postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
    POSTACTION(ms);
    తిరిగి mem;
  }

  తిరిగి 0;
}

శూన్య mspace_free(mspace msp, శూన్య* mem) {
  ఒకవేళ (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#ఒకవేళ FOOTERS
    mstate fm = get_mstate_for(p);
    (శూన్య)msp; /* placate people compiling -Wunused */
#లేకపోతే /* FOOTERS */
    mstate fm = (mstate)msp;
#అయితే_ముగింపు /* FOOTERS */
    ఒకవేళ (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      తిరిగి;
    }
    ఒకవేళ (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      ఒకవేళ (RTCHECK(ok_చిరునామా(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr తదుపరి = chunk_plus_స్థానభ్రంశం(p, psize);
        ఒకవేళ (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          ఒకవేళ (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            ఒకవేళ (CALL_MUNMAP((అక్షరం*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            వెళ్ళు postaction;
          }
          లేకపోతే {
            mchunkptr prev = chunk_minus_స్థానభ్రంశం(p, prevsize);
            psize += prevsize;
            p = prev;
            ఒకవేళ (RTCHECK(ok_చిరునామా(fm, prev))) { /* consolidate backward */
              ఒకవేళ (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              లేకపోతే ఒకవేళ ((తదుపరి->తల & INUSE_బిట్లు) == INUSE_బిట్లు) {
                fm->dvsize = psize;
                సమితి_free_with_pinuse(p, psize, తదుపరి);
                వెళ్ళు postaction;
              }
            }
            లేకపోతే
              వెళ్ళు erroraction;
          }
        }

        ఒకవేళ (RTCHECK(ok_తదుపరి(p, తదుపరి) && ok_pinuse(తదుపరి))) {
          ఒకవేళ (!cinuse(తదుపరి)) {  /* consolidate forward */
            ఒకవేళ (తదుపరి == fm->శిఖరం) {
              size_t tsize = fm->topsize += psize;
              fm->శిఖరం = p;
              p->తల = tsize | PINUSE_బిట్టు;
              ఒకవేళ (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              ఒకవేళ (should_trim(fm, tsize))
                sys_trim(fm, 0);
              వెళ్ళు postaction;
            }
            లేకపోతే ఒకవేళ (తదుపరి == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, dsize);
              వెళ్ళు postaction;
            }
            లేకపోతే {
              size_t nsize = chunksize(తదుపరి);
              psize += nsize;
              unlink_chunk(fm, తదుపరి, nsize);
              సమితి_కొలత_and_pinuse_యొక్క_free_chunk(p, psize);
              ఒకవేళ (p == fm->dv) {
                fm->dvsize = psize;
                వెళ్ళు postaction;
              }
            }
          }
          లేకపోతే
            సమితి_free_with_pinuse(p, psize, తదుపరి);

          ఒకవేళ (is_small(psize)) {
            చొప్పించు_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          లేకపోతే {
            tchunkptr tp = (tchunkptr)p;
            చొప్పించు_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            ఒకవేళ (--fm->release_checks == 0)
              release_వాడని_segments(fm);
          }
          వెళ్ళు postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
}

శూన్య* mspace_calloc(mspace msp, size_t n_elements, size_t elem_కొలత) {
  శూన్య* mem;
  size_t req = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    తిరిగి 0;
  }
  ఒకవేళ (n_elements != 0) {
    req = n_elements * elem_కొలత;
    ఒకవేళ (((n_elements | elem_కొలత) & ~(size_t)0xffff) &&
        (req / n_elements != elem_కొలత))
      req = MAX_కొలత_T; /* force downstream failure on overflow */
  }
  mem = అంతర్గత_malloc(ms, req);
  ఒకవేళ (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  తిరిగి mem;
}

శూన్య* mspace_realloc(mspace msp, శూన్య* oldmem, size_t బైట్లు) {
  శూన్య* mem = 0;
  ఒకవేళ (oldmem == 0) {
    mem = mspace_malloc(msp, బైట్లు);
  }
  లేకపోతే ఒకవేళ (బైట్లు >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#నిర్వచితమైతే REALLOC_ZERO_బైట్లు_FREES
  లేకపోతే ఒకవేళ (బైట్లు == 0) {
    mspace_free(msp, oldmem);
  }
#అయితే_ముగింపు /* REALLOC_ZERO_BYTES_FREES */
  లేకపోతే {
    size_t nb = request2size(బైట్లు);
    mchunkptr oldp = mem2chunk(oldmem);
#ఒకవేళ ! FOOTERS
    mstate m = (mstate)msp;
#లేకపోతే /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    ఒకవేళ (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      తిరిగి 0;
    }
#అయితే_ముగింపు /* FOOTERS */
    ఒకవేళ (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      ఒకవేళ (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      లేకపోతే {
        mem = mspace_malloc(m, బైట్లు);
        ఒకవేళ (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          జ్ఞాపకం_నకలు(mem, oldmem, (oc < బైట్లు)? oc : బైట్లు);
          mspace_free(m, oldmem);
        }
      }
    }
  }
  తిరిగి mem;
}

శూన్య* mspace_realloc_లోపల_ఉంచు(mspace msp, శూన్య* oldmem, size_t బైట్లు) {
  శూన్య* mem = 0;
  ఒకవేళ (oldmem != 0) {
    ఒకవేళ (బైట్లు >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    లేకపోతే {
      size_t nb = request2size(బైట్లు);
      mchunkptr oldp = mem2chunk(oldmem);
#ఒకవేళ ! FOOTERS
      mstate m = (mstate)msp;
#లేకపోతే /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      (శూన్య)msp; /* placate people compiling -Wunused */
      ఒకవేళ (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        తిరిగి 0;
      }
#అయితే_ముగింపు /* FOOTERS */
      ఒకవేళ (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        ఒకవేళ (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  తిరిగి mem;
}

శూన్య* mspace_memalign(mspace msp, size_t alignment, size_t బైట్లు) {
  mstate ms = (mstate)msp;
  ఒకవేళ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    తిరిగి 0;
  }
  ఒకవేళ (alignment <= MALLOC_ALIGNMENT)
    తిరిగి mspace_malloc(msp, బైట్లు);
  తిరిగి అంతర్గత_memalign(ms, alignment, బైట్లు);
}

శూన్య** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_కొలత, శూన్య* chunks[]) {
  size_t sz = elem_కొలత; /* serves as 1-element array */
  mstate ms = (mstate)msp;
  ఒకవేళ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    తిరిగి 0;
  }
  తిరిగి ialloc(ms, n_elements, &sz, 3, chunks);
}

శూన్య** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], శూన్య* chunks[]) {
  mstate ms = (mstate)msp;
  ఒకవేళ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    తిరిగి 0;
  }
  తిరిగి ialloc(ms, n_elements, sizes, 0, chunks);
}

size_t mspace_bulk_free(mspace msp, శూన్య* array[], size_t nelem) {
  తిరిగి అంతర్గత_bulk_free((mstate)msp, array, nelem);
}

#ఒకవేళ MALLOC_INSPECT_ALL
శూన్య mspace_inspect_all(mspace msp,
                        శూన్య(*handler)(శూన్య *ప్రారంభం,
                                       శూన్య *చివర,
                                       size_t వాడిన_బైట్లు,
                                       శూన్య* callback_arg),
                        శూన్య* arg) {
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    ఒకవేళ (!PREACTION(ms)) {
      అంతర్గత_inspect_all(ms, handler, arg);
      POSTACTION(ms);
    }
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#అయితే_ముగింపు /* MALLOC_INSPECT_ALL */

పూర్ణాంకం mspace_trim(mspace msp, size_t pad) {
  పూర్ణాంకం ఫలితం = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    ఒకవేళ (!PREACTION(ms)) {
      ఫలితం = sys_trim(ms, pad);
      POSTACTION(ms);
    }
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
  తిరిగి ఫలితం;
}

#ఒకవేళ !NO_MALLOC_STATS
శూన్య mspace_malloc_stats(mspace msp) {
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    అంతర్గత_malloc_stats(ms);
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#అయితే_ముగింపు /* NO_MALLOC_STATS */

size_t mspace_footprint(mspace msp) {
  size_t ఫలితం = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    ఫలితం = ms->footprint;
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
  తిరిగి ఫలితం;
}

size_t mspace_max_footprint(mspace msp) {
  size_t ఫలితం = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    ఫలితం = ms->max_footprint;
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
  తిరిగి ఫలితం;
}

size_t mspace_footprint_హద్దు(mspace msp) {
  size_t ఫలితం = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    size_t maf = ms->footprint_హద్దు;
    ఫలితం = (maf == 0) ? MAX_కొలత_T : maf;
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
  తిరిగి ఫలితం;
}

size_t mspace_సమితి_footprint_హద్దు(mspace msp, size_t బైట్లు) {
  size_t ఫలితం = 0;
  mstate ms = (mstate)msp;
  ఒకవేళ (ok_magic(ms)) {
    ఒకవేళ (బైట్లు == 0)
      ఫలితం = granularity_align(1); /* Use minimal size */
    ఒకవేళ (బైట్లు == MAX_కొలత_T)
      ఫలితం = 0;                    /* disable */
    లేకపోతే
      ఫలితం = granularity_align(బైట్లు);
    ms->footprint_హద్దు = ఫలితం;
  }
  లేకపోతే {
    USAGE_ERROR_ACTION(ms,ms);
  }
  తిరిగి ఫలితం;
}

#ఒకవేళ !NO_MALLINFO
నిర్మాణం mallinfo mspace_mallinfo(mspace msp) {
  mstate ms = (mstate)msp;
  ఒకవేళ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
  }
  తిరిగి అంతర్గత_mallinfo(ms);
}
#అయితే_ముగింపు /* NO_MALLINFO */

size_t mspace_usable_కొలత(స్థిరరాశి శూన్య* mem) {
  ఒకవేళ (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    ఒకవేళ (is_inuse(p))
      తిరిగి chunksize(p) - overhead_for(p);
  }
  తిరిగి 0;
}

పూర్ణాంకం mspace_mallopt(పూర్ణాంకం param_సంఖ్య, పూర్ణాంకం విలువ) {
  తిరిగి change_mparam(param_సంఖ్య, విలువ);
}

#అయితే_ముగింపు /* MSPACES */


/* -------------------- Alternative MORECORE functions ------------------- */

/*
  Guidelines for creating a custom version of MORECORE:

  * For best performance, MORECORE should allocate in multiples of pagesize.
  * MORECORE may allocate more memory than requested. (Or even less,
      but this will usually result in a malloc failure.)
  * MORECORE must not allocate memory when given argument zero, but
      instead return one past the end address of memory from previous
      nonzero call.
  * For best performance, consecutive calls to MORECORE with positive
      arguments should return increasing addresses, indicating that
      space has been contiguously extended.
  * Even though consecutive calls to MORECORE need not return contiguous
      addresses, it must be OK for malloc'ed chunks to span multiple
      regions in those cases where they do happen to be contiguous.
  * MORECORE need not handle negative arguments -- it may instead
      just return MFAIL when given negative arguments.
      Negative arguments are always multiples of pagesize. MORECORE
      must not misinterpret negative args as large positive unsigned
      args. You can suppress all such calls from even occurring by defining
      MORECORE_CANNOT_TRIM,

  As an example alternative MORECORE, here is a custom allocator
  kindly contributed for pre-OSX macOS.  It uses virtually but not
  necessarily physically contiguous non-paged memory (locked in,
  present and won't get swapped out).  You can use it by uncommenting
  this section, adding some #includes, and setting up the appropriate
  defines above:

      #define MORECORE osMoreCore

  There is also a shutdown routine that should somehow be called for
  cleanup upon program exit.

  #define MAX_POOL_ENTRIES 100
  #define MINIMUM_MORECORE_SIZE  (64 * 1024U)
  static int next_os_pool;
  void *our_os_pools[MAX_POOL_ENTRIES];

  void *osMoreCore(int size)
  {
    void *ptr = 0;
    static void *sbrk_top = 0;

    if (size > 0)
    {
      if (size < MINIMUM_MORECORE_SIZE)
         size = MINIMUM_MORECORE_SIZE;
      if (CurrentExecutionLevel() == kTaskLevel)
         ptr = PoolAllocateResident(size + RM_PAGE_SIZE, 0);
      if (ptr == 0)
      {
        return (void *) MFAIL;
      }
      // save ptrs so they can be freed during cleanup
      our_os_pools[next_os_pool] = ptr;
      next_os_pool++;
      ptr = (void *) ((((size_t) ptr) + RM_PAGE_MASK) & ~RM_PAGE_MASK);
      sbrk_top = (char *) ptr + size;
      return ptr;
    }
    else if (size < 0)
    {
      // we don't currently support shrink behavior
      return (void *) MFAIL;
    }
    else
    {
      return sbrk_top;
    }
  }

  // cleanup any allocated memory pools
  // called as last thing before shutting down driver

  void osCleanupMem(void)
  {
    void **ptr;

    for (ptr = our_os_pools; ptr < &our_os_pools[MAX_POOL_ENTRIES]; ptr++)
      if (*ptr)
      {
         PoolDeallocate(*ptr);
         *ptr = 0;
      }
  }

*/


/* -----------------------------------------------------------------------
History:
    v2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea
      * fix bad comparison in dlposix_memalign
      * don't reuse adjusted asize in sys_alloc
      * add LOCK_AT_FORK -- thanks to Kirill Artamonov for the suggestion
      * reduce compiler warnings -- thanks to all who reported/suggested these

    v2.8.5 Sun May 22 10:26:02 2011  Doug Lea  (dl at gee)
      * Always perform unlink checks unless INSECURE
      * Add posix_memalign.
      * Improve realloc to expand in more cases; expose realloc_in_place.
        Thanks to Peter Buhr for the suggestion.
      * Add footprint_limit, inspect_all, bulk_free. Thanks
        to Barry Hayes and others for the suggestions.
      * Internal refactorings to avoid calls while holding locks
      * Use non-reentrant locks by default. Thanks to Roland McGrath
        for the suggestion.
      * Small fixes to mspace_destroy, reset_on_error.
      * Various configuration extensions/changes. Thanks
         to all who contributed these.

    V2.8.4a Thu Apr 28 14:39:43 2011 (dl at gee.cs.oswego.edu)
      * Update Creative Commons URL

    V2.8.4 Wed May 27 09:56:23 2009  Doug Lea  (dl at gee)
      * Use zeros instead of prev foot for is_mmapped
      * Add mspace_track_large_chunks; thanks to Jean Brouwers
      * Fix set_inuse in internal_realloc; thanks to Jean Brouwers
      * Fix insufficient sys_alloc padding when using 16byte alignment
      * Fix bad error check in mspace_footprint
      * Adaptations for ptmalloc; thanks to Wolfram Gloger.
      * Reentrant spin locks; thanks to Earl Chew and others
      * Win32 improvements; thanks to Niall Douglas and Earl Chew
      * Add NO_SEGMENT_TRAVERSAL and MAX_RELEASE_CHECK_RATE options
      * Extension hook in malloc_state
      * Various small adjustments to reduce warnings on some compilers
      * Various configuration extensions/changes for more platforms. Thanks
         to all who contributed these.

    V2.8.3 Thu Sep 22 11:16:32 2005  Doug Lea  (dl at gee)
      * Add max_footprint functions
      * Ensure all appropriate literals are size_t
      * Fix conditional compilation problem for some #define settings
      * Avoid concatenating segments with the one provided
        in create_mspace_with_base
      * Rename some variables to avoid compiler shadowing warnings
      * Use explicit lock initialization.
      * Better handling of sbrk interference.
      * Simplify and fix segment insertion, trimming and mspace_destroy
      * Reinstate REALLOC_ZERO_BYTES_FREES option from 2.7.x
      * Thanks especially to Dennis Flanagan for help on these.

    V2.8.2 Sun Jun 12 16:01:10 2005  Doug Lea  (dl at gee)
      * Fix memalign brace error.

    V2.8.1 Wed Jun  8 16:11:46 2005  Doug Lea  (dl at gee)
      * Fix improper #endif nesting in C++
      * Add explicit casts needed for C++

    V2.8.0 Mon May 30 14:09:02 2005  Doug Lea  (dl at gee)
      * Use trees for large bins
      * Support mspaces
      * Use segments to unify sbrk-based and mmap-based system allocation,
        removing need for emulation on most platforms without sbrk.
      * Default safety checks
      * Optional footer checks. Thanks to William Robertson for the idea.
      * Internal code refactoring
      * Incorporate suggestions and platform-specific changes.
        Thanks to Dennis Flanagan, Colin Plumb, Niall Douglas,
        Aaron Bachmann,  Emery Berger, and others.
      * Speed up non-fastbin processing enough to remove fastbins.
      * Remove useless cfree() to avoid conflicts with other apps.
      * Remove internal memcpy, memset. Compilers handle builtins better.
      * Remove some options that no one ever used and rename others.

    V2.7.2 Sat Aug 17 09:07:30 2002  Doug Lea  (dl at gee)
      * Fix malloc_state bitmap array misdeclaration

    V2.7.1 Thu Jul 25 10:58:03 2002  Doug Lea  (dl at gee)
      * Allow tuning of FIRST_SORTED_BIN_SIZE
      * Use PTR_UINT as type for all ptr->int casts. Thanks to John Belmonte.
      * Better detection and support for non-contiguousness of MORECORE.
        Thanks to Andreas Mueller, Conal Walsh, and Wolfram Gloger
      * Bypass most of malloc if no frees. Thanks To Emery Berger.
      * Fix freeing of old top non-contiguous chunk im sysmalloc.
      * Raised default trim and map thresholds to 256K.
      * Fix mmap-related #defines. Thanks to Lubos Lunak.
      * Fix copy macros; added LACKS_FCNTL_H. Thanks to Neal Walfield.
      * Branch-free bin calculation
      * Default trim and mmap thresholds now 256K.

    V2.7.0 Sun Mar 11 14:14:06 2001  Doug Lea  (dl at gee)
      * Introduce independent_comalloc and independent_calloc.
        Thanks to Michael Pachos for motivation and help.
      * Make optional .h file available
      * Allow > 2GB requests on 32bit systems.
      * new WIN32 sbrk, mmap, munmap, lock code from <Walter@GeNeSys-e.de>.
        Thanks also to Andreas Mueller <a.mueller at paradatec.de>,
        and Anonymous.
      * Allow override of MALLOC_ALIGNMENT (Thanks to Ruud Waij for
        helping test this.)
      * memalign: check alignment arg
      * realloc: don't try to shift chunks backwards, since this
        leads to  more fragmentation in some programs and doesn't
        seem to help in any others.
      * Collect all cases in malloc requiring system memory into sysmalloc
      * Use mmap as backup to sbrk
      * Place all internal state in malloc_state
      * Introduce fastbins (although similar to 2.5.1)
      * Many minor tunings and cosmetic improvements
      * Introduce USE_PUBLIC_MALLOC_WRAPPERS, USE_MALLOC_LOCK
      * Introduce MALLOC_FAILURE_ACTION, MORECORE_CONTIGUOUS
        Thanks to Tony E. Bennett <tbennett@nvidia.com> and others.
      * Include errno.h to support default failure action.

    V2.6.6 Sun Dec  5 07:42:19 1999  Doug Lea  (dl at gee)
      * return null for negative arguments
      * Added Several WIN32 cleanups from Martin C. Fong <mcfong at yahoo.com>
         * Add 'LACKS_SYS_PARAM_H' for those systems without 'sys/param.h'
          (e.g. WIN32 platforms)
         * Cleanup header file inclusion for WIN32 platforms
         * Cleanup code to avoid Microsoft Visual C++ compiler complaints
         * Add 'USE_DL_PREFIX' to quickly allow co-existence with existing
           memory allocation routines
         * Set 'malloc_getpagesize' for WIN32 platforms (needs more work)
         * Use 'assert' rather than 'ASSERT' in WIN32 code to conform to
           usage of 'assert' in non-WIN32 code
         * Improve WIN32 'sbrk()' emulation's 'findRegion()' routine to
           avoid infinite loop
      * Always call 'fREe()' rather than 'free()'

    V2.6.5 Wed Jun 17 15:57:31 1998  Doug Lea  (dl at gee)
      * Fixed ordering problem with boundary-stamping

    V2.6.3 Sun May 19 08:17:58 1996  Doug Lea  (dl at gee)
      * Added pvalloc, as recommended by H.J. Liu
      * Added 64bit pointer support mainly from Wolfram Gloger
      * Added anonymously donated WIN32 sbrk emulation
      * Malloc, calloc, getpagesize: add optimizations from Raymond Nijssen
      * malloc_extend_top: fix mask error that caused wastage after
        foreign sbrks
      * Add linux mremap support code from HJ Liu

    V2.6.2 Tue Dec  5 06:52:55 1995  Doug Lea  (dl at gee)
      * Integrated most documentation with the code.
      * Add support for mmap, with help from
        Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Use last_remainder in more cases.
      * Pack bins using idea from  colin@nyx10.cs.du.edu
      * Use ordered bins instead of best-fit threshold
      * Eliminate block-local decls to simplify tracing and debugging.
      * Support another case of realloc via move into top
      * Fix error occurring when initial sbrk_base not word-aligned.
      * Rely on page size for units instead of SBRK_UNIT to
        avoid surprises about sbrk alignment conventions.
      * Add mallinfo, mallopt. Thanks to Raymond Nijssen
        (raymond@es.ele.tue.nl) for the suggestion.
      * Add `pad' argument to malloc_trim and top_pad mallopt parameter.
      * More precautions for cases where other routines call sbrk,
        courtesy of Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Added macros etc., allowing use in linux libc from
        H.J. Lu (hjl@gnu.ai.mit.edu)
      * Inverted this history list

    V2.6.1 Sat Dec  2 14:10:57 1995  Doug Lea  (dl at gee)
      * Re-tuned and fixed to behave more nicely with V2.6.0 changes.
      * Removed all preallocation code since under current scheme
        the work required to undo bad preallocations exceeds
        the work saved in good cases for most test programs.
      * No longer use return list or unconsolidated bins since
        no scheme using them consistently outperforms those that don't
        given above changes.
      * Use best fit for very large chunks to prevent some worst-cases.
      * Added some support for debugging

    V2.6.0 Sat Nov  4 07:05:23 1995  Doug Lea  (dl at gee)
      * Removed footers when chunks are in use. Thanks to
        Paul Wilson (wilson@cs.texas.edu) for the suggestion.

    V2.5.4 Wed Nov  1 07:54:51 1995  Doug Lea  (dl at gee)
      * Added malloc_trim, with help from Wolfram Gloger
        (wmglo@Dent.MED.Uni-Muenchen.DE).

    V2.5.3 Tue Apr 26 10:16:01 1994  Doug Lea  (dl at g)

    V2.5.2 Tue Apr  5 16:20:40 1994  Doug Lea  (dl at g)
      * realloc: try to expand in both directions
      * malloc: swap order of clean-bin strategy;
      * realloc: only conditionally expand backwards
      * Try not to scavenge used bins
      * Use bin counts as a guide to preallocation
      * Occasionally bin return list chunks in first scan
      * Add a few optimizations from colin@nyx10.cs.du.edu

    V2.5.1 Sat Aug 14 15:40:43 1993  Doug Lea  (dl at g)
      * faster bin computation & slightly different binning
      * merged all consolidations to one part of malloc proper
         (eliminating old malloc_find_space & malloc_clean_bin)
      * Scan 2 returns chunks (not just 1)
      * Propagate failure in realloc if malloc returns 0
      * Add stuff to allow compilation on non-ANSI compilers
          from kpv@research.att.com

    V2.5 Sat Aug  7 07:41:59 1993  Doug Lea  (dl at g.oswego.edu)
      * removed potential for odd address access in prev_chunk
      * removed dependency on getpagesize.h
      * misc cosmetics and a bit more internal documentation
      * anticosmetics: mangled names in macros to evade debugger strangeness
      * tested on sparc, hp-700, dec-mips, rs6000
          with gcc & native cc (hp, dec only) allowing
          Detlefs & Zorn comparison study (in SIGPLAN Notices.)

    Trial version Fri Aug 28 13:14:29 1992  Doug Lea  (dl at g.oswego.edu)
      * Based loosely on libg++-1.2X malloc. (It retains some of the overall
         structure of old version,  but most details differ.)

*/
