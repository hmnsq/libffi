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
#ifndef DLMALLOC_VERSION
#define DLMALLOC_VERSION 20806
#endif /* DLMALLOC_VERSION */

#ifndef DLMALLOC_EXPORT
#define DLMALLOC_EXPORT بیرونی
#endif

#اگر defined __linux__ && !defined _GNU_SOURCE
/* mremap() on Linux requires this via sys/mman.h */
#define _GNU_SOURCE 1
#endif

#ifndef WIN32
#ifdef _WIN32
#define WIN32 1
#endif  /* _WIN32 */
#ifdef _WIN32_WCE
#define LACKS_FCNTL_H
#define WIN32 1
#endif /* _WIN32_WCE */
#endif  /* WIN32 */
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#شامل <windows.h>
#شامل <tchar.h>
#define HAVE_MMAP 1
#define HAVE_MORECORE 0
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H
#define LACKS_SYS_MMAN_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#define LACKS_SCHED_H
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION
#endif /* MALLOC_FAILURE_ACTION */
#ifndef MMAP_CLEARS
#ifdef _WIN32_WCE /* WINCE reportedly does not clear */
#define MMAP_CLEARS 0
#وگرنه
#define MMAP_CLEARS 1
#endif /* _WIN32_WCE */
#endif /*MMAP_CLEARS */
#endif  /* WIN32 */

#ifdef __OS2__
#define INCL_DOS
#شامل <os2.h>
#define HAVE_MMAP 1
#define HAVE_MORECORE 0
#define LACKS_SYS_MMAN_H
#endif  /* __OS2__ */

#اگر defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#ifndef HAVE_MORECORE
#define HAVE_MORECORE 0
#define HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)16U)
#endif
#endif  /* HAVE_MORECORE */
#endif  /* DARWIN */

#ifndef LACKS_SYS_TYPES_H
#شامل <sys/types.h>  /* For size_t */
#endif  /* LACKS_SYS_TYPES_H */

/* The maximum possible size_t value has all bits set */
#define MAX_SIZE_T           (~(size_t)0)

#ifndef USE_LOCKS /* ensure true if spin or recursive locks set */
#define USE_LOCKS  ((defined(USE_SPIN_LOCKS) && USE_SPIN_LOCKS != 0) || \
                    (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0))
#endif /* USE_LOCKS */

#اگر USE_LOCKS /* Spin locks for gcc >= 4.1, older gcc on x86, MSC >= 1310 */
#اگر ((defined(__GNUC__) &&                                              \
      ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) ||      \
       defined(__i386__) || defined(__x86_64__))) ||                    \
     (defined(_MSC_VER) && _MSC_VER>=1310))
#ifndef USE_SPIN_LOCKS
#define USE_SPIN_LOCKS 1
#endif /* USE_SPIN_LOCKS */
#وگرنهاگر USE_SPIN_LOCKS
#error "USE_SPIN_LOCKS defined without implementation"
#endif /* ... locks available... */
#وگرنهاگر !defined(USE_SPIN_LOCKS)
#define USE_SPIN_LOCKS 0
#endif /* USE_LOCKS */

#ifndef ONLY_MSPACES
#define ONLY_MSPACES 0
#endif  /* ONLY_MSPACES */
#ifndef MSPACES
#اگر ONLY_MSPACES
#define MSPACES 1
#وگرنه   /* ONLY_MSPACES */
#define MSPACES 0
#endif  /* ONLY_MSPACES */
#endif  /* MSPACES */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)(2 * اندازه(تهی *)))
#endif  /* MALLOC_ALIGNMENT */
#ifndef FOOTERS
#define FOOTERS 0
#endif  /* FOOTERS */
#ifndef ABORT
#define ABORT  abort()
#endif  /* ABORT */
#ifndef ABORT_ON_ASSERT_FAILURE
#define ABORT_ON_ASSERT_FAILURE 1
#endif  /* ABORT_ON_ASSERT_FAILURE */
#ifndef PROCEED_ON_ERROR
#define PROCEED_ON_ERROR 0
#endif  /* PROCEED_ON_ERROR */

#ifndef INSECURE
#define INSECURE 0
#endif  /* INSECURE */
#ifndef MALLOC_INSPECT_ALL
#define MALLOC_INSPECT_ALL 0
#endif  /* MALLOC_INSPECT_ALL */
#ifndef HAVE_MMAP
#define HAVE_MMAP 1
#endif  /* HAVE_MMAP */
#ifndef MMAP_CLEARS
#define MMAP_CLEARS 1
#endif  /* MMAP_CLEARS */
#ifndef HAVE_MREMAP
#ifdef linux
#define HAVE_MREMAP 1
#define _GNU_SOURCE /* Turns on mremap() definition */
#وگرنه   /* linux */
#define HAVE_MREMAP 0
#endif  /* linux */
#endif  /* HAVE_MREMAP */
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION  errno = ENOMEM;
#endif  /* MALLOC_FAILURE_ACTION */
#ifndef HAVE_MORECORE
#اگر ONLY_MSPACES
#define HAVE_MORECORE 0
#وگرنه   /* ONLY_MSPACES */
#define HAVE_MORECORE 1
#endif  /* ONLY_MSPACES */
#endif  /* HAVE_MORECORE */
#اگر !HAVE_MORECORE
#define MORECORE_CONTIGUOUS 0
#وگرنه   /* !HAVE_MORECORE */
#define MORECORE_DEFAULT sbrk
#ifndef MORECORE_CONTIGUOUS
#define MORECORE_CONTIGUOUS 1
#endif  /* MORECORE_CONTIGUOUS */
#endif  /* HAVE_MORECORE */
#ifndef DEFAULT_GRANULARITY
#اگر (MORECORE_CONTIGUOUS || defined(WIN32))
#define DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#وگرنه   /* MORECORE_CONTIGUOUS */
#define DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#endif  /* MORECORE_CONTIGUOUS */
#endif  /* DEFAULT_GRANULARITY */
#ifndef DEFAULT_TRIM_THRESHOLD
#ifndef MORECORE_CANNOT_TRIM
#define DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#وگرنه   /* MORECORE_CANNOT_TRIM */
#define DEFAULT_TRIM_THRESHOLD MAX_SIZE_T
#endif  /* MORECORE_CANNOT_TRIM */
#endif  /* DEFAULT_TRIM_THRESHOLD */
#ifndef DEFAULT_MMAP_THRESHOLD
#اگر HAVE_MMAP
#define DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#وگرنه   /* HAVE_MMAP */
#define DEFAULT_MMAP_THRESHOLD MAX_SIZE_T
#endif  /* HAVE_MMAP */
#endif  /* DEFAULT_MMAP_THRESHOLD */
#ifndef MAX_RELEASE_CHECK_RATE
#اگر HAVE_MMAP
#define MAX_RELEASE_CHECK_RATE 4095
#وگرنه
#define MAX_RELEASE_CHECK_RATE MAX_SIZE_T
#endif /* HAVE_MMAP */
#endif /* MAX_RELEASE_CHECK_RATE */
#ifndef USE_BUILTIN_FFS
#define USE_BUILTIN_FFS 0
#endif  /* USE_BUILTIN_FFS */
#ifndef USE_DEV_RANDOM
#define USE_DEV_RANDOM 0
#endif  /* USE_DEV_RANDOM */
#ifndef NO_MALLINFO
#define NO_MALLINFO 0
#endif  /* NO_MALLINFO */
#ifndef MALLINFO_FIELD_TYPE
#define MALLINFO_FIELD_TYPE size_t
#endif  /* MALLINFO_FIELD_TYPE */
#ifndef NO_MALLOC_STATS
#define NO_MALLOC_STATS 0
#endif  /* NO_MALLOC_STATS */
#ifndef NO_SEGMENT_TRAVERSAL
#define NO_SEGMENT_TRAVERSAL 0
#endif /* NO_SEGMENT_TRAVERSAL */

/*
  mallopt tuning options.  SVID/XPG defines four standard parameter
  numbers for mallopt, normally defined in malloc.h.  None of these
  are used in this malloc, so setting them has no effect. But this
  malloc does support the following options.
*/

/* The system's malloc.h may have conflicting defines. */
#undef M_TRIM_THRESHOLD
#undef M_GRANULARITY
#undef M_MMAP_THRESHOLD

#define M_TRIM_THRESHOLD     (-1)
#define M_GRANULARITY        (-2)
#define M_MMAP_THRESHOLD     (-3)

/* ------------------------ Mallinfo declarations ------------------------ */

#اگر !NO_MALLINFO
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

#ifdef HAVE_USR_INCLUDE_MALLOC_H
#شامل "/usr/include/malloc.h"
#وگرنه /* HAVE_USR_INCLUDE_MALLOC_H */
#ifndef STRUCT_MALLINFO_DECLARED
/* HP-UX (and others?) redefines mallinfo unless _STRUCT_MALLINFO is defined */
#define _STRUCT_MALLINFO
#define STRUCT_MALLINFO_DECLARED 1
ساختار mallinfo {
  MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
  MALLINFO_FIELD_TYPE smblks;   /* always 0 */
  MALLINFO_FIELD_TYPE hblks;    /* always 0 */
  MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
  MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
  MALLINFO_FIELD_TYPE fordblks; /* total free space */
  MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#endif /* STRUCT_MALLINFO_DECLARED */
#endif /* HAVE_USR_INCLUDE_MALLOC_H */
#endif /* NO_MALLINFO */

/*
  Try to persuade compilers to inline. The most critical functions for
  inlining are defined as macros, so these aren't used for them.
*/

#ifndef FORCEINLINE
  #اگر defined(__GNUC__)
#define FORCEINLINE __inline __attribute__ ((always_inline))
  #وگرنهاگر defined(_MSC_VER)
    #define FORCEINLINE __forceinline
  #endif
#endif
#ifndef NOINLINE
  #اگر defined(__GNUC__)
    #define NOINLINE __attribute__ ((noinline))
  #وگرنهاگر defined(_MSC_VER)
    #define NOINLINE __declspec(noinline)
  #وگرنه
    #define NOINLINE
  #endif
#endif

#ifdef __cplusplus
بیرونی "C" {
#ifndef FORCEINLINE
 #define FORCEINLINE درونخطی
#endif
#endif /* __cplusplus */
#ifndef FORCEINLINE
 #define FORCEINLINE
#endif

#اگر !ONLY_MSPACES

/* ------------------- Declarations of public routines ------------------- */

#ifndef USE_DL_PREFIX
#define dlcalloc               calloc
#define dlfree                 آزاد
#define dlmalloc               malloc
#define dlmemalign             memalign
#define dlposix_memalign       posix_memalign
#define dlrealloc              realloc
#define dlrealloc_in_place     realloc_in_place
#define dlvalloc               valloc
#define dlpvalloc              pvalloc
#define dlmallinfo             mallinfo
#define dlmallopt              mallopt
#define dlmalloc_trim          malloc_trim
#define dlmalloc_stats         malloc_stats
#define dlmalloc_usable_size   malloc_usable_size
#define dlmalloc_footprint     malloc_footprint
#define dlmalloc_max_footprint malloc_max_footprint
#define dlmalloc_footprint_limit malloc_footprint_limit
#define dlmalloc_set_footprint_limit malloc_set_footprint_limit
#define dlmalloc_inspect_all   malloc_inspect_all
#define dlindependent_calloc   independent_calloc
#define dlindependent_comalloc independent_comalloc
#define dlbulk_free            bulk_free
#endif /* USE_DL_PREFIX */

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
DLMALLOC_EXPORT تهی* dlmalloc(size_t);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
DLMALLOC_EXPORT تهی  dlfree(تهی*);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
DLMALLOC_EXPORT تهی* dlcalloc(size_t, size_t);

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
DLMALLOC_EXPORT تهی* dlrealloc(تهی*, size_t);

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
DLMALLOC_EXPORT تهی* dlrealloc_in_place(تهی*, size_t);

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
DLMALLOC_EXPORT تهی* dlmemalign(size_t, size_t);

/*
  int posix_memalign(void** pp, size_t alignment, size_t n);
  Allocates a chunk of n bytes, aligned in accord with the alignment
  argument. Differs from memalign only in that it (1) assigns the
  allocated memory to *pp rather than returning it, (2) fails and
  returns EINVAL if the alignment is not a power of two (3) fails and
  returns ENOMEM if memory cannot be allocated.
*/
DLMALLOC_EXPORT صحیح dlposix_memalign(تهی**, size_t, size_t);

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
DLMALLOC_EXPORT تهی* dlvalloc(size_t);

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
DLMALLOC_EXPORT صحیح dlmallopt(صحیح, صحیح);

/*
  malloc_footprint();
  Returns the number of bytes obtained from the system.  The total
  number of bytes allocated by malloc, realloc etc., is less than this
  value. Unlike mallinfo, this function returns only a precomputed
  result, so can be called frequently to monitor memory consumption.
  Even if locks are otherwise defined, this function does not use them,
  so results might not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint(تهی);

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
DLMALLOC_EXPORT size_t dlmalloc_max_footprint(تهی);

/*
  malloc_footprint_limit();
  Returns the number of bytes that the heap is allowed to obtain from
  the system, returning the last value returned by
  malloc_set_footprint_limit, or the maximum size_t value if
  never set. The returned value reflects a permission. There is no
  guarantee that this number of bytes can actually be obtained from
  the system.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint_limit();

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
DLMALLOC_EXPORT size_t dlmalloc_set_footprint_limit(size_t bytes);

#اگر MALLOC_INSPECT_ALL
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
DLMALLOC_EXPORT تهی dlmalloc_inspect_all(تهی(*handler)(تهی*, تهی *, size_t, تهی*),
                           تهی* arg);

#endif /* MALLOC_INSPECT_ALL */

#اگر !NO_MALLINFO
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
DLMALLOC_EXPORT ساختار mallinfo dlmallinfo(تهی);
#endif /* NO_MALLINFO */

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
DLMALLOC_EXPORT تهی** dlindependent_calloc(size_t, size_t, تهی**);

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
DLMALLOC_EXPORT تهی** dlindependent_comalloc(size_t, size_t*, تهی**);

/*
  bulk_free(void* array[], size_t n_elements)
  Frees and clears (sets to null) each non-null pointer in the given
  array.  This is likely to be faster than freeing them one-by-one.
  If footers are used, pointers that have been allocated in different
  mspaces are not freed or cleared, and the count of all such pointers
  is returned.  For large arrays of pointers with poor locality, it
  may be worthwhile to sort this array before calling bulk_free.
*/
DLMALLOC_EXPORT size_t  dlbulk_free(تهی**, size_t n_elements);

/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
DLMALLOC_EXPORT تهی*  dlpvalloc(size_t);

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
DLMALLOC_EXPORT صحیح  dlmalloc_trim(size_t);

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
DLMALLOC_EXPORT تهی  dlmalloc_stats(تهی);

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
size_t dlmalloc_usable_size(تهی*);

#endif /* ONLY_MSPACES */

#اگر MSPACES

/*
  mspace is an opaque type representing an independent
  region of space that supports mspace_malloc, etc.
*/
تعریفنوع تهی* mspace;

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
DLMALLOC_EXPORT mspace create_mspace(size_t capacity, صحیح locked);

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
DLMALLOC_EXPORT mspace create_mspace_with_base(تهی* base, size_t capacity, صحیح locked);

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
DLMALLOC_EXPORT صحیح mspace_track_large_chunks(mspace msp, صحیح enable);


/*
  mspace_malloc behaves as malloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT تهی* mspace_malloc(mspace msp, size_t bytes);

/*
  mspace_free behaves as free, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_free is not actually needed.
  free may be called instead of mspace_free because freed chunks from
  any space are handled by their originating spaces.
*/
DLMALLOC_EXPORT تهی mspace_free(mspace msp, تهی* mem);

/*
  mspace_realloc behaves as realloc, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_realloc is not actually
  needed.  realloc may be called instead of mspace_realloc because
  realloced chunks from any space are handled by their originating
  spaces.
*/
DLMALLOC_EXPORT تهی* mspace_realloc(mspace msp, تهی* mem, size_t newsize);

/*
  mspace_calloc behaves as calloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT تهی* mspace_calloc(mspace msp, size_t n_elements, size_t elem_size);

/*
  mspace_memalign behaves as memalign, but operates within
  the given space.
*/
DLMALLOC_EXPORT تهی* mspace_memalign(mspace msp, size_t alignment, size_t bytes);

/*
  mspace_independent_calloc behaves as independent_calloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT تهی** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_size, تهی* chunks[]);

/*
  mspace_independent_comalloc behaves as independent_comalloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT تهی** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], تهی* chunks[]);

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


#اگر !NO_MALLINFO
/*
  mspace_mallinfo behaves as mallinfo, but reports properties of
  the given space.
*/
DLMALLOC_EXPORT ساختار mallinfo mspace_mallinfo(mspace msp);
#endif /* NO_MALLINFO */

/*
  malloc_usable_size(void* p) behaves the same as malloc_usable_size;
*/
DLMALLOC_EXPORT size_t mspace_usable_size(ثابت تهی* mem);

/*
  mspace_malloc_stats behaves as malloc_stats, but reports
  properties of the given space.
*/
DLMALLOC_EXPORT تهی mspace_malloc_stats(mspace msp);

/*
  mspace_trim behaves as malloc_trim, but
  operates within the given space.
*/
DLMALLOC_EXPORT صحیح mspace_trim(mspace msp, size_t pad);

/*
  An alias for mallopt.
*/
DLMALLOC_EXPORT صحیح mspace_mallopt(صحیح, صحیح);

#endif /* MSPACES */

#ifdef __cplusplus
}  /* end of extern "C" */
#endif /* __cplusplus */

/*
  ========================================================================
  To make a fully customizable malloc.h header file, cut everything
  above this line, put into file malloc.h, edit to suit, and #include it
  on the next line, as well as in programs that use this malloc.
  ========================================================================
*/

/* #include "malloc.h" */

/*------------------------------ internal #includes ---------------------- */

#ifdef _MSC_VER
#pragma warning( disable : 4146 ) /* no "unsigned" warnings */
#endif /* _MSC_VER */
#اگر !NO_MALLOC_STATS
#شامل <stdio.h>       /* for printing in malloc_stats */
#endif /* NO_MALLOC_STATS */
#ifndef LACKS_ERRNO_H
#شامل <errno.h>       /* for MALLOC_FAILURE_ACTION */
#endif /* LACKS_ERRNO_H */
#ifdef DEBUG
#اگر ABORT_ON_ASSERT_FAILURE
#undef أكد
#define أكد(x) اگر(!(x)) ABORT
#وگرنه /* ABORT_ON_ASSERT_FAILURE */
#شامل <assert.h>
#endif /* ABORT_ON_ASSERT_FAILURE */
#وگرنه  /* DEBUG */
#ifndef أكد
#define أكد(x)
#endif
#define DEBUG 0
#endif /* DEBUG */
#اگر !defined(WIN32) && !defined(LACKS_TIME_H)
#شامل <time.h>        /* for magic initialization */
#endif /* WIN32 */
#ifndef LACKS_STDLIB_H
#شامل <stdlib.h>      /* for abort() */
#endif /* LACKS_STDLIB_H */
#ifndef LACKS_STRING_H
#شامل <string.h>      /* for memset etc */
#endif  /* LACKS_STRING_H */
#اگر USE_BUILTIN_FFS
#ifndef LACKS_STRINGS_H
#شامل <strings.h>     /* for ffs */
#endif /* LACKS_STRINGS_H */
#endif /* USE_BUILTIN_FFS */
#اگر HAVE_MMAP
#ifndef LACKS_SYS_MMAN_H
/* On some versions of linux, mremap decl in mman.h needs __USE_GNU set */
#اگر (defined(linux) && !defined(__USE_GNU))
#define __USE_GNU 1
#شامل <sys/mman.h>    /* for mmap */
#undef __USE_GNU
#وگرنه
#شامل <sys/mman.h>    /* for mmap */
#endif /* linux */
#endif /* LACKS_SYS_MMAN_H */
#ifndef LACKS_FCNTL_H
#شامل <fcntl.h>
#endif /* LACKS_FCNTL_H */
#endif /* HAVE_MMAP */
#ifndef LACKS_UNISTD_H
#شامل <unistd.h>     /* for sbrk, sysconf */
#وگرنه /* LACKS_UNISTD_H */
#اگر !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
بیرونی تهی*     sbrk(ptrdiff_t);
#endif /* FreeBSD etc */
#endif /* LACKS_UNISTD_H */

/* Declarations for locking */
#اگر USE_LOCKS
#ifndef WIN32
#اگر defined (__SVR4) && defined (__sun)  /* solaris */
#شامل <thread.h>
#وگرنهاگر !defined(LACKS_SCHED_H)
#شامل <sched.h>
#endif /* solaris or LACKS_SCHED_H */
#اگر (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0) || !USE_SPIN_LOCKS
#شامل <pthread.h>
#endif /* USE_RECURSIVE_LOCKS ... */
#وگرنهاگر defined(_MSC_VER)
#ifndef _M_AMD64
/* These are already defined on AMD64 builds */
#ifdef __cplusplus
بیرونی "C" {
#endif /* __cplusplus */
LONG __cdecl _InterlockedCompareExchange(LONG فرار *Dest, LONG Exchange, LONG Comp);
LONG __cdecl _InterlockedExchange(LONG فرار *Target, LONG مقدار_2);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _M_AMD64 */
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedExchange)
#define interlockedcompareexchange _InterlockedCompareExchange
#define interlockedexchange _InterlockedExchange
#وگرنهاگر defined(WIN32) && defined(__GNUC__)
#define interlockedcompareexchange(a, b, c) __sync_val_compare_and_swap(a, c, b)
#define interlockedexchange __sync_lock_test_and_set
#endif /* Win32 */
#وگرنه /* USE_LOCKS */
#endif /* USE_LOCKS */

#ifndef LOCK_AT_FORK
#define LOCK_AT_FORK 0
#endif

/* Declarations for bit scanning on win32 */
#اگر defined(_MSC_VER) && _MSC_VER>=1300
#ifndef BitScanForward /* Try to avoid pulling in WinNT.h */
#ifdef __cplusplus
بیرونی "C" {
#endif /* __cplusplus */
unsigned نویسه _BitScanForward(unsigned long *index, unsigned long mask);
unsigned نویسه _BitScanReverse(unsigned long *index, unsigned long mask);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#define BitScanForward _BitScanForward
#define BitScanReverse _BitScanReverse
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
#endif /* BitScanForward */
#endif /* defined(_MSC_VER) && _MSC_VER>=1300 */

#ifndef WIN32
#ifndef malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef _SC_PAGE_SIZE
#      define _SC_PAGE_SIZE _SC_PAGESIZE
#    endif
#  endif
#  ifdef _SC_PAGE_SIZE
#    define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#  وگرنه
#    اگر defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       بیرونی size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    وگرنه
#      ifdef WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize()
#      وگرنه
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        وگرنه
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            وگرنه
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          وگرنه
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            وگرنه
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              وگرنه /* just guess */
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#endif
#endif

/* libffi: the upstream malloc_getpagesize definition above is gated on
   !WIN32 (WIN32 builds use GetSystemInfo in init_mparams instead).  Guarantee
   the symbol is always defined so the !WIN32 init_mparams branch still compiles
   on toolchains where it is not preprocessed out as expected (e.g. MSVC).  */
#ifndef malloc_getpagesize
#define malloc_getpagesize ((size_t)4096U)
#endif

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#define SIZE_T_SIZE         (اندازه(size_t))
#define SIZE_T_BITSIZE      (اندازه(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#define SIZE_T_ZERO         ((size_t)0)
#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZE_T_FOUR         ((size_t)4)
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
#define FOUR_SIZE_T_SIZES   (SIZE_T_SIZE<<2)
#define SIX_SIZE_T_SIZES    (FOUR_SIZE_T_SIZES+TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T     (MAX_SIZE_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)

/* True if address a has acceptable alignment */
#define is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

/* the number of bytes to offset an address to align it */
#define align_offset(A)\
 ((((size_t)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))

/* -------------------------- MMAP preliminaries ------------------------- */

/*
   If HAVE_MORECORE or HAVE_MMAP are false, we just define calls and
   checks to fail so compiler optimizer can delete code rather than
   using so many "#if"s.
*/


/* MORECORE and MMAP must return MFAIL on failure */
#define MFAIL                ((تهی*)(MAX_SIZE_T))
#define CMFAIL               ((نویسه*)(MFAIL)) /* defined for convenience */

#اگر HAVE_MMAP

#اگر !defined(WIN32) && !defined (__OS2__)
#define MUNMAP_DEFAULT(a, s)  munmap((a), (s))
#define MMAP_PROT            (PROT_READ|PROT_WRITE)
#اگر !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS        MAP_ANON
#endif /* MAP_ANON */
#ifdef MAP_ANONYMOUS
#define MMAP_FLAGS           (MAP_PRIVATE|MAP_ANONYMOUS)
#define MMAP_DEFAULT(s)       mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#وگرنه /* MAP_ANONYMOUS */
/*
   Nearly all versions of mmap support MAP_ANONYMOUS, so the following
   is unlikely to be needed, but is supplied just in case.
*/
#define MMAP_FLAGS           (MAP_PRIVATE)
ایستا صحیح dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#define MMAP_DEFAULT(s) ((dev_zero_fd < 0) ? \
           (dev_zero_fd = open("/dev/zero", O_RDWR), \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#endif /* MAP_ANONYMOUS */

#define DIRECT_MMAP_DEFAULT(s) MMAP_DEFAULT(s)

#وگرنهاگر defined(__OS2__)

/* OS/2 MMAP via DosAllocMem */
ایستا تهی* os2mmap(size_t size) {
  تهی* ptr;
  اگر (DosAllocMem(&ptr, size, OBJ_ANY|PAG_COMMIT|PAG_READ|PAG_WRITE) &&
      DosAllocMem(&ptr, size, PAG_COMMIT|PAG_READ|PAG_WRITE))
    بازگشت MFAIL;
  بازگشت ptr;
}

#define os2direct_mmap(n)     os2mmap(n)

/* This function supports releasing coalesed segments */
ایستا صحیح os2munmap(تهی* ptr, size_t size) {
  وقتی (size) {
    ULONG ulSize = size;
    ULONG ulFlags = 0;
    اگر (DosQueryMem(ptr, &ulSize, &ulFlags) != 0)
      بازگشت -1;
    اگر ((ulFlags & PAG_BASE) == 0 ||(ulFlags & PAG_COMMIT) == 0 ||
        ulSize > size)
      بازگشت -1;
    اگر (DosFreeMem(ptr) != 0)
      بازگشت -1;
    ptr = ( تهی * ) ( ( نویسه * ) ptr + ulSize );
    size -= ulSize;
  }
  بازگشت 0;
}

#define MMAP_DEFAULT(s)         os2mmap(s)
#define MUNMAP_DEFAULT(a, s)    os2munmap((a), (s))
#define DIRECT_MMAP_DEFAULT(s)  os2direct_mmap(s)

#وگرنه /* WIN32 */

/* Win32 MMAP via VirtualAlloc */
/* libffi: plain static (not FORCEINLINE) -- mingw's <windows.h> defines
   FORCEINLINE with an 'extern' storage class, which clashes with 'static'.  */
ایستا تهی* win32mmap(size_t size) {
  تهی* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  بازگشت (ptr != 0)? ptr: MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
ایستا تهی* win32direct_mmap(size_t size) {
  تهی* ptr = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT|MEM_بالاترین_DOWN,
                           PAGE_EXECUTE_READWRITE);
  بازگشت (ptr != 0)? ptr: MFAIL;
}

/* This function supports releasing coalesed segments */
ایستا صحیح win32munmap(تهی* ptr, size_t size) {
  MEMORY_BASIC_INFORMATION minfo;
  نویسه* cptr = (نویسه*)ptr;
  وقتی (size) {
    اگر (VirtualQuery(cptr, &minfo, اندازه(minfo)) == 0)
      بازگشت -1;
    اگر (minfo.BaseAddress != cptr || minfo.AllocationBase != cptr ||
        minfo.State != MEM_COMMIT || minfo.RegionSize > size)
      بازگشت -1;
    اگر (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
      بازگشت -1;
    cptr += minfo.RegionSize;
    size -= minfo.RegionSize;
  }
  بازگشت 0;
}

#define MMAP_DEFAULT(s)             win32mmap(s)
#define MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
#define DIRECT_MMAP_DEFAULT(s)      win32direct_mmap(s)
#endif /* WIN32 */
#endif /* HAVE_MMAP */

#اگر HAVE_MREMAP
#ifndef WIN32
#define MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#endif /* WIN32 */
#endif /* HAVE_MREMAP */

/**
 * Define CALL_MORECORE
 */
#اگر HAVE_MORECORE
    #ifdef MORECORE
        #define CALL_MORECORE(S)    MORECORE(S)
    #وگرنه  /* MORECORE */
        #define CALL_MORECORE(S)    MORECORE_DEFAULT(S)
    #endif /* MORECORE */
#وگرنه  /* HAVE_MORECORE */
    #define CALL_MORECORE(S)        MFAIL
#endif /* HAVE_MORECORE */

/**
 * Define CALL_MMAP/CALL_MUNMAP/CALL_DIRECT_MMAP
 */
#اگر HAVE_MMAP
    #define USE_MMAP_BIT            (SIZE_T_ONE)

    #ifdef MMAP
        #define CALL_MMAP(s)        MMAP(s)
    #وگرنه /* MMAP */
        #define CALL_MMAP(s)        MMAP_DEFAULT(s)
    #endif /* MMAP */
    #ifdef MUNMAP
        #define CALL_MUNMAP(a, s)   MUNMAP((a), (s))
    #وگرنه /* MUNMAP */
        #define CALL_MUNMAP(a, s)   MUNMAP_DEFAULT((a), (s))
    #endif /* MUNMAP */
    #ifdef DIRECT_MMAP
        #define CALL_DIRECT_MMAP(s) DIRECT_MMAP(s)
    #وگرنه /* DIRECT_MMAP */
        #define CALL_DIRECT_MMAP(s) DIRECT_MMAP_DEFAULT(s)
    #endif /* DIRECT_MMAP */
#وگرنه  /* HAVE_MMAP */
    #define USE_MMAP_BIT            (SIZE_T_ZERO)

    #define MMAP(s)                 MFAIL
    #define MUNMAP(a, s)            (-1)
    #define DIRECT_MMAP(s)          MFAIL
    #define CALL_DIRECT_MMAP(s)     DIRECT_MMAP(s)
    #define CALL_MMAP(s)            MMAP(s)
    #define CALL_MUNMAP(a, s)       MUNMAP((a), (s))
#endif /* HAVE_MMAP */

/**
 * Define CALL_MREMAP
 */
#اگر HAVE_MMAP && HAVE_MREMAP
    #ifdef MREMAP
        #define CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
    #وگرنه /* MREMAP */
        #define CALL_MREMAP(addr, osz, nsz, mv) MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
    #endif /* MREMAP */
#وگرنه  /* HAVE_MMAP && HAVE_MREMAP */
    #define CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#endif /* HAVE_MMAP && HAVE_MREMAP */

/* mstate bit set if contiguous morecore disabled or failed */
#define USE_NONCONTIGUOUS_BIT (4U)

/* segment bit set in create_mspace_with_base */
#define EXTERN_BIT            (8U)


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

#اگر !USE_LOCKS
#define USE_LOCK_BIT               (0U)
#define INITIAL_LOCK(l)            (0)
#define DESTROY_LOCK(l)            (0)
#define ACQUIRE_MALLOC_GLOBAL_LOCK()
#define RELEASE_MALLOC_GLOBAL_LOCK()

#وگرنه
#اگر USE_LOCKS > 1
/* -----------------------  User-defined locks ------------------------ */
/* Define your own lock implementation here */
/* #define INITIAL_LOCK(lk)  ... */
/* #define DESTROY_LOCK(lk)  ... */
/* #define ACQUIRE_LOCK(lk)  ... */
/* #define RELEASE_LOCK(lk)  ... */
/* #define TRY_LOCK(lk) ... */
/* static MLOCK_T malloc_global_mutex = ... */

#وگرنهاگر USE_SPIN_LOCKS

/* First, define CAS_LOCK and CLEAR_LOCK on ints */
/* Note CAS_LOCK defined to return 0 on success */

#اگر defined(__GNUC__)&& (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#define CAS_LOCK(sl)     __sync_lock_test_and_set(sl, 1)
#define CLEAR_LOCK(sl)   __sync_lock_release(sl)

#وگرنهاگر (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
/* Custom spin locks for older gcc on x86 */
ایستا FORCEINLINE صحیح x86_cas_lock(صحیح *sl) {
  صحیح ret;
  صحیح val = 1;
  صحیح cmp = 0;
  __asm__ __volatile__  ("lock; cmpxchgl %1, %2"
                         : "=a" (ret)
                         : "r" (val), "m" (*(sl)), "0"(cmp)
                         : "memory", "cc");
  بازگشت ret;
}

ایستا FORCEINLINE تهی x86_clear_lock(صحیح* sl) {
  أكد(*sl != 0);
  صحیح prev = 0;
  صحیح ret;
  __asm__ __volatile__ ("lock; xchgl %0, %1"
                        : "=r" (ret)
                        : "m" (*(sl)), "0"(prev)
                        : "memory");
}

#define CAS_LOCK(sl)     x86_cas_lock(sl)
#define CLEAR_LOCK(sl)   x86_clear_lock(sl)

#وگرنه /* Win32 MSC */
#define CAS_LOCK(sl)     interlockedexchange((LONG فرار *)(sl), (LONG)1)
#define CLEAR_LOCK(sl)   interlockedexchange ((LONG فرار *)(sl), (LONG)0)

#endif /* ... gcc spins locks ... */

/* How to yield for a spin lock */
#define SPINS_PER_YIELD       63
#اگر defined(_MSC_VER)
#define SLEEP_EX_DURATION     50 /* delay for yield/sleep */
#define SPIN_LOCK_YIELD  SleepEx(SLEEP_EX_DURATION, FALSE)
#وگرنهاگر defined (__SVR4) && defined (__sun) /* solaris */
#define SPIN_LOCK_YIELD   thr_yield();
#وگرنهاگر !defined(LACKS_SCHED_H)
#define SPIN_LOCK_YIELD   sched_yield();
#وگرنه
#define SPIN_LOCK_YIELD
#endif /* ... yield ... */

/* libffi: read the spin-lock word with a relaxed atomic load for the lock-free
   "is it held?" peek, so ThreadSanitizer does not flag it as racing with the
   atomic CLEAR_LOCK release on unlock.  Relaxed ordering is sufficient: the
   peek is only a hint to avoid a costly CAS, and CAS_LOCK still provides the
   acquire ordering on success.  Falls back to the original volatile read when
   atomic builtins are unavailable.  */
#اگر defined(__ATOMIC_RELAXED)
# define ffi_spin_peek(sl) __atomic_load_n((sl), __ATOMIC_RELAXED)
#وگرنهاگر defined(_MSC_VER)
# define ffi_spin_peek(sl) \
  ((صحیح)_InterlockedCompareExchange((LONG فرار *)(sl), (LONG)0, (LONG)0))
#وگرنه
# define ffi_spin_peek(sl) (*(فرار صحیح *)(sl))
#endif

#اگر !defined(USE_RECURSIVE_LOCKS) || USE_RECURSIVE_LOCKS == 0
/* Plain spin locks use single word (embedded in malloc_states) */
ایستا صحیح spin_acquire_lock(صحیح *sl) {
  صحیح spins = 0;
  وقتی (ffi_spin_peek(sl) != 0 || CAS_LOCK(sl)) {
    اگر ((++spins & SPINS_PER_YIELD) == 0) {
      SPIN_LOCK_YIELD;
    }
  }
  بازگشت 0;
}

#define MLOCK_T               صحیح
#define TRY_LOCK(sl)          !CAS_LOCK(sl)
#define RELEASE_LOCK(sl)      CLEAR_LOCK(sl)
#define ACQUIRE_LOCK(sl)      (CAS_LOCK(sl)? spin_acquire_lock(sl) : 0)
#define INITIAL_LOCK(sl)      (*sl = 0)
#define DESTROY_LOCK(sl)      (0)
ایستا MLOCK_T malloc_global_mutex = 0;

#وگرنه /* USE_RECURSIVE_LOCKS */
/* types for lock owners */
#ifdef WIN32
#define THREAD_ID_T           DWORD
#define CURRENT_THREAD        GetCurrentThreadId()
#define EQ_OWNER(X,Y)         ((X) == (Y))
#وگرنه
/*
  Note: the following assume that pthread_t is a type that can be
  initialized to (casted) zero. If this is not the case, you will need to
  somehow redefine these or not use spin locks.
*/
#define THREAD_ID_T           pthread_t
#define CURRENT_THREAD        pthread_self()
#define EQ_OWNER(X,Y)         pthread_equal(X, Y)
#endif

ساختار malloc_recursive_lock {
  صحیح sl;
  unsigned صحیح c;
  THREAD_ID_T threadid;
};

#define MLOCK_T  ساختار malloc_recursive_lock
ایستا MLOCK_T malloc_global_mutex = { 0, 0, (THREAD_ID_T)0};

ایستا FORCEINLINE تهی recursive_release_lock(MLOCK_T *lk) {
  أكد(lk->sl != 0);
  اگر (--lk->c == 0) {
    CLEAR_LOCK(&lk->sl);
  }
}

ایستا FORCEINLINE صحیح recursive_acquire_lock(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  صحیح spins = 0;
  برای (;;) {
    اگر (ffi_spin_peek(&lk->sl) == 0) {
      اگر (!CAS_LOCK(&lk->sl)) {
        lk->threadid = mythreadid;
        lk->c = 1;
        بازگشت 0;
      }
    }
    وگرنه اگر (EQ_OWNER(lk->threadid, mythreadid)) {
      ++lk->c;
      بازگشت 0;
    }
    اگر ((++spins & SPINS_PER_YIELD) == 0) {
      SPIN_LOCK_YIELD;
    }
  }
}

ایستا FORCEINLINE صحیح recursive_try_lock(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  اگر (ffi_spin_peek(&lk->sl) == 0) {
    اگر (!CAS_LOCK(&lk->sl)) {
      lk->threadid = mythreadid;
      lk->c = 1;
      بازگشت 1;
    }
  }
  وگرنه اگر (EQ_OWNER(lk->threadid, mythreadid)) {
    ++lk->c;
    بازگشت 1;
  }
  بازگشت 0;
}

#define RELEASE_LOCK(lk)      recursive_release_lock(lk)
#define TRY_LOCK(lk)          recursive_try_lock(lk)
#define ACQUIRE_LOCK(lk)      recursive_acquire_lock(lk)
#define INITIAL_LOCK(lk)      ((lk)->threadid = (THREAD_ID_T)0, (lk)->sl = 0, (lk)->c = 0)
#define DESTROY_LOCK(lk)      (0)
#endif /* USE_RECURSIVE_LOCKS */

#وگرنهاگر defined(WIN32) /* Win32 critical sections */
#define MLOCK_T               CRITICAL_SECTION
#define ACQUIRE_LOCK(lk)      (EnterCriticalSection(lk), 0)
#define RELEASE_LOCK(lk)      LeaveCriticalSection(lk)
#define TRY_LOCK(lk)          TryEnterCriticalSection(lk)
#define INITIAL_LOCK(lk)      (!Initialize_Critical_Section_And_Spin_شمار((lk), 0x80000000|4000))
#define DESTROY_LOCK(lk)      (DeleteCriticalSection(lk), 0)
#define NEED_GLOBAL_LOCK_INIT

ایستا MLOCK_T malloc_global_mutex;
ایستا فرار LONG malloc_global_mutex_status;

/* Use spin loop to initialize global lock */
ایستا تهی init_malloc_global_mutex() {
  برای (;;) {
    long stat = malloc_global_mutex_status;
    اگر (stat > 0)
      بازگشت;
    /* transition to < 0 while initializing, then to > 0) */
    اگر (stat == 0 &&
        interlockedcompareexchange(&malloc_global_mutex_status, (LONG)-1, (LONG)0) == 0) {
      InitializeCriticalSection(&malloc_global_mutex);
      interlockedexchange(&malloc_global_mutex_status, (LONG)1);
      بازگشت;
    }
    SleepEx(0, FALSE);
  }
}

#وگرنه /* pthreads-based locks */
#define MLOCK_T               pthread_mutex_t
#define ACQUIRE_LOCK(lk)      pthread_mutex_lock(lk)
#define RELEASE_LOCK(lk)      pthread_mutex_unlock(lk)
#define TRY_LOCK(lk)          (!pthread_mutex_trylock(lk))
#define INITIAL_LOCK(lk)      pthread_init_lock(lk)
#define DESTROY_LOCK(lk)      pthread_mutex_destroy(lk)

#اگر defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0 && defined(linux) && !defined(PTHREAD_MUTEX_RECURSIVE)
/* Cope with old-style linux recursive lock initialization by adding */
/* skipped internal declaration from pthread.h */
بیرونی صحیح pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
                                              صحیح __kind));
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#define pthread_mutexattr_settype(x,y) pthread_mutexattr_setkind_np(x,y)
#endif /* USE_RECURSIVE_LOCKS ... */

ایستا MLOCK_T malloc_global_mutex = PTHREAD_MUTEX_INITIALIZER;

ایستا صحیح pthread_init_lock (MLOCK_T *lk) {
  pthread_mutexattr_t attr;
  اگر (pthread_mutexattr_init(&attr)) بازگشت 1;
#اگر defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0
  اگر (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) بازگشت 1;
#endif
  اگر (pthread_mutex_init(lk, &attr)) بازگشت 1;
  اگر (pthread_mutexattr_destroy(&attr)) بازگشت 1;
  بازگشت 0;
}

#endif /* ... lock types ... */

/* Common code for all lock types */
#define USE_LOCK_BIT               (2U)

#ifndef ACQUIRE_MALLOC_GLOBAL_LOCK
#define ACQUIRE_MALLOC_GLOBAL_LOCK()  ACQUIRE_LOCK(&malloc_global_mutex);
#endif

#ifndef RELEASE_MALLOC_GLOBAL_LOCK
#define RELEASE_MALLOC_GLOBAL_LOCK()  RELEASE_LOCK(&malloc_global_mutex);
#endif

#endif /* USE_LOCKS */

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

ساختار malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               head;       /* Size and inuse bits. */
  ساختار malloc_chunk* fd;         /* double links -- used only if free. */
  ساختار malloc_chunk* bk;
};

تعریفنوع ساختار malloc_chunk  mchunk;
تعریفنوع ساختار malloc_chunk* mchunkptr;
تعریفنوع ساختار malloc_chunk* sbinptr;  /* The type of bins of chunks */
تعریفنوع unsigned صحیح bindex_t;         /* Described below */
تعریفنوع unsigned صحیح binmap_t;         /* Described below */
تعریفنوع unsigned صحیح flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#define MCHUNK_SIZE         (اندازه(mchunk))

#اگر FOOTERS
#define CHUNK_OVERHEAD      (TWO_SIZE_T_SIZES)
#وگرنه /* FOOTERS */
#define CHUNK_OVERHEAD      (SIZE_T_SIZE)
#endif /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#define MMAP_CHUNK_OVERHEAD (TWO_SIZE_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#define MMAP_FOOT_PAD       (FOUR_SIZE_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#define MIN_CHUNK_SIZE\
  ((MCHUNK_SIZE + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* conversion from malloc headers to user pointers, and back */
#define chunk2mem(p)        ((تهی*)((نویسه*)(p)       + TWO_SIZE_T_SIZES))
#define mem2chunk(mem)      ((mchunkptr)((نویسه*)(mem) - TWO_SIZE_T_SIZES))
/* chunk associated with aligned address A */
#define align_as_chunk(A)   (mchunkptr)((A) + align_offset(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#define MAX_REQUEST         ((-MIN_CHUNK_SIZE) << 2)
#define MIN_REQUEST         (MIN_CHUNK_SIZE - CHUNK_OVERHEAD - SIZE_T_ONE)

/* pad request bytes into a usable size */
#define pad_request(req) \
   (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* pad request, checking for minimum (but not maximum) */
#define request2size(req) \
  (((req) < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#define PINUSE_BIT          (SIZE_T_ONE)
#define CINUSE_BIT          (SIZE_T_TWO)
#define FLAG4_BIT           (SIZE_T_FOUR)
#define INUSE_BITS          (PINUSE_BIT|CINUSE_BIT)
#define FLAG_BITS           (PINUSE_BIT|CINUSE_BIT|FLAG4_BIT)

/* Head value for fenceposts */
#define FENCEPOST_HEAD      (INUSE_BITS|SIZE_T_SIZE)

/* extraction of fields from head words */
#define cinuse(p)           ((p)->head & CINUSE_BIT)
#define pinuse(p)           ((p)->head & PINUSE_BIT)
#define flag4inuse(p)       ((p)->head & FLAG4_BIT)
#define is_inuse(p)         (((p)->head & INUSE_BITS) != PINUSE_BIT)
#define is_mmapped(p)       (((p)->head & INUSE_BITS) == 0)

#define chunksize(p)        ((p)->head & ~(FLAG_BITS))

#define clear_pinuse(p)     ((p)->head &= ~PINUSE_BIT)
#define set_flag4(p)        ((p)->head |= FLAG4_BIT)
#define clear_flag4(p)      ((p)->head &= ~FLAG4_BIT)

/* Treat space at ptr +/- offset as a chunk */
#define chunk_plus_offset(p, s)  ((mchunkptr)(((نویسه*)(p)) + (s)))
#define chunk_minus_offset(p, s) ((mchunkptr)(((نویسه*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((نویسه*)(p)) + ((p)->head & ~FLAG_BITS)))
#define prev_chunk(p) ((mchunkptr)( ((نویسه*)(p)) - ((p)->prev_foot) ))

/* extract next chunk's pinuse bit */
#define next_pinuse(p)  ((next_chunk(p)->head) & PINUSE_BIT)

/* Get/set size at footer */
#define get_foot(p, s)  (((mchunkptr)((نویسه*)(p) + (s)))->prev_foot)
#define set_foot(p, s)  (((mchunkptr)((نویسه*)(p) + (s)))->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#define set_size_and_pinuse_of_free_chunk(p, s)\
  ((p)->head = (s|PINUSE_BIT), set_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#define set_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#define overhead_for(p)\
 (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)

/* Return true if malloced space is not necessarily cleared */
#اگر MMAP_CLEARS
#define calloc_must_clear(p) (!is_mmapped(p))
#وگرنه /* MMAP_CLEARS */
#define calloc_must_clear(p) (1)
#endif /* MMAP_CLEARS */

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

ساختار malloc_درخت_chunk {
  /* The first four fields must be compatible with malloc_chunk */
  size_t                    prev_foot;
  size_t                    head;
  ساختار malloc_درخت_chunk* fd;
  ساختار malloc_درخت_chunk* bk;

  ساختار malloc_درخت_chunk* child[2];
  ساختار malloc_درخت_chunk* parent;
  bindex_t                  index;
};

تعریفنوع ساختار malloc_درخت_chunk  tchunk;
تعریفنوع ساختار malloc_درخت_chunk* tchunkptr;
تعریفنوع ساختار malloc_درخت_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#define leftmost_child(t) ((t)->child[0] != 0? (t)->child[0] : (t)->child[1])

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

ساختار malloc_segment {
  نویسه*        base;             /* base address */
  size_t       size;             /* allocated size */
  ساختار malloc_segment* next;   /* ptr to next segment */
#اگر FFI_MMAP_EXEC_WRIT
  /* The mmap magic is supposed to store the address of the executable
     segment at the very end of the requested block.  */

# define mmap_exec_offset(b,s) (*(ptrdiff_t*)((b)+(s)-اندازه(ptrdiff_t)))

  /* We can only merge segments if their corresponding executable
     segments are at identical offsets.  */
# define check_segment_merge(S,b,s) \
  (mmap_exec_offset((b),(s)) == (S)->exec_offset)

# define add_segment_exec_offset(p,S) ((نویسه*)(p) + (S)->exec_offset)
# define sub_segment_exec_offset(p,S) ((نویسه*)(p) - (S)->exec_offset)

  /* The removal of sflags only works with HAVE_MORECORE == 0.  */

# define get_segment_flags(S)   (USE_MMAP_BIT)
# define set_segment_flags(S,v) \
  (((v) != USE_MMAP_BIT) ? (ABORT, (v)) :				\
   (((S)->exec_offset =							\
     mmap_exec_offset((S)->base, (S)->size)),				\
    (mmap_exec_offset((S)->base + (S)->exec_offset, (S)->size) !=	\
     (S)->exec_offset) ? (ABORT, (v)) :					\
   (mmap_exec_offset((S)->base, (S)->size) = 0), (v)))

  /* We use an offset here, instead of a pointer, because then, when
     base changes, we don't have to modify this.  On architectures
     with segmented addresses, this might not work.  */
  ptrdiff_t    exec_offset;
#وگرنه

# define get_segment_flags(S)   ((S)->sflags)
# define set_segment_flags(S,v) ((S)->sflags = (v))
# define check_segment_merge(S,b,s) (1)

  flag_t       sflags;           /* mmap and extern flag */
#endif
};

#define is_mmapped_segment(S)  (get_segment_flags(S) & USE_MMAP_BIT)
#define is_extern_segment(S)   (get_segment_flags(S) & EXTERN_BIT)

تعریفنوع ساختار malloc_segment  msegment;
تعریفنوع ساختار malloc_segment* msegmentptr;

/* ---------------------------- malloc_state ----------------------------- */

/*
   A malloc_state holds all of the bookkeeping for a space.
   The main fields are:

بالاترین
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
#define NSMALLBINS        (32U)
#define NTREEBINS         (32U)
#define SMALLBIN_SHIFT    (3U)
#define SMALLBIN_WIDTH    (SIZE_T_ONE << SMALLBIN_SHIFT)
#define TREEBIN_SHIFT     (8U)
#define MIN_LARGE_SIZE    (SIZE_T_ONE << TREEBIN_SHIFT)
#define MAX_SMALL_SIZE    (MIN_LARGE_SIZE - SIZE_T_ONE)
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD)

ساختار malloc_state {
  binmap_t   smallmap;
  binmap_t   treemap;
  size_t     dvsize;
  size_t     topsize;
  نویسه*      least_addr;
  mchunkptr  dv;
  mchunkptr  بالاترین;
  size_t     trim_check;
  size_t     release_checks;
  size_t     magic;
  mchunkptr  smallbins[(NSMALLBINS+1)*2];
  tbinptr    treebins[NTREEBINS];
  size_t     footprint;
  size_t     max_footprint;
  size_t     footprint_limit; /* zero means no limit */
  flag_t     mflags;
#اگر USE_LOCKS
  MLOCK_T    mutex;     /* locate lock among fields that rarely change */
#endif /* USE_LOCKS */
  msegment   seg;
  تهی*      extp;      /* Unused but available for extensions */
  size_t     exts;
};

تعریفنوع ساختار malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

ساختار malloc_params {
  size_t magic;
  size_t page_size;
  size_t granularity;
  size_t mmap_threshold;
  size_t trim_threshold;
  flag_t default_mflags;
};

ایستا ساختار malloc_params mparams;

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
#اگر USE_LOCKS && defined(__ATOMIC_ACQUIRE)
# define ffi_mparams_magic_load()   __atomic_load_n(&mparams.magic, __ATOMIC_ACQUIRE)
# define ffi_mparams_magic_store(v) __atomic_store_n(&mparams.magic, (size_t)(v), __ATOMIC_RELEASE)
#وگرنهاگر USE_LOCKS && defined(_MSC_VER)
# اگر defined(_WIN64)
#  define ffi_mparams_magic_load() \
  ((size_t)_InterlockedCompareExchange64((__int64 فرار *)&mparams.magic, \
                                         (__int64)0, (__int64)0))
#  define ffi_mparams_magic_store(v) \
  ((تهی)_InterlockedExchange64((__int64 فرار *)&mparams.magic, \
                                (__int64)(v)))
# وگرنه
#  define ffi_mparams_magic_load() \
  ((size_t)_InterlockedCompareExchange((LONG فرار *)&mparams.magic, \
                                       (LONG)0, (LONG)0))
#  define ffi_mparams_magic_store(v) \
  ((تهی)_InterlockedExchange((LONG فرار *)&mparams.magic, (LONG)(v)))
# endif
#وگرنه
# define ffi_mparams_magic_load()   (mparams.magic)
# define ffi_mparams_magic_store(v) (*(فرار size_t *)(&(mparams.magic)) = (size_t)(v))
#endif

/* Ensure mparams initialized */
#define ensure_initialization() (تهی)(ffi_mparams_magic_load() != 0 || init_mparams())

#اگر !ONLY_MSPACES

/* The global malloc_state used for all non-"mspace" calls */
ایستا ساختار malloc_state _gm_;
#define gm                 (&_gm_)
#define is_global(M)       ((M) == &_gm_)

#endif /* !ONLY_MSPACES */

#define is_initialized(M)  ((M)->بالاترین != 0)

/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#define use_lock(M)           ((M)->mflags &   USE_LOCK_BIT)
#define enable_lock(M)        ((M)->mflags |=  USE_LOCK_BIT)
#اگر USE_LOCKS
#define disable_lock(M)       ((M)->mflags &= ~USE_LOCK_BIT)
#وگرنه
#define disable_lock(M)
#endif

#define use_mmap(M)           ((M)->mflags &   USE_MMAP_BIT)
#define enable_mmap(M)        ((M)->mflags |=  USE_MMAP_BIT)
#اگر HAVE_MMAP
#define disable_mmap(M)       ((M)->mflags &= ~USE_MMAP_BIT)
#وگرنه
#define disable_mmap(M)
#endif

#define use_noncontiguous(M)  ((M)->mflags &   USE_NONCONTIGUOUS_BIT)
#define disable_contiguous(M) ((M)->mflags |=  USE_NONCONTIGUOUS_BIT)

#define set_lock(M,L)\
 ((M)->mflags = (L)?\
  ((M)->mflags | USE_LOCK_BIT) :\
  ((M)->mflags & ~USE_LOCK_BIT))

/* page-align a size */
#define page_align(S)\
 (((S) + (mparams.page_size - SIZE_T_ONE)) & ~(mparams.page_size - SIZE_T_ONE))

/* granularity-align a size */
#define granularity_align(S)\
  (((S) + (mparams.granularity - SIZE_T_ONE))\
   & ~(mparams.granularity - SIZE_T_ONE))


/* For mmap, use granularity alignment on windows, else page-align */
#ifdef WIN32
#define mmap_align(S) granularity_align(S)
#وگرنه
#define mmap_align(S) page_align(S)
#endif

/* For sys_alloc, enough padding to ensure can malloc request on success */
#define SYS_ALLOC_PADDING (بالاترین_FOOT_SIZE + MALLOC_ALIGNMENT)

#define is_page_aligned(S)\
   (((size_t)(S) & (mparams.page_size - SIZE_T_ONE)) == 0)
#define is_granularity_aligned(S)\
   (((size_t)(S) & (mparams.granularity - SIZE_T_ONE)) == 0)

/*  True if segment S holds address A */
#define segment_holds(S, A)\
  ((نویسه*)(A) >= S->base && (نویسه*)(A) < S->base + S->size)

/* Return segment holding given address */
ایستا msegmentptr segment_holding(mstate m, نویسه* addr) {
  msegmentptr sp = &m->seg;
  برای (;;) {
    اگر (addr >= sp->base && addr < sp->base + sp->size)
      بازگشت sp;
    اگر ((sp = sp->next) == 0)
      بازگشت 0;
  }
}

/* Return true if segment contains a segment link */
ایستا صحیح has_segment_link(mstate m, msegmentptr ss) {
  msegmentptr sp = &m->seg;
  برای (;;) {
    اگر ((نویسه*)sp >= ss->base && (نویسه*)sp < ss->base + ss->size)
      بازگشت 1;
    اگر ((sp = sp->next) == 0)
      بازگشت 0;
  }
}

#ifndef MORECORE_CANNOT_TRIM
#define should_trim(M,s)  ((s) > (M)->trim_check)
#وگرنه  /* MORECORE_CANNOT_TRIM */
#define should_trim(M,s)  (0)
#endif /* MORECORE_CANNOT_TRIM */

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#define بالاترین_FOOT_SIZE\
  (align_offset(chunk2mem(0))+pad_request(اندازه(ساختار malloc_segment))+MIN_CHUNK_SIZE)


/* -------------------------------  Hooks -------------------------------- */

/*
  PREACTION should be defined to return 0 on success, and nonzero on
  failure. If you are not using locking, you can redefine these to do
  anything you like.
*/

#اگر USE_LOCKS
#define PREACTION(M)  ((use_lock(M))? ACQUIRE_LOCK(&(M)->mutex) : 0)
#define POSTACTION(M) { اگر (use_lock(M)) RELEASE_LOCK(&(M)->mutex); }
#وگرنه /* USE_LOCKS */

#ifndef PREACTION
#define PREACTION(M) (0)
#endif  /* PREACTION */

#ifndef POSTACTION
#define POSTACTION(M)
#endif  /* POSTACTION */

#endif /* USE_LOCKS */

/*
  CORRUPTION_ERROR_ACTION is triggered upon detected bad addresses.
  USAGE_ERROR_ACTION is triggered on detected bad frees and
  reallocs. The argument p is an address that might have triggered the
  fault. It is ignored by the two predefined actions, but might be
  useful in custom actions that try to help diagnose errors.
*/

#اگر PROCEED_ON_ERROR

/* A count of the number of corruption errors causing resets */
صحیح malloc_corruption_error_شمار;

/* default corruption action */
ایستا تهی reset_on_error(mstate m);

#define CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#define USAGE_ERROR_ACTION(m, p)

#وگرنه /* PROCEED_ON_ERROR */

#ifndef CORRUPTION_ERROR_ACTION
#define CORRUPTION_ERROR_ACTION(m) ABORT
#endif /* CORRUPTION_ERROR_ACTION */

#ifndef USAGE_ERROR_ACTION
#define USAGE_ERROR_ACTION(m,p) ABORT
#endif /* USAGE_ERROR_ACTION */

#endif /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#اگر ! DEBUG

#define check_free_chunk(M,P)
#define check_inuse_chunk(M,P)
#define check_malloced_chunk(M,P,N)
#define check_mmapped_chunk(M,P)
#define check_malloc_state(M)
#define check_بالاترین_chunk(M,P)

#وگرنه /* DEBUG */
#define check_free_chunk(M,P)       do_check_free_chunk(M,P)
#define check_inuse_chunk(M,P)      do_check_inuse_chunk(M,P)
#define check_بالاترین_chunk(M,P)        do_check_بالاترین_chunk(M,P)
#define check_malloced_chunk(M,P,N) do_check_malloced_chunk(M,P,N)
#define check_mmapped_chunk(M,P)    do_check_mmapped_chunk(M,P)
#define check_malloc_state(M)       do_check_malloc_state(M)

ایستا تهی   do_check_any_chunk(mstate m, mchunkptr p);
ایستا تهی   do_check_بالاترین_chunk(mstate m, mchunkptr p);
ایستا تهی   do_check_mmapped_chunk(mstate m, mchunkptr p);
ایستا تهی   do_check_inuse_chunk(mstate m, mchunkptr p);
ایستا تهی   do_check_free_chunk(mstate m, mchunkptr p);
ایستا تهی   do_check_malloced_chunk(mstate m, تهی* mem, size_t s);
ایستا تهی   do_check_درخت(mstate m, tchunkptr t);
ایستا تهی   do_check_treebin(mstate m, bindex_t i);
ایستا تهی   do_check_smallbin(mstate m, bindex_t i);
ایستا تهی   do_check_malloc_state(mstate m);
ایستا صحیح    bin_find(mstate m, mchunkptr x);
ایستا size_t traverse_and_check(mstate m);
#endif /* DEBUG */

/* ---------------------------- Indexing Bins ---------------------------- */

#define is_small(s)         (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)      (bindex_t)((s)  >> SMALLBIN_SHIFT)
#define small_index2size(i) ((i)  << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX     (small_index(MIN_CHUNK_SIZE))

/* addressing by index. See above about smallbin repositioning */
#define smallbin_at(M, i)   ((sbinptr)((نویسه*)&((M)->smallbins[(i)<<1])))
#define treebin_at(M,i)     (&((M)->treebins[i]))

/* assign tree index for size S to variable I.
   libffi: enable the __builtin_clz path on any GNU-compatible compiler, not
   only x86, so the log2 is an intrinsic rather than the generic C idiom that
   the compiler can't always recognize (libffi #754).  */
#اگر defined(__GNUC__)
#define compute_درخت_index(S, I)\
{\
  unsigned صحیح X = S >> TREEBIN_SHIFT;\
  اگر (X == 0)\
    I = 0;\
  وگرنه اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وگرنه {\
    unsigned صحیح K = (unsigned) اندازه(X)*__CHAR_BIT__ - 1 - (unsigned) __builtin_clz(X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

#وگرنهاگر defined (__INTEL_COMPILER)
#define compute_درخت_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  اگر (X == 0)\
    I = 0;\
  وگرنه اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وگرنه {\
    unsigned صحیح K = _bit_scan_reverse (X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

#وگرنهاگر defined(_MSC_VER) && _MSC_VER>=1300
#define compute_درخت_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  اگر (X == 0)\
    I = 0;\
  وگرنه اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وگرنه {\
    unsigned صحیح K;\
    _BitScanReverse((DWORD *) &K, (DWORD) X);\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

#وگرنه /* GNUC */
#define compute_درخت_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  اگر (X == 0)\
    I = 0;\
  وگرنه اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وگرنه {\
    unsigned صحیح Y = (unsigned صحیح)X;\
    unsigned صحیح N = ((Y - 0x100) >> 16) & 8;\
    unsigned صحیح K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1));\
  }\
}
#endif /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#define bit_for_درخت_index(i) \
   (i == NTREEBINS-1)? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#define leftshift_for_درخت_index(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_درخت_index(i) \
   ((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
   (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#define idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#define clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#define smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#define mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#define clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#define treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#define least_bit(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#define چپ_bits(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#define same_or_چپ_bits(x) ((x) | -(x))

/* index corresponding to given bit.
   libffi: use __builtin_ctz on any GNU-compatible compiler, not only x86
   (libffi #754).  */

#اگر defined(__GNUC__)
#define compute_bit2idx(X, I)\
{\
  unsigned صحیح J;\
  J = __builtin_ctz(X); \
  I = (bindex_t)J;\
}

#وگرنهاگر defined (__INTEL_COMPILER)
#define compute_bit2idx(X, I)\
{\
  unsigned صحیح J;\
  J = _bit_scan_forward (X); \
  I = (bindex_t)J;\
}

#وگرنهاگر defined(_MSC_VER) && _MSC_VER>=1300
#define compute_bit2idx(X, I)\
{\
  unsigned صحیح J;\
  _BitScanForward((DWORD *) &J, X);\
  I = (bindex_t)J;\
}

#وگرنهاگر USE_BUILTIN_FFS
#define compute_bit2idx(X, I) I = ffs(X)-1

#وگرنه
#define compute_bit2idx(X, I)\
{\
  unsigned صحیح Y = X - 1;\
  unsigned صحیح K = Y >> (16-4) & 16;\
  unsigned صحیح N = K;        Y >>= K;\
  N += K = Y >> (8-3) &  8;  Y >>= K;\
  N += K = Y >> (4-2) &  4;  Y >>= K;\
  N += K = Y >> (2-1) &  2;  Y >>= K;\
  N += K = Y >> (1-0) &  1;  Y >>= K;\
  I = (bindex_t)(N + Y);\
}
#endif /* GNUC */


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

#اگر !INSECURE
/* Check if address a is at least as high as any from MORECORE or MMAP */
#define ok_address(M, a) ((نویسه*)(a) >= (M)->least_addr)
/* Check if address of next chunk n is higher than base chunk p */
#define ok_next(p, n)    ((نویسه*)(p) < (نویسه*)(n))
/* Check if p has inuse status */
#define ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#define ok_pinuse(p)     pinuse(p)

#وگرنه /* !INSECURE */
#define ok_address(M, a) (1)
#define ok_next(b, n)    (1)
#define ok_inuse(p)      (1)
#define ok_pinuse(p)     (1)
#endif /* !INSECURE */

#اگر (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#define ok_magic(M)      ((M)->magic == mparams.magic)
#وگرنه  /* (FOOTERS && !INSECURE) */
#define ok_magic(M)      (1)
#endif /* (FOOTERS && !INSECURE) */

/* In gcc, use __builtin_expect to minimize impact of checks */
#اگر !INSECURE
#اگر defined(__GNUC__) && __GNUC__ >= 3
#define RTCHECK(e)  __builtin_expect(e, 1)
#وگرنه /* GNUC */
#define RTCHECK(e)  (e)
#endif /* GNUC */
#وگرنه /* !INSECURE */
#define RTCHECK(e)  (1)
#endif /* !INSECURE */

/* macros to set up inuse chunks with or without footers */

#اگر !FOOTERS

#define mark_inuse_foot(M,p,s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#define set_inuse(M,p,s)\
  ((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
  ((mchunkptr)(((نویسه*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#define set_inuse_and_pinuse(M,p,s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  ((mchunkptr)(((نویسه*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set size, cinuse and pinuse bit of this chunk */
#define set_size_and_pinuse_of_inuse_chunk(M, p, s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT))

#وگرنه /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#define mark_inuse_foot(M,p,s)\
  (((mchunkptr)((نویسه*)(p) + (s)))->prev_foot = ((size_t)(M) ^ mparams.magic))

#define get_mstate_for(p)\
  ((mstate)(((mchunkptr)((نویسه*)(p) +\
    (chunksize(p))))->prev_foot ^ mparams.magic))

#define set_inuse(M,p,s)\
  ((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
  (((mchunkptr)(((نویسه*)(p)) + (s)))->head |= PINUSE_BIT), \
  mark_inuse_foot(M,p,s))

#define set_inuse_and_pinuse(M,p,s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  (((mchunkptr)(((نویسه*)(p)) + (s)))->head |= PINUSE_BIT),\
 mark_inuse_foot(M,p,s))

#define set_size_and_pinuse_of_inuse_chunk(M, p, s)\
  ((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
  mark_inuse_foot(M, p, s))

#endif /* !FOOTERS */

/* ---------------------------- setting mparams -------------------------- */

#اگر LOCK_AT_FORK
ایستا تهی pre_fork(تهی)         { ACQUIRE_LOCK(&(gm)->mutex); }
ایستا تهی post_fork_parent(تهی) { RELEASE_LOCK(&(gm)->mutex); }
ایستا تهی post_fork_child(تهی)  { INITIAL_LOCK(&(gm)->mutex); }
#endif /* LOCK_AT_FORK */

/* Initialize mparams */
ایستا صحیح init_mparams(تهی) {
#ifdef NEED_GLOBAL_LOCK_INIT
  اگر (malloc_global_mutex_status <= 0)
    init_malloc_global_mutex();
#endif

  ACQUIRE_MALLOC_GLOBAL_LOCK();
  اگر (mparams.magic == 0) {
    size_t magic;
    size_t psize;
    size_t gsize;

#اگر !defined(WIN32) && !defined(__OS2__)
    psize = malloc_getpagesize;
    gsize = ((DEFAULT_GRANULARITY != 0)? DEFAULT_GRANULARITY : psize);
#وگرنهاگر defined (__OS2__)
    /* if low-memory is used, os2munmap() would break
       if it were anything other than 64k */
    psize = 4096u;
    gsize = 65536u;
#وگرنه /* WIN32 */
    {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      psize = system_info.dwPageSize;
      gsize = ((DEFAULT_GRANULARITY != 0)?
               DEFAULT_GRANULARITY : system_info.dwAllocationGranularity);
    }
#endif /* WIN32 */

    /* Sanity-check configuration:
       size_t must be unsigned and as wide as pointer type.
       ints must be at least 4 bytes.
       alignment must be at least 8.
       Alignment, min chunk size, and page size must all be powers of 2.
    */
    اگر ((اندازه(size_t) != اندازه(نویسه*)) ||
        (MAX_SIZE_T < MIN_CHUNK_SIZE)  ||
        (اندازه(صحیح) < 4)  ||
        (MALLOC_ALIGNMENT < (size_t)8U) ||
        ((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-SIZE_T_ONE)) != 0) ||
        ((MCHUNK_SIZE      & (MCHUNK_SIZE-SIZE_T_ONE))      != 0) ||
        ((gsize            & (gsize-SIZE_T_ONE))            != 0) ||
        ((psize            & (psize-SIZE_T_ONE))            != 0))
      ABORT;
    mparams.granularity = gsize;
    mparams.page_size = psize;
    mparams.mmap_threshold = DEFAULT_MMAP_THRESHOLD;
    mparams.trim_threshold = DEFAULT_TRIM_THRESHOLD;
#اگر MORECORE_CONTIGUOUS
    mparams.default_mflags = USE_LOCK_BIT|USE_MMAP_BIT;
#وگرنه  /* MORECORE_CONTIGUOUS */
    mparams.default_mflags = USE_LOCK_BIT|USE_MMAP_BIT|USE_NONCONTIGUOUS_BIT;
#endif /* MORECORE_CONTIGUOUS */

#اگر !ONLY_MSPACES
    /* Set up lock for main malloc area */
    gm->mflags = mparams.default_mflags;
    (تهی)INITIAL_LOCK(&gm->mutex);
#endif
#اگر LOCK_AT_FORK
    pthread_atfork(&pre_fork, &post_fork_parent, &post_fork_child);
#endif

    {
#اگر USE_DEV_RANDOM
      صحیح fd;
      unsigned نویسه buf[اندازه(size_t)];
      /* Try to use /dev/urandom, else fall back on using time */
      اگر ((fd = open("/dev/urandom", O_RDONLY)) >= 0 &&
          اقرأ(fd, buf, اندازه(buf)) == اندازه(buf)) {
        magic = *((size_t *) buf);
        close(fd);
      }
      وگرنه
#endif /* USE_DEV_RANDOM */
#ifdef WIN32
      magic = (size_t)(Get_Tick_شمار() ^ (size_t)0x55555555U);
#وگرنهاگر defined(LACKS_TIME_H)
      magic = (size_t)&magic ^ (size_t)0x55555555U;
#وگرنه
      magic = (size_t)(time(0) ^ (size_t)0x55555555U);
#endif
      magic |= (size_t)8U;    /* ensure nonzero */
      magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
      /* libffi: release-store so any thread that observes magic via the
         acquire load in ensure_initialization() sees the writes above (#873).  */
      ffi_mparams_magic_store(magic);
    }
  }

  RELEASE_MALLOC_GLOBAL_LOCK();
  بازگشت 1;
}

/* support for mallopt */
ایستا صحیح change_mparam(صحیح param_عدد, صحیح مقدار) {
  size_t val;
  ensure_initialization();
  val = (مقدار == -1)? MAX_SIZE_T : (size_t)مقدار;
  انتخاب(param_عدد) {
  حالت M_TRIM_THRESHOLD:
    mparams.trim_threshold = val;
    بازگشت 1;
  حالت M_GRANULARITY:
    اگر (val >= mparams.page_size && ((val & (val-1)) == 0)) {
      mparams.granularity = val;
      بازگشت 1;
    }
    وگرنه
      بازگشت 0;
  حالت M_MMAP_THRESHOLD:
    mparams.mmap_threshold = val;
    بازگشت 1;
  پیشفرض:
    بازگشت 0;
  }
}

#اگر DEBUG
/* ------------------------- Debugging Support --------------------------- */

/* Check properties of any chunk, whether free, inuse, mmapped etc  */
ایستا تهی do_check_any_chunk(mstate m, mchunkptr p) {
  أكد((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
  أكد(ok_address(m, p));
}

/* Check properties of top chunk */
ایستا تهی do_check_بالاترین_chunk(mstate m, mchunkptr p) {
  msegmentptr sp = segment_holding(m, (نویسه*)p);
  size_t  sz = p->head & ~INUSE_BITS; /* third-lowest bit can be set! */
  أكد(sp != 0);
  أكد((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
  أكد(ok_address(m, p));
  أكد(sz == m->topsize);
  أكد(sz > 0);
  أكد(sz == ((sp->base + sp->size) - (نویسه*)p) - بالاترین_FOOT_SIZE);
  أكد(pinuse(p));
  أكد(!pinuse(chunk_plus_offset(p, sz)));
}

/* Check properties of (inuse) mmapped chunks */
ایستا تهی do_check_mmapped_chunk(mstate m, mchunkptr p) {
  size_t  sz = chunksize(p);
  size_t len = (sz + (p->prev_foot) + MMAP_FOOT_PAD);
  أكد(is_mmapped(p));
  أكد(use_mmap(m));
  أكد((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
  أكد(ok_address(m, p));
  أكد(!is_small(sz));
  أكد((len & (mparams.page_size-SIZE_T_ONE)) == 0);
  أكد(chunk_plus_offset(p, sz)->head == FENCEPOST_HEAD);
  أكد(chunk_plus_offset(p, sz+SIZE_T_SIZE)->head == 0);
}

/* Check properties of inuse chunks */
ایستا تهی do_check_inuse_chunk(mstate m, mchunkptr p) {
  do_check_any_chunk(m, p);
  أكد(is_inuse(p));
  أكد(next_pinuse(p));
  /* If not pinuse and not mmapped, previous chunk has OK offset */
  أكد(is_mmapped(p) || pinuse(p) || next_chunk(prev_chunk(p)) == p);
  اگر (is_mmapped(p))
    do_check_mmapped_chunk(m, p);
}

/* Check properties of free chunks */
ایستا تهی do_check_free_chunk(mstate m, mchunkptr p) {
  size_t sz = chunksize(p);
  mchunkptr next = chunk_plus_offset(p, sz);
  do_check_any_chunk(m, p);
  أكد(!is_inuse(p));
  أكد(!next_pinuse(p));
  أكد (!is_mmapped(p));
  اگر (p != m->dv && p != m->بالاترین) {
    اگر (sz >= MIN_CHUNK_SIZE) {
      أكد((sz & CHUNK_ALIGN_MASK) == 0);
      أكد(is_aligned(chunk2mem(p)));
      أكد(next->prev_foot == sz);
      أكد(pinuse(p));
      أكد (next == m->بالاترین || is_inuse(next));
      أكد(p->fd->bk == p);
      أكد(p->bk->fd == p);
    }
    وگرنه  /* markers are always of size SIZE_T_SIZE */
      أكد(sz == SIZE_T_SIZE);
  }
}

/* Check properties of malloced chunks at the point they are malloced */
ایستا تهی do_check_malloced_chunk(mstate m, تهی* mem, size_t s) {
  اگر (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    size_t sz = p->head & ~INUSE_BITS;
    do_check_inuse_chunk(m, p);
    أكد((sz & CHUNK_ALIGN_MASK) == 0);
    أكد(sz >= MIN_CHUNK_SIZE);
    أكد(sz >= s);
    /* unless mmapped, size is less than MIN_CHUNK_SIZE more than request */
    أكد(is_mmapped(p) || sz < (s + MIN_CHUNK_SIZE));
  }
}

/* Check a tree and its subtrees.  */
ایستا تهی do_check_درخت(mstate m, tchunkptr t) {
  tchunkptr head = 0;
  tchunkptr u = t;
  bindex_t tindex = t->index;
  size_t tsize = chunksize(t);
  bindex_t idx;
  compute_درخت_index(tsize, idx);
  أكد(tindex == idx);
  أكد(tsize >= MIN_LARGE_SIZE);
  أكد(tsize >= minsize_for_درخت_index(idx));
  أكد((idx == NTREEBINS-1) || (tsize < minsize_for_درخت_index((idx+1))));

  انجامبده { /* traverse through chain of same-sized nodes */
    do_check_any_chunk(m, ((mchunkptr)u));
    أكد(u->index == tindex);
    أكد(chunksize(u) == tsize);
    أكد(!is_inuse(u));
    أكد(!next_pinuse(u));
    أكد(u->fd->bk == u);
    أكد(u->bk->fd == u);
    اگر (u->parent == 0) {
      أكد(u->child[0] == 0);
      أكد(u->child[1] == 0);
    }
    وگرنه {
      أكد(head == 0); /* only one node on chain has parent */
      head = u;
      أكد(u->parent != u);
      أكد (u->parent->child[0] == u ||
              u->parent->child[1] == u ||
              *((tbinptr*)(u->parent)) == u);
      اگر (u->child[0] != 0) {
        أكد(u->child[0]->parent == u);
        أكد(u->child[0] != u);
        do_check_درخت(m, u->child[0]);
      }
      اگر (u->child[1] != 0) {
        أكد(u->child[1]->parent == u);
        أكد(u->child[1] != u);
        do_check_درخت(m, u->child[1]);
      }
      اگر (u->child[0] != 0 && u->child[1] != 0) {
        أكد(chunksize(u->child[0]) < chunksize(u->child[1]));
      }
    }
    u = u->fd;
  } وقتی (u != t);
  أكد(head != 0);
}

/*  Check all the chunks in a treebin.  */
ایستا تهی do_check_treebin(mstate m, bindex_t i) {
  tbinptr* tb = treebin_at(m, i);
  tchunkptr t = *tb;
  صحیح empty = (m->treemap & (1U << i)) == 0;
  اگر (t == 0)
    أكد(empty);
  اگر (!empty)
    do_check_درخت(m, t);
}

/*  Check all the chunks in a smallbin.  */
ایستا تهی do_check_smallbin(mstate m, bindex_t i) {
  sbinptr b = smallbin_at(m, i);
  mchunkptr p = b->bk;
  unsigned صحیح empty = (m->smallmap & (1U << i)) == 0;
  اگر (p == b)
    أكد(empty);
  اگر (!empty) {
    برای (; p != b; p = p->bk) {
      size_t size = chunksize(p);
      mchunkptr q;
      /* each chunk claims to be free */
      do_check_free_chunk(m, p);
      /* chunk belongs in bin */
      أكد(small_index(size) == i);
      أكد(p->bk == b || chunksize(p->bk) == chunksize(p));
      /* chunk is followed by an inuse chunk */
      q = next_chunk(p);
      اگر (q->head != FENCEPOST_HEAD)
        do_check_inuse_chunk(m, q);
    }
  }
}

/* Find x in a bin. Used in other check functions. */
ایستا صحیح bin_find(mstate m, mchunkptr x) {
  size_t size = chunksize(x);
  اگر (is_small(size)) {
    bindex_t sidx = small_index(size);
    sbinptr b = smallbin_at(m, sidx);
    اگر (smallmap_is_marked(m, sidx)) {
      mchunkptr p = b;
      انجامبده {
        اگر (p == x)
          بازگشت 1;
      } وقتی ((p = p->fd) != b);
    }
  }
  وگرنه {
    bindex_t tidx;
    compute_درخت_index(size, tidx);
    اگر (treemap_is_marked(m, tidx)) {
      tchunkptr t = *treebin_at(m, tidx);
      size_t sizebits = size << leftshift_for_درخت_index(tidx);
      وقتی (t != 0 && chunksize(t) != size) {
        t = t->child[(sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1];
        sizebits <<= 1;
      }
      اگر (t != 0) {
        tchunkptr u = t;
        انجامبده {
          اگر (u == (tchunkptr)x)
            بازگشت 1;
        } وقتی ((u = u->fd) != t);
      }
    }
  }
  بازگشت 0;
}

/* Traverse each chunk and check it; return total */
ایستا size_t traverse_and_check(mstate m) {
  size_t sum = 0;
  اگر (is_initialized(m)) {
    msegmentptr s = &m->seg;
    sum += m->topsize + بالاترین_FOOT_SIZE;
    وقتی (s != 0) {
      mchunkptr q = align_as_chunk(s->base);
      mchunkptr lastq = 0;
      أكد(pinuse(q));
      وقتی (segment_holds(s, q) &&
             q != m->بالاترین && q->head != FENCEPOST_HEAD) {
        sum += chunksize(q);
        اگر (is_inuse(q)) {
          أكد(!bin_find(m, q));
          do_check_inuse_chunk(m, q);
        }
        وگرنه {
          أكد(q == m->dv || bin_find(m, q));
          أكد(lastq == 0 || is_inuse(lastq)); /* Not 2 consecutive free */
          do_check_free_chunk(m, q);
        }
        lastq = q;
        q = next_chunk(q);
      }
      s = s->next;
    }
  }
  بازگشت sum;
}


/* Check all properties of malloc_state. */
ایستا تهی do_check_malloc_state(mstate m) {
  bindex_t i;
  size_t total;
  /* check bins */
  برای (i = 0; i < NSMALLBINS; ++i)
    do_check_smallbin(m, i);
  برای (i = 0; i < NTREEBINS; ++i)
    do_check_treebin(m, i);

  اگر (m->dvsize != 0) { /* check dv chunk */
    do_check_any_chunk(m, m->dv);
    أكد(m->dvsize == chunksize(m->dv));
    أكد(m->dvsize >= MIN_CHUNK_SIZE);
    أكد(bin_find(m, m->dv) == 0);
  }

  اگر (m->بالاترین != 0) {   /* check top chunk */
    do_check_بالاترین_chunk(m, m->بالاترین);
    /*assert(m->topsize == chunksize(m->top)); redundant */
    أكد(m->topsize > 0);
    أكد(bin_find(m, m->بالاترین) == 0);
  }

  total = traverse_and_check(m);
  أكد(total <= m->footprint);
  أكد(m->footprint <= m->max_footprint);
}
#endif /* DEBUG */

/* ----------------------------- statistics ------------------------------ */

#اگر !NO_MALLINFO
ایستا ساختار mallinfo internal_mallinfo(mstate m) {
  ساختار mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ensure_initialization();
  اگر (!PREACTION(m)) {
    check_malloc_state(m);
    اگر (is_initialized(m)) {
      size_t nfree = SIZE_T_ONE; /* top always free */
      size_t mfree = m->topsize + بالاترین_FOOT_SIZE;
      size_t sum = mfree;
      msegmentptr s = &m->seg;
      وقتی (s != 0) {
        mchunkptr q = align_as_chunk(s->base);
        وقتی (segment_holds(s, q) &&
               q != m->بالاترین && q->head != FENCEPOST_HEAD) {
          size_t sz = chunksize(q);
          sum += sz;
          اگر (!is_inuse(q)) {
            mfree += sz;
            ++nfree;
          }
          q = next_chunk(q);
        }
        s = s->next;
      }

      nm.arena    = sum;
      nm.ordblks  = nfree;
      nm.hblkhd   = m->footprint - sum;
      nm.usmblks  = m->max_footprint;
      nm.uordblks = m->footprint - mfree;
      nm.fordblks = mfree;
      nm.keepcost = m->topsize;
    }

    POSTACTION(m);
  }
  بازگشت nm;
}
#endif /* !NO_MALLINFO */

#اگر !NO_MALLOC_STATS
ایستا تهی internal_malloc_stats(mstate m) {
  ensure_initialization();
  اگر (!PREACTION(m)) {
    size_t maxfp = 0;
    size_t fp = 0;
    size_t used = 0;
    check_malloc_state(m);
    اگر (is_initialized(m)) {
      msegmentptr s = &m->seg;
      maxfp = m->max_footprint;
      fp = m->footprint;
      used = fp - (m->topsize + بالاترین_FOOT_SIZE);

      وقتی (s != 0) {
        mchunkptr q = align_as_chunk(s->base);
        وقتی (segment_holds(s, q) &&
               q != m->بالاترین && q->head != FENCEPOST_HEAD) {
          اگر (!is_inuse(q))
            used -= chunksize(q);
          q = next_chunk(q);
        }
        s = s->next;
      }
    }
    POSTACTION(m); /* drop lock */
    fprintf(stderr, "max system bytes = %10lu\n", (unsigned long)(maxfp));
    fprintf(stderr, "system bytes     = %10lu\n", (unsigned long)(fp));
    fprintf(stderr, "in use bytes     = %10lu\n", (unsigned long)(used));
  }
}
#endif /* NO_MALLOC_STATS */

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#define درج_small_chunk(M, P, S) {\
  bindex_t I  = small_index(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  أكد(S >= MIN_CHUNK_SIZE);\
  اگر (!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  وگرنه اگر (RTCHECK(ok_address(M, B->fd)))\
    F = B->fd;\
  وگرنه {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#define unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  bindex_t I = small_index(S);\
  أكد(P != B);\
  أكد(P != F);\
  أكد(chunksize(P) == small_index2size(I));\
  اگر (RTCHECK(F == smallbin_at(M,I) || (ok_address(M, F) && F->bk == P))) { \
    اگر (B == F) {\
      clear_smallmap(M, I);\
    }\
    وگرنه اگر (RTCHECK(B == smallbin_at(M,I) ||\
                     (ok_address(M, B) && B->fd == P))) {\
      F->bk = B;\
      B->fd = F;\
    }\
    وگرنه {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  وگرنه {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Unlink the first chunk from a smallbin */
#define unlink_first_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  أكد(P != B);\
  أكد(P != F);\
  أكد(chunksize(P) == small_index2size(I));\
  اگر (B == F) {\
    clear_smallmap(M, I);\
  }\
  وگرنه اگر (RTCHECK(ok_address(M, F) && F->bk == P)) {\
    F->bk = B;\
    B->fd = F;\
  }\
  وگرنه {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#define replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  أكد(is_small(DVS));\
  اگر (DVS != 0) {\
    mchunkptr DV = M->dv;\
    درج_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#define درج_large_chunk(M, X, S) {\
  tbinptr* H;\
  bindex_t I;\
  compute_درخت_index(S, I);\
  H = treebin_at(M, I);\
  X->index = I;\
  X->child[0] = X->child[1] = 0;\
  اگر (!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->parent = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  وگرنه {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_درخت_index(I);\
    برای (;;) {\
      اگر (chunksize(T) != S) {\
        tchunkptr* C = &(T->child[(K >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1]);\
        K <<= 1;\
        اگر (*C != 0)\
          T = *C;\
        وگرنه اگر (RTCHECK(ok_address(M, C))) {\
          *C = X;\
          X->parent = T;\
          X->fd = X->bk = X;\
          بشکن;\
        }\
        وگرنه {\
          CORRUPTION_ERROR_ACTION(M);\
          بشکن;\
        }\
      }\
      وگرنه {\
        tchunkptr F = T->fd;\
        اگر (RTCHECK(ok_address(M, T) && ok_address(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->parent = 0;\
          بشکن;\
        }\
        وگرنه {\
          CORRUPTION_ERROR_ACTION(M);\
          بشکن;\
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

#define unlink_large_chunk(M, X) {\
  tchunkptr XP = X->parent;\
  tchunkptr R;\
  اگر (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    اگر (RTCHECK(ok_address(M, F) && F->bk == X && R->fd == X)) {\
      F->bk = R;\
      R->fd = F;\
    }\
    وگرنه {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  وگرنه {\
    tchunkptr* RP;\
    اگر (((R = *(RP = &(X->child[1]))) != 0) ||\
        ((R = *(RP = &(X->child[0]))) != 0)) {\
      tchunkptr* CP;\
      وقتی ((*(CP = &(R->child[1])) != 0) ||\
             (*(CP = &(R->child[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      اگر (RTCHECK(ok_address(M, RP)))\
        *RP = 0;\
      وگرنه {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  اگر (XP != 0) {\
    tbinptr* H = treebin_at(M, X->index);\
    اگر (X == *H) {\
      اگر ((*H = R) == 0) \
        clear_treemap(M, X->index);\
    }\
    وگرنه اگر (RTCHECK(ok_address(M, XP))) {\
      اگر (XP->child[0] == X) \
        XP->child[0] = R;\
      وگرنه \
        XP->child[1] = R;\
    }\
    وگرنه\
      CORRUPTION_ERROR_ACTION(M);\
    اگر (R != 0) {\
      اگر (RTCHECK(ok_address(M, R))) {\
        tchunkptr C0, C1;\
        R->parent = XP;\
        اگر ((C0 = X->child[0]) != 0) {\
          اگر (RTCHECK(ok_address(M, C0))) {\
            R->child[0] = C0;\
            C0->parent = R;\
          }\
          وگرنه\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        اگر ((C1 = X->child[1]) != 0) {\
          اگر (RTCHECK(ok_address(M, C1))) {\
            R->child[1] = C1;\
            C1->parent = R;\
          }\
          وگرنه\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      وگرنه\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}

/* Relays to large vs small bin operations */

#define درج_chunk(M, P, S)\
  اگر (is_small(S)) درج_small_chunk(M, P, S)\
  وگرنه { tchunkptr TP = (tchunkptr)(P); درج_large_chunk(M, TP, S); }

#define unlink_chunk(M, P, S)\
  اگر (is_small(S)) unlink_small_chunk(M, P, S)\
  وگرنه { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#اگر ONLY_MSPACES
#define internal_malloc(m, b) mspace_malloc(m, b)
#define internal_free(m, mem) mspace_free(m,mem);
#وگرنه /* ONLY_MSPACES */
#اگر MSPACES
#define internal_malloc(m, b)\
  ((m == gm)? dlmalloc(b) : mspace_malloc(m, b))
#define internal_free(m, mem)\
   اگر (m == gm) dlfree(mem); وگرنه mspace_free(m,mem);
#وگرنه /* MSPACES */
#define internal_malloc(m, b) dlmalloc(b)
#define internal_free(m, mem) dlfree(mem)
#endif /* MSPACES */
#endif /* ONLY_MSPACES */

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
ایستا تهی* mmap_alloc(mstate m, size_t nb) {
  size_t mmsize = mmap_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
  اگر (m->footprint_limit != 0) {
    size_t fp = m->footprint + mmsize;
    اگر (fp <= m->footprint || fp > m->footprint_limit)
      بازگشت 0;
  }
  اگر (mmsize > nb) {     /* Check for wrap around 0 */
    نویسه* mm = (نویسه*)(CALL_DIRECT_MMAP(mmsize));
    اگر (mm != CMFAIL) {
      size_t offset = align_offset(chunk2mem(mm));
      size_t psize = mmsize - offset - MMAP_FOOT_PAD;
      mchunkptr p = (mchunkptr)(mm + offset);
      p->prev_foot = offset;
      p->head = psize;
      mark_inuse_foot(m, p, psize);
      chunk_plus_offset(p, psize)->head = FENCEPOST_HEAD;
      chunk_plus_offset(p, psize+SIZE_T_SIZE)->head = 0;

      اگر (m->least_addr == 0 || mm < m->least_addr)
        m->least_addr = mm;
      اگر ((m->footprint += mmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      أكد(is_aligned(chunk2mem(p)));
      check_mmapped_chunk(m, p);
      بازگشت chunk2mem(p);
    }
  }
  بازگشت 0;
}

/* Realloc using mmap */
ایستا mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb, صحیح flags) {
  size_t oldsize = chunksize(oldp);
  (تهی)flags; /* placate people compiling -Wunused */
  اگر (is_small(nb)) /* Can't shrink mmap regions below small size */
    بازگشت 0;
  /* Keep old chunk if big enough but not too big */
  اگر (oldsize >= nb + SIZE_T_SIZE &&
      (oldsize - nb) <= (mparams.granularity << 1))
    بازگشت oldp;
  وگرنه {
    size_t offset = oldp->prev_foot;
    size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
    size_t newmmsize = mmap_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
    نویسه* cp = (نویسه*)CALL_MREMAP((نویسه*)oldp - offset,
                                  oldmmsize, newmmsize, flags);
    اگر (cp != CMFAIL) {
      mchunkptr newp = (mchunkptr)(cp + offset);
      size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
      newp->head = psize;
      mark_inuse_foot(m, newp, psize);
      chunk_plus_offset(newp, psize)->head = FENCEPOST_HEAD;
      chunk_plus_offset(newp, psize+SIZE_T_SIZE)->head = 0;

      اگر (cp < m->least_addr)
        m->least_addr = cp;
      اگر ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      check_mmapped_chunk(m, newp);
      بازگشت newp;
    }
  }
  بازگشت 0;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
ایستا تهی init_بالاترین(mstate m, mchunkptr p, size_t psize) {
  /* Ensure alignment */
  size_t offset = align_offset(chunk2mem(p));
  p = (mchunkptr)((نویسه*)p + offset);
  psize -= offset;

  m->بالاترین = p;
  m->topsize = psize;
  p->head = psize | PINUSE_BIT;
  /* set size of fake trailing chunk holding overhead space only once */
  chunk_plus_offset(p, psize)->head = بالاترین_FOOT_SIZE;
  m->trim_check = mparams.trim_threshold; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
ایستا تهی init_bins(mstate m) {
  /* Establish circular links for smallbins */
  bindex_t i;
  برای (i = 0; i < NSMALLBINS; ++i) {
    sbinptr bin = smallbin_at(m,i);
    bin->fd = bin->bk = bin;
  }
}

#اگر PROCEED_ON_ERROR

/* default corruption action */
ایستا تهی reset_on_error(mstate m) {
  صحیح i;
  ++malloc_corruption_error_شمار;
  /* Reinitialize fields to forget about all memory */
  m->smallmap = m->treemap = 0;
  m->dvsize = m->topsize = 0;
  m->seg.base = 0;
  m->seg.size = 0;
  m->seg.next = 0;
  m->بالاترین = m->dv = 0;
  برای (i = 0; i < NTREEBINS; ++i)
    *treebin_at(m, i) = 0;
  init_bins(m);
}
#endif /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
ایستا تهی* prepend_alloc(mstate m, نویسه* newbase, نویسه* oldbase,
                           size_t nb) {
  mchunkptr p = align_as_chunk(newbase);
  mchunkptr oldfirst = align_as_chunk(oldbase);
  size_t psize = (نویسه*)oldfirst - (نویسه*)p;
  mchunkptr q = chunk_plus_offset(p, nb);
  size_t qsize = psize - nb;
  set_size_and_pinuse_of_inuse_chunk(m, p, nb);

  أكد((نویسه*)oldfirst > (نویسه*)q);
  أكد(pinuse(oldfirst));
  أكد(qsize >= MIN_CHUNK_SIZE);

  /* consolidate remainder with first chunk of old base */
  اگر (oldfirst == m->بالاترین) {
    size_t tsize = m->topsize += qsize;
    m->بالاترین = q;
    q->head = tsize | PINUSE_BIT;
    check_بالاترین_chunk(m, q);
  }
  وگرنه اگر (oldfirst == m->dv) {
    size_t dsize = m->dvsize += qsize;
    m->dv = q;
    set_size_and_pinuse_of_free_chunk(q, dsize);
  }
  وگرنه {
    اگر (!is_inuse(oldfirst)) {
      size_t nsize = chunksize(oldfirst);
      unlink_chunk(m, oldfirst, nsize);
      oldfirst = chunk_plus_offset(oldfirst, nsize);
      qsize += nsize;
    }
    set_free_with_pinuse(q, qsize, oldfirst);
    درج_chunk(m, q, qsize);
    check_free_chunk(m, q);
  }

  check_malloced_chunk(m, chunk2mem(p), nb);
  بازگشت chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
ایستا تهی add_segment(mstate m, نویسه* tbase, size_t tsize, flag_t mmapped) {
  /* Determine locations and sizes of segment, fenceposts, old top */
  نویسه* old_بالاترین = (نویسه*)m->بالاترین;
  msegmentptr oldsp = segment_holding(m, old_بالاترین);
  نویسه* old_end = oldsp->base + oldsp->size;
  size_t ssize = pad_request(اندازه(ساختار malloc_segment));
  نویسه* rawsp = old_end - (ssize + FOUR_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
  size_t offset = align_offset(chunk2mem(rawsp));
  نویسه* asp = rawsp + offset;
  نویسه* csp = (asp < (old_بالاترین + MIN_CHUNK_SIZE))? old_بالاترین : asp;
  mchunkptr sp = (mchunkptr)csp;
  msegmentptr ss = (msegmentptr)(chunk2mem(sp));
  mchunkptr tnext = chunk_plus_offset(sp, ssize);
  mchunkptr p = tnext;
  صحیح nfences = 0;
  (تهی)nfences; /* Suppress unused variable warning */

  /* reset top to new space */
  init_بالاترین(m, (mchunkptr)tbase, tsize - بالاترین_FOOT_SIZE);

  /* Set up segment record */
  أكد(is_aligned(ss));
  set_size_and_pinuse_of_inuse_chunk(m, sp, ssize);
  *ss = m->seg; /* Push current record */
  m->seg.base = tbase;
  m->seg.size = tsize;
  (تهی)set_segment_flags(&m->seg, mmapped);
  m->seg.next = ss;

  /* Insert trailing fenceposts */
  برای (;;) {
    mchunkptr nextp = chunk_plus_offset(p, SIZE_T_SIZE);
    p->head = FENCEPOST_HEAD;
    ++nfences;
    اگر ((نویسه*)(&(nextp->head)) < old_end)
      p = nextp;
    وگرنه
      بشکن;
  }
  أكد(nfences >= 2);

  /* Insert the rest of old top into a bin as an ordinary free chunk */
  اگر (csp != old_بالاترین) {
    mchunkptr q = (mchunkptr)old_بالاترین;
    size_t psize = csp - old_بالاترین;
    mchunkptr tn = chunk_plus_offset(q, psize);
    set_free_with_pinuse(q, psize, tn);
    درج_chunk(m, q, psize);
  }

  check_بالاترین_chunk(m, m->بالاترین);
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
ایستا تهی* sys_alloc(mstate m, size_t nb) {
  نویسه* tbase = CMFAIL;
  size_t tsize = 0;
  flag_t mmap_flag = 0;
  size_t asize; /* allocation size */

  ensure_initialization();

  /* Directly map large chunks, but only if already initialized */
  اگر (use_mmap(m) && nb >= mparams.mmap_threshold && m->topsize != 0) {
    تهی* mem = mmap_alloc(m, nb);
    اگر (mem != 0)
      بازگشت mem;
  }

  asize = granularity_align(nb + SYS_ALLOC_PADDING);
  اگر (asize <= nb)
    بازگشت 0; /* wraparound */
  اگر (m->footprint_limit != 0) {
    size_t fp = m->footprint + asize;
    اگر (fp <= m->footprint || fp > m->footprint_limit)
      بازگشت 0;
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

  اگر (MORECORE_CONTIGUOUS && !use_noncontiguous(m)) {
    نویسه* br = CMFAIL;
    size_t ssize = asize; /* sbrk call size */
    msegmentptr ss = (m->بالاترین == 0)? 0 : segment_holding(m, (نویسه*)m->بالاترین);
    ACQUIRE_MALLOC_GLOBAL_LOCK();

    اگر (ss == 0) {  /* First time through or recovery */
      نویسه* base = (نویسه*)CALL_MORECORE(0);
      اگر (base != CMFAIL) {
        size_t fp;
        /* Adjust to end on a page boundary */
        اگر (!is_page_aligned(base))
          ssize += (page_align((size_t)base) - (size_t)base);
        fp = m->footprint + ssize; /* recheck limits */
        اگر (ssize > nb && ssize < HALF_MAX_SIZE_T &&
            (m->footprint_limit == 0 ||
             (fp > m->footprint && fp <= m->footprint_limit)) &&
            (br = (نویسه*)(CALL_MORECORE(ssize))) == base) {
          tbase = base;
          tsize = ssize;
        }
      }
    }
    وگرنه {
      /* Subtract out existing available top space from MORECORE request. */
      ssize = granularity_align(nb - m->topsize + SYS_ALLOC_PADDING);
      /* Use mem here only if it did continuously extend old space */
      اگر (ssize < HALF_MAX_SIZE_T &&
          (br = (نویسه*)(CALL_MORECORE(ssize))) == ss->base+ss->size) {
        tbase = br;
        tsize = ssize;
      }
    }

    اگر (tbase == CMFAIL) {    /* Cope with partial failure */
      اگر (br != CMFAIL) {    /* Try to use/extend the space we did get */
        اگر (ssize < HALF_MAX_SIZE_T &&
            ssize < nb + SYS_ALLOC_PADDING) {
          size_t esize = granularity_align(nb + SYS_ALLOC_PADDING - ssize);
          اگر (esize < HALF_MAX_SIZE_T) {
            نویسه* end = (نویسه*)CALL_MORECORE(esize);
            اگر (end != CMFAIL)
              ssize += esize;
            وگرنه {            /* Can't use; try to release */
              (تهی) CALL_MORECORE(-ssize);
              br = CMFAIL;
            }
          }
        }
      }
      اگر (br != CMFAIL) {    /* Use the space we did get */
        tbase = br;
        tsize = ssize;
      }
      وگرنه
        disable_contiguous(m); /* Don't try contiguous path in the future */
    }

    RELEASE_MALLOC_GLOBAL_LOCK();
  }

  اگر (HAVE_MMAP && tbase == CMFAIL) {  /* Try MMAP */
    نویسه* mp = (نویسه*)(CALL_MMAP(asize));
    اگر (mp != CMFAIL) {
      tbase = mp;
      tsize = asize;
      mmap_flag = USE_MMAP_BIT;
    }
  }

  اگر (HAVE_MORECORE && tbase == CMFAIL) { /* Try noncontiguous MORECORE */
    اگر (asize < HALF_MAX_SIZE_T) {
      نویسه* br = CMFAIL;
      نویسه* end = CMFAIL;
      ACQUIRE_MALLOC_GLOBAL_LOCK();
      br = (نویسه*)(CALL_MORECORE(asize));
      end = (نویسه*)(CALL_MORECORE(0));
      RELEASE_MALLOC_GLOBAL_LOCK();
      اگر (br != CMFAIL && end != CMFAIL && br < end) {
        size_t ssize = end - br;
        اگر (ssize > nb + بالاترین_FOOT_SIZE) {
          tbase = br;
          tsize = ssize;
        }
      }
    }
  }

  اگر (tbase != CMFAIL) {

    اگر ((m->footprint += tsize) > m->max_footprint)
      m->max_footprint = m->footprint;

    اگر (!is_initialized(m)) { /* first-time initialization */
      اگر (m->least_addr == 0 || tbase < m->least_addr)
        m->least_addr = tbase;
      m->seg.base = tbase;
      m->seg.size = tsize;
      (تهی)set_segment_flags(&m->seg, mmap_flag);
      m->magic = mparams.magic;
      m->release_checks = MAX_RELEASE_CHECK_RATE;
      init_bins(m);
#اگر !ONLY_MSPACES
      اگر (is_global(m))
        init_بالاترین(m, (mchunkptr)tbase, tsize - بالاترین_FOOT_SIZE);
      وگرنه
#endif
      {
        /* Offset top by embedded malloc_state */
        mchunkptr mn = next_chunk(mem2chunk(m));
        init_بالاترین(m, mn, (size_t)((tbase + tsize) - (نویسه*)mn) -بالاترین_FOOT_SIZE);
      }
    }

    وگرنه {
      /* Try to merge with an existing segment */
      msegmentptr sp = &m->seg;
      /* Only consider most recent segment if traversal suppressed */
      وقتی (sp != 0 && tbase != sp->base + sp->size)
        sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->next;
      اگر (sp != 0 &&
          !is_extern_segment(sp) &&
          check_segment_merge(sp, tbase, tsize) &&
          (get_segment_flags(sp) & USE_MMAP_BIT) == mmap_flag &&
          segment_holds(sp, m->بالاترین)) { /* append */
        sp->size += tsize;
        init_بالاترین(m, m->بالاترین, m->topsize + tsize);
      }
      وگرنه {
        اگر (tbase < m->least_addr)
          m->least_addr = tbase;
        sp = &m->seg;
        وقتی (sp != 0 && sp->base != tbase + tsize)
          sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->next;
        اگر (sp != 0 &&
            !is_extern_segment(sp) &&
            check_segment_merge(sp, tbase, tsize) &&
            (get_segment_flags(sp) & USE_MMAP_BIT) == mmap_flag) {
          نویسه* oldbase = sp->base;
          sp->base = tbase;
          sp->size += tsize;
          بازگشت prepend_alloc(m, tbase, oldbase, nb);
        }
        وگرنه
          add_segment(m, tbase, tsize, mmap_flag);
      }
    }

    اگر (nb < m->topsize) { /* Allocate from new or extended top space */
      size_t rsize = m->topsize -= nb;
      mchunkptr p = m->بالاترین;
      mchunkptr r = m->بالاترین = chunk_plus_offset(p, nb);
      r->head = rsize | PINUSE_BIT;
      set_size_and_pinuse_of_inuse_chunk(m, p, nb);
      check_بالاترین_chunk(m, m->بالاترین);
      check_malloced_chunk(m, chunk2mem(p), nb);
      بازگشت chunk2mem(p);
    }
  }

  MALLOC_FAILURE_ACTION;
  بازگشت 0;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
ایستا size_t release_unused_segments(mstate m) {
  size_t released = 0;
  صحیح nsegs = 0;
  msegmentptr pred = &m->seg;
  msegmentptr sp = pred->next;
  وقتی (sp != 0) {
    نویسه* base = sp->base;
    size_t size = sp->size;
    msegmentptr next = sp->next;
    ++nsegs;
    اگر (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
      mchunkptr p = align_as_chunk(base);
      size_t psize = chunksize(p);
      /* Can unmap if first chunk holds entire segment and not pinned */
      اگر (!is_inuse(p) && (نویسه*)p + psize >= base + size - بالاترین_FOOT_SIZE) {
        tchunkptr tp = (tchunkptr)p;
        أكد(segment_holds(sp, (نویسه*)sp));
        اگر (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        وگرنه {
          unlink_large_chunk(m, tp);
        }
        اگر (CALL_MUNMAP(base, size) == 0) {
          released += size;
          m->footprint -= size;
          /* unlink obsoleted record */
          sp = pred;
          sp->next = next;
        }
        وگرنه { /* back out if cannot unmap */
          درج_large_chunk(m, tp, psize);
        }
      }
    }
    اگر (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
      بشکن;
    pred = sp;
    sp = next;
  }
  /* Reset check counter */
  m->release_checks = (((size_t) nsegs > (size_t) MAX_RELEASE_CHECK_RATE)?
                       (size_t) nsegs : (size_t) MAX_RELEASE_CHECK_RATE);
  بازگشت released;
}

ایستا صحیح sys_trim(mstate m, size_t pad) {
  size_t released = 0;
  ensure_initialization();
  اگر (pad < MAX_REQUEST && is_initialized(m)) {
    pad += بالاترین_FOOT_SIZE; /* ensure enough room for segment overhead */

    اگر (m->topsize > pad) {
      /* Shrink top space in granularity-size units, keeping at least one */
      size_t unit = mparams.granularity;
      size_t extra = ((m->topsize - pad + (unit - SIZE_T_ONE)) / unit -
                      SIZE_T_ONE) * unit;
      msegmentptr sp = segment_holding(m, (نویسه*)m->بالاترین);

      اگر (sp == 0)
        CORRUPTION_ERROR_ACTION(m);
      اگر (!is_extern_segment(sp)) {
        اگر (is_mmapped_segment(sp)) {
          اگر (HAVE_MMAP &&
              sp->size >= extra &&
              !has_segment_link(m, sp)) { /* can't shrink if pinned */
            size_t newsize = sp->size - extra;
            (تهی)newsize; /* placate people compiling -Wunused-variable */
            /* Prefer mremap, fall back to munmap */
            اگر ((CALL_MREMAP(sp->base, sp->size, newsize, 0) != MFAIL) ||
                (CALL_MUNMAP(sp->base + newsize, extra) == 0)) {
              released = extra;
            }
          }
        }
        وگرنه اگر (HAVE_MORECORE) {
          اگر (extra >= HALF_MAX_SIZE_T) /* Avoid wrapping negative */
            extra = (HALF_MAX_SIZE_T) + SIZE_T_ONE - unit;
          ACQUIRE_MALLOC_GLOBAL_LOCK();
          {
            /* Make sure end of memory is where we last set it. */
            نویسه* old_br = (نویسه*)(CALL_MORECORE(0));
            اگر (old_br == sp->base + sp->size) {
              نویسه* rel_br = (نویسه*)(CALL_MORECORE(-extra));
              نویسه* new_br = (نویسه*)(CALL_MORECORE(0));
              اگر (rel_br != CMFAIL && new_br < old_br)
                released = old_br - new_br;
            }
          }
          RELEASE_MALLOC_GLOBAL_LOCK();
        }
      }

      اگر (released != 0) {
        sp->size -= released;
        m->footprint -= released;
        init_بالاترین(m, m->بالاترین, m->topsize - released);
        check_بالاترین_chunk(m, m->بالاترین);
      }
    }

    /* Unmap any unused mmapped segments */
    اگر (HAVE_MMAP)
      released += release_unused_segments(m);

    /* On failure, disable autotrim to avoid repeated failed future calls */
    اگر (released == 0 && m->topsize > m->trim_check)
      m->trim_check = MAX_SIZE_T;
  }

  بازگشت (released != 0)? 1 : 0;
}

/* Consolidate and bin a chunk. Differs from exported versions
   of free mainly in that the chunk need not be marked as inuse.
*/
ایستا تهی dispose_chunk(mstate m, mchunkptr p, size_t psize) {
  mchunkptr next = chunk_plus_offset(p, psize);
  اگر (!pinuse(p)) {
    mchunkptr prev;
    size_t prevsize = p->prev_foot;
    اگر (is_mmapped(p)) {
      psize += prevsize + MMAP_FOOT_PAD;
      اگر (CALL_MUNMAP((نویسه*)p - prevsize, psize) == 0)
        m->footprint -= psize;
      بازگشت;
    }
    prev = chunk_minus_offset(p, prevsize);
    psize += prevsize;
    p = prev;
    اگر (RTCHECK(ok_address(m, prev))) { /* consolidate backward */
      اگر (p != m->dv) {
        unlink_chunk(m, p, prevsize);
      }
      وگرنه اگر ((next->head & INUSE_BITS) == INUSE_BITS) {
        m->dvsize = psize;
        set_free_with_pinuse(p, psize, next);
        بازگشت;
      }
    }
    وگرنه {
      CORRUPTION_ERROR_ACTION(m);
      بازگشت;
    }
  }
  اگر (RTCHECK(ok_address(m, next))) {
    اگر (!cinuse(next)) {  /* consolidate forward */
      اگر (next == m->بالاترین) {
        size_t tsize = m->topsize += psize;
        m->بالاترین = p;
        p->head = tsize | PINUSE_BIT;
        اگر (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        بازگشت;
      }
      وگرنه اگر (next == m->dv) {
        size_t dsize = m->dvsize += psize;
        m->dv = p;
        set_size_and_pinuse_of_free_chunk(p, dsize);
        بازگشت;
      }
      وگرنه {
        size_t nsize = chunksize(next);
        psize += nsize;
        unlink_chunk(m, next, nsize);
        set_size_and_pinuse_of_free_chunk(p, psize);
        اگر (p == m->dv) {
          m->dvsize = psize;
          بازگشت;
        }
      }
    }
    وگرنه {
      set_free_with_pinuse(p, psize, next);
    }
    درج_chunk(m, p, psize);
  }
  وگرنه {
    CORRUPTION_ERROR_ACTION(m);
  }
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
ایستا تهی* tmalloc_large(mstate m, size_t nb) {
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  bindex_t idx;
  compute_درخت_index(nb, idx);
  اگر ((t = *treebin_at(m, idx)) != 0) {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_درخت_index(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    برای (;;) {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      اگر (trem < rsize) {
        v = t;
        اگر ((rsize = trem) == 0)
          بشکن;
      }
      rt = t->child[1];
      t = t->child[(sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1];
      اگر (rt != 0 && rt != t)
        rst = rt;
      اگر (t == 0) {
        t = rst; /* set t to least subtree holding sizes > nb */
        بشکن;
      }
      sizebits <<= 1;
    }
  }
  اگر (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
    binmap_t leftbits = چپ_bits(idx2bit(idx)) & m->treemap;
    اگر (leftbits != 0) {
      bindex_t i;
      binmap_t leastbit = least_bit(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  وقتی (t != 0) { /* find smallest of tree or subtree */
    size_t trem = chunksize(t) - nb;
    اگر (trem < rsize) {
      rsize = trem;
      v = t;
    }
    t = leftmost_child(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  اگر (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
    اگر (RTCHECK(ok_address(m, v))) { /* split */
      mchunkptr r = chunk_plus_offset(v, nb);
      أكد(chunksize(v) == rsize + nb);
      اگر (RTCHECK(ok_next(v, r))) {
        unlink_large_chunk(m, v);
        اگر (rsize < MIN_CHUNK_SIZE)
          set_inuse_and_pinuse(m, v, (rsize + nb));
        وگرنه {
          set_size_and_pinuse_of_inuse_chunk(m, v, nb);
          set_size_and_pinuse_of_free_chunk(r, rsize);
          درج_chunk(m, r, rsize);
        }
        بازگشت chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  بازگشت 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
ایستا تهی* tmalloc_small(mstate m, size_t nb) {
  tchunkptr t, v;
  size_t rsize;
  bindex_t i;
  binmap_t leastbit = least_bit(m->treemap);
  compute_bit2idx(leastbit, i);
  v = t = *treebin_at(m, i);
  rsize = chunksize(t) - nb;

  وقتی ((t = leftmost_child(t)) != 0) {
    size_t trem = chunksize(t) - nb;
    اگر (trem < rsize) {
      rsize = trem;
      v = t;
    }
  }

  اگر (RTCHECK(ok_address(m, v))) {
    mchunkptr r = chunk_plus_offset(v, nb);
    أكد(chunksize(v) == rsize + nb);
    اگر (RTCHECK(ok_next(v, r))) {
      unlink_large_chunk(m, v);
      اگر (rsize < MIN_CHUNK_SIZE)
        set_inuse_and_pinuse(m, v, (rsize + nb));
      وگرنه {
        set_size_and_pinuse_of_inuse_chunk(m, v, nb);
        set_size_and_pinuse_of_free_chunk(r, rsize);
        replace_dv(m, r, rsize);
      }
      بازگشت chunk2mem(v);
    }
  }

  CORRUPTION_ERROR_ACTION(m);
  بازگشت 0;
}

#اگر !ONLY_MSPACES

تهی* dlmalloc(size_t bytes) {
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

#اگر USE_LOCKS
  ensure_initialization(); /* initialize in sys_alloc if not using locks */
#endif

  اگر (!PREACTION(gm)) {
    تهی* mem;
    size_t nb;
    اگر (bytes <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (bytes < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(bytes);
      idx = small_index(nb);
      smallbits = gm->smallmap >> idx;

      اگر ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(gm, idx);
        p = b->fd;
        أكد(chunksize(p) == small_index2size(idx));
        unlink_first_small_chunk(gm, b, p, idx);
        set_inuse_and_pinuse(gm, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(gm, mem, nb);
        برو postaction;
      }

      وگرنه اگر (nb > gm->dvsize) {
        اگر (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & چپ_bits(idx2bit(idx));
          binmap_t leastbit = least_bit(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(gm, i);
          p = b->fd;
          أكد(chunksize(p) == small_index2size(i));
          unlink_first_small_chunk(gm, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          اگر (SIZE_T_SIZE != 4 && rsize < MIN_CHUNK_SIZE)
            set_inuse_and_pinuse(gm, p, small_index2size(i));
          وگرنه {
            set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
            r = chunk_plus_offset(p, nb);
            set_size_and_pinuse_of_free_chunk(r, rsize);
            replace_dv(gm, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(gm, mem, nb);
          برو postaction;
        }

        وگرنه اگر (gm->treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0) {
          check_malloced_chunk(gm, mem, nb);
          برو postaction;
        }
      }
    }
    وگرنه اگر (bytes >= MAX_REQUEST)
      nb = MAX_SIZE_T; /* Too big to allocate. Force failure (in sys alloc) */
    وگرنه {
      nb = pad_request(bytes);
      اگر (gm->treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0) {
        check_malloced_chunk(gm, mem, nb);
        برو postaction;
      }
    }

    اگر (nb <= gm->dvsize) {
      size_t rsize = gm->dvsize - nb;
      mchunkptr p = gm->dv;
      اگر (rsize >= MIN_CHUNK_SIZE) { /* split dv */
        mchunkptr r = gm->dv = chunk_plus_offset(p, nb);
        gm->dvsize = rsize;
        set_size_and_pinuse_of_free_chunk(r, rsize);
        set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
      }
      وگرنه { /* exhaust dv */
        size_t dvs = gm->dvsize;
        gm->dvsize = 0;
        gm->dv = 0;
        set_inuse_and_pinuse(gm, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(gm, mem, nb);
      برو postaction;
    }

    وگرنه اگر (nb < gm->topsize) { /* Split top */
      size_t rsize = gm->topsize -= nb;
      mchunkptr p = gm->بالاترین;
      mchunkptr r = gm->بالاترین = chunk_plus_offset(p, nb);
      r->head = rsize | PINUSE_BIT;
      set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
      mem = chunk2mem(p);
      check_بالاترین_chunk(gm, gm->بالاترین);
      check_malloced_chunk(gm, mem, nb);
      برو postaction;
    }

    mem = sys_alloc(gm, nb);

  postaction:
    POSTACTION(gm);
    بازگشت mem;
  }

  بازگشت 0;
}

/* ---------------------------- free --------------------------- */

تهی dlfree(تهی* mem) {
  /*
     Consolidate freed chunks with preceding or succeeding bordering
     free chunks, if they exist, and then place in a bin.  Intermixed
     with special cases for top, dv, mmapped chunks, and usage errors.
  */

  اگر (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#اگر FOOTERS
    mstate fm = get_mstate_for(p);
    اگر (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      بازگشت;
    }
#وگرنه /* FOOTERS */
#define fm gm
#endif /* FOOTERS */
    اگر (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      اگر (RTCHECK(ok_address(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr next = chunk_plus_offset(p, psize);
        اگر (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          اگر (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            اگر (CALL_MUNMAP((نویسه*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            برو postaction;
          }
          وگرنه {
            mchunkptr prev = chunk_minus_offset(p, prevsize);
            psize += prevsize;
            p = prev;
            اگر (RTCHECK(ok_address(fm, prev))) { /* consolidate backward */
              اگر (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              وگرنه اگر ((next->head & INUSE_BITS) == INUSE_BITS) {
                fm->dvsize = psize;
                set_free_with_pinuse(p, psize, next);
                برو postaction;
              }
            }
            وگرنه
              برو erroraction;
          }
        }

        اگر (RTCHECK(ok_next(p, next) && ok_pinuse(next))) {
          اگر (!cinuse(next)) {  /* consolidate forward */
            اگر (next == fm->بالاترین) {
              size_t tsize = fm->topsize += psize;
              fm->بالاترین = p;
              p->head = tsize | PINUSE_BIT;
              اگر (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              اگر (should_trim(fm, tsize))
                sys_trim(fm, 0);
              برو postaction;
            }
            وگرنه اگر (next == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              set_size_and_pinuse_of_free_chunk(p, dsize);
              برو postaction;
            }
            وگرنه {
              size_t nsize = chunksize(next);
              psize += nsize;
              unlink_chunk(fm, next, nsize);
              set_size_and_pinuse_of_free_chunk(p, psize);
              اگر (p == fm->dv) {
                fm->dvsize = psize;
                برو postaction;
              }
            }
          }
          وگرنه
            set_free_with_pinuse(p, psize, next);

          اگر (is_small(psize)) {
            درج_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          وگرنه {
            tchunkptr tp = (tchunkptr)p;
            درج_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            اگر (--fm->release_checks == 0)
              release_unused_segments(fm);
          }
          برو postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
#اگر !FOOTERS
#undef fm
#endif /* FOOTERS */
}

تهی* dlcalloc(size_t n_elements, size_t elem_size) {
  تهی* mem;
  size_t req = 0;
  اگر (n_elements != 0) {
    req = n_elements * elem_size;
    اگر (((n_elements | elem_size) & ~(size_t)0xffff) &&
        (req / n_elements != elem_size))
      req = MAX_SIZE_T; /* force downstream failure on overflow */
  }
  mem = dlmalloc(req);
  اگر (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  بازگشت mem;
}

#endif /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
ایستا mchunkptr try_realloc_chunk(mstate m, mchunkptr p, size_t nb,
                                   صحیح can_move) {
  mchunkptr newp = 0;
  size_t oldsize = chunksize(p);
  mchunkptr next = chunk_plus_offset(p, oldsize);
  اگر (RTCHECK(ok_address(m, p) && ok_inuse(p) &&
              ok_next(p, next) && ok_pinuse(next))) {
    اگر (is_mmapped(p)) {
      newp = mmap_resize(m, p, nb, can_move);
    }
    وگرنه اگر (oldsize >= nb) {             /* already big enough */
      size_t rsize = oldsize - nb;
      اگر (rsize >= MIN_CHUNK_SIZE) {      /* split off remainder */
        mchunkptr r = chunk_plus_offset(p, nb);
        set_inuse(m, p, nb);
        set_inuse(m, r, rsize);
        dispose_chunk(m, r, rsize);
      }
      newp = p;
    }
    وگرنه اگر (next == m->بالاترین) {  /* extend into top */
      اگر (oldsize + m->topsize > nb) {
        size_t newsize = oldsize + m->topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_offset(p, nb);
        set_inuse(m, p, nb);
        newtop->head = newtopsize |PINUSE_BIT;
        m->بالاترین = newtop;
        m->topsize = newtopsize;
        newp = p;
      }
    }
    وگرنه اگر (next == m->dv) { /* extend into dv */
      size_t dvs = m->dvsize;
      اگر (oldsize + dvs >= nb) {
        size_t dsize = oldsize + dvs - nb;
        اگر (dsize >= MIN_CHUNK_SIZE) {
          mchunkptr r = chunk_plus_offset(p, nb);
          mchunkptr n = chunk_plus_offset(r, dsize);
          set_inuse(m, p, nb);
          set_size_and_pinuse_of_free_chunk(r, dsize);
          clear_pinuse(n);
          m->dvsize = dsize;
          m->dv = r;
        }
        وگرنه { /* exhaust dv */
          size_t newsize = oldsize + dvs;
          set_inuse(m, p, newsize);
          m->dvsize = 0;
          m->dv = 0;
        }
        newp = p;
      }
    }
    وگرنه اگر (!cinuse(next)) { /* extend into next free chunk */
      size_t nextsize = chunksize(next);
      اگر (oldsize + nextsize >= nb) {
        size_t rsize = oldsize + nextsize - nb;
        unlink_chunk(m, next, nextsize);
        اگر (rsize < MIN_CHUNK_SIZE) {
          size_t newsize = oldsize + nextsize;
          set_inuse(m, p, newsize);
        }
        وگرنه {
          mchunkptr r = chunk_plus_offset(p, nb);
          set_inuse(m, p, nb);
          set_inuse(m, r, rsize);
          dispose_chunk(m, r, rsize);
        }
        newp = p;
      }
    }
  }
  وگرنه {
    USAGE_ERROR_ACTION(m, chunk2mem(p));
  }
  بازگشت newp;
}

ایستا تهی* internal_memalign(mstate m, size_t alignment, size_t bytes) {
  تهی* mem = 0;
  اگر (alignment <  MIN_CHUNK_SIZE) /* must be at least a minimum chunk size */
    alignment = MIN_CHUNK_SIZE;
  اگر ((alignment & (alignment-SIZE_T_ONE)) != 0) {/* Ensure a power of 2 */
    size_t a = MALLOC_ALIGNMENT << 1;
    وقتی (a < alignment) a <<= 1;
    alignment = a;
  }
  اگر (bytes >= MAX_REQUEST - alignment) {
    اگر (m != 0)  { /* Test isn't needed but avoids compiler warning */
      MALLOC_FAILURE_ACTION;
    }
  }
  وگرنه {
    size_t nb = request2size(bytes);
    size_t req = nb + alignment + MIN_CHUNK_SIZE - CHUNK_OVERHEAD;
    mem = internal_malloc(m, req);
    اگر (mem != 0) {
      mchunkptr p = mem2chunk(mem);
      اگر (PREACTION(m))
        بازگشت 0;
      اگر ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
        نویسه* br = (نویسه*)mem2chunk((size_t)(((size_t)((نویسه*)mem + alignment -
                                                       SIZE_T_ONE)) &
                                             -alignment));
        نویسه* pos = ((size_t)(br - (نویسه*)(p)) >= MIN_CHUNK_SIZE)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (نویسه*)(p);
        size_t newsize = chunksize(p) - leadsize;

        اگر (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
          newp->prev_foot = p->prev_foot + leadsize;
          newp->head = newsize;
        }
        وگرنه { /* Otherwise, give back leader, use the rest */
          set_inuse(m, newp, newsize);
          set_inuse(m, p, leadsize);
          dispose_chunk(m, p, leadsize);
        }
        p = newp;
      }

      /* Give back spare room at the end */
      اگر (!is_mmapped(p)) {
        size_t size = chunksize(p);
        اگر (size > nb + MIN_CHUNK_SIZE) {
          size_t remainder_size = size - nb;
          mchunkptr remainder = chunk_plus_offset(p, nb);
          set_inuse(m, p, nb);
          set_inuse(m, remainder, remainder_size);
          dispose_chunk(m, remainder, remainder_size);
        }
      }

      mem = chunk2mem(p);
      أكد (chunksize(p) >= nb);
      أكد(((size_t)mem & (alignment - 1)) == 0);
      check_inuse_chunk(m, p);
      POSTACTION(m);
    }
  }
  بازگشت mem;
}

/*
  Common support for independent_X routines, handling
    all of the combinations that can result.
  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/
ایستا تهی** ialloc(mstate m,
                     size_t n_elements,
                     size_t* sizes,
                     صحیح opts,
                     تهی* chunks[]) {

  size_t    element_size;   /* chunksize of each element, if all same */
  size_t    contents_size;  /* total size of elements */
  size_t    array_size;     /* request size of pointer array */
  تهی*     mem;            /* malloced aggregate space */
  mchunkptr p;              /* corresponding chunk */
  size_t    remainder_size; /* remaining bytes while splitting */
  تهی**    marray;         /* either "chunks" or malloced ptr array */
  mchunkptr array_chunk;    /* chunk for malloced ptr array */
  flag_t    was_enabled;    /* to disable mmap */
  size_t    size;
  size_t    i;

  ensure_initialization();
  /* compute array length, if needed */
  اگر (chunks != 0) {
    اگر (n_elements == 0)
      بازگشت chunks; /* nothing to do */
    marray = chunks;
    array_size = 0;
  }
  وگرنه {
    /* if empty req, must still return chunk representing empty array */
    اگر (n_elements == 0)
      بازگشت (تهی**)internal_malloc(m, 0);
    marray = 0;
    array_size = request2size(n_elements * (اندازه(تهی*)));
  }

  /* compute total element size */
  اگر (opts & 0x1) { /* all-same-size */
    element_size = request2size(*sizes);
    contents_size = n_elements * element_size;
  }
  وگرنه { /* add up all the sizes */
    element_size = 0;
    contents_size = 0;
    برای (i = 0; i != n_elements; ++i)
      contents_size += request2size(sizes[i]);
  }

  size = contents_size + array_size;

  /*
     Allocate the aggregate chunk.  First disable direct-mmapping so
     malloc won't use it, since we would not be able to later
     free/realloc space internal to a segregated mmap region.
  */
  was_enabled = use_mmap(m);
  disable_mmap(m);
  mem = internal_malloc(m, size - CHUNK_OVERHEAD);
  اگر (was_enabled)
    enable_mmap(m);
  اگر (mem == 0)
    بازگشت 0;

  اگر (PREACTION(m)) بازگشت 0;
  p = mem2chunk(mem);
  remainder_size = chunksize(p);

  أكد(!is_mmapped(p));

  اگر (opts & 0x2) {       /* optionally clear the elements */
    memset((size_t*)mem, 0, remainder_size - SIZE_T_SIZE - array_size);
  }

  /* If not provided, allocate the pointer array as final part of chunk */
  اگر (marray == 0) {
    size_t  array_chunk_size;
    array_chunk = chunk_plus_offset(p, contents_size);
    array_chunk_size = remainder_size - contents_size;
    marray = (تهی**) (chunk2mem(array_chunk));
    set_size_and_pinuse_of_inuse_chunk(m, array_chunk, array_chunk_size);
    remainder_size = contents_size;
  }

  /* split out elements */
  برای (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    اگر (i != n_elements-1) {
      اگر (element_size != 0)
        size = element_size;
      وگرنه
        size = request2size(sizes[i]);
      remainder_size -= size;
      set_size_and_pinuse_of_inuse_chunk(m, p, size);
      p = chunk_plus_offset(p, size);
    }
    وگرنه { /* the final element absorbs any overallocation slop */
      set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
      بشکن;
    }
  }

#اگر DEBUG
  اگر (marray != chunks) {
    /* final element must have exactly exhausted chunk */
    اگر (element_size != 0) {
      أكد(remainder_size == element_size);
    }
    وگرنه {
      أكد(remainder_size == request2size(sizes[i]));
    }
    check_inuse_chunk(m, mem2chunk(marray));
  }
  برای (i = 0; i != n_elements; ++i)
    check_inuse_chunk(m, mem2chunk(marray[i]));

#endif /* DEBUG */

  POSTACTION(m);
  بازگشت marray;
}

/* Try to free all pointers in the given array.
   Note: this could be made faster, by delaying consolidation,
   at the price of disabling some user integrity checks, We
   still optimize some consolidations by combining adjacent
   chunks before freeing, which will occur often if allocated
   with ialloc or the array is sorted.
*/
ایستا size_t internal_bulk_free(mstate m, تهی* array[], size_t nelem) {
  size_t unfreed = 0;
  اگر (!PREACTION(m)) {
    تهی** a;
    تهی** fence = &(array[nelem]);
    برای (a = array; a != fence; ++a) {
      تهی* mem = *a;
      اگر (mem != 0) {
        mchunkptr p = mem2chunk(mem);
        size_t psize = chunksize(p);
#اگر FOOTERS
        اگر (get_mstate_for(p) != m) {
          ++unfreed;
          ادامه;
        }
#endif
        check_inuse_chunk(m, p);
        *a = 0;
        اگر (RTCHECK(ok_address(m, p) && ok_inuse(p))) {
          تهی ** b = a + 1; /* try to merge with next chunk */
          mchunkptr next = next_chunk(p);
          اگر (b != fence && *b == chunk2mem(next)) {
            size_t newsize = chunksize(next) + psize;
            set_inuse(m, p, newsize);
            *b = chunk2mem(p);
          }
          وگرنه
            dispose_chunk(m, p, psize);
        }
        وگرنه {
          CORRUPTION_ERROR_ACTION(m);
          بشکن;
        }
      }
    }
    اگر (should_trim(m, m->topsize))
      sys_trim(m, 0);
    POSTACTION(m);
  }
  بازگشت unfreed;
}

/* Traversal */
#اگر MALLOC_INSPECT_ALL
ایستا تهی internal_inspect_all(mstate m,
                                 تهی(*handler)(تهی *start,
                                                تهی *end,
                                                size_t used_bytes,
                                                تهی* callback_arg),
                                 تهی* arg) {
  اگر (is_initialized(m)) {
    mchunkptr بالاترین = m->بالاترین;
    msegmentptr s;
    برای (s = &m->seg; s != 0; s = s->next) {
      mchunkptr q = align_as_chunk(s->base);
      وقتی (segment_holds(s, q) && q->head != FENCEPOST_HEAD) {
        mchunkptr next = next_chunk(q);
        size_t sz = chunksize(q);
        size_t used;
        تهی* start;
        اگر (is_inuse(q)) {
          used = sz - CHUNK_OVERHEAD; /* must not be mmapped */
          start = chunk2mem(q);
        }
        وگرنه {
          used = 0;
          اگر (is_small(sz)) {     /* offset by possible bookkeeping */
            start = (تهی*)((نویسه*)q + اندازه(ساختار malloc_chunk));
          }
          وگرنه {
            start = (تهی*)((نویسه*)q + اندازه(ساختار malloc_درخت_chunk));
          }
        }
        اگر (start < (تهی*)next)  /* skip if all space is bookkeeping */
          handler(start, next, used, arg);
        اگر (q == بالاترین)
          بشکن;
        q = next;
      }
    }
  }
}
#endif /* MALLOC_INSPECT_ALL */

/* ------------------ Exported realloc, memalign, etc -------------------- */

#اگر !ONLY_MSPACES

تهی* dlrealloc(تهی* oldmem, size_t bytes) {
  تهی* mem = 0;
  اگر (oldmem == 0) {
    mem = dlmalloc(bytes);
  }
  وگرنه اگر (bytes >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#ifdef REALLOC_ZERO_BYTES_FREES
  وگرنه اگر (bytes == 0) {
    dlfree(oldmem);
  }
#endif /* REALLOC_ZERO_BYTES_FREES */
  وگرنه {
    size_t nb = request2size(bytes);
    mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
    mstate m = gm;
#وگرنه /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    اگر (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      بازگشت 0;
    }
#endif /* FOOTERS */
    اگر (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      اگر (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      وگرنه {
        mem = internal_malloc(m, bytes);
        اگر (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          memcpy(mem, oldmem, (oc < bytes)? oc : bytes);
          internal_free(m, oldmem);
        }
      }
    }
  }
  بازگشت mem;
}

تهی* dlrealloc_in_place(تهی* oldmem, size_t bytes) {
  تهی* mem = 0;
  اگر (oldmem != 0) {
    اگر (bytes >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    وگرنه {
      size_t nb = request2size(bytes);
      mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
      mstate m = gm;
#وگرنه /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      اگر (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        بازگشت 0;
      }
#endif /* FOOTERS */
      اگر (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        اگر (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  بازگشت mem;
}

تهی* dlmemalign(size_t alignment, size_t bytes) {
  اگر (alignment <= MALLOC_ALIGNMENT) {
    بازگشت dlmalloc(bytes);
  }
  بازگشت internal_memalign(gm, alignment, bytes);
}

صحیح dlposix_memalign(تهی** pp, size_t alignment, size_t bytes) {
  تهی* mem = 0;
  اگر (alignment == MALLOC_ALIGNMENT)
    mem = dlmalloc(bytes);
  وگرنه {
    size_t d = alignment / اندازه(تهی*);
    size_t r = alignment % اندازه(تهی*);
    اگر (r != 0 || d == 0 || (d & (d-SIZE_T_ONE)) != 0)
      بازگشت EINVAL;
    وگرنه اگر (bytes <= MAX_REQUEST - alignment) {
      اگر (alignment <  MIN_CHUNK_SIZE)
        alignment = MIN_CHUNK_SIZE;
      mem = internal_memalign(gm, alignment, bytes);
    }
  }
  اگر (mem == 0)
    بازگشت ENOMEM;
  وگرنه {
    *pp = mem;
    بازگشت 0;
  }
}

تهی* dlvalloc(size_t bytes) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.page_size;
  بازگشت dlmemalign(pagesz, bytes);
}

تهی* dlpvalloc(size_t bytes) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.page_size;
  بازگشت dlmemalign(pagesz, (bytes + pagesz - SIZE_T_ONE) & ~(pagesz - SIZE_T_ONE));
}

تهی** dlindependent_calloc(size_t n_elements, size_t elem_size,
                            تهی* chunks[]) {
  size_t sz = elem_size; /* serves as 1-element array */
  بازگشت ialloc(gm, n_elements, &sz, 3, chunks);
}

تهی** dlindependent_comalloc(size_t n_elements, size_t sizes[],
                              تهی* chunks[]) {
  بازگشت ialloc(gm, n_elements, sizes, 0, chunks);
}

size_t dlbulk_free(تهی* array[], size_t nelem) {
  بازگشت internal_bulk_free(gm, array, nelem);
}

#اگر MALLOC_INSPECT_ALL
تهی dlmalloc_inspect_all(تهی(*handler)(تهی *start,
                                         تهی *end,
                                         size_t used_bytes,
                                         تهی* callback_arg),
                          تهی* arg) {
  ensure_initialization();
  اگر (!PREACTION(gm)) {
    internal_inspect_all(gm, handler, arg);
    POSTACTION(gm);
  }
}
#endif /* MALLOC_INSPECT_ALL */

صحیح dlmalloc_trim(size_t pad) {
  صحیح result = 0;
  ensure_initialization();
  اگر (!PREACTION(gm)) {
    result = sys_trim(gm, pad);
    POSTACTION(gm);
  }
  بازگشت result;
}

size_t dlmalloc_footprint(تهی) {
  بازگشت gm->footprint;
}

size_t dlmalloc_max_footprint(تهی) {
  بازگشت gm->max_footprint;
}

size_t dlmalloc_footprint_limit(تهی) {
  size_t maf = gm->footprint_limit;
  بازگشت maf == 0 ? MAX_SIZE_T : maf;
}

size_t dlmalloc_set_footprint_limit(size_t bytes) {
  size_t result;  /* invert sense of 0 */
  اگر (bytes == 0)
    result = granularity_align(1); /* Use minimal size */
  اگر (bytes == MAX_SIZE_T)
    result = 0;                    /* disable */
  وگرنه
    result = granularity_align(bytes);
  بازگشت gm->footprint_limit = result;
}

#اگر !NO_MALLINFO
ساختار mallinfo dlmallinfo(تهی) {
  بازگشت internal_mallinfo(gm);
}
#endif /* NO_MALLINFO */

#اگر !NO_MALLOC_STATS
تهی dlmalloc_stats(تهی) {
  internal_malloc_stats(gm);
}
#endif /* NO_MALLOC_STATS */

صحیح dlmallopt(صحیح param_عدد, صحیح مقدار) {
  بازگشت change_mparam(param_عدد, مقدار);
}

size_t dlmalloc_usable_size(تهی* mem) {
  اگر (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    اگر (is_inuse(p))
      بازگشت chunksize(p) - overhead_for(p);
  }
  بازگشت 0;
}

#endif /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#اگر MSPACES

ایستا mstate init_user_mstate(نویسه* tbase, size_t tsize) {
  size_t msize = pad_request(اندازه(ساختار malloc_state));
  mchunkptr mn;
  mchunkptr msp = align_as_chunk(tbase);
  mstate m = (mstate)(chunk2mem(msp));
  memset(m, 0, msize);
  (تهی)INITIAL_LOCK(&m->mutex);
  msp->head = (msize|INUSE_BITS);
  m->seg.base = m->least_addr = tbase;
  m->seg.size = m->footprint = m->max_footprint = tsize;
  m->magic = mparams.magic;
  m->release_checks = MAX_RELEASE_CHECK_RATE;
  m->mflags = mparams.default_mflags;
  m->extp = 0;
  m->exts = 0;
  disable_contiguous(m);
  init_bins(m);
  mn = next_chunk(mem2chunk(m));
  init_بالاترین(m, mn, (size_t)((tbase + tsize) - (نویسه*)mn) - بالاترین_FOOT_SIZE);
  check_بالاترین_chunk(m, m->بالاترین);
  بازگشت m;
}

mspace create_mspace(size_t capacity, صحیح locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(اندازه(ساختار malloc_state));
  اگر (capacity < (size_t) -(msize + بالاترین_FOOT_SIZE + mparams.page_size)) {
    size_t rs = ((capacity == 0)? mparams.granularity :
                 (capacity + بالاترین_FOOT_SIZE + msize));
    size_t tsize = granularity_align(rs);
    نویسه* tbase = (نویسه*)(CALL_MMAP(tsize));
    اگر (tbase != CMFAIL) {
      m = init_user_mstate(tbase, tsize);
      set_segment_flags(&m->seg, USE_MMAP_BIT);
      set_lock(m, locked);
    }
  }
  بازگشت (mspace)m;
}

mspace create_mspace_with_base(تهی* base, size_t capacity, صحیح locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(اندازه(ساختار malloc_state));
  اگر (capacity > msize + بالاترین_FOOT_SIZE &&
      capacity < (size_t) -(msize + بالاترین_FOOT_SIZE + mparams.page_size)) {
    m = init_user_mstate((نویسه*)base, capacity);
    set_segment_flags(&m->seg, EXTERN_BIT);
    set_lock(m, locked);
  }
  بازگشت (mspace)m;
}

صحیح mspace_track_large_chunks(mspace msp, صحیح enable) {
  صحیح ret = 0;
  mstate ms = (mstate)msp;
  اگر (!PREACTION(ms)) {
    اگر (!use_mmap(ms)) {
      ret = 1;
    }
    اگر (!enable) {
      enable_mmap(ms);
    } وگرنه {
      disable_mmap(ms);
    }
    POSTACTION(ms);
  }
  بازگشت ret;
}

size_t destroy_mspace(mspace msp) {
  size_t freed = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    msegmentptr sp = &ms->seg;
    (تهی)DESTROY_LOCK(&ms->mutex); /* destroy before unmapped */
    وقتی (sp != 0) {
      نویسه* base = sp->base;
      size_t size = sp->size;
      flag_t flag = get_segment_flags(sp);
      (تهی)base; /* placate people compiling -Wunused-variable */
      sp = sp->next;
      اگر ((flag & USE_MMAP_BIT) && !(flag & EXTERN_BIT) &&
          CALL_MUNMAP(base, size) == 0)
        freed += size;
    }
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
  بازگشت freed;
}

/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

تهی* mspace_malloc(mspace msp, size_t bytes) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    بازگشت 0;
  }
  اگر (!PREACTION(ms)) {
    تهی* mem;
    size_t nb;
    اگر (bytes <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (bytes < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(bytes);
      idx = small_index(nb);
      smallbits = ms->smallmap >> idx;

      اگر ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(ms, idx);
        p = b->fd;
        أكد(chunksize(p) == small_index2size(idx));
        unlink_first_small_chunk(ms, b, p, idx);
        set_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        برو postaction;
      }

      وگرنه اگر (nb > ms->dvsize) {
        اگر (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & چپ_bits(idx2bit(idx));
          binmap_t leastbit = least_bit(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b->fd;
          أكد(chunksize(p) == small_index2size(i));
          unlink_first_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          اگر (SIZE_T_SIZE != 4 && rsize < MIN_CHUNK_SIZE)
            set_inuse_and_pinuse(ms, p, small_index2size(i));
          وگرنه {
            set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
            r = chunk_plus_offset(p, nb);
            set_size_and_pinuse_of_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          برو postaction;
        }

        وگرنه اگر (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
          check_malloced_chunk(ms, mem, nb);
          برو postaction;
        }
      }
    }
    وگرنه اگر (bytes >= MAX_REQUEST)
      nb = MAX_SIZE_T; /* Too big to allocate. Force failure (in sys alloc) */
    وگرنه {
      nb = pad_request(bytes);
      اگر (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
        check_malloced_chunk(ms, mem, nb);
        برو postaction;
      }
    }

    اگر (nb <= ms->dvsize) {
      size_t rsize = ms->dvsize - nb;
      mchunkptr p = ms->dv;
      اگر (rsize >= MIN_CHUNK_SIZE) { /* split dv */
        mchunkptr r = ms->dv = chunk_plus_offset(p, nb);
        ms->dvsize = rsize;
        set_size_and_pinuse_of_free_chunk(r, rsize);
        set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
      }
      وگرنه { /* exhaust dv */
        size_t dvs = ms->dvsize;
        ms->dvsize = 0;
        ms->dv = 0;
        set_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      برو postaction;
    }

    وگرنه اگر (nb < ms->topsize) { /* Split top */
      size_t rsize = ms->topsize -= nb;
      mchunkptr p = ms->بالاترین;
      mchunkptr r = ms->بالاترین = chunk_plus_offset(p, nb);
      r->head = rsize | PINUSE_BIT;
      set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_بالاترین_chunk(ms, ms->بالاترین);
      check_malloced_chunk(ms, mem, nb);
      برو postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
    POSTACTION(ms);
    بازگشت mem;
  }

  بازگشت 0;
}

تهی mspace_free(mspace msp, تهی* mem) {
  اگر (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#اگر FOOTERS
    mstate fm = get_mstate_for(p);
    (تهی)msp; /* placate people compiling -Wunused */
#وگرنه /* FOOTERS */
    mstate fm = (mstate)msp;
#endif /* FOOTERS */
    اگر (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      بازگشت;
    }
    اگر (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      اگر (RTCHECK(ok_address(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr next = chunk_plus_offset(p, psize);
        اگر (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          اگر (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            اگر (CALL_MUNMAP((نویسه*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            برو postaction;
          }
          وگرنه {
            mchunkptr prev = chunk_minus_offset(p, prevsize);
            psize += prevsize;
            p = prev;
            اگر (RTCHECK(ok_address(fm, prev))) { /* consolidate backward */
              اگر (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              وگرنه اگر ((next->head & INUSE_BITS) == INUSE_BITS) {
                fm->dvsize = psize;
                set_free_with_pinuse(p, psize, next);
                برو postaction;
              }
            }
            وگرنه
              برو erroraction;
          }
        }

        اگر (RTCHECK(ok_next(p, next) && ok_pinuse(next))) {
          اگر (!cinuse(next)) {  /* consolidate forward */
            اگر (next == fm->بالاترین) {
              size_t tsize = fm->topsize += psize;
              fm->بالاترین = p;
              p->head = tsize | PINUSE_BIT;
              اگر (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              اگر (should_trim(fm, tsize))
                sys_trim(fm, 0);
              برو postaction;
            }
            وگرنه اگر (next == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              set_size_and_pinuse_of_free_chunk(p, dsize);
              برو postaction;
            }
            وگرنه {
              size_t nsize = chunksize(next);
              psize += nsize;
              unlink_chunk(fm, next, nsize);
              set_size_and_pinuse_of_free_chunk(p, psize);
              اگر (p == fm->dv) {
                fm->dvsize = psize;
                برو postaction;
              }
            }
          }
          وگرنه
            set_free_with_pinuse(p, psize, next);

          اگر (is_small(psize)) {
            درج_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          وگرنه {
            tchunkptr tp = (tchunkptr)p;
            درج_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            اگر (--fm->release_checks == 0)
              release_unused_segments(fm);
          }
          برو postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
}

تهی* mspace_calloc(mspace msp, size_t n_elements, size_t elem_size) {
  تهی* mem;
  size_t req = 0;
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    بازگشت 0;
  }
  اگر (n_elements != 0) {
    req = n_elements * elem_size;
    اگر (((n_elements | elem_size) & ~(size_t)0xffff) &&
        (req / n_elements != elem_size))
      req = MAX_SIZE_T; /* force downstream failure on overflow */
  }
  mem = internal_malloc(ms, req);
  اگر (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  بازگشت mem;
}

تهی* mspace_realloc(mspace msp, تهی* oldmem, size_t bytes) {
  تهی* mem = 0;
  اگر (oldmem == 0) {
    mem = mspace_malloc(msp, bytes);
  }
  وگرنه اگر (bytes >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#ifdef REALLOC_ZERO_BYTES_FREES
  وگرنه اگر (bytes == 0) {
    mspace_free(msp, oldmem);
  }
#endif /* REALLOC_ZERO_BYTES_FREES */
  وگرنه {
    size_t nb = request2size(bytes);
    mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
    mstate m = (mstate)msp;
#وگرنه /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    اگر (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      بازگشت 0;
    }
#endif /* FOOTERS */
    اگر (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      اگر (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      وگرنه {
        mem = mspace_malloc(m, bytes);
        اگر (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          memcpy(mem, oldmem, (oc < bytes)? oc : bytes);
          mspace_free(m, oldmem);
        }
      }
    }
  }
  بازگشت mem;
}

تهی* mspace_realloc_in_place(mspace msp, تهی* oldmem, size_t bytes) {
  تهی* mem = 0;
  اگر (oldmem != 0) {
    اگر (bytes >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    وگرنه {
      size_t nb = request2size(bytes);
      mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
      mstate m = (mstate)msp;
#وگرنه /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      (تهی)msp; /* placate people compiling -Wunused */
      اگر (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        بازگشت 0;
      }
#endif /* FOOTERS */
      اگر (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        اگر (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  بازگشت mem;
}

تهی* mspace_memalign(mspace msp, size_t alignment, size_t bytes) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    بازگشت 0;
  }
  اگر (alignment <= MALLOC_ALIGNMENT)
    بازگشت mspace_malloc(msp, bytes);
  بازگشت internal_memalign(ms, alignment, bytes);
}

تهی** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_size, تهی* chunks[]) {
  size_t sz = elem_size; /* serves as 1-element array */
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    بازگشت 0;
  }
  بازگشت ialloc(ms, n_elements, &sz, 3, chunks);
}

تهی** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], تهی* chunks[]) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    بازگشت 0;
  }
  بازگشت ialloc(ms, n_elements, sizes, 0, chunks);
}

size_t mspace_bulk_free(mspace msp, تهی* array[], size_t nelem) {
  بازگشت internal_bulk_free((mstate)msp, array, nelem);
}

#اگر MALLOC_INSPECT_ALL
تهی mspace_inspect_all(mspace msp,
                        تهی(*handler)(تهی *start,
                                       تهی *end,
                                       size_t used_bytes,
                                       تهی* callback_arg),
                        تهی* arg) {
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    اگر (!PREACTION(ms)) {
      internal_inspect_all(ms, handler, arg);
      POSTACTION(ms);
    }
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#endif /* MALLOC_INSPECT_ALL */

صحیح mspace_trim(mspace msp, size_t pad) {
  صحیح result = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    اگر (!PREACTION(ms)) {
      result = sys_trim(ms, pad);
      POSTACTION(ms);
    }
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
  بازگشت result;
}

#اگر !NO_MALLOC_STATS
تهی mspace_malloc_stats(mspace msp) {
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    internal_malloc_stats(ms);
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#endif /* NO_MALLOC_STATS */

size_t mspace_footprint(mspace msp) {
  size_t result = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    result = ms->footprint;
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
  بازگشت result;
}

size_t mspace_max_footprint(mspace msp) {
  size_t result = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    result = ms->max_footprint;
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
  بازگشت result;
}

size_t mspace_footprint_limit(mspace msp) {
  size_t result = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    size_t maf = ms->footprint_limit;
    result = (maf == 0) ? MAX_SIZE_T : maf;
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
  بازگشت result;
}

size_t mspace_set_footprint_limit(mspace msp, size_t bytes) {
  size_t result = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    اگر (bytes == 0)
      result = granularity_align(1); /* Use minimal size */
    اگر (bytes == MAX_SIZE_T)
      result = 0;                    /* disable */
    وگرنه
      result = granularity_align(bytes);
    ms->footprint_limit = result;
  }
  وگرنه {
    USAGE_ERROR_ACTION(ms,ms);
  }
  بازگشت result;
}

#اگر !NO_MALLINFO
ساختار mallinfo mspace_mallinfo(mspace msp) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
  }
  بازگشت internal_mallinfo(ms);
}
#endif /* NO_MALLINFO */

size_t mspace_usable_size(ثابت تهی* mem) {
  اگر (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    اگر (is_inuse(p))
      بازگشت chunksize(p) - overhead_for(p);
  }
  بازگشت 0;
}

صحیح mspace_mallopt(صحیح param_عدد, صحیح مقدار) {
  بازگشت change_mparam(param_عدد, مقدار);
}

#endif /* MSPACES */


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
