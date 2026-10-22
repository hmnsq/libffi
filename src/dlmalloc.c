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
#جيڪڏهن_وصف_نه_ٿيل DLMALLOC_نسخو
#وصف_ڪريو DLMALLOC_نسخو 20806
#جيڪڏهن_پورو /* DLMALLOC_VERSION */

#جيڪڏهن_وصف_نه_ٿيل DLMALLOC_EXPORT
#وصف_ڪريو DLMALLOC_EXPORT خارجي
#جيڪڏهن_پورو

#إذا defined __linux__ && !defined GNU_ذريعو
/* mremap() on Linux requires this via sys/mman.h */
#وصف_ڪريو GNU_ذريعو 1
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل WIN32
#جيڪڏهن_وصف_ٿيل _WIN32
#وصف_ڪريو WIN32 1
#جيڪڏهن_پورو  /* _WIN32 */
#جيڪڏهن_وصف_ٿيل _WIN32_WCE
#وصف_ڪريو LACKS_FCNTL_H
#وصف_ڪريو WIN32 1
#جيڪڏهن_پورو /* _WIN32_WCE */
#جيڪڏهن_پورو  /* WIN32 */
#جيڪڏهن_وصف_ٿيل WIN32
#وصف_ڪريو WIN32_LEAN_AND_سراسري
#ضمن <windows.h>
#ضمن <tchar.h>
#وصف_ڪريو HAVE_MMAP 1
#وصف_ڪريو HAVE_MORECORE 0
#وصف_ڪريو LACKS_UNISTD_H
#وصف_ڪريو LACKS_SYS_PARAM_H
#وصف_ڪريو LACKS_SYS_MMAN_H
#وصف_ڪريو LACKS_STRING_H
#وصف_ڪريو LACKS_لڙيون_H
#وصف_ڪريو LACKS_SYS_TYPES_H
#وصف_ڪريو LACKS_ERRNO_H
#وصف_ڪريو LACKS_SCHED_H
#جيڪڏهن_وصف_نه_ٿيل MALLOC_FAILURE_ACTION
#وصف_ڪريو MALLOC_FAILURE_ACTION
#جيڪڏهن_پورو /* MALLOC_FAILURE_ACTION */
#جيڪڏهن_وصف_نه_ٿيل MMAP_CLEARS
#جيڪڏهن_وصف_ٿيل _WIN32_WCE /* WINCE reportedly does not clear */
#وصف_ڪريو MMAP_CLEARS 0
#وإلا
#وصف_ڪريو MMAP_CLEARS 1
#جيڪڏهن_پورو /* _WIN32_WCE */
#جيڪڏهن_پورو /*MMAP_CLEARS */
#جيڪڏهن_پورو  /* WIN32 */

#جيڪڏهن_وصف_ٿيل __OS2__
#وصف_ڪريو INCL_DOS
#ضمن <os2.h>
#وصف_ڪريو HAVE_MMAP 1
#وصف_ڪريو HAVE_MORECORE 0
#وصف_ڪريو LACKS_SYS_MMAN_H
#جيڪڏهن_پورو  /* __OS2__ */

#إذا defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#جيڪڏهن_وصف_نه_ٿيل HAVE_MORECORE
#وصف_ڪريو HAVE_MORECORE 0
#وصف_ڪريو HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#جيڪڏهن_وصف_نه_ٿيل MALLOC_ALIGNMENT
#وصف_ڪريو MALLOC_ALIGNMENT ((size_t)16U)
#جيڪڏهن_پورو
#جيڪڏهن_پورو  /* HAVE_MORECORE */
#جيڪڏهن_پورو  /* DARWIN */

#جيڪڏهن_وصف_نه_ٿيل LACKS_SYS_TYPES_H
#ضمن <sys/types.h>  /* For size_t */
#جيڪڏهن_پورو  /* LACKS_SYS_TYPES_H */

/* The maximum possible size_t value has all bits set */
#وصف_ڪريو MAX_سائيز_T           (~(size_t)0)

#جيڪڏهن_وصف_نه_ٿيل USE_LOCKS /* ensure true if spin or recursive locks set */
#وصف_ڪريو USE_LOCKS  ((defined(USE_SPIN_LOCKS) && USE_SPIN_LOCKS != 0) || \
                    (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0))
#جيڪڏهن_پورو /* USE_LOCKS */

#إذا USE_LOCKS /* Spin locks for gcc >= 4.1, older gcc on x86, MSC >= 1310 */
#إذا ((defined(__GNUC__) &&                                              \
      ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) ||      \
       defined(__i386__) || defined(__x86_64__))) ||                    \
     (defined(_MSC_VER) && _MSC_VER>=1310))
#جيڪڏهن_وصف_نه_ٿيل USE_SPIN_LOCKS
#وصف_ڪريو USE_SPIN_LOCKS 1
#جيڪڏهن_پورو /* USE_SPIN_LOCKS */
#وإلاإذا USE_SPIN_LOCKS
#غلطي "USE_SPIN_LOCKS defined without implementation"
#جيڪڏهن_پورو /* ... locks available... */
#وإلاإذا !defined(USE_SPIN_LOCKS)
#وصف_ڪريو USE_SPIN_LOCKS 0
#جيڪڏهن_پورو /* USE_LOCKS */

#جيڪڏهن_وصف_نه_ٿيل ONLY_MSPACES
#وصف_ڪريو ONLY_MSPACES 0
#جيڪڏهن_پورو  /* ONLY_MSPACES */
#جيڪڏهن_وصف_نه_ٿيل MSPACES
#إذا ONLY_MSPACES
#وصف_ڪريو MSPACES 1
#وإلا   /* ONLY_MSPACES */
#وصف_ڪريو MSPACES 0
#جيڪڏهن_پورو  /* ONLY_MSPACES */
#جيڪڏهن_پورو  /* MSPACES */
#جيڪڏهن_وصف_نه_ٿيل MALLOC_ALIGNMENT
#وصف_ڪريو MALLOC_ALIGNMENT ((size_t)(2 * حجم(فراغ *)))
#جيڪڏهن_پورو  /* MALLOC_ALIGNMENT */
#جيڪڏهن_وصف_نه_ٿيل FOOTERS
#وصف_ڪريو FOOTERS 0
#جيڪڏهن_پورو  /* FOOTERS */
#جيڪڏهن_وصف_نه_ٿيل ABORT
#وصف_ڪريو ABORT  abort()
#جيڪڏهن_پورو  /* ABORT */
#جيڪڏهن_وصف_نه_ٿيل ABORT_ON_ASSERT_FAILURE
#وصف_ڪريو ABORT_ON_ASSERT_FAILURE 1
#جيڪڏهن_پورو  /* ABORT_ON_ASSERT_FAILURE */
#جيڪڏهن_وصف_نه_ٿيل PROCEED_ON_ERROR
#وصف_ڪريو PROCEED_ON_ERROR 0
#جيڪڏهن_پورو  /* PROCEED_ON_ERROR */

#جيڪڏهن_وصف_نه_ٿيل INSECURE
#وصف_ڪريو INSECURE 0
#جيڪڏهن_پورو  /* INSECURE */
#جيڪڏهن_وصف_نه_ٿيل MALLOC_INSPECT_ALL
#وصف_ڪريو MALLOC_INSPECT_ALL 0
#جيڪڏهن_پورو  /* MALLOC_INSPECT_ALL */
#جيڪڏهن_وصف_نه_ٿيل HAVE_MMAP
#وصف_ڪريو HAVE_MMAP 1
#جيڪڏهن_پورو  /* HAVE_MMAP */
#جيڪڏهن_وصف_نه_ٿيل MMAP_CLEARS
#وصف_ڪريو MMAP_CLEARS 1
#جيڪڏهن_پورو  /* MMAP_CLEARS */
#جيڪڏهن_وصف_نه_ٿيل HAVE_MREMAP
#جيڪڏهن_وصف_ٿيل linux
#وصف_ڪريو HAVE_MREMAP 1
#وصف_ڪريو GNU_ذريعو /* Turns on mremap() definition */
#وإلا   /* linux */
#وصف_ڪريو HAVE_MREMAP 0
#جيڪڏهن_پورو  /* linux */
#جيڪڏهن_پورو  /* HAVE_MREMAP */
#جيڪڏهن_وصف_نه_ٿيل MALLOC_FAILURE_ACTION
#وصف_ڪريو MALLOC_FAILURE_ACTION  errno = ENOMEM;
#جيڪڏهن_پورو  /* MALLOC_FAILURE_ACTION */
#جيڪڏهن_وصف_نه_ٿيل HAVE_MORECORE
#إذا ONLY_MSPACES
#وصف_ڪريو HAVE_MORECORE 0
#وإلا   /* ONLY_MSPACES */
#وصف_ڪريو HAVE_MORECORE 1
#جيڪڏهن_پورو  /* ONLY_MSPACES */
#جيڪڏهن_پورو  /* HAVE_MORECORE */
#إذا !HAVE_MORECORE
#وصف_ڪريو MORECORE_CONTIGUOUS 0
#وإلا   /* !HAVE_MORECORE */
#وصف_ڪريو MORECORE_DEFAULT sbrk
#جيڪڏهن_وصف_نه_ٿيل MORECORE_CONTIGUOUS
#وصف_ڪريو MORECORE_CONTIGUOUS 1
#جيڪڏهن_پورو  /* MORECORE_CONTIGUOUS */
#جيڪڏهن_پورو  /* HAVE_MORECORE */
#جيڪڏهن_وصف_نه_ٿيل DEFAULT_GRANULARITY
#إذا (MORECORE_CONTIGUOUS || defined(WIN32))
#وصف_ڪريو DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#وإلا   /* MORECORE_CONTIGUOUS */
#وصف_ڪريو DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#جيڪڏهن_پورو  /* MORECORE_CONTIGUOUS */
#جيڪڏهن_پورو  /* DEFAULT_GRANULARITY */
#جيڪڏهن_وصف_نه_ٿيل DEFAULT_TRIM_حد
#جيڪڏهن_وصف_نه_ٿيل MORECORE_CANNOT_TRIM
#وصف_ڪريو DEFAULT_TRIM_حد ((size_t)2U * (size_t)1024U * (size_t)1024U)
#وإلا   /* MORECORE_CANNOT_TRIM */
#وصف_ڪريو DEFAULT_TRIM_حد MAX_سائيز_T
#جيڪڏهن_پورو  /* MORECORE_CANNOT_TRIM */
#جيڪڏهن_پورو  /* DEFAULT_TRIM_THRESHOLD */
#جيڪڏهن_وصف_نه_ٿيل DEFAULT_MMAP_حد
#إذا HAVE_MMAP
#وصف_ڪريو DEFAULT_MMAP_حد ((size_t)256U * (size_t)1024U)
#وإلا   /* HAVE_MMAP */
#وصف_ڪريو DEFAULT_MMAP_حد MAX_سائيز_T
#جيڪڏهن_پورو  /* HAVE_MMAP */
#جيڪڏهن_پورو  /* DEFAULT_MMAP_THRESHOLD */
#جيڪڏهن_وصف_نه_ٿيل MAX_RELEASE_CHECK_شرح
#إذا HAVE_MMAP
#وصف_ڪريو MAX_RELEASE_CHECK_شرح 4095
#وإلا
#وصف_ڪريو MAX_RELEASE_CHECK_شرح MAX_سائيز_T
#جيڪڏهن_پورو /* HAVE_MMAP */
#جيڪڏهن_پورو /* MAX_RELEASE_CHECK_RATE */
#جيڪڏهن_وصف_نه_ٿيل USE_BUILTIN_FFS
#وصف_ڪريو USE_BUILTIN_FFS 0
#جيڪڏهن_پورو  /* USE_BUILTIN_FFS */
#جيڪڏهن_وصف_نه_ٿيل USE_DEV_RANDOM
#وصف_ڪريو USE_DEV_RANDOM 0
#جيڪڏهن_پورو  /* USE_DEV_RANDOM */
#جيڪڏهن_وصف_نه_ٿيل NO_MALLINFO
#وصف_ڪريو NO_MALLINFO 0
#جيڪڏهن_پورو  /* NO_MALLINFO */
#جيڪڏهن_وصف_نه_ٿيل MALLINFO_ميدان_TYPE
#وصف_ڪريو MALLINFO_ميدان_TYPE size_t
#جيڪڏهن_پورو  /* MALLINFO_FIELD_TYPE */
#جيڪڏهن_وصف_نه_ٿيل NO_MALLOC_STATS
#وصف_ڪريو NO_MALLOC_STATS 0
#جيڪڏهن_پورو  /* NO_MALLOC_STATS */
#جيڪڏهن_وصف_نه_ٿيل NO_SEGMENT_TRAVERSAL
#وصف_ڪريو NO_SEGMENT_TRAVERSAL 0
#جيڪڏهن_پورو /* NO_SEGMENT_TRAVERSAL */

/*
  mallopt tuning options.  SVID/XPG defines four standard parameter
  numbers for mallopt, normally defined in malloc.h.  None of these
  are used in this malloc, so setting them has no effect. But this
  malloc does support the following options.
*/

/* The system's malloc.h may have conflicting defines. */
#وصف_رد M_TRIM_حد
#وصف_رد M_GRANULARITY
#وصف_رد M_MMAP_حد

#وصف_ڪريو M_TRIM_حد     (-1)
#وصف_ڪريو M_GRANULARITY        (-2)
#وصف_ڪريو M_MMAP_حد     (-3)

/* ------------------------ Mallinfo declarations ------------------------ */

#إذا !NO_MALLINFO
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

#جيڪڏهن_وصف_ٿيل HAVE_USR_INCLUDE_MALLOC_H
#ضمن "/usr/include/malloc.h"
#وإلا /* HAVE_USR_INCLUDE_MALLOC_H */
#جيڪڏهن_وصف_نه_ٿيل STRUCT_MALLINFO_DECLARED
/* HP-UX (and others?) redefines mallinfo unless _STRUCT_MALLINFO is defined */
#وصف_ڪريو _STRUCT_MALLINFO
#وصف_ڪريو STRUCT_MALLINFO_DECLARED 1
بنية mallinfo {
  MALLINFO_ميدان_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_ميدان_TYPE ordblks;  /* number of free chunks */
  MALLINFO_ميدان_TYPE smblks;   /* always 0 */
  MALLINFO_ميدان_TYPE hblks;    /* always 0 */
  MALLINFO_ميدان_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_ميدان_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_ميدان_TYPE fsmblks;  /* always 0 */
  MALLINFO_ميدان_TYPE uordblks; /* total allocated space */
  MALLINFO_ميدان_TYPE fordblks; /* total free space */
  MALLINFO_ميدان_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#جيڪڏهن_پورو /* STRUCT_MALLINFO_DECLARED */
#جيڪڏهن_پورو /* HAVE_USR_INCLUDE_MALLOC_H */
#جيڪڏهن_پورو /* NO_MALLINFO */

/*
  Try to persuade compilers to inline. The most critical functions for
  inlining are defined as macros, so these aren't used for them.
*/

#جيڪڏهن_وصف_نه_ٿيل FORCEINLINE
  #إذا defined(__GNUC__)
#وصف_ڪريو FORCEINLINE __inline __attribute__ ((always_inline))
  #وإلاإذا defined(_MSC_VER)
    #وصف_ڪريو FORCEINLINE __forceinline
  #جيڪڏهن_پورو
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل NOINLINE
  #إذا defined(__GNUC__)
    #وصف_ڪريو NOINLINE __attribute__ ((noinline))
  #وإلاإذا defined(_MSC_VER)
    #وصف_ڪريو NOINLINE __declspec(noinline)
  #وإلا
    #وصف_ڪريو NOINLINE
  #جيڪڏهن_پورو
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_وصف_نه_ٿيل FORCEINLINE
 #وصف_ڪريو FORCEINLINE مضمن
#جيڪڏهن_پورو
#جيڪڏهن_پورو /* __cplusplus */
#جيڪڏهن_وصف_نه_ٿيل FORCEINLINE
 #وصف_ڪريو FORCEINLINE
#جيڪڏهن_پورو

#إذا !ONLY_MSPACES

/* ------------------- Declarations of public routines ------------------- */

#جيڪڏهن_وصف_نه_ٿيل USE_DL_اڳياڙي
#وصف_ڪريو dlcalloc               صفر_يادگيري_ڏيو
#وصف_ڪريو dlfree                 حرر
#وصف_ڪريو dlmalloc               يادگيري_ڏيو
#وصف_ڪريو dlmemalign             memalign
#وصف_ڪريو dlposix_memalign       posix_memalign
#وصف_ڪريو dlrealloc              realloc
#وصف_ڪريو dlrealloc_اندريون_رکو     realloc_اندريون_رکو
#وصف_ڪريو dlvalloc               valloc
#وصف_ڪريو dlpvalloc              pvalloc
#وصف_ڪريو dlmallinfo             mallinfo
#وصف_ڪريو dlmallopt              mallopt
#وصف_ڪريو dlmalloc_trim          malloc_trim
#وصف_ڪريو dlmalloc_stats         malloc_stats
#وصف_ڪريو dlmalloc_usable_سائيز   malloc_usable_سائيز
#وصف_ڪريو dlmalloc_footprint     malloc_footprint
#وصف_ڪريو dlmalloc_max_footprint malloc_max_footprint
#وصف_ڪريو dlmalloc_footprint_حد_بندي malloc_footprint_حد_بندي
#وصف_ڪريو dlmalloc_سيٽ_footprint_حد_بندي malloc_سيٽ_footprint_حد_بندي
#وصف_ڪريو dlmalloc_inspect_all   malloc_inspect_all
#وصف_ڪريو dlindependent_calloc   independent_calloc
#وصف_ڪريو dlindependent_comalloc independent_comalloc
#وصف_ڪريو dlbulk_free            bulk_free
#جيڪڏهن_پورو /* USE_DL_PREFIX */

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
DLMALLOC_EXPORT فراغ* dlmalloc(size_t);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
DLMALLOC_EXPORT فراغ  dlfree(فراغ*);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
DLMALLOC_EXPORT فراغ* dlcalloc(size_t, size_t);

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
DLMALLOC_EXPORT فراغ* dlrealloc(فراغ*, size_t);

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
DLMALLOC_EXPORT فراغ* dlrealloc_اندريون_رکو(فراغ*, size_t);

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
DLMALLOC_EXPORT فراغ* dlmemalign(size_t, size_t);

/*
  int posix_memalign(void** pp, size_t alignment, size_t n);
  Allocates a chunk of n bytes, aligned in accord with the alignment
  argument. Differs from memalign only in that it (1) assigns the
  allocated memory to *pp rather than returning it, (2) fails and
  returns EINVAL if the alignment is not a power of two (3) fails and
  returns ENOMEM if memory cannot be allocated.
*/
DLMALLOC_EXPORT صحيح dlposix_memalign(فراغ**, size_t, size_t);

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
DLMALLOC_EXPORT فراغ* dlvalloc(size_t);

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
DLMALLOC_EXPORT صحيح dlmallopt(صحيح, صحيح);

/*
  malloc_footprint();
  Returns the number of bytes obtained from the system.  The total
  number of bytes allocated by malloc, realloc etc., is less than this
  value. Unlike mallinfo, this function returns only a precomputed
  result, so can be called frequently to monitor memory consumption.
  Even if locks are otherwise defined, this function does not use them,
  so results might not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint(فراغ);

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
DLMALLOC_EXPORT size_t dlmalloc_max_footprint(فراغ);

/*
  malloc_footprint_limit();
  Returns the number of bytes that the heap is allowed to obtain from
  the system, returning the last value returned by
  malloc_set_footprint_limit, or the maximum size_t value if
  never set. The returned value reflects a permission. There is no
  guarantee that this number of bytes can actually be obtained from
  the system.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint_حد_بندي();

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
DLMALLOC_EXPORT size_t dlmalloc_سيٽ_footprint_حد_بندي(size_t بائيٽون);

#إذا MALLOC_INSPECT_ALL
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
DLMALLOC_EXPORT فراغ dlmalloc_inspect_all(فراغ(*handler)(فراغ*, فراغ *, size_t, فراغ*),
                           فراغ* arg);

#جيڪڏهن_پورو /* MALLOC_INSPECT_ALL */

#إذا !NO_MALLINFO
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
DLMALLOC_EXPORT بنية mallinfo dlmallinfo(فراغ);
#جيڪڏهن_پورو /* NO_MALLINFO */

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
DLMALLOC_EXPORT فراغ** dlindependent_calloc(size_t, size_t, فراغ**);

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
DLMALLOC_EXPORT فراغ** dlindependent_comalloc(size_t, size_t*, فراغ**);

/*
  bulk_free(void* array[], size_t n_elements)
  Frees and clears (sets to null) each non-null pointer in the given
  array.  This is likely to be faster than freeing them one-by-one.
  If footers are used, pointers that have been allocated in different
  mspaces are not freed or cleared, and the count of all such pointers
  is returned.  For large arrays of pointers with poor locality, it
  may be worthwhile to sort this array before calling bulk_free.
*/
DLMALLOC_EXPORT size_t  dlbulk_free(فراغ**, size_t n_elements);

/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
DLMALLOC_EXPORT فراغ*  dlpvalloc(size_t);

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
DLMALLOC_EXPORT صحيح  dlmalloc_trim(size_t);

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
DLMALLOC_EXPORT فراغ  dlmalloc_stats(فراغ);

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
size_t dlmalloc_usable_سائيز(فراغ*);

#جيڪڏهن_پورو /* ONLY_MSPACES */

#إذا MSPACES

/*
  mspace is an opaque type representing an independent
  region of space that supports mspace_malloc, etc.
*/
تعريفنوع فراغ* mspace;

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
DLMALLOC_EXPORT mspace create_mspace(size_t صلاحيت, صحيح locked);

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
DLMALLOC_EXPORT mspace create_mspace_with_بنياد(فراغ* بنياد, size_t صلاحيت, صحيح locked);

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
DLMALLOC_EXPORT صحيح mspace_track_large_chunks(mspace msp, صحيح enable);


/*
  mspace_malloc behaves as malloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT فراغ* mspace_malloc(mspace msp, size_t بائيٽون);

/*
  mspace_free behaves as free, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_free is not actually needed.
  free may be called instead of mspace_free because freed chunks from
  any space are handled by their originating spaces.
*/
DLMALLOC_EXPORT فراغ mspace_free(mspace msp, فراغ* mem);

/*
  mspace_realloc behaves as realloc, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_realloc is not actually
  needed.  realloc may be called instead of mspace_realloc because
  realloced chunks from any space are handled by their originating
  spaces.
*/
DLMALLOC_EXPORT فراغ* mspace_realloc(mspace msp, فراغ* mem, size_t newsize);

/*
  mspace_calloc behaves as calloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT فراغ* mspace_calloc(mspace msp, size_t n_elements, size_t elem_سائيز);

/*
  mspace_memalign behaves as memalign, but operates within
  the given space.
*/
DLMALLOC_EXPORT فراغ* mspace_memalign(mspace msp, size_t alignment, size_t بائيٽون);

/*
  mspace_independent_calloc behaves as independent_calloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT فراغ** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_سائيز, فراغ* chunks[]);

/*
  mspace_independent_comalloc behaves as independent_comalloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT فراغ** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], فراغ* chunks[]);

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


#إذا !NO_MALLINFO
/*
  mspace_mallinfo behaves as mallinfo, but reports properties of
  the given space.
*/
DLMALLOC_EXPORT بنية mallinfo mspace_mallinfo(mspace msp);
#جيڪڏهن_پورو /* NO_MALLINFO */

/*
  malloc_usable_size(void* p) behaves the same as malloc_usable_size;
*/
DLMALLOC_EXPORT size_t mspace_usable_سائيز(ثابت فراغ* mem);

/*
  mspace_malloc_stats behaves as malloc_stats, but reports
  properties of the given space.
*/
DLMALLOC_EXPORT فراغ mspace_malloc_stats(mspace msp);

/*
  mspace_trim behaves as malloc_trim, but
  operates within the given space.
*/
DLMALLOC_EXPORT صحيح mspace_trim(mspace msp, size_t pad);

/*
  An alias for mallopt.
*/
DLMALLOC_EXPORT صحيح mspace_mallopt(صحيح, صحيح);

#جيڪڏهن_پورو /* MSPACES */

#جيڪڏهن_وصف_ٿيل __cplusplus
}  /* end of extern "C" */
#جيڪڏهن_پورو /* __cplusplus */

/*
  ========================================================================
  To make a fully customizable malloc.h header file, cut everything
  above this line, put into file malloc.h, edit to suit, and #include it
  on the next line, as well as in programs that use this malloc.
  ========================================================================
*/

/* #include "malloc.h" */

/*------------------------------ internal #includes ---------------------- */

#جيڪڏهن_وصف_ٿيل _MSC_VER
#هدايت warning( disable : 4146 ) /* no "unsigned" warnings */
#جيڪڏهن_پورو /* _MSC_VER */
#إذا !NO_MALLOC_STATS
#ضمن <stdio.h>       /* for printing in malloc_stats */
#جيڪڏهن_پورو /* NO_MALLOC_STATS */
#جيڪڏهن_وصف_نه_ٿيل LACKS_ERRNO_H
#ضمن <errno.h>       /* for MALLOC_FAILURE_ACTION */
#جيڪڏهن_پورو /* LACKS_ERRNO_H */
#جيڪڏهن_وصف_ٿيل DEBUG
#إذا ABORT_ON_ASSERT_FAILURE
#وصف_رد أكد
#وصف_ڪريو أكد(x) إذا(!(x)) ABORT
#وإلا /* ABORT_ON_ASSERT_FAILURE */
#ضمن <assert.h>
#جيڪڏهن_پورو /* ABORT_ON_ASSERT_FAILURE */
#وإلا  /* DEBUG */
#جيڪڏهن_وصف_نه_ٿيل أكد
#وصف_ڪريو أكد(x)
#جيڪڏهن_پورو
#وصف_ڪريو DEBUG 0
#جيڪڏهن_پورو /* DEBUG */
#إذا !defined(WIN32) && !defined(LACKS_وقت_H)
#ضمن <time.h>        /* for magic initialization */
#جيڪڏهن_پورو /* WIN32 */
#جيڪڏهن_وصف_نه_ٿيل LACKS_STDLIB_H
#ضمن <stdlib.h>      /* for abort() */
#جيڪڏهن_پورو /* LACKS_STDLIB_H */
#جيڪڏهن_وصف_نه_ٿيل LACKS_STRING_H
#ضمن <string.h>      /* for memset etc */
#جيڪڏهن_پورو  /* LACKS_STRING_H */
#إذا USE_BUILTIN_FFS
#جيڪڏهن_وصف_نه_ٿيل LACKS_لڙيون_H
#ضمن <strings.h>     /* for ffs */
#جيڪڏهن_پورو /* LACKS_STRINGS_H */
#جيڪڏهن_پورو /* USE_BUILTIN_FFS */
#إذا HAVE_MMAP
#جيڪڏهن_وصف_نه_ٿيل LACKS_SYS_MMAN_H
/* On some versions of linux, mremap decl in mman.h needs __USE_GNU set */
#إذا (defined(linux) && !defined(__USE_GNU))
#وصف_ڪريو __USE_GNU 1
#ضمن <sys/mman.h>    /* for mmap */
#وصف_رد __USE_GNU
#وإلا
#ضمن <sys/mman.h>    /* for mmap */
#جيڪڏهن_پورو /* linux */
#جيڪڏهن_پورو /* LACKS_SYS_MMAN_H */
#جيڪڏهن_وصف_نه_ٿيل LACKS_FCNTL_H
#ضمن <fcntl.h>
#جيڪڏهن_پورو /* LACKS_FCNTL_H */
#جيڪڏهن_پورو /* HAVE_MMAP */
#جيڪڏهن_وصف_نه_ٿيل LACKS_UNISTD_H
#ضمن <unistd.h>     /* for sbrk, sysconf */
#وإلا /* LACKS_UNISTD_H */
#إذا !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
خارجي فراغ*     sbrk(ptrdiff_t);
#جيڪڏهن_پورو /* FreeBSD etc */
#جيڪڏهن_پورو /* LACKS_UNISTD_H */

/* Declarations for locking */
#إذا USE_LOCKS
#جيڪڏهن_وصف_نه_ٿيل WIN32
#إذا defined (__SVR4) && defined (__sun)  /* solaris */
#ضمن <thread.h>
#وإلاإذا !defined(LACKS_SCHED_H)
#ضمن <sched.h>
#جيڪڏهن_پورو /* solaris or LACKS_SCHED_H */
#إذا (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0) || !USE_SPIN_LOCKS
#ضمن <pthread.h>
#جيڪڏهن_پورو /* USE_RECURSIVE_LOCKS ... */
#وإلاإذا defined(_MSC_VER)
#جيڪڏهن_وصف_نه_ٿيل _M_AMD64
/* These are already defined on AMD64 builds */
#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو /* __cplusplus */
LONG __cdecl _InterlockedCompareExchange(LONG متطاير *Dest, LONG Exchange, LONG Comp);
LONG __cdecl _InterlockedExchange(LONG متطاير *هدف, LONG قدر_2);
#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو /* __cplusplus */
#جيڪڏهن_پورو /* _M_AMD64 */
#هدايت intrinsic (_InterlockedCompareExchange)
#هدايت intrinsic (_InterlockedExchange)
#وصف_ڪريو interlockedcompareexchange _InterlockedCompareExchange
#وصف_ڪريو interlockedexchange _InterlockedExchange
#وإلاإذا defined(WIN32) && defined(__GNUC__)
#وصف_ڪريو interlockedcompareexchange(a, b, c) __sync_val_compare_and_swap(a, c, b)
#وصف_ڪريو interlockedexchange __sync_lock_test_and_set
#جيڪڏهن_پورو /* Win32 */
#وإلا /* USE_LOCKS */
#جيڪڏهن_پورو /* USE_LOCKS */

#جيڪڏهن_وصف_نه_ٿيل تالو_AT_FORK
#وصف_ڪريو تالو_AT_FORK 0
#جيڪڏهن_پورو

/* Declarations for bit scanning on win32 */
#إذا defined(_MSC_VER) && _MSC_VER>=1300
#جيڪڏهن_وصف_نه_ٿيل ٻٽو_انگ_جاچ_Forward /* Try to avoid pulling in WinNT.h */
#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو /* __cplusplus */
بنانشان حرف ٻٽو_انگ_جاچ_Forward_2(بنانشان ڊگهو *اشاريو, بنانشان ڊگهو نقاب);
بنانشان حرف ٻٽو_انگ_جاچ_ابتو(بنانشان ڊگهو *اشاريو, بنانشان ڊگهو نقاب);
#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو /* __cplusplus */

#وصف_ڪريو ٻٽو_انگ_جاچ_Forward ٻٽو_انگ_جاچ_Forward_2
#وصف_ڪريو ٻٽو_انگ_جاچ_ابتو_2 ٻٽو_انگ_جاچ_ابتو
#هدايت intrinsic(ٻٽو_انگ_جاچ_Forward_2)
#هدايت intrinsic(ٻٽو_انگ_جاچ_ابتو)
#جيڪڏهن_پورو /* BitScanForward */
#جيڪڏهن_پورو /* defined(_MSC_VER) && _MSC_VER>=1300 */

#جيڪڏهن_وصف_نه_ٿيل WIN32
#جيڪڏهن_وصف_نه_ٿيل malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef SC_صفحو_سائيز
#      define SC_صفحو_سائيز _SC_PAGESIZE
#    endif
#  endif
#  ifdef SC_صفحو_سائيز
#    define malloc_getpagesize sysconf(SC_صفحو_سائيز)
#  وإلا
#    إذا defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       خارجي size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    وإلا
#      ifdef WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize()
#      وإلا
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        وإلا
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            وإلا
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          وإلا
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            وإلا
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              وإلا /* just guess */
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#جيڪڏهن_پورو
#جيڪڏهن_پورو

/* libffi: the upstream malloc_getpagesize definition above is gated on
   !WIN32 (WIN32 builds use GetSystemInfo in init_mparams instead).  Guarantee
   the symbol is always defined so the !WIN32 init_mparams branch still compiles
   on toolchains where it is not preprocessed out as expected (e.g. MSVC).  */
#جيڪڏهن_وصف_نه_ٿيل malloc_getpagesize
#وصف_ڪريو malloc_getpagesize ((size_t)4096U)
#جيڪڏهن_پورو

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#وصف_ڪريو سائيز_T_سائيز         (حجم(size_t))
#وصف_ڪريو سائيز_T_BITSIZE      (حجم(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#وصف_ڪريو سائيز_T_ZERO         ((size_t)0)
#وصف_ڪريو سائيز_T_ONE          ((size_t)1)
#وصف_ڪريو سائيز_T_TWO          ((size_t)2)
#وصف_ڪريو سائيز_T_FOUR         ((size_t)4)
#وصف_ڪريو TWO_سائيز_T_SIZES    (سائيز_T_سائيز<<1)
#وصف_ڪريو FOUR_سائيز_T_SIZES   (سائيز_T_سائيز<<2)
#وصف_ڪريو SIX_سائيز_T_SIZES    (FOUR_سائيز_T_SIZES+TWO_سائيز_T_SIZES)
#وصف_ڪريو HALF_MAX_سائيز_T     (MAX_سائيز_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#وصف_ڪريو CHUNK_ALIGN_نقاب    (MALLOC_ALIGNMENT - سائيز_T_ONE)

/* True if address a has acceptable alignment */
#وصف_ڪريو is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_نقاب)) == 0)

/* the number of bytes to offset an address to align it */
#وصف_ڪريو align_وٿي(A)\
 ((((size_t)(A) & CHUNK_ALIGN_نقاب) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_نقاب)) & CHUNK_ALIGN_نقاب))

/* -------------------------- MMAP preliminaries ------------------------- */

/*
   If HAVE_MORECORE or HAVE_MMAP are false, we just define calls and
   checks to fail so compiler optimizer can delete code rather than
   using so many "#if"s.
*/


/* MORECORE and MMAP must return MFAIL on failure */
#وصف_ڪريو MFAIL                ((فراغ*)(MAX_سائيز_T))
#وصف_ڪريو CMFAIL               ((حرف*)(MFAIL)) /* defined for convenience */

#إذا HAVE_MMAP

#إذا !defined(WIN32) && !defined (__OS2__)
#وصف_ڪريو MUNMAP_DEFAULT(a, s)  munmap((a), (s))
#وصف_ڪريو MMAP_PROT            (PROT_READ|PROT_WRITE)
#إذا !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#وصف_ڪريو MAP_ANONYMOUS        MAP_ANON
#جيڪڏهن_پورو /* MAP_ANON */
#جيڪڏهن_وصف_ٿيل MAP_ANONYMOUS
#وصف_ڪريو MMAP_FLAGS           (MAP_ذاتي|MAP_ANONYMOUS)
#وصف_ڪريو MMAP_DEFAULT(s)       mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#وإلا /* MAP_ANONYMOUS */
/*
   Nearly all versions of mmap support MAP_ANONYMOUS, so the following
   is unlikely to be needed, but is supplied just in case.
*/
#وصف_ڪريو MMAP_FLAGS           (MAP_ذاتي)
ساكن صحيح dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#وصف_ڪريو MMAP_DEFAULT(s) ((dev_zero_fd < 0) ? \
           (dev_zero_fd = open("/dev/zero", O_RDWR), \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#جيڪڏهن_پورو /* MAP_ANONYMOUS */

#وصف_ڪريو سڌو_MMAP_DEFAULT(s) MMAP_DEFAULT(s)

#وإلاإذا defined(__OS2__)

/* OS/2 MMAP via DosAllocMem */
ساكن فراغ* os2mmap(size_t سائيز) {
  فراغ* ptr;
  إذا (DosAllocMem(&ptr, سائيز, OBJ_ANY|PAG_COMMIT|PAG_READ|PAG_WRITE) &&
      DosAllocMem(&ptr, سائيز, PAG_COMMIT|PAG_READ|PAG_WRITE))
    أرجع MFAIL;
  أرجع ptr;
}

#وصف_ڪريو os2direct_mmap(n)     os2mmap(n)

/* This function supports releasing coalesed segments */
ساكن صحيح os2munmap(فراغ* ptr, size_t سائيز) {
  طالما (سائيز) {
    ULONG ul_سائيز = سائيز;
    ULONG ulFlags = 0;
    إذا (DosQueryMem(ptr, &ul_سائيز, &ulFlags) != 0)
      أرجع -1;
    إذا ((ulFlags & PAG_بنياد) == 0 ||(ulFlags & PAG_COMMIT) == 0 ||
        ul_سائيز > سائيز)
      أرجع -1;
    إذا (DosFreeMem(ptr) != 0)
      أرجع -1;
    ptr = ( فراغ * ) ( ( حرف * ) ptr + ul_سائيز );
    سائيز -= ul_سائيز;
  }
  أرجع 0;
}

#وصف_ڪريو MMAP_DEFAULT(s)         os2mmap(s)
#وصف_ڪريو MUNMAP_DEFAULT(a, s)    os2munmap((a), (s))
#وصف_ڪريو سڌو_MMAP_DEFAULT(s)  os2direct_mmap(s)

#وإلا /* WIN32 */

/* Win32 MMAP via VirtualAlloc */
/* libffi: plain static (not FORCEINLINE) -- mingw's <windows.h> defines
   FORCEINLINE with an 'extern' storage class, which clashes with 'static'.  */
ساكن فراغ* win32mmap(size_t سائيز) {
  فراغ* ptr = VirtualAlloc(0, سائيز, MEM_RESERVE|MEM_COMMIT, صفحو_EXECUTE_READWRITE);
  أرجع (ptr != 0)? ptr: MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
ساكن فراغ* win32direct_mmap(size_t سائيز) {
  فراغ* ptr = VirtualAlloc(0, سائيز, MEM_RESERVE|MEM_COMMIT|MEM_چوٽي_هيٺ_ڏانهن,
                           صفحو_EXECUTE_READWRITE);
  أرجع (ptr != 0)? ptr: MFAIL;
}

/* This function supports releasing coalesed segments */
ساكن صحيح win32munmap(فراغ* ptr, size_t سائيز) {
  يادگيري_BASIC_INFORMATION minfo;
  حرف* cptr = (حرف*)ptr;
  طالما (سائيز) {
    إذا (VirtualQuery(cptr, &minfo, حجم(minfo)) == 0)
      أرجع -1;
    إذا (minfo.بنياد_پتو != cptr || minfo.ورهاست_بنياد != cptr ||
        minfo.State != MEM_COMMIT || minfo.Region_سائيز > سائيز)
      أرجع -1;
    إذا (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
      أرجع -1;
    cptr += minfo.Region_سائيز;
    سائيز -= minfo.Region_سائيز;
  }
  أرجع 0;
}

#وصف_ڪريو MMAP_DEFAULT(s)             win32mmap(s)
#وصف_ڪريو MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
#وصف_ڪريو سڌو_MMAP_DEFAULT(s)      win32direct_mmap(s)
#جيڪڏهن_پورو /* WIN32 */
#جيڪڏهن_پورو /* HAVE_MMAP */

#إذا HAVE_MREMAP
#جيڪڏهن_وصف_نه_ٿيل WIN32
#وصف_ڪريو MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#جيڪڏهن_پورو /* WIN32 */
#جيڪڏهن_پورو /* HAVE_MREMAP */

/**
 * Define CALL_MORECORE
 */
#إذا HAVE_MORECORE
    #جيڪڏهن_وصف_ٿيل MORECORE
        #وصف_ڪريو CALL_MORECORE(S)    MORECORE(S)
    #وإلا  /* MORECORE */
        #وصف_ڪريو CALL_MORECORE(S)    MORECORE_DEFAULT(S)
    #جيڪڏهن_پورو /* MORECORE */
#وإلا  /* HAVE_MORECORE */
    #وصف_ڪريو CALL_MORECORE(S)        MFAIL
#جيڪڏهن_پورو /* HAVE_MORECORE */

/**
 * Define CALL_MMAP/CALL_MUNMAP/CALL_DIRECT_MMAP
 */
#إذا HAVE_MMAP
    #وصف_ڪريو USE_MMAP_ٻٽو_انگ            (سائيز_T_ONE)

    #جيڪڏهن_وصف_ٿيل MMAP
        #وصف_ڪريو CALL_MMAP(s)        MMAP(s)
    #وإلا /* MMAP */
        #وصف_ڪريو CALL_MMAP(s)        MMAP_DEFAULT(s)
    #جيڪڏهن_پورو /* MMAP */
    #جيڪڏهن_وصف_ٿيل MUNMAP
        #وصف_ڪريو CALL_MUNMAP(a, s)   MUNMAP((a), (s))
    #وإلا /* MUNMAP */
        #وصف_ڪريو CALL_MUNMAP(a, s)   MUNMAP_DEFAULT((a), (s))
    #جيڪڏهن_پورو /* MUNMAP */
    #جيڪڏهن_وصف_ٿيل سڌو_MMAP
        #وصف_ڪريو CALL_سڌو_MMAP(s) سڌو_MMAP(s)
    #وإلا /* DIRECT_MMAP */
        #وصف_ڪريو CALL_سڌو_MMAP(s) سڌو_MMAP_DEFAULT(s)
    #جيڪڏهن_پورو /* DIRECT_MMAP */
#وإلا  /* HAVE_MMAP */
    #وصف_ڪريو USE_MMAP_ٻٽو_انگ            (سائيز_T_ZERO)

    #وصف_ڪريو MMAP(s)                 MFAIL
    #وصف_ڪريو MUNMAP(a, s)            (-1)
    #وصف_ڪريو سڌو_MMAP(s)          MFAIL
    #وصف_ڪريو CALL_سڌو_MMAP(s)     سڌو_MMAP(s)
    #وصف_ڪريو CALL_MMAP(s)            MMAP(s)
    #وصف_ڪريو CALL_MUNMAP(a, s)       MUNMAP((a), (s))
#جيڪڏهن_پورو /* HAVE_MMAP */

/**
 * Define CALL_MREMAP
 */
#إذا HAVE_MMAP && HAVE_MREMAP
    #جيڪڏهن_وصف_ٿيل MREMAP
        #وصف_ڪريو CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
    #وإلا /* MREMAP */
        #وصف_ڪريو CALL_MREMAP(addr, osz, nsz, mv) MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
    #جيڪڏهن_پورو /* MREMAP */
#وإلا  /* HAVE_MMAP && HAVE_MREMAP */
    #وصف_ڪريو CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#جيڪڏهن_پورو /* HAVE_MMAP && HAVE_MREMAP */

/* mstate bit set if contiguous morecore disabled or failed */
#وصف_ڪريو USE_NONCONTIGUOUS_ٻٽو_انگ (4U)

/* segment bit set in create_mspace_with_base */
#وصف_ڪريو EXTERN_ٻٽو_انگ            (8U)


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

#إذا !USE_LOCKS
#وصف_ڪريو USE_تالو_ٻٽو_انگ               (0U)
#وصف_ڪريو INITIAL_تالو(l)            (0)
#وصف_ڪريو DESTROY_تالو(l)            (0)
#وصف_ڪريو ACQUIRE_MALLOC_GLOBAL_تالو()
#وصف_ڪريو RELEASE_MALLOC_GLOBAL_تالو()

#وإلا
#إذا USE_LOCKS > 1
/* -----------------------  User-defined locks ------------------------ */
/* Define your own lock implementation here */
/* #define INITIAL_LOCK(lk)  ... */
/* #define DESTROY_LOCK(lk)  ... */
/* #define ACQUIRE_LOCK(lk)  ... */
/* #define RELEASE_LOCK(lk)  ... */
/* #define TRY_LOCK(lk) ... */
/* static MLOCK_T malloc_global_mutex = ... */

#وإلاإذا USE_SPIN_LOCKS

/* First, define CAS_LOCK and CLEAR_LOCK on ints */
/* Note CAS_LOCK defined to return 0 on success */

#إذا defined(__GNUC__)&& (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#وصف_ڪريو CAS_تالو(sl)     __sync_lock_test_and_set(sl, 1)
#وصف_ڪريو CLEAR_تالو(sl)   __sync_lock_release(sl)

#وإلاإذا (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
/* Custom spin locks for older gcc on x86 */
ساكن FORCEINLINE صحيح x86_cas_تالو(صحيح *sl) {
  صحيح ret;
  صحيح val = 1;
  صحيح cmp = 0;
  __asm__ __volatile__  ("lock; cmpxchgl %1, %2"
                         : "=a" (ret)
                         : "r" (val), "m" (*(sl)), "0"(cmp)
                         : "memory", "cc");
  أرجع ret;
}

ساكن FORCEINLINE فراغ x86_clear_تالو(صحيح* sl) {
  أكد(*sl != 0);
  صحيح prev = 0;
  صحيح ret;
  __asm__ __volatile__ ("lock; xchgl %0, %1"
                        : "=r" (ret)
                        : "m" (*(sl)), "0"(prev)
                        : "memory");
}

#وصف_ڪريو CAS_تالو(sl)     x86_cas_تالو(sl)
#وصف_ڪريو CLEAR_تالو(sl)   x86_clear_تالو(sl)

#وإلا /* Win32 MSC */
#وصف_ڪريو CAS_تالو(sl)     interlockedexchange((LONG متطاير *)(sl), (LONG)1)
#وصف_ڪريو CLEAR_تالو(sl)   interlockedexchange ((LONG متطاير *)(sl), (LONG)0)

#جيڪڏهن_پورو /* ... gcc spins locks ... */

/* How to yield for a spin lock */
#وصف_ڪريو SPINS_في_YIELD       63
#إذا defined(_MSC_VER)
#وصف_ڪريو SLEEP_EX_DURATION     50 /* delay for yield/sleep */
#وصف_ڪريو SPIN_تالو_YIELD  SleepEx(SLEEP_EX_DURATION, FALSE)
#وإلاإذا defined (__SVR4) && defined (__sun) /* solaris */
#وصف_ڪريو SPIN_تالو_YIELD   thr_yield();
#وإلاإذا !defined(LACKS_SCHED_H)
#وصف_ڪريو SPIN_تالو_YIELD   sched_yield();
#وإلا
#وصف_ڪريو SPIN_تالو_YIELD
#جيڪڏهن_پورو /* ... yield ... */

/* libffi: read the spin-lock word with a relaxed atomic load for the lock-free
   "is it held?" peek, so ThreadSanitizer does not flag it as racing with the
   atomic CLEAR_LOCK release on unlock.  Relaxed ordering is sufficient: the
   peek is only a hint to avoid a costly CAS, and CAS_LOCK still provides the
   acquire ordering on success.  Falls back to the original volatile read when
   atomic builtins are unavailable.  */
#إذا defined(__ATOMIC_RELAXED)
# define ffi_spin_peek(sl) __atomic_load_n((sl), __ATOMIC_RELAXED)
#وإلاإذا defined(_MSC_VER)
# define ffi_spin_peek(sl) \
  ((صحيح)_InterlockedCompareExchange((LONG متطاير *)(sl), (LONG)0, (LONG)0))
#وإلا
# define ffi_spin_peek(sl) (*(متطاير صحيح *)(sl))
#جيڪڏهن_پورو

#إذا !defined(USE_RECURSIVE_LOCKS) || USE_RECURSIVE_LOCKS == 0
/* Plain spin locks use single word (embedded in malloc_states) */
ساكن صحيح spin_acquire_تالو(صحيح *sl) {
  صحيح spins = 0;
  طالما (ffi_spin_peek(sl) != 0 || CAS_تالو(sl)) {
    إذا ((++spins & SPINS_في_YIELD) == 0) {
      SPIN_تالو_YIELD;
    }
  }
  أرجع 0;
}

#وصف_ڪريو MLOCK_T               صحيح
#وصف_ڪريو TRY_تالو(sl)          !CAS_تالو(sl)
#وصف_ڪريو RELEASE_تالو(sl)      CLEAR_تالو(sl)
#وصف_ڪريو ACQUIRE_تالو(sl)      (CAS_تالو(sl)? spin_acquire_تالو(sl) : 0)
#وصف_ڪريو INITIAL_تالو(sl)      (*sl = 0)
#وصف_ڪريو DESTROY_تالو(sl)      (0)
ساكن MLOCK_T malloc_global_mutex = 0;

#وإلا /* USE_RECURSIVE_LOCKS */
/* types for lock owners */
#جيڪڏهن_وصف_ٿيل WIN32
#وصف_ڪريو THREAD_ID_T           DWORD
#وصف_ڪريو CURRENT_THREAD        GetCurrentThreadId()
#وصف_ڪريو EQ_OWNER(X,Y)         ((X) == (Y))
#وإلا
/*
  Note: the following assume that pthread_t is a type that can be
  initialized to (casted) zero. If this is not the case, you will need to
  somehow redefine these or not use spin locks.
*/
#وصف_ڪريو THREAD_ID_T           pthread_t
#وصف_ڪريو CURRENT_THREAD        pthread_self()
#وصف_ڪريو EQ_OWNER(X,Y)         pthread_equal(X, Y)
#جيڪڏهن_پورو

بنية malloc_recursive_تالو {
  صحيح sl;
  بنانشان صحيح c;
  THREAD_ID_T threadid;
};

#وصف_ڪريو MLOCK_T  بنية malloc_recursive_تالو
ساكن MLOCK_T malloc_global_mutex = { 0, 0, (THREAD_ID_T)0};

ساكن FORCEINLINE فراغ recursive_release_تالو(MLOCK_T *lk) {
  أكد(lk->sl != 0);
  إذا (--lk->c == 0) {
    CLEAR_تالو(&lk->sl);
  }
}

ساكن FORCEINLINE صحيح recursive_acquire_تالو(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  صحيح spins = 0;
  لكل (;;) {
    إذا (ffi_spin_peek(&lk->sl) == 0) {
      إذا (!CAS_تالو(&lk->sl)) {
        lk->threadid = mythreadid;
        lk->c = 1;
        أرجع 0;
      }
    }
    وإلا إذا (EQ_OWNER(lk->threadid, mythreadid)) {
      ++lk->c;
      أرجع 0;
    }
    إذا ((++spins & SPINS_في_YIELD) == 0) {
      SPIN_تالو_YIELD;
    }
  }
}

ساكن FORCEINLINE صحيح recursive_try_تالو(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  إذا (ffi_spin_peek(&lk->sl) == 0) {
    إذا (!CAS_تالو(&lk->sl)) {
      lk->threadid = mythreadid;
      lk->c = 1;
      أرجع 1;
    }
  }
  وإلا إذا (EQ_OWNER(lk->threadid, mythreadid)) {
    ++lk->c;
    أرجع 1;
  }
  أرجع 0;
}

#وصف_ڪريو RELEASE_تالو(lk)      recursive_release_تالو(lk)
#وصف_ڪريو TRY_تالو(lk)          recursive_try_تالو(lk)
#وصف_ڪريو ACQUIRE_تالو(lk)      recursive_acquire_تالو(lk)
#وصف_ڪريو INITIAL_تالو(lk)      ((lk)->threadid = (THREAD_ID_T)0, (lk)->sl = 0, (lk)->c = 0)
#وصف_ڪريو DESTROY_تالو(lk)      (0)
#جيڪڏهن_پورو /* USE_RECURSIVE_LOCKS */

#وإلاإذا defined(WIN32) /* Win32 critical sections */
#وصف_ڪريو MLOCK_T               CRITICAL_SECTION
#وصف_ڪريو ACQUIRE_تالو(lk)      (EnterCriticalSection(lk), 0)
#وصف_ڪريو RELEASE_تالو(lk)      LeaveCriticalSection(lk)
#وصف_ڪريو TRY_تالو(lk)          TryEnterCriticalSection(lk)
#وصف_ڪريو INITIAL_تالو(lk)      (!Initialize_Critical_Section_And_Spin_ڳڻپ((lk), 0x80000000|4000))
#وصف_ڪريو DESTROY_تالو(lk)      (DeleteCriticalSection(lk), 0)
#وصف_ڪريو NEED_GLOBAL_تالو_INIT

ساكن MLOCK_T malloc_global_mutex;
ساكن متطاير LONG malloc_global_mutex_حالت;

/* Use spin loop to initialize global lock */
ساكن فراغ init_malloc_global_mutex() {
  لكل (;;) {
    ڊگهو stat = malloc_global_mutex_حالت;
    إذا (stat > 0)
      أرجع;
    /* transition to < 0 while initializing, then to > 0) */
    إذا (stat == 0 &&
        interlockedcompareexchange(&malloc_global_mutex_حالت, (LONG)-1, (LONG)0) == 0) {
      InitializeCriticalSection(&malloc_global_mutex);
      interlockedexchange(&malloc_global_mutex_حالت, (LONG)1);
      أرجع;
    }
    SleepEx(0, FALSE);
  }
}

#وإلا /* pthreads-based locks */
#وصف_ڪريو MLOCK_T               pthread_mutex_t
#وصف_ڪريو ACQUIRE_تالو(lk)      pthread_mutex_تالو(lk)
#وصف_ڪريو RELEASE_تالو(lk)      pthread_mutex_unlock(lk)
#وصف_ڪريو TRY_تالو(lk)          (!pthread_mutex_trylock(lk))
#وصف_ڪريو INITIAL_تالو(lk)      pthread_init_تالو(lk)
#وصف_ڪريو DESTROY_تالو(lk)      pthread_mutex_destroy(lk)

#إذا defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0 && defined(linux) && !defined(PTHREAD_MUTEX_RECURSIVE)
/* Cope with old-style linux recursive lock initialization by adding */
/* skipped internal declaration from pthread.h */
خارجي صحيح pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
                                              صحيح __kind));
#وصف_ڪريو PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#وصف_ڪريو pthread_mutexattr_settype(x,y) pthread_mutexattr_setkind_np(x,y)
#جيڪڏهن_پورو /* USE_RECURSIVE_LOCKS ... */

ساكن MLOCK_T malloc_global_mutex = PTHREAD_MUTEX_INITIALIZER;

ساكن صحيح pthread_init_تالو (MLOCK_T *lk) {
  pthread_mutexattr_t attr;
  إذا (pthread_mutexattr_init(&attr)) أرجع 1;
#إذا defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0
  إذا (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) أرجع 1;
#جيڪڏهن_پورو
  إذا (pthread_mutex_init(lk, &attr)) أرجع 1;
  إذا (pthread_mutexattr_destroy(&attr)) أرجع 1;
  أرجع 0;
}

#جيڪڏهن_پورو /* ... lock types ... */

/* Common code for all lock types */
#وصف_ڪريو USE_تالو_ٻٽو_انگ               (2U)

#جيڪڏهن_وصف_نه_ٿيل ACQUIRE_MALLOC_GLOBAL_تالو
#وصف_ڪريو ACQUIRE_MALLOC_GLOBAL_تالو()  ACQUIRE_تالو(&malloc_global_mutex);
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل RELEASE_MALLOC_GLOBAL_تالو
#وصف_ڪريو RELEASE_MALLOC_GLOBAL_تالو()  RELEASE_تالو(&malloc_global_mutex);
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* USE_LOCKS */

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

بنية malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               سر;       /* Size and inuse bits. */
  بنية malloc_chunk* fd;         /* double links -- used only if free. */
  بنية malloc_chunk* bk;
};

تعريفنوع بنية malloc_chunk  mchunk;
تعريفنوع بنية malloc_chunk* mchunkptr;
تعريفنوع بنية malloc_chunk* sbinptr;  /* The type of bins of chunks */
تعريفنوع بنانشان صحيح bindex_t;         /* Described below */
تعريفنوع بنانشان صحيح binmap_t;         /* Described below */
تعريفنوع بنانشان صحيح flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#وصف_ڪريو MCHUNK_سائيز         (حجم(mchunk))

#إذا FOOTERS
#وصف_ڪريو CHUNK_OVERHEAD      (TWO_سائيز_T_SIZES)
#وإلا /* FOOTERS */
#وصف_ڪريو CHUNK_OVERHEAD      (سائيز_T_سائيز)
#جيڪڏهن_پورو /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#وصف_ڪريو MMAP_CHUNK_OVERHEAD (TWO_سائيز_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#وصف_ڪريو MMAP_FOOT_PAD       (FOUR_سائيز_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#وصف_ڪريو MIN_CHUNK_سائيز\
  ((MCHUNK_سائيز + CHUNK_ALIGN_نقاب) & ~CHUNK_ALIGN_نقاب)

/* conversion from malloc headers to user pointers, and back */
#وصف_ڪريو chunk2mem(p)        ((فراغ*)((حرف*)(p)       + TWO_سائيز_T_SIZES))
#وصف_ڪريو mem2chunk(mem)      ((mchunkptr)((حرف*)(mem) - TWO_سائيز_T_SIZES))
/* chunk associated with aligned address A */
#وصف_ڪريو align_as_chunk(A)   (mchunkptr)((A) + align_وٿي(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#وصف_ڪريو MAX_REQUEST         ((-MIN_CHUNK_سائيز) << 2)
#وصف_ڪريو MIN_REQUEST         (MIN_CHUNK_سائيز - CHUNK_OVERHEAD - سائيز_T_ONE)

/* pad request bytes into a usable size */
#وصف_ڪريو pad_request(req) \
   (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_نقاب) & ~CHUNK_ALIGN_نقاب)

/* pad request, checking for minimum (but not maximum) */
#وصف_ڪريو request2size(req) \
  (((req) < MIN_REQUEST)? MIN_CHUNK_سائيز : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#وصف_ڪريو PINUSE_ٻٽو_انگ          (سائيز_T_ONE)
#وصف_ڪريو CINUSE_ٻٽو_انگ          (سائيز_T_TWO)
#وصف_ڪريو FLAG4_ٻٽو_انگ           (سائيز_T_FOUR)
#وصف_ڪريو INUSE_بٽ          (PINUSE_ٻٽو_انگ|CINUSE_ٻٽو_انگ)
#وصف_ڪريو FLAG_بٽ           (PINUSE_ٻٽو_انگ|CINUSE_ٻٽو_انگ|FLAG4_ٻٽو_انگ)

/* Head value for fenceposts */
#وصف_ڪريو FENCEPOST_سر      (INUSE_بٽ|سائيز_T_سائيز)

/* extraction of fields from head words */
#وصف_ڪريو cinuse(p)           ((p)->سر & CINUSE_ٻٽو_انگ)
#وصف_ڪريو pinuse(p)           ((p)->سر & PINUSE_ٻٽو_انگ)
#وصف_ڪريو flag4inuse(p)       ((p)->سر & FLAG4_ٻٽو_انگ)
#وصف_ڪريو is_inuse(p)         (((p)->سر & INUSE_بٽ) != PINUSE_ٻٽو_انگ)
#وصف_ڪريو is_mmapped(p)       (((p)->سر & INUSE_بٽ) == 0)

#وصف_ڪريو chunksize(p)        ((p)->سر & ~(FLAG_بٽ))

#وصف_ڪريو clear_pinuse(p)     ((p)->سر &= ~PINUSE_ٻٽو_انگ)
#وصف_ڪريو سيٽ_flag4(p)        ((p)->سر |= FLAG4_ٻٽو_انگ)
#وصف_ڪريو clear_flag4(p)      ((p)->سر &= ~FLAG4_ٻٽو_انگ)

/* Treat space at ptr +/- offset as a chunk */
#وصف_ڪريو chunk_plus_وٿي(p, s)  ((mchunkptr)(((حرف*)(p)) + (s)))
#وصف_ڪريو chunk_minus_وٿي(p, s) ((mchunkptr)(((حرف*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#وصف_ڪريو ايندڙ_chunk(p) ((mchunkptr)( ((حرف*)(p)) + ((p)->سر & ~FLAG_بٽ)))
#وصف_ڪريو prev_chunk(p) ((mchunkptr)( ((حرف*)(p)) - ((p)->prev_foot) ))

/* extract next chunk's pinuse bit */
#وصف_ڪريو ايندڙ_pinuse(p)  ((ايندڙ_chunk(p)->سر) & PINUSE_ٻٽو_انگ)

/* Get/set size at footer */
#وصف_ڪريو get_foot(p, s)  (((mchunkptr)((حرف*)(p) + (s)))->prev_foot)
#وصف_ڪريو سيٽ_foot(p, s)  (((mchunkptr)((حرف*)(p) + (s)))->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#وصف_ڪريو سيٽ_سائيز_and_pinuse_جو_free_chunk(p, s)\
  ((p)->سر = (s|PINUSE_ٻٽو_انگ), سيٽ_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#وصف_ڪريو سيٽ_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), سيٽ_سائيز_and_pinuse_جو_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#وصف_ڪريو overhead_for(p)\
 (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)

/* Return true if malloced space is not necessarily cleared */
#إذا MMAP_CLEARS
#وصف_ڪريو calloc_must_clear(p) (!is_mmapped(p))
#وإلا /* MMAP_CLEARS */
#وصف_ڪريو calloc_must_clear(p) (1)
#جيڪڏهن_پورو /* MMAP_CLEARS */

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

بنية malloc_وڻ_chunk {
  /* The first four fields must be compatible with malloc_chunk */
  size_t                    prev_foot;
  size_t                    سر;
  بنية malloc_وڻ_chunk* fd;
  بنية malloc_وڻ_chunk* bk;

  بنية malloc_وڻ_chunk* ٻار[2];
  بنية malloc_وڻ_chunk* والد;
  bindex_t                  اشاريو;
};

تعريفنوع بنية malloc_وڻ_chunk  tchunk;
تعريفنوع بنية malloc_وڻ_chunk* tchunkptr;
تعريفنوع بنية malloc_وڻ_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#وصف_ڪريو leftmost_ٻار(t) ((t)->ٻار[0] != 0? (t)->ٻار[0] : (t)->ٻار[1])

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

بنية malloc_segment {
  حرف*        بنياد;             /* base address */
  size_t       سائيز;             /* allocated size */
  بنية malloc_segment* ايندڙ;   /* ptr to next segment */
#إذا FFI_MMAP_EXEC_WRIT
  /* The mmap magic is supposed to store the address of the executable
     segment at the very end of the requested block.  */

# define mmap_exec_وٿي(b,s) (*(ptrdiff_t*)((b)+(s)-حجم(ptrdiff_t)))

  /* We can only merge segments if their corresponding executable
     segments are at identical offsets.  */
# define check_segment_ملايو(S,b,s) \
  (mmap_exec_وٿي((b),(s)) == (S)->exec_وٿي)

# define جوڙيو_segment_exec_وٿي(p,S) ((حرف*)(p) + (S)->exec_وٿي)
# define sub_segment_exec_وٿي(p,S) ((حرف*)(p) - (S)->exec_وٿي)

  /* The removal of sflags only works with HAVE_MORECORE == 0.  */

# define get_segment_flags(S)   (USE_MMAP_ٻٽو_انگ)
# define سيٽ_segment_flags(S,v) \
  (((v) != USE_MMAP_ٻٽو_انگ) ? (ABORT, (v)) :				\
   (((S)->exec_وٿي =							\
     mmap_exec_وٿي((S)->بنياد, (S)->سائيز)),				\
    (mmap_exec_وٿي((S)->بنياد + (S)->exec_وٿي, (S)->سائيز) !=	\
     (S)->exec_وٿي) ? (ABORT, (v)) :					\
   (mmap_exec_وٿي((S)->بنياد, (S)->سائيز) = 0), (v)))

  /* We use an offset here, instead of a pointer, because then, when
     base changes, we don't have to modify this.  On architectures
     with segmented addresses, this might not work.  */
  ptrdiff_t    exec_وٿي;
#وإلا

# define get_segment_flags(S)   ((S)->sflags)
# define سيٽ_segment_flags(S,v) ((S)->sflags = (v))
# define check_segment_ملايو(S,b,s) (1)

  flag_t       sflags;           /* mmap and extern flag */
#جيڪڏهن_پورو
};

#وصف_ڪريو is_mmapped_segment(S)  (get_segment_flags(S) & USE_MMAP_ٻٽو_انگ)
#وصف_ڪريو is_extern_segment(S)   (get_segment_flags(S) & EXTERN_ٻٽو_انگ)

تعريفنوع بنية malloc_segment  msegment;
تعريفنوع بنية malloc_segment* msegmentptr;

/* ---------------------------- malloc_state ----------------------------- */

/*
   A malloc_state holds all of the bookkeeping for a space.
   The main fields are:

چوٽي
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
#وصف_ڪريو NSMALLBINS        (32U)
#وصف_ڪريو NTREEBINS         (32U)
#وصف_ڪريو SMALLBIN_سرڪايو    (3U)
#وصف_ڪريو SMALLBIN_ويڪر    (سائيز_T_ONE << SMALLBIN_سرڪايو)
#وصف_ڪريو TREEBIN_سرڪايو     (8U)
#وصف_ڪريو MIN_LARGE_سائيز    (سائيز_T_ONE << TREEBIN_سرڪايو)
#وصف_ڪريو MAX_SMALL_سائيز    (MIN_LARGE_سائيز - سائيز_T_ONE)
#وصف_ڪريو MAX_SMALL_REQUEST (MAX_SMALL_سائيز - CHUNK_ALIGN_نقاب - CHUNK_OVERHEAD)

بنية malloc_state {
  binmap_t   smallmap;
  binmap_t   treemap;
  size_t     dvsize;
  size_t     topsize;
  حرف*      گهٽ_کان_گهٽ_addr;
  mchunkptr  dv;
  mchunkptr  چوٽي;
  size_t     trim_check;
  size_t     release_checks;
  size_t     magic;
  mchunkptr  smallbins[(NSMALLBINS+1)*2];
  tbinptr    treebins[NTREEBINS];
  size_t     footprint;
  size_t     max_footprint;
  size_t     footprint_حد_بندي; /* zero means no limit */
  flag_t     mflags;
#إذا USE_LOCKS
  MLOCK_T    mutex;     /* locate lock among fields that rarely change */
#جيڪڏهن_پورو /* USE_LOCKS */
  msegment   seg;
  فراغ*      extp;      /* Unused but available for extensions */
  size_t     exts;
};

تعريفنوع بنية malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

بنية malloc_params {
  size_t magic;
  size_t صفحو_سائيز;
  size_t granularity;
  size_t mmap_حد;
  size_t trim_حد;
  flag_t default_mflags;
};

ساكن بنية malloc_params mparams;

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
#إذا USE_LOCKS && defined(__ATOMIC_ACQUIRE)
# define ffi_mparams_magic_ڀريو()   __atomic_load_n(&mparams.magic, __ATOMIC_ACQUIRE)
# define ffi_mparams_magic_store(v) __atomic_store_n(&mparams.magic, (size_t)(v), __ATOMIC_RELEASE)
#وإلاإذا USE_LOCKS && defined(_MSC_VER)
# إذا defined(_WIN64)
#  define ffi_mparams_magic_ڀريو() \
  ((size_t)_InterlockedCompareExchange64((__int64 متطاير *)&mparams.magic, \
                                         (__int64)0, (__int64)0))
#  define ffi_mparams_magic_store(v) \
  ((فراغ)_InterlockedExchange64((__int64 متطاير *)&mparams.magic, \
                                (__int64)(v)))
# وإلا
#  define ffi_mparams_magic_ڀريو() \
  ((size_t)_InterlockedCompareExchange((LONG متطاير *)&mparams.magic, \
                                       (LONG)0, (LONG)0))
#  define ffi_mparams_magic_store(v) \
  ((فراغ)_InterlockedExchange((LONG متطاير *)&mparams.magic, (LONG)(v)))
# endif
#وإلا
# define ffi_mparams_magic_ڀريو()   (mparams.magic)
# define ffi_mparams_magic_store(v) (*(متطاير size_t *)(&(mparams.magic)) = (size_t)(v))
#جيڪڏهن_پورو

/* Ensure mparams initialized */
#وصف_ڪريو ensure_initialization() (فراغ)(ffi_mparams_magic_ڀريو() != 0 || init_mparams())

#إذا !ONLY_MSPACES

/* The global malloc_state used for all non-"mspace" calls */
ساكن بنية malloc_state _gm_;
#وصف_ڪريو gm                 (&_gm_)
#وصف_ڪريو is_global(M)       ((M) == &_gm_)

#جيڪڏهن_پورو /* !ONLY_MSPACES */

#وصف_ڪريو is_initialized(M)  ((M)->چوٽي != 0)

/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#وصف_ڪريو use_تالو(M)           ((M)->mflags &   USE_تالو_ٻٽو_انگ)
#وصف_ڪريو enable_تالو(M)        ((M)->mflags |=  USE_تالو_ٻٽو_انگ)
#إذا USE_LOCKS
#وصف_ڪريو disable_تالو(M)       ((M)->mflags &= ~USE_تالو_ٻٽو_انگ)
#وإلا
#وصف_ڪريو disable_تالو(M)
#جيڪڏهن_پورو

#وصف_ڪريو use_mmap(M)           ((M)->mflags &   USE_MMAP_ٻٽو_انگ)
#وصف_ڪريو enable_mmap(M)        ((M)->mflags |=  USE_MMAP_ٻٽو_انگ)
#إذا HAVE_MMAP
#وصف_ڪريو disable_mmap(M)       ((M)->mflags &= ~USE_MMAP_ٻٽو_انگ)
#وإلا
#وصف_ڪريو disable_mmap(M)
#جيڪڏهن_پورو

#وصف_ڪريو use_noncontiguous(M)  ((M)->mflags &   USE_NONCONTIGUOUS_ٻٽو_انگ)
#وصف_ڪريو disable_contiguous(M) ((M)->mflags |=  USE_NONCONTIGUOUS_ٻٽو_انگ)

#وصف_ڪريو سيٽ_تالو(M,L)\
 ((M)->mflags = (L)?\
  ((M)->mflags | USE_تالو_ٻٽو_انگ) :\
  ((M)->mflags & ~USE_تالو_ٻٽو_انگ))

/* page-align a size */
#وصف_ڪريو صفحو_align(S)\
 (((S) + (mparams.صفحو_سائيز - سائيز_T_ONE)) & ~(mparams.صفحو_سائيز - سائيز_T_ONE))

/* granularity-align a size */
#وصف_ڪريو granularity_align(S)\
  (((S) + (mparams.granularity - سائيز_T_ONE))\
   & ~(mparams.granularity - سائيز_T_ONE))


/* For mmap, use granularity alignment on windows, else page-align */
#جيڪڏهن_وصف_ٿيل WIN32
#وصف_ڪريو mmap_align(S) granularity_align(S)
#وإلا
#وصف_ڪريو mmap_align(S) صفحو_align(S)
#جيڪڏهن_پورو

/* For sys_alloc, enough padding to ensure can malloc request on success */
#وصف_ڪريو SYS_ALLOC_PADDING (چوٽي_FOOT_سائيز + MALLOC_ALIGNMENT)

#وصف_ڪريو is_صفحو_aligned(S)\
   (((size_t)(S) & (mparams.صفحو_سائيز - سائيز_T_ONE)) == 0)
#وصف_ڪريو is_granularity_aligned(S)\
   (((size_t)(S) & (mparams.granularity - سائيز_T_ONE)) == 0)

/*  True if segment S holds address A */
#وصف_ڪريو segment_holds(S, A)\
  ((حرف*)(A) >= S->بنياد && (حرف*)(A) < S->بنياد + S->سائيز)

/* Return segment holding given address */
ساكن msegmentptr segment_holding(mstate m, حرف* addr) {
  msegmentptr sp = &m->seg;
  لكل (;;) {
    إذا (addr >= sp->بنياد && addr < sp->بنياد + sp->سائيز)
      أرجع sp;
    إذا ((sp = sp->ايندڙ) == 0)
      أرجع 0;
  }
}

/* Return true if segment contains a segment link */
ساكن صحيح has_segment_link(mstate m, msegmentptr ss) {
  msegmentptr sp = &m->seg;
  لكل (;;) {
    إذا ((حرف*)sp >= ss->بنياد && (حرف*)sp < ss->بنياد + ss->سائيز)
      أرجع 1;
    إذا ((sp = sp->ايندڙ) == 0)
      أرجع 0;
  }
}

#جيڪڏهن_وصف_نه_ٿيل MORECORE_CANNOT_TRIM
#وصف_ڪريو should_trim(M,s)  ((s) > (M)->trim_check)
#وإلا  /* MORECORE_CANNOT_TRIM */
#وصف_ڪريو should_trim(M,s)  (0)
#جيڪڏهن_پورو /* MORECORE_CANNOT_TRIM */

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#وصف_ڪريو چوٽي_FOOT_سائيز\
  (align_وٿي(chunk2mem(0))+pad_request(حجم(بنية malloc_segment))+MIN_CHUNK_سائيز)


/* -------------------------------  Hooks -------------------------------- */

/*
  PREACTION should be defined to return 0 on success, and nonzero on
  failure. If you are not using locking, you can redefine these to do
  anything you like.
*/

#إذا USE_LOCKS
#وصف_ڪريو PREACTION(M)  ((use_تالو(M))? ACQUIRE_تالو(&(M)->mutex) : 0)
#وصف_ڪريو POSTACTION(M) { إذا (use_تالو(M)) RELEASE_تالو(&(M)->mutex); }
#وإلا /* USE_LOCKS */

#جيڪڏهن_وصف_نه_ٿيل PREACTION
#وصف_ڪريو PREACTION(M) (0)
#جيڪڏهن_پورو  /* PREACTION */

#جيڪڏهن_وصف_نه_ٿيل POSTACTION
#وصف_ڪريو POSTACTION(M)
#جيڪڏهن_پورو  /* POSTACTION */

#جيڪڏهن_پورو /* USE_LOCKS */

/*
  CORRUPTION_ERROR_ACTION is triggered upon detected bad addresses.
  USAGE_ERROR_ACTION is triggered on detected bad frees and
  reallocs. The argument p is an address that might have triggered the
  fault. It is ignored by the two predefined actions, but might be
  useful in custom actions that try to help diagnose errors.
*/

#إذا PROCEED_ON_ERROR

/* A count of the number of corruption errors causing resets */
صحيح malloc_corruption_error_ڳڻپ;

/* default corruption action */
ساكن فراغ reset_on_error(mstate m);

#وصف_ڪريو CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#وصف_ڪريو USAGE_ERROR_ACTION(m, p)

#وإلا /* PROCEED_ON_ERROR */

#جيڪڏهن_وصف_نه_ٿيل CORRUPTION_ERROR_ACTION
#وصف_ڪريو CORRUPTION_ERROR_ACTION(m) ABORT
#جيڪڏهن_پورو /* CORRUPTION_ERROR_ACTION */

#جيڪڏهن_وصف_نه_ٿيل USAGE_ERROR_ACTION
#وصف_ڪريو USAGE_ERROR_ACTION(m,p) ABORT
#جيڪڏهن_پورو /* USAGE_ERROR_ACTION */

#جيڪڏهن_پورو /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#إذا ! DEBUG

#وصف_ڪريو check_free_chunk(M,P)
#وصف_ڪريو check_inuse_chunk(M,P)
#وصف_ڪريو check_malloced_chunk(M,P,N)
#وصف_ڪريو check_mmapped_chunk(M,P)
#وصف_ڪريو check_malloc_state(M)
#وصف_ڪريو check_چوٽي_chunk(M,P)

#وإلا /* DEBUG */
#وصف_ڪريو check_free_chunk(M,P)       do_check_free_chunk(M,P)
#وصف_ڪريو check_inuse_chunk(M,P)      do_check_inuse_chunk(M,P)
#وصف_ڪريو check_چوٽي_chunk(M,P)        do_check_چوٽي_chunk(M,P)
#وصف_ڪريو check_malloced_chunk(M,P,N) do_check_malloced_chunk(M,P,N)
#وصف_ڪريو check_mmapped_chunk(M,P)    do_check_mmapped_chunk(M,P)
#وصف_ڪريو check_malloc_state(M)       do_check_malloc_state(M)

ساكن فراغ   do_check_any_chunk(mstate m, mchunkptr p);
ساكن فراغ   do_check_چوٽي_chunk(mstate m, mchunkptr p);
ساكن فراغ   do_check_mmapped_chunk(mstate m, mchunkptr p);
ساكن فراغ   do_check_inuse_chunk(mstate m, mchunkptr p);
ساكن فراغ   do_check_free_chunk(mstate m, mchunkptr p);
ساكن فراغ   do_check_malloced_chunk(mstate m, فراغ* mem, size_t s);
ساكن فراغ   do_check_وڻ(mstate m, tchunkptr t);
ساكن فراغ   do_check_treebin(mstate m, bindex_t i);
ساكن فراغ   do_check_smallbin(mstate m, bindex_t i);
ساكن فراغ   do_check_malloc_state(mstate m);
ساكن صحيح    bin_ڳوليو(mstate m, mchunkptr x);
ساكن size_t traverse_and_check(mstate m);
#جيڪڏهن_پورو /* DEBUG */

/* ---------------------------- Indexing Bins ---------------------------- */

#وصف_ڪريو is_small(s)         (((s) >> SMALLBIN_سرڪايو) < NSMALLBINS)
#وصف_ڪريو small_اشاريو(s)      (bindex_t)((s)  >> SMALLBIN_سرڪايو)
#وصف_ڪريو small_index2size(i) ((i)  << SMALLBIN_سرڪايو)
#وصف_ڪريو MIN_SMALL_اشاريو     (small_اشاريو(MIN_CHUNK_سائيز))

/* addressing by index. See above about smallbin repositioning */
#وصف_ڪريو smallbin_at(M, i)   ((sbinptr)((حرف*)&((M)->smallbins[(i)<<1])))
#وصف_ڪريو treebin_at(M,i)     (&((M)->treebins[i]))

/* assign tree index for size S to variable I.
   libffi: enable the __builtin_clz path on any GNU-compatible compiler, not
   only x86, so the log2 is an intrinsic rather than the generic C idiom that
   the compiler can't always recognize (libffi #754).  */
#إذا defined(__GNUC__)
#وصف_ڪريو compute_وڻ_اشاريو(S, I)\
{\
  بنانشان صحيح X = S >> TREEBIN_سرڪايو;\
  إذا (X == 0)\
    I = 0;\
  وإلا إذا (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وإلا {\
    بنانشان صحيح K = (بنانشان) حجم(X)*__CHAR_BIT__ - 1 - (بنانشان) __builtin_clz(X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_سرڪايو-1)) & 1)));\
  }\
}

#وإلاإذا defined (__INTEL_COMPILER)
#وصف_ڪريو compute_وڻ_اشاريو(S, I)\
{\
  size_t X = S >> TREEBIN_سرڪايو;\
  إذا (X == 0)\
    I = 0;\
  وإلا إذا (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وإلا {\
    بنانشان صحيح K = ٻٽو_انگ_جاچ_ابتو_3 (X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_سرڪايو-1)) & 1)));\
  }\
}

#وإلاإذا defined(_MSC_VER) && _MSC_VER>=1300
#وصف_ڪريو compute_وڻ_اشاريو(S, I)\
{\
  size_t X = S >> TREEBIN_سرڪايو;\
  إذا (X == 0)\
    I = 0;\
  وإلا إذا (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وإلا {\
    بنانشان صحيح K;\
    ٻٽو_انگ_جاچ_ابتو((DWORD *) &K, (DWORD) X);\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_سرڪايو-1)) & 1)));\
  }\
}

#وإلا /* GNUC */
#وصف_ڪريو compute_وڻ_اشاريو(S, I)\
{\
  size_t X = S >> TREEBIN_سرڪايو;\
  إذا (X == 0)\
    I = 0;\
  وإلا إذا (X > 0xFFFF)\
    I = NTREEBINS-1;\
  وإلا {\
    بنانشان صحيح Y = (بنانشان صحيح)X;\
    بنانشان صحيح N = ((Y - 0x100) >> 16) & 8;\
    بنانشان صحيح K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_سرڪايو-1)) & 1));\
  }\
}
#جيڪڏهن_پورو /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#وصف_ڪريو ٻٽو_انگ_for_وڻ_اشاريو(i) \
   (i == NTREEBINS-1)? (سائيز_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_سرڪايو - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#وصف_ڪريو leftshift_for_وڻ_اشاريو(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((سائيز_T_BITSIZE-سائيز_T_ONE) - (((i) >> 1) + TREEBIN_سرڪايو - 2)))

/* The size of the smallest chunk held in bin with index i */
#وصف_ڪريو minsize_for_وڻ_اشاريو(i) \
   ((سائيز_T_ONE << (((i) >> 1) + TREEBIN_سرڪايو)) |  \
   (((size_t)((i) & سائيز_T_ONE)) << (((i) >> 1) + TREEBIN_سرڪايو - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#وصف_ڪريو idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#وصف_ڪريو mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#وصف_ڪريو clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#وصف_ڪريو smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#وصف_ڪريو mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#وصف_ڪريو clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#وصف_ڪريو treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#وصف_ڪريو گهٽ_کان_گهٽ_ٻٽو_انگ(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#وصف_ڪريو کاٻو_بٽ(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#وصف_ڪريو same_or_کاٻو_بٽ(x) ((x) | -(x))

/* index corresponding to given bit.
   libffi: use __builtin_ctz on any GNU-compatible compiler, not only x86
   (libffi #754).  */

#إذا defined(__GNUC__)
#وصف_ڪريو compute_bit2idx(X, I)\
{\
  بنانشان صحيح J;\
  J = __builtin_ctz(X); \
  I = (bindex_t)J;\
}

#وإلاإذا defined (__INTEL_COMPILER)
#وصف_ڪريو compute_bit2idx(X, I)\
{\
  بنانشان صحيح J;\
  J = ٻٽو_انگ_جاچ_forward (X); \
  I = (bindex_t)J;\
}

#وإلاإذا defined(_MSC_VER) && _MSC_VER>=1300
#وصف_ڪريو compute_bit2idx(X, I)\
{\
  بنانشان صحيح J;\
  ٻٽو_انگ_جاچ_Forward_2((DWORD *) &J, X);\
  I = (bindex_t)J;\
}

#وإلاإذا USE_BUILTIN_FFS
#وصف_ڪريو compute_bit2idx(X, I) I = ffs(X)-1

#وإلا
#وصف_ڪريو compute_bit2idx(X, I)\
{\
  بنانشان صحيح Y = X - 1;\
  بنانشان صحيح K = Y >> (16-4) & 16;\
  بنانشان صحيح N = K;        Y >>= K;\
  N += K = Y >> (8-3) &  8;  Y >>= K;\
  N += K = Y >> (4-2) &  4;  Y >>= K;\
  N += K = Y >> (2-1) &  2;  Y >>= K;\
  N += K = Y >> (1-0) &  1;  Y >>= K;\
  I = (bindex_t)(N + Y);\
}
#جيڪڏهن_پورو /* GNUC */


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

#إذا !INSECURE
/* Check if address a is at least as high as any from MORECORE or MMAP */
#وصف_ڪريو ok_پتو(M, a) ((حرف*)(a) >= (M)->گهٽ_کان_گهٽ_addr)
/* Check if address of next chunk n is higher than base chunk p */
#وصف_ڪريو ok_ايندڙ(p, n)    ((حرف*)(p) < (حرف*)(n))
/* Check if p has inuse status */
#وصف_ڪريو ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#وصف_ڪريو ok_pinuse(p)     pinuse(p)

#وإلا /* !INSECURE */
#وصف_ڪريو ok_پتو(M, a) (1)
#وصف_ڪريو ok_ايندڙ(b, n)    (1)
#وصف_ڪريو ok_inuse(p)      (1)
#وصف_ڪريو ok_pinuse(p)     (1)
#جيڪڏهن_پورو /* !INSECURE */

#إذا (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#وصف_ڪريو ok_magic(M)      ((M)->magic == mparams.magic)
#وإلا  /* (FOOTERS && !INSECURE) */
#وصف_ڪريو ok_magic(M)      (1)
#جيڪڏهن_پورو /* (FOOTERS && !INSECURE) */

/* In gcc, use __builtin_expect to minimize impact of checks */
#إذا !INSECURE
#إذا defined(__GNUC__) && __GNUC__ >= 3
#وصف_ڪريو RTCHECK(e)  __builtin_expect(e, 1)
#وإلا /* GNUC */
#وصف_ڪريو RTCHECK(e)  (e)
#جيڪڏهن_پورو /* GNUC */
#وإلا /* !INSECURE */
#وصف_ڪريو RTCHECK(e)  (1)
#جيڪڏهن_پورو /* !INSECURE */

/* macros to set up inuse chunks with or without footers */

#إذا !FOOTERS

#وصف_ڪريو mark_inuse_foot(M,p,s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#وصف_ڪريو سيٽ_inuse(M,p,s)\
  ((p)->سر = (((p)->سر & PINUSE_ٻٽو_انگ)|s|CINUSE_ٻٽو_انگ),\
  ((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ٻٽو_انگ)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#وصف_ڪريو سيٽ_inuse_and_pinuse(M,p,s)\
  ((p)->سر = (s|PINUSE_ٻٽو_انگ|CINUSE_ٻٽو_انگ),\
  ((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ٻٽو_انگ)

/* Set size, cinuse and pinuse bit of this chunk */
#وصف_ڪريو سيٽ_سائيز_and_pinuse_جو_inuse_chunk(M, p, s)\
  ((p)->سر = (s|PINUSE_ٻٽو_انگ|CINUSE_ٻٽو_انگ))

#وإلا /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#وصف_ڪريو mark_inuse_foot(M,p,s)\
  (((mchunkptr)((حرف*)(p) + (s)))->prev_foot = ((size_t)(M) ^ mparams.magic))

#وصف_ڪريو get_mstate_for(p)\
  ((mstate)(((mchunkptr)((حرف*)(p) +\
    (chunksize(p))))->prev_foot ^ mparams.magic))

#وصف_ڪريو سيٽ_inuse(M,p,s)\
  ((p)->سر = (((p)->سر & PINUSE_ٻٽو_انگ)|s|CINUSE_ٻٽو_انگ),\
  (((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ٻٽو_انگ), \
  mark_inuse_foot(M,p,s))

#وصف_ڪريو سيٽ_inuse_and_pinuse(M,p,s)\
  ((p)->سر = (s|PINUSE_ٻٽو_انگ|CINUSE_ٻٽو_انگ),\
  (((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ٻٽو_انگ),\
 mark_inuse_foot(M,p,s))

#وصف_ڪريو سيٽ_سائيز_and_pinuse_جو_inuse_chunk(M, p, s)\
  ((p)->سر = (s|PINUSE_ٻٽو_انگ|CINUSE_ٻٽو_انگ),\
  mark_inuse_foot(M, p, s))

#جيڪڏهن_پورو /* !FOOTERS */

/* ---------------------------- setting mparams -------------------------- */

#إذا تالو_AT_FORK
ساكن فراغ pre_fork(فراغ)         { ACQUIRE_تالو(&(gm)->mutex); }
ساكن فراغ post_fork_والد(فراغ) { RELEASE_تالو(&(gm)->mutex); }
ساكن فراغ post_fork_ٻار(فراغ)  { INITIAL_تالو(&(gm)->mutex); }
#جيڪڏهن_پورو /* LOCK_AT_FORK */

/* Initialize mparams */
ساكن صحيح init_mparams(فراغ) {
#جيڪڏهن_وصف_ٿيل NEED_GLOBAL_تالو_INIT
  إذا (malloc_global_mutex_حالت <= 0)
    init_malloc_global_mutex();
#جيڪڏهن_پورو

  ACQUIRE_MALLOC_GLOBAL_تالو();
  إذا (mparams.magic == 0) {
    size_t magic;
    size_t psize;
    size_t gsize;

#إذا !defined(WIN32) && !defined(__OS2__)
    psize = malloc_getpagesize;
    gsize = ((DEFAULT_GRANULARITY != 0)? DEFAULT_GRANULARITY : psize);
#وإلاإذا defined (__OS2__)
    /* if low-memory is used, os2munmap() would break
       if it were anything other than 64k */
    psize = 4096u;
    gsize = 65536u;
#وإلا /* WIN32 */
    {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      psize = system_info.dw_صفحو_سائيز;
      gsize = ((DEFAULT_GRANULARITY != 0)?
               DEFAULT_GRANULARITY : system_info.dw_ورهاست_Granularity);
    }
#جيڪڏهن_پورو /* WIN32 */

    /* Sanity-check configuration:
       size_t must be unsigned and as wide as pointer type.
       ints must be at least 4 bytes.
       alignment must be at least 8.
       Alignment, min chunk size, and page size must all be powers of 2.
    */
    إذا ((حجم(size_t) != حجم(حرف*)) ||
        (MAX_سائيز_T < MIN_CHUNK_سائيز)  ||
        (حجم(صحيح) < 4)  ||
        (MALLOC_ALIGNMENT < (size_t)8U) ||
        ((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-سائيز_T_ONE)) != 0) ||
        ((MCHUNK_سائيز      & (MCHUNK_سائيز-سائيز_T_ONE))      != 0) ||
        ((gsize            & (gsize-سائيز_T_ONE))            != 0) ||
        ((psize            & (psize-سائيز_T_ONE))            != 0))
      ABORT;
    mparams.granularity = gsize;
    mparams.صفحو_سائيز = psize;
    mparams.mmap_حد = DEFAULT_MMAP_حد;
    mparams.trim_حد = DEFAULT_TRIM_حد;
#إذا MORECORE_CONTIGUOUS
    mparams.default_mflags = USE_تالو_ٻٽو_انگ|USE_MMAP_ٻٽو_انگ;
#وإلا  /* MORECORE_CONTIGUOUS */
    mparams.default_mflags = USE_تالو_ٻٽو_انگ|USE_MMAP_ٻٽو_انگ|USE_NONCONTIGUOUS_ٻٽو_انگ;
#جيڪڏهن_پورو /* MORECORE_CONTIGUOUS */

#إذا !ONLY_MSPACES
    /* Set up lock for main malloc area */
    gm->mflags = mparams.default_mflags;
    (فراغ)INITIAL_تالو(&gm->mutex);
#جيڪڏهن_پورو
#إذا تالو_AT_FORK
    pthread_atfork(&pre_fork, &post_fork_والد, &post_fork_ٻار);
#جيڪڏهن_پورو

    {
#إذا USE_DEV_RANDOM
      صحيح fd;
      بنانشان حرف buf[حجم(size_t)];
      /* Try to use /dev/urandom, else fall back on using time */
      إذا ((fd = open("/dev/urandom", O_RDONLY)) >= 0 &&
          اقرأ(fd, buf, حجم(buf)) == حجم(buf)) {
        magic = *((size_t *) buf);
        close(fd);
      }
      وإلا
#جيڪڏهن_پورو /* USE_DEV_RANDOM */
#جيڪڏهن_وصف_ٿيل WIN32
      magic = (size_t)(Get_Tick_ڳڻپ() ^ (size_t)0x55555555U);
#وإلاإذا defined(LACKS_وقت_H)
      magic = (size_t)&magic ^ (size_t)0x55555555U;
#وإلا
      magic = (size_t)(time(0) ^ (size_t)0x55555555U);
#جيڪڏهن_پورو
      magic |= (size_t)8U;    /* ensure nonzero */
      magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
      /* libffi: release-store so any thread that observes magic via the
         acquire load in ensure_initialization() sees the writes above (#873).  */
      ffi_mparams_magic_store(magic);
    }
  }

  RELEASE_MALLOC_GLOBAL_تالو();
  أرجع 1;
}

/* support for mallopt */
ساكن صحيح change_mparam(صحيح param_عدد, صحيح قدر) {
  size_t val;
  ensure_initialization();
  val = (قدر == -1)? MAX_سائيز_T : (size_t)قدر;
  بدل(param_عدد) {
  حالة M_TRIM_حد:
    mparams.trim_حد = val;
    أرجع 1;
  حالة M_GRANULARITY:
    إذا (val >= mparams.صفحو_سائيز && ((val & (val-1)) == 0)) {
      mparams.granularity = val;
      أرجع 1;
    }
    وإلا
      أرجع 0;
  حالة M_MMAP_حد:
    mparams.mmap_حد = val;
    أرجع 1;
  افتراضي:
    أرجع 0;
  }
}

#إذا DEBUG
/* ------------------------- Debugging Support --------------------------- */

/* Check properties of any chunk, whether free, inuse, mmapped etc  */
ساكن فراغ do_check_any_chunk(mstate m, mchunkptr p) {
  أكد((is_aligned(chunk2mem(p))) || (p->سر == FENCEPOST_سر));
  أكد(ok_پتو(m, p));
}

/* Check properties of top chunk */
ساكن فراغ do_check_چوٽي_chunk(mstate m, mchunkptr p) {
  msegmentptr sp = segment_holding(m, (حرف*)p);
  size_t  sz = p->سر & ~INUSE_بٽ; /* third-lowest bit can be set! */
  أكد(sp != 0);
  أكد((is_aligned(chunk2mem(p))) || (p->سر == FENCEPOST_سر));
  أكد(ok_پتو(m, p));
  أكد(sz == m->topsize);
  أكد(sz > 0);
  أكد(sz == ((sp->بنياد + sp->سائيز) - (حرف*)p) - چوٽي_FOOT_سائيز);
  أكد(pinuse(p));
  أكد(!pinuse(chunk_plus_وٿي(p, sz)));
}

/* Check properties of (inuse) mmapped chunks */
ساكن فراغ do_check_mmapped_chunk(mstate m, mchunkptr p) {
  size_t  sz = chunksize(p);
  size_t len = (sz + (p->prev_foot) + MMAP_FOOT_PAD);
  أكد(is_mmapped(p));
  أكد(use_mmap(m));
  أكد((is_aligned(chunk2mem(p))) || (p->سر == FENCEPOST_سر));
  أكد(ok_پتو(m, p));
  أكد(!is_small(sz));
  أكد((len & (mparams.صفحو_سائيز-سائيز_T_ONE)) == 0);
  أكد(chunk_plus_وٿي(p, sz)->سر == FENCEPOST_سر);
  أكد(chunk_plus_وٿي(p, sz+سائيز_T_سائيز)->سر == 0);
}

/* Check properties of inuse chunks */
ساكن فراغ do_check_inuse_chunk(mstate m, mchunkptr p) {
  do_check_any_chunk(m, p);
  أكد(is_inuse(p));
  أكد(ايندڙ_pinuse(p));
  /* If not pinuse and not mmapped, previous chunk has OK offset */
  أكد(is_mmapped(p) || pinuse(p) || ايندڙ_chunk(prev_chunk(p)) == p);
  إذا (is_mmapped(p))
    do_check_mmapped_chunk(m, p);
}

/* Check properties of free chunks */
ساكن فراغ do_check_free_chunk(mstate m, mchunkptr p) {
  size_t sz = chunksize(p);
  mchunkptr ايندڙ = chunk_plus_وٿي(p, sz);
  do_check_any_chunk(m, p);
  أكد(!is_inuse(p));
  أكد(!ايندڙ_pinuse(p));
  أكد (!is_mmapped(p));
  إذا (p != m->dv && p != m->چوٽي) {
    إذا (sz >= MIN_CHUNK_سائيز) {
      أكد((sz & CHUNK_ALIGN_نقاب) == 0);
      أكد(is_aligned(chunk2mem(p)));
      أكد(ايندڙ->prev_foot == sz);
      أكد(pinuse(p));
      أكد (ايندڙ == m->چوٽي || is_inuse(ايندڙ));
      أكد(p->fd->bk == p);
      أكد(p->bk->fd == p);
    }
    وإلا  /* markers are always of size SIZE_T_SIZE */
      أكد(sz == سائيز_T_سائيز);
  }
}

/* Check properties of malloced chunks at the point they are malloced */
ساكن فراغ do_check_malloced_chunk(mstate m, فراغ* mem, size_t s) {
  إذا (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    size_t sz = p->سر & ~INUSE_بٽ;
    do_check_inuse_chunk(m, p);
    أكد((sz & CHUNK_ALIGN_نقاب) == 0);
    أكد(sz >= MIN_CHUNK_سائيز);
    أكد(sz >= s);
    /* unless mmapped, size is less than MIN_CHUNK_SIZE more than request */
    أكد(is_mmapped(p) || sz < (s + MIN_CHUNK_سائيز));
  }
}

/* Check a tree and its subtrees.  */
ساكن فراغ do_check_وڻ(mstate m, tchunkptr t) {
  tchunkptr سر = 0;
  tchunkptr u = t;
  bindex_t tindex = t->اشاريو;
  size_t tsize = chunksize(t);
  bindex_t idx;
  compute_وڻ_اشاريو(tsize, idx);
  أكد(tindex == idx);
  أكد(tsize >= MIN_LARGE_سائيز);
  أكد(tsize >= minsize_for_وڻ_اشاريو(idx));
  أكد((idx == NTREEBINS-1) || (tsize < minsize_for_وڻ_اشاريو((idx+1))));

  افعل { /* traverse through chain of same-sized nodes */
    do_check_any_chunk(m, ((mchunkptr)u));
    أكد(u->اشاريو == tindex);
    أكد(chunksize(u) == tsize);
    أكد(!is_inuse(u));
    أكد(!ايندڙ_pinuse(u));
    أكد(u->fd->bk == u);
    أكد(u->bk->fd == u);
    إذا (u->والد == 0) {
      أكد(u->ٻار[0] == 0);
      أكد(u->ٻار[1] == 0);
    }
    وإلا {
      أكد(سر == 0); /* only one node on chain has parent */
      سر = u;
      أكد(u->والد != u);
      أكد (u->والد->ٻار[0] == u ||
              u->والد->ٻار[1] == u ||
              *((tbinptr*)(u->والد)) == u);
      إذا (u->ٻار[0] != 0) {
        أكد(u->ٻار[0]->والد == u);
        أكد(u->ٻار[0] != u);
        do_check_وڻ(m, u->ٻار[0]);
      }
      إذا (u->ٻار[1] != 0) {
        أكد(u->ٻار[1]->والد == u);
        أكد(u->ٻار[1] != u);
        do_check_وڻ(m, u->ٻار[1]);
      }
      إذا (u->ٻار[0] != 0 && u->ٻار[1] != 0) {
        أكد(chunksize(u->ٻار[0]) < chunksize(u->ٻار[1]));
      }
    }
    u = u->fd;
  } طالما (u != t);
  أكد(سر != 0);
}

/*  Check all the chunks in a treebin.  */
ساكن فراغ do_check_treebin(mstate m, bindex_t i) {
  tbinptr* tb = treebin_at(m, i);
  tchunkptr t = *tb;
  صحيح خالي_جايون = (m->treemap & (1U << i)) == 0;
  إذا (t == 0)
    أكد(خالي_جايون);
  إذا (!خالي_جايون)
    do_check_وڻ(m, t);
}

/*  Check all the chunks in a smallbin.  */
ساكن فراغ do_check_smallbin(mstate m, bindex_t i) {
  sbinptr b = smallbin_at(m, i);
  mchunkptr p = b->bk;
  بنانشان صحيح خالي_جايون = (m->smallmap & (1U << i)) == 0;
  إذا (p == b)
    أكد(خالي_جايون);
  إذا (!خالي_جايون) {
    لكل (; p != b; p = p->bk) {
      size_t سائيز = chunksize(p);
      mchunkptr q;
      /* each chunk claims to be free */
      do_check_free_chunk(m, p);
      /* chunk belongs in bin */
      أكد(small_اشاريو(سائيز) == i);
      أكد(p->bk == b || chunksize(p->bk) == chunksize(p));
      /* chunk is followed by an inuse chunk */
      q = ايندڙ_chunk(p);
      إذا (q->سر != FENCEPOST_سر)
        do_check_inuse_chunk(m, q);
    }
  }
}

/* Find x in a bin. Used in other check functions. */
ساكن صحيح bin_ڳوليو(mstate m, mchunkptr x) {
  size_t سائيز = chunksize(x);
  إذا (is_small(سائيز)) {
    bindex_t sidx = small_اشاريو(سائيز);
    sbinptr b = smallbin_at(m, sidx);
    إذا (smallmap_is_marked(m, sidx)) {
      mchunkptr p = b;
      افعل {
        إذا (p == x)
          أرجع 1;
      } طالما ((p = p->fd) != b);
    }
  }
  وإلا {
    bindex_t tidx;
    compute_وڻ_اشاريو(سائيز, tidx);
    إذا (treemap_is_marked(m, tidx)) {
      tchunkptr t = *treebin_at(m, tidx);
      size_t sizebits = سائيز << leftshift_for_وڻ_اشاريو(tidx);
      طالما (t != 0 && chunksize(t) != سائيز) {
        t = t->ٻار[(sizebits >> (سائيز_T_BITSIZE-سائيز_T_ONE)) & 1];
        sizebits <<= 1;
      }
      إذا (t != 0) {
        tchunkptr u = t;
        افعل {
          إذا (u == (tchunkptr)x)
            أرجع 1;
        } طالما ((u = u->fd) != t);
      }
    }
  }
  أرجع 0;
}

/* Traverse each chunk and check it; return total */
ساكن size_t traverse_and_check(mstate m) {
  size_t جوڙ = 0;
  إذا (is_initialized(m)) {
    msegmentptr s = &m->seg;
    جوڙ += m->topsize + چوٽي_FOOT_سائيز;
    طالما (s != 0) {
      mchunkptr q = align_as_chunk(s->بنياد);
      mchunkptr lastq = 0;
      أكد(pinuse(q));
      طالما (segment_holds(s, q) &&
             q != m->چوٽي && q->سر != FENCEPOST_سر) {
        جوڙ += chunksize(q);
        إذا (is_inuse(q)) {
          أكد(!bin_ڳوليو(m, q));
          do_check_inuse_chunk(m, q);
        }
        وإلا {
          أكد(q == m->dv || bin_ڳوليو(m, q));
          أكد(lastq == 0 || is_inuse(lastq)); /* Not 2 consecutive free */
          do_check_free_chunk(m, q);
        }
        lastq = q;
        q = ايندڙ_chunk(q);
      }
      s = s->ايندڙ;
    }
  }
  أرجع جوڙ;
}


/* Check all properties of malloc_state. */
ساكن فراغ do_check_malloc_state(mstate m) {
  bindex_t i;
  size_t ڪل;
  /* check bins */
  لكل (i = 0; i < NSMALLBINS; ++i)
    do_check_smallbin(m, i);
  لكل (i = 0; i < NTREEBINS; ++i)
    do_check_treebin(m, i);

  إذا (m->dvsize != 0) { /* check dv chunk */
    do_check_any_chunk(m, m->dv);
    أكد(m->dvsize == chunksize(m->dv));
    أكد(m->dvsize >= MIN_CHUNK_سائيز);
    أكد(bin_ڳوليو(m, m->dv) == 0);
  }

  إذا (m->چوٽي != 0) {   /* check top chunk */
    do_check_چوٽي_chunk(m, m->چوٽي);
    /*assert(m->topsize == chunksize(m->top)); redundant */
    أكد(m->topsize > 0);
    أكد(bin_ڳوليو(m, m->چوٽي) == 0);
  }

  ڪل = traverse_and_check(m);
  أكد(ڪل <= m->footprint);
  أكد(m->footprint <= m->max_footprint);
}
#جيڪڏهن_پورو /* DEBUG */

/* ----------------------------- statistics ------------------------------ */

#إذا !NO_MALLINFO
ساكن بنية mallinfo اندروني_mallinfo(mstate m) {
  بنية mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ensure_initialization();
  إذا (!PREACTION(m)) {
    check_malloc_state(m);
    إذا (is_initialized(m)) {
      size_t nfree = سائيز_T_ONE; /* top always free */
      size_t mfree = m->topsize + چوٽي_FOOT_سائيز;
      size_t جوڙ = mfree;
      msegmentptr s = &m->seg;
      طالما (s != 0) {
        mchunkptr q = align_as_chunk(s->بنياد);
        طالما (segment_holds(s, q) &&
               q != m->چوٽي && q->سر != FENCEPOST_سر) {
          size_t sz = chunksize(q);
          جوڙ += sz;
          إذا (!is_inuse(q)) {
            mfree += sz;
            ++nfree;
          }
          q = ايندڙ_chunk(q);
        }
        s = s->ايندڙ;
      }

      nm.arena    = جوڙ;
      nm.ordblks  = nfree;
      nm.hblkhd   = m->footprint - جوڙ;
      nm.usmblks  = m->max_footprint;
      nm.uordblks = m->footprint - mfree;
      nm.fordblks = mfree;
      nm.keepcost = m->topsize;
    }

    POSTACTION(m);
  }
  أرجع nm;
}
#جيڪڏهن_پورو /* !NO_MALLINFO */

#إذا !NO_MALLOC_STATS
ساكن فراغ اندروني_malloc_stats(mstate m) {
  ensure_initialization();
  إذا (!PREACTION(m)) {
    size_t maxfp = 0;
    size_t fp = 0;
    size_t استعمال_ٿيل = 0;
    check_malloc_state(m);
    إذا (is_initialized(m)) {
      msegmentptr s = &m->seg;
      maxfp = m->max_footprint;
      fp = m->footprint;
      استعمال_ٿيل = fp - (m->topsize + چوٽي_FOOT_سائيز);

      طالما (s != 0) {
        mchunkptr q = align_as_chunk(s->بنياد);
        طالما (segment_holds(s, q) &&
               q != m->چوٽي && q->سر != FENCEPOST_سر) {
          إذا (!is_inuse(q))
            استعمال_ٿيل -= chunksize(q);
          q = ايندڙ_chunk(q);
        }
        s = s->ايندڙ;
      }
    }
    POSTACTION(m); /* drop lock */
    fprintf(stderr, "max system bytes = %10lu\n", (بنانشان ڊگهو)(maxfp));
    fprintf(stderr, "system bytes     = %10lu\n", (بنانشان ڊگهو)(fp));
    fprintf(stderr, "in use bytes     = %10lu\n", (بنانشان ڊگهو)(استعمال_ٿيل));
  }
}
#جيڪڏهن_پورو /* NO_MALLOC_STATS */

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#وصف_ڪريو وجهو_small_chunk(M, P, S) {\
  bindex_t I  = small_اشاريو(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  أكد(S >= MIN_CHUNK_سائيز);\
  إذا (!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  وإلا إذا (RTCHECK(ok_پتو(M, B->fd)))\
    F = B->fd;\
  وإلا {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#وصف_ڪريو unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  bindex_t I = small_اشاريو(S);\
  أكد(P != B);\
  أكد(P != F);\
  أكد(chunksize(P) == small_index2size(I));\
  إذا (RTCHECK(F == smallbin_at(M,I) || (ok_پتو(M, F) && F->bk == P))) { \
    إذا (B == F) {\
      clear_smallmap(M, I);\
    }\
    وإلا إذا (RTCHECK(B == smallbin_at(M,I) ||\
                     (ok_پتو(M, B) && B->fd == P))) {\
      F->bk = B;\
      B->fd = F;\
    }\
    وإلا {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  وإلا {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Unlink the first chunk from a smallbin */
#وصف_ڪريو unlink_پهريون_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  أكد(P != B);\
  أكد(P != F);\
  أكد(chunksize(P) == small_index2size(I));\
  إذا (B == F) {\
    clear_smallmap(M, I);\
  }\
  وإلا إذا (RTCHECK(ok_پتو(M, F) && F->bk == P)) {\
    F->bk = B;\
    B->fd = F;\
  }\
  وإلا {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#وصف_ڪريو replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  أكد(is_small(DVS));\
  إذا (DVS != 0) {\
    mchunkptr DV = M->dv;\
    وجهو_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#وصف_ڪريو وجهو_large_chunk(M, X, S) {\
  tbinptr* H;\
  bindex_t I;\
  compute_وڻ_اشاريو(S, I);\
  H = treebin_at(M, I);\
  X->اشاريو = I;\
  X->ٻار[0] = X->ٻار[1] = 0;\
  إذا (!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->والد = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  وإلا {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_وڻ_اشاريو(I);\
    لكل (;;) {\
      إذا (chunksize(T) != S) {\
        tchunkptr* C = &(T->ٻار[(K >> (سائيز_T_BITSIZE-سائيز_T_ONE)) & 1]);\
        K <<= 1;\
        إذا (*C != 0)\
          T = *C;\
        وإلا إذا (RTCHECK(ok_پتو(M, C))) {\
          *C = X;\
          X->والد = T;\
          X->fd = X->bk = X;\
          اكسر;\
        }\
        وإلا {\
          CORRUPTION_ERROR_ACTION(M);\
          اكسر;\
        }\
      }\
      وإلا {\
        tchunkptr F = T->fd;\
        إذا (RTCHECK(ok_پتو(M, T) && ok_پتو(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->والد = 0;\
          اكسر;\
        }\
        وإلا {\
          CORRUPTION_ERROR_ACTION(M);\
          اكسر;\
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

#وصف_ڪريو unlink_large_chunk(M, X) {\
  tchunkptr XP = X->والد;\
  tchunkptr R;\
  إذا (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    إذا (RTCHECK(ok_پتو(M, F) && F->bk == X && R->fd == X)) {\
      F->bk = R;\
      R->fd = F;\
    }\
    وإلا {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  وإلا {\
    tchunkptr* RP;\
    إذا (((R = *(RP = &(X->ٻار[1]))) != 0) ||\
        ((R = *(RP = &(X->ٻار[0]))) != 0)) {\
      tchunkptr* CP;\
      طالما ((*(CP = &(R->ٻار[1])) != 0) ||\
             (*(CP = &(R->ٻار[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      إذا (RTCHECK(ok_پتو(M, RP)))\
        *RP = 0;\
      وإلا {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  إذا (XP != 0) {\
    tbinptr* H = treebin_at(M, X->اشاريو);\
    إذا (X == *H) {\
      إذا ((*H = R) == 0) \
        clear_treemap(M, X->اشاريو);\
    }\
    وإلا إذا (RTCHECK(ok_پتو(M, XP))) {\
      إذا (XP->ٻار[0] == X) \
        XP->ٻار[0] = R;\
      وإلا \
        XP->ٻار[1] = R;\
    }\
    وإلا\
      CORRUPTION_ERROR_ACTION(M);\
    إذا (R != 0) {\
      إذا (RTCHECK(ok_پتو(M, R))) {\
        tchunkptr C0, C1;\
        R->والد = XP;\
        إذا ((C0 = X->ٻار[0]) != 0) {\
          إذا (RTCHECK(ok_پتو(M, C0))) {\
            R->ٻار[0] = C0;\
            C0->والد = R;\
          }\
          وإلا\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        إذا ((C1 = X->ٻار[1]) != 0) {\
          إذا (RTCHECK(ok_پتو(M, C1))) {\
            R->ٻار[1] = C1;\
            C1->والد = R;\
          }\
          وإلا\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      وإلا\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}

/* Relays to large vs small bin operations */

#وصف_ڪريو وجهو_chunk(M, P, S)\
  إذا (is_small(S)) وجهو_small_chunk(M, P, S)\
  وإلا { tchunkptr TP = (tchunkptr)(P); وجهو_large_chunk(M, TP, S); }

#وصف_ڪريو unlink_chunk(M, P, S)\
  إذا (is_small(S)) unlink_small_chunk(M, P, S)\
  وإلا { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#إذا ONLY_MSPACES
#وصف_ڪريو اندروني_malloc(m, b) mspace_malloc(m, b)
#وصف_ڪريو اندروني_free(m, mem) mspace_free(m,mem);
#وإلا /* ONLY_MSPACES */
#إذا MSPACES
#وصف_ڪريو اندروني_malloc(m, b)\
  ((m == gm)? dlmalloc(b) : mspace_malloc(m, b))
#وصف_ڪريو اندروني_free(m, mem)\
   إذا (m == gm) dlfree(mem); وإلا mspace_free(m,mem);
#وإلا /* MSPACES */
#وصف_ڪريو اندروني_malloc(m, b) dlmalloc(b)
#وصف_ڪريو اندروني_free(m, mem) dlfree(mem)
#جيڪڏهن_پورو /* MSPACES */
#جيڪڏهن_پورو /* ONLY_MSPACES */

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
ساكن فراغ* mmap_alloc(mstate m, size_t nb) {
  size_t mmsize = mmap_align(nb + SIX_سائيز_T_SIZES + CHUNK_ALIGN_نقاب);
  إذا (m->footprint_حد_بندي != 0) {
    size_t fp = m->footprint + mmsize;
    إذا (fp <= m->footprint || fp > m->footprint_حد_بندي)
      أرجع 0;
  }
  إذا (mmsize > nb) {     /* Check for wrap around 0 */
    حرف* mm = (حرف*)(CALL_سڌو_MMAP(mmsize));
    إذا (mm != CMFAIL) {
      size_t offset = align_وٿي(chunk2mem(mm));
      size_t psize = mmsize - offset - MMAP_FOOT_PAD;
      mchunkptr p = (mchunkptr)(mm + offset);
      p->prev_foot = offset;
      p->سر = psize;
      mark_inuse_foot(m, p, psize);
      chunk_plus_وٿي(p, psize)->سر = FENCEPOST_سر;
      chunk_plus_وٿي(p, psize+سائيز_T_سائيز)->سر = 0;

      إذا (m->گهٽ_کان_گهٽ_addr == 0 || mm < m->گهٽ_کان_گهٽ_addr)
        m->گهٽ_کان_گهٽ_addr = mm;
      إذا ((m->footprint += mmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      أكد(is_aligned(chunk2mem(p)));
      check_mmapped_chunk(m, p);
      أرجع chunk2mem(p);
    }
  }
  أرجع 0;
}

/* Realloc using mmap */
ساكن mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb, صحيح flags) {
  size_t oldsize = chunksize(oldp);
  (فراغ)flags; /* placate people compiling -Wunused */
  إذا (is_small(nb)) /* Can't shrink mmap regions below small size */
    أرجع 0;
  /* Keep old chunk if big enough but not too big */
  إذا (oldsize >= nb + سائيز_T_سائيز &&
      (oldsize - nb) <= (mparams.granularity << 1))
    أرجع oldp;
  وإلا {
    size_t offset = oldp->prev_foot;
    size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
    size_t newmmsize = mmap_align(nb + SIX_سائيز_T_SIZES + CHUNK_ALIGN_نقاب);
    حرف* cp = (حرف*)CALL_MREMAP((حرف*)oldp - offset,
                                  oldmmsize, newmmsize, flags);
    إذا (cp != CMFAIL) {
      mchunkptr newp = (mchunkptr)(cp + offset);
      size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
      newp->سر = psize;
      mark_inuse_foot(m, newp, psize);
      chunk_plus_وٿي(newp, psize)->سر = FENCEPOST_سر;
      chunk_plus_وٿي(newp, psize+سائيز_T_سائيز)->سر = 0;

      إذا (cp < m->گهٽ_کان_گهٽ_addr)
        m->گهٽ_کان_گهٽ_addr = cp;
      إذا ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      check_mmapped_chunk(m, newp);
      أرجع newp;
    }
  }
  أرجع 0;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
ساكن فراغ init_چوٽي(mstate m, mchunkptr p, size_t psize) {
  /* Ensure alignment */
  size_t offset = align_وٿي(chunk2mem(p));
  p = (mchunkptr)((حرف*)p + offset);
  psize -= offset;

  m->چوٽي = p;
  m->topsize = psize;
  p->سر = psize | PINUSE_ٻٽو_انگ;
  /* set size of fake trailing chunk holding overhead space only once */
  chunk_plus_وٿي(p, psize)->سر = چوٽي_FOOT_سائيز;
  m->trim_check = mparams.trim_حد; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
ساكن فراغ init_bins(mstate m) {
  /* Establish circular links for smallbins */
  bindex_t i;
  لكل (i = 0; i < NSMALLBINS; ++i) {
    sbinptr bin = smallbin_at(m,i);
    bin->fd = bin->bk = bin;
  }
}

#إذا PROCEED_ON_ERROR

/* default corruption action */
ساكن فراغ reset_on_error(mstate m) {
  صحيح i;
  ++malloc_corruption_error_ڳڻپ;
  /* Reinitialize fields to forget about all memory */
  m->smallmap = m->treemap = 0;
  m->dvsize = m->topsize = 0;
  m->seg.بنياد = 0;
  m->seg.سائيز = 0;
  m->seg.ايندڙ = 0;
  m->چوٽي = m->dv = 0;
  لكل (i = 0; i < NTREEBINS; ++i)
    *treebin_at(m, i) = 0;
  init_bins(m);
}
#جيڪڏهن_پورو /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
ساكن فراغ* prepend_alloc(mstate m, حرف* newbase, حرف* oldbase,
                           size_t nb) {
  mchunkptr p = align_as_chunk(newbase);
  mchunkptr oldfirst = align_as_chunk(oldbase);
  size_t psize = (حرف*)oldfirst - (حرف*)p;
  mchunkptr q = chunk_plus_وٿي(p, nb);
  size_t qsize = psize - nb;
  سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, p, nb);

  أكد((حرف*)oldfirst > (حرف*)q);
  أكد(pinuse(oldfirst));
  أكد(qsize >= MIN_CHUNK_سائيز);

  /* consolidate remainder with first chunk of old base */
  إذا (oldfirst == m->چوٽي) {
    size_t tsize = m->topsize += qsize;
    m->چوٽي = q;
    q->سر = tsize | PINUSE_ٻٽو_انگ;
    check_چوٽي_chunk(m, q);
  }
  وإلا إذا (oldfirst == m->dv) {
    size_t dsize = m->dvsize += qsize;
    m->dv = q;
    سيٽ_سائيز_and_pinuse_جو_free_chunk(q, dsize);
  }
  وإلا {
    إذا (!is_inuse(oldfirst)) {
      size_t nsize = chunksize(oldfirst);
      unlink_chunk(m, oldfirst, nsize);
      oldfirst = chunk_plus_وٿي(oldfirst, nsize);
      qsize += nsize;
    }
    سيٽ_free_with_pinuse(q, qsize, oldfirst);
    وجهو_chunk(m, q, qsize);
    check_free_chunk(m, q);
  }

  check_malloced_chunk(m, chunk2mem(p), nb);
  أرجع chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
ساكن فراغ جوڙيو_segment(mstate m, حرف* tbase, size_t tsize, flag_t mmapped) {
  /* Determine locations and sizes of segment, fenceposts, old top */
  حرف* old_چوٽي = (حرف*)m->چوٽي;
  msegmentptr oldsp = segment_holding(m, old_چوٽي);
  حرف* old_پڇاڙي = oldsp->بنياد + oldsp->سائيز;
  size_t ssize = pad_request(حجم(بنية malloc_segment));
  حرف* rawsp = old_پڇاڙي - (ssize + FOUR_سائيز_T_SIZES + CHUNK_ALIGN_نقاب);
  size_t offset = align_وٿي(chunk2mem(rawsp));
  حرف* asp = rawsp + offset;
  حرف* csp = (asp < (old_چوٽي + MIN_CHUNK_سائيز))? old_چوٽي : asp;
  mchunkptr sp = (mchunkptr)csp;
  msegmentptr ss = (msegmentptr)(chunk2mem(sp));
  mchunkptr tnext = chunk_plus_وٿي(sp, ssize);
  mchunkptr p = tnext;
  صحيح nfences = 0;
  (فراغ)nfences; /* Suppress unused variable warning */

  /* reset top to new space */
  init_چوٽي(m, (mchunkptr)tbase, tsize - چوٽي_FOOT_سائيز);

  /* Set up segment record */
  أكد(is_aligned(ss));
  سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, sp, ssize);
  *ss = m->seg; /* Push current record */
  m->seg.بنياد = tbase;
  m->seg.سائيز = tsize;
  (فراغ)سيٽ_segment_flags(&m->seg, mmapped);
  m->seg.ايندڙ = ss;

  /* Insert trailing fenceposts */
  لكل (;;) {
    mchunkptr nextp = chunk_plus_وٿي(p, سائيز_T_سائيز);
    p->سر = FENCEPOST_سر;
    ++nfences;
    إذا ((حرف*)(&(nextp->سر)) < old_پڇاڙي)
      p = nextp;
    وإلا
      اكسر;
  }
  أكد(nfences >= 2);

  /* Insert the rest of old top into a bin as an ordinary free chunk */
  إذا (csp != old_چوٽي) {
    mchunkptr q = (mchunkptr)old_چوٽي;
    size_t psize = csp - old_چوٽي;
    mchunkptr tn = chunk_plus_وٿي(q, psize);
    سيٽ_free_with_pinuse(q, psize, tn);
    وجهو_chunk(m, q, psize);
  }

  check_چوٽي_chunk(m, m->چوٽي);
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
ساكن فراغ* sys_alloc(mstate m, size_t nb) {
  حرف* tbase = CMFAIL;
  size_t tsize = 0;
  flag_t mmap_flag = 0;
  size_t asize; /* allocation size */

  ensure_initialization();

  /* Directly map large chunks, but only if already initialized */
  إذا (use_mmap(m) && nb >= mparams.mmap_حد && m->topsize != 0) {
    فراغ* mem = mmap_alloc(m, nb);
    إذا (mem != 0)
      أرجع mem;
  }

  asize = granularity_align(nb + SYS_ALLOC_PADDING);
  إذا (asize <= nb)
    أرجع 0; /* wraparound */
  إذا (m->footprint_حد_بندي != 0) {
    size_t fp = m->footprint + asize;
    إذا (fp <= m->footprint || fp > m->footprint_حد_بندي)
      أرجع 0;
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

  إذا (MORECORE_CONTIGUOUS && !use_noncontiguous(m)) {
    حرف* br = CMFAIL;
    size_t ssize = asize; /* sbrk call size */
    msegmentptr ss = (m->چوٽي == 0)? 0 : segment_holding(m, (حرف*)m->چوٽي);
    ACQUIRE_MALLOC_GLOBAL_تالو();

    إذا (ss == 0) {  /* First time through or recovery */
      حرف* بنياد = (حرف*)CALL_MORECORE(0);
      إذا (بنياد != CMFAIL) {
        size_t fp;
        /* Adjust to end on a page boundary */
        إذا (!is_صفحو_aligned(بنياد))
          ssize += (صفحو_align((size_t)بنياد) - (size_t)بنياد);
        fp = m->footprint + ssize; /* recheck limits */
        إذا (ssize > nb && ssize < HALF_MAX_سائيز_T &&
            (m->footprint_حد_بندي == 0 ||
             (fp > m->footprint && fp <= m->footprint_حد_بندي)) &&
            (br = (حرف*)(CALL_MORECORE(ssize))) == بنياد) {
          tbase = بنياد;
          tsize = ssize;
        }
      }
    }
    وإلا {
      /* Subtract out existing available top space from MORECORE request. */
      ssize = granularity_align(nb - m->topsize + SYS_ALLOC_PADDING);
      /* Use mem here only if it did continuously extend old space */
      إذا (ssize < HALF_MAX_سائيز_T &&
          (br = (حرف*)(CALL_MORECORE(ssize))) == ss->بنياد+ss->سائيز) {
        tbase = br;
        tsize = ssize;
      }
    }

    إذا (tbase == CMFAIL) {    /* Cope with partial failure */
      إذا (br != CMFAIL) {    /* Try to use/extend the space we did get */
        إذا (ssize < HALF_MAX_سائيز_T &&
            ssize < nb + SYS_ALLOC_PADDING) {
          size_t esize = granularity_align(nb + SYS_ALLOC_PADDING - ssize);
          إذا (esize < HALF_MAX_سائيز_T) {
            حرف* پڇاڙي = (حرف*)CALL_MORECORE(esize);
            إذا (پڇاڙي != CMFAIL)
              ssize += esize;
            وإلا {            /* Can't use; try to release */
              (فراغ) CALL_MORECORE(-ssize);
              br = CMFAIL;
            }
          }
        }
      }
      إذا (br != CMFAIL) {    /* Use the space we did get */
        tbase = br;
        tsize = ssize;
      }
      وإلا
        disable_contiguous(m); /* Don't try contiguous path in the future */
    }

    RELEASE_MALLOC_GLOBAL_تالو();
  }

  إذا (HAVE_MMAP && tbase == CMFAIL) {  /* Try MMAP */
    حرف* mp = (حرف*)(CALL_MMAP(asize));
    إذا (mp != CMFAIL) {
      tbase = mp;
      tsize = asize;
      mmap_flag = USE_MMAP_ٻٽو_انگ;
    }
  }

  إذا (HAVE_MORECORE && tbase == CMFAIL) { /* Try noncontiguous MORECORE */
    إذا (asize < HALF_MAX_سائيز_T) {
      حرف* br = CMFAIL;
      حرف* پڇاڙي = CMFAIL;
      ACQUIRE_MALLOC_GLOBAL_تالو();
      br = (حرف*)(CALL_MORECORE(asize));
      پڇاڙي = (حرف*)(CALL_MORECORE(0));
      RELEASE_MALLOC_GLOBAL_تالو();
      إذا (br != CMFAIL && پڇاڙي != CMFAIL && br < پڇاڙي) {
        size_t ssize = پڇاڙي - br;
        إذا (ssize > nb + چوٽي_FOOT_سائيز) {
          tbase = br;
          tsize = ssize;
        }
      }
    }
  }

  إذا (tbase != CMFAIL) {

    إذا ((m->footprint += tsize) > m->max_footprint)
      m->max_footprint = m->footprint;

    إذا (!is_initialized(m)) { /* first-time initialization */
      إذا (m->گهٽ_کان_گهٽ_addr == 0 || tbase < m->گهٽ_کان_گهٽ_addr)
        m->گهٽ_کان_گهٽ_addr = tbase;
      m->seg.بنياد = tbase;
      m->seg.سائيز = tsize;
      (فراغ)سيٽ_segment_flags(&m->seg, mmap_flag);
      m->magic = mparams.magic;
      m->release_checks = MAX_RELEASE_CHECK_شرح;
      init_bins(m);
#إذا !ONLY_MSPACES
      إذا (is_global(m))
        init_چوٽي(m, (mchunkptr)tbase, tsize - چوٽي_FOOT_سائيز);
      وإلا
#جيڪڏهن_پورو
      {
        /* Offset top by embedded malloc_state */
        mchunkptr mn = ايندڙ_chunk(mem2chunk(m));
        init_چوٽي(m, mn, (size_t)((tbase + tsize) - (حرف*)mn) -چوٽي_FOOT_سائيز);
      }
    }

    وإلا {
      /* Try to merge with an existing segment */
      msegmentptr sp = &m->seg;
      /* Only consider most recent segment if traversal suppressed */
      طالما (sp != 0 && tbase != sp->بنياد + sp->سائيز)
        sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->ايندڙ;
      إذا (sp != 0 &&
          !is_extern_segment(sp) &&
          check_segment_ملايو(sp, tbase, tsize) &&
          (get_segment_flags(sp) & USE_MMAP_ٻٽو_انگ) == mmap_flag &&
          segment_holds(sp, m->چوٽي)) { /* append */
        sp->سائيز += tsize;
        init_چوٽي(m, m->چوٽي, m->topsize + tsize);
      }
      وإلا {
        إذا (tbase < m->گهٽ_کان_گهٽ_addr)
          m->گهٽ_کان_گهٽ_addr = tbase;
        sp = &m->seg;
        طالما (sp != 0 && sp->بنياد != tbase + tsize)
          sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->ايندڙ;
        إذا (sp != 0 &&
            !is_extern_segment(sp) &&
            check_segment_ملايو(sp, tbase, tsize) &&
            (get_segment_flags(sp) & USE_MMAP_ٻٽو_انگ) == mmap_flag) {
          حرف* oldbase = sp->بنياد;
          sp->بنياد = tbase;
          sp->سائيز += tsize;
          أرجع prepend_alloc(m, tbase, oldbase, nb);
        }
        وإلا
          جوڙيو_segment(m, tbase, tsize, mmap_flag);
      }
    }

    إذا (nb < m->topsize) { /* Allocate from new or extended top space */
      size_t rsize = m->topsize -= nb;
      mchunkptr p = m->چوٽي;
      mchunkptr r = m->چوٽي = chunk_plus_وٿي(p, nb);
      r->سر = rsize | PINUSE_ٻٽو_انگ;
      سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, p, nb);
      check_چوٽي_chunk(m, m->چوٽي);
      check_malloced_chunk(m, chunk2mem(p), nb);
      أرجع chunk2mem(p);
    }
  }

  MALLOC_FAILURE_ACTION;
  أرجع 0;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
ساكن size_t release_اڻ_استعمال_ٿيل_segments(mstate m) {
  size_t released = 0;
  صحيح nsegs = 0;
  msegmentptr pred = &m->seg;
  msegmentptr sp = pred->ايندڙ;
  طالما (sp != 0) {
    حرف* بنياد = sp->بنياد;
    size_t سائيز = sp->سائيز;
    msegmentptr ايندڙ = sp->ايندڙ;
    ++nsegs;
    إذا (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
      mchunkptr p = align_as_chunk(بنياد);
      size_t psize = chunksize(p);
      /* Can unmap if first chunk holds entire segment and not pinned */
      إذا (!is_inuse(p) && (حرف*)p + psize >= بنياد + سائيز - چوٽي_FOOT_سائيز) {
        tchunkptr tp = (tchunkptr)p;
        أكد(segment_holds(sp, (حرف*)sp));
        إذا (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        وإلا {
          unlink_large_chunk(m, tp);
        }
        إذا (CALL_MUNMAP(بنياد, سائيز) == 0) {
          released += سائيز;
          m->footprint -= سائيز;
          /* unlink obsoleted record */
          sp = pred;
          sp->ايندڙ = ايندڙ;
        }
        وإلا { /* back out if cannot unmap */
          وجهو_large_chunk(m, tp, psize);
        }
      }
    }
    إذا (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
      اكسر;
    pred = sp;
    sp = ايندڙ;
  }
  /* Reset check counter */
  m->release_checks = (((size_t) nsegs > (size_t) MAX_RELEASE_CHECK_شرح)?
                       (size_t) nsegs : (size_t) MAX_RELEASE_CHECK_شرح);
  أرجع released;
}

ساكن صحيح sys_trim(mstate m, size_t pad) {
  size_t released = 0;
  ensure_initialization();
  إذا (pad < MAX_REQUEST && is_initialized(m)) {
    pad += چوٽي_FOOT_سائيز; /* ensure enough room for segment overhead */

    إذا (m->topsize > pad) {
      /* Shrink top space in granularity-size units, keeping at least one */
      size_t unit = mparams.granularity;
      size_t extra = ((m->topsize - pad + (unit - سائيز_T_ONE)) / unit -
                      سائيز_T_ONE) * unit;
      msegmentptr sp = segment_holding(m, (حرف*)m->چوٽي);

      إذا (sp == 0)
        CORRUPTION_ERROR_ACTION(m);
      إذا (!is_extern_segment(sp)) {
        إذا (is_mmapped_segment(sp)) {
          إذا (HAVE_MMAP &&
              sp->سائيز >= extra &&
              !has_segment_link(m, sp)) { /* can't shrink if pinned */
            size_t newsize = sp->سائيز - extra;
            (فراغ)newsize; /* placate people compiling -Wunused-variable */
            /* Prefer mremap, fall back to munmap */
            إذا ((CALL_MREMAP(sp->بنياد, sp->سائيز, newsize, 0) != MFAIL) ||
                (CALL_MUNMAP(sp->بنياد + newsize, extra) == 0)) {
              released = extra;
            }
          }
        }
        وإلا إذا (HAVE_MORECORE) {
          إذا (extra >= HALF_MAX_سائيز_T) /* Avoid wrapping negative */
            extra = (HALF_MAX_سائيز_T) + سائيز_T_ONE - unit;
          ACQUIRE_MALLOC_GLOBAL_تالو();
          {
            /* Make sure end of memory is where we last set it. */
            حرف* old_br = (حرف*)(CALL_MORECORE(0));
            إذا (old_br == sp->بنياد + sp->سائيز) {
              حرف* rel_br = (حرف*)(CALL_MORECORE(-extra));
              حرف* new_br = (حرف*)(CALL_MORECORE(0));
              إذا (rel_br != CMFAIL && new_br < old_br)
                released = old_br - new_br;
            }
          }
          RELEASE_MALLOC_GLOBAL_تالو();
        }
      }

      إذا (released != 0) {
        sp->سائيز -= released;
        m->footprint -= released;
        init_چوٽي(m, m->چوٽي, m->topsize - released);
        check_چوٽي_chunk(m, m->چوٽي);
      }
    }

    /* Unmap any unused mmapped segments */
    إذا (HAVE_MMAP)
      released += release_اڻ_استعمال_ٿيل_segments(m);

    /* On failure, disable autotrim to avoid repeated failed future calls */
    إذا (released == 0 && m->topsize > m->trim_check)
      m->trim_check = MAX_سائيز_T;
  }

  أرجع (released != 0)? 1 : 0;
}

/* Consolidate and bin a chunk. Differs from exported versions
   of free mainly in that the chunk need not be marked as inuse.
*/
ساكن فراغ dispose_chunk(mstate m, mchunkptr p, size_t psize) {
  mchunkptr ايندڙ = chunk_plus_وٿي(p, psize);
  إذا (!pinuse(p)) {
    mchunkptr prev;
    size_t prevsize = p->prev_foot;
    إذا (is_mmapped(p)) {
      psize += prevsize + MMAP_FOOT_PAD;
      إذا (CALL_MUNMAP((حرف*)p - prevsize, psize) == 0)
        m->footprint -= psize;
      أرجع;
    }
    prev = chunk_minus_وٿي(p, prevsize);
    psize += prevsize;
    p = prev;
    إذا (RTCHECK(ok_پتو(m, prev))) { /* consolidate backward */
      إذا (p != m->dv) {
        unlink_chunk(m, p, prevsize);
      }
      وإلا إذا ((ايندڙ->سر & INUSE_بٽ) == INUSE_بٽ) {
        m->dvsize = psize;
        سيٽ_free_with_pinuse(p, psize, ايندڙ);
        أرجع;
      }
    }
    وإلا {
      CORRUPTION_ERROR_ACTION(m);
      أرجع;
    }
  }
  إذا (RTCHECK(ok_پتو(m, ايندڙ))) {
    إذا (!cinuse(ايندڙ)) {  /* consolidate forward */
      إذا (ايندڙ == m->چوٽي) {
        size_t tsize = m->topsize += psize;
        m->چوٽي = p;
        p->سر = tsize | PINUSE_ٻٽو_انگ;
        إذا (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        أرجع;
      }
      وإلا إذا (ايندڙ == m->dv) {
        size_t dsize = m->dvsize += psize;
        m->dv = p;
        سيٽ_سائيز_and_pinuse_جو_free_chunk(p, dsize);
        أرجع;
      }
      وإلا {
        size_t nsize = chunksize(ايندڙ);
        psize += nsize;
        unlink_chunk(m, ايندڙ, nsize);
        سيٽ_سائيز_and_pinuse_جو_free_chunk(p, psize);
        إذا (p == m->dv) {
          m->dvsize = psize;
          أرجع;
        }
      }
    }
    وإلا {
      سيٽ_free_with_pinuse(p, psize, ايندڙ);
    }
    وجهو_chunk(m, p, psize);
  }
  وإلا {
    CORRUPTION_ERROR_ACTION(m);
  }
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
ساكن فراغ* tmalloc_large(mstate m, size_t nb) {
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  bindex_t idx;
  compute_وڻ_اشاريو(nb, idx);
  إذا ((t = *treebin_at(m, idx)) != 0) {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_وڻ_اشاريو(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    لكل (;;) {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      إذا (trem < rsize) {
        v = t;
        إذا ((rsize = trem) == 0)
          اكسر;
      }
      rt = t->ٻار[1];
      t = t->ٻار[(sizebits >> (سائيز_T_BITSIZE-سائيز_T_ONE)) & 1];
      إذا (rt != 0 && rt != t)
        rst = rt;
      إذا (t == 0) {
        t = rst; /* set t to least subtree holding sizes > nb */
        اكسر;
      }
      sizebits <<= 1;
    }
  }
  إذا (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
    binmap_t leftbits = کاٻو_بٽ(idx2bit(idx)) & m->treemap;
    إذا (leftbits != 0) {
      bindex_t i;
      binmap_t leastbit = گهٽ_کان_گهٽ_ٻٽو_انگ(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  طالما (t != 0) { /* find smallest of tree or subtree */
    size_t trem = chunksize(t) - nb;
    إذا (trem < rsize) {
      rsize = trem;
      v = t;
    }
    t = leftmost_ٻار(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  إذا (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
    إذا (RTCHECK(ok_پتو(m, v))) { /*ورهايو*/
      mchunkptr r = chunk_plus_وٿي(v, nb);
      أكد(chunksize(v) == rsize + nb);
      إذا (RTCHECK(ok_ايندڙ(v, r))) {
        unlink_large_chunk(m, v);
        إذا (rsize < MIN_CHUNK_سائيز)
          سيٽ_inuse_and_pinuse(m, v, (rsize + nb));
        وإلا {
          سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, v, nb);
          سيٽ_سائيز_and_pinuse_جو_free_chunk(r, rsize);
          وجهو_chunk(m, r, rsize);
        }
        أرجع chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  أرجع 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
ساكن فراغ* tmalloc_small(mstate m, size_t nb) {
  tchunkptr t, v;
  size_t rsize;
  bindex_t i;
  binmap_t leastbit = گهٽ_کان_گهٽ_ٻٽو_انگ(m->treemap);
  compute_bit2idx(leastbit, i);
  v = t = *treebin_at(m, i);
  rsize = chunksize(t) - nb;

  طالما ((t = leftmost_ٻار(t)) != 0) {
    size_t trem = chunksize(t) - nb;
    إذا (trem < rsize) {
      rsize = trem;
      v = t;
    }
  }

  إذا (RTCHECK(ok_پتو(m, v))) {
    mchunkptr r = chunk_plus_وٿي(v, nb);
    أكد(chunksize(v) == rsize + nb);
    إذا (RTCHECK(ok_ايندڙ(v, r))) {
      unlink_large_chunk(m, v);
      إذا (rsize < MIN_CHUNK_سائيز)
        سيٽ_inuse_and_pinuse(m, v, (rsize + nb));
      وإلا {
        سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, v, nb);
        سيٽ_سائيز_and_pinuse_جو_free_chunk(r, rsize);
        replace_dv(m, r, rsize);
      }
      أرجع chunk2mem(v);
    }
  }

  CORRUPTION_ERROR_ACTION(m);
  أرجع 0;
}

#إذا !ONLY_MSPACES

فراغ* dlmalloc(size_t بائيٽون) {
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

#إذا USE_LOCKS
  ensure_initialization(); /* initialize in sys_alloc if not using locks */
#جيڪڏهن_پورو

  إذا (!PREACTION(gm)) {
    فراغ* mem;
    size_t nb;
    إذا (بائيٽون <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (بائيٽون < MIN_REQUEST)? MIN_CHUNK_سائيز : pad_request(بائيٽون);
      idx = small_اشاريو(nb);
      smallbits = gm->smallmap >> idx;

      إذا ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(gm, idx);
        p = b->fd;
        أكد(chunksize(p) == small_index2size(idx));
        unlink_پهريون_small_chunk(gm, b, p, idx);
        سيٽ_inuse_and_pinuse(gm, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(gm, mem, nb);
        اذهب postaction;
      }

      وإلا إذا (nb > gm->dvsize) {
        إذا (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & کاٻو_بٽ(idx2bit(idx));
          binmap_t leastbit = گهٽ_کان_گهٽ_ٻٽو_انگ(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(gm, i);
          p = b->fd;
          أكد(chunksize(p) == small_index2size(i));
          unlink_پهريون_small_chunk(gm, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          إذا (سائيز_T_سائيز != 4 && rsize < MIN_CHUNK_سائيز)
            سيٽ_inuse_and_pinuse(gm, p, small_index2size(i));
          وإلا {
            سيٽ_سائيز_and_pinuse_جو_inuse_chunk(gm, p, nb);
            r = chunk_plus_وٿي(p, nb);
            سيٽ_سائيز_and_pinuse_جو_free_chunk(r, rsize);
            replace_dv(gm, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(gm, mem, nb);
          اذهب postaction;
        }

        وإلا إذا (gm->treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0) {
          check_malloced_chunk(gm, mem, nb);
          اذهب postaction;
        }
      }
    }
    وإلا إذا (بائيٽون >= MAX_REQUEST)
      nb = MAX_سائيز_T; /* Too big to allocate. Force failure (in sys alloc) */
    وإلا {
      nb = pad_request(بائيٽون);
      إذا (gm->treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0) {
        check_malloced_chunk(gm, mem, nb);
        اذهب postaction;
      }
    }

    إذا (nb <= gm->dvsize) {
      size_t rsize = gm->dvsize - nb;
      mchunkptr p = gm->dv;
      إذا (rsize >= MIN_CHUNK_سائيز) { /* split dv */
        mchunkptr r = gm->dv = chunk_plus_وٿي(p, nb);
        gm->dvsize = rsize;
        سيٽ_سائيز_and_pinuse_جو_free_chunk(r, rsize);
        سيٽ_سائيز_and_pinuse_جو_inuse_chunk(gm, p, nb);
      }
      وإلا { /* exhaust dv */
        size_t dvs = gm->dvsize;
        gm->dvsize = 0;
        gm->dv = 0;
        سيٽ_inuse_and_pinuse(gm, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(gm, mem, nb);
      اذهب postaction;
    }

    وإلا إذا (nb < gm->topsize) { /* Split top */
      size_t rsize = gm->topsize -= nb;
      mchunkptr p = gm->چوٽي;
      mchunkptr r = gm->چوٽي = chunk_plus_وٿي(p, nb);
      r->سر = rsize | PINUSE_ٻٽو_انگ;
      سيٽ_سائيز_and_pinuse_جو_inuse_chunk(gm, p, nb);
      mem = chunk2mem(p);
      check_چوٽي_chunk(gm, gm->چوٽي);
      check_malloced_chunk(gm, mem, nb);
      اذهب postaction;
    }

    mem = sys_alloc(gm, nb);

  postaction:
    POSTACTION(gm);
    أرجع mem;
  }

  أرجع 0;
}

/* ---------------------------- free --------------------------- */

فراغ dlfree(فراغ* mem) {
  /*
     Consolidate freed chunks with preceding or succeeding bordering
     free chunks, if they exist, and then place in a bin.  Intermixed
     with special cases for top, dv, mmapped chunks, and usage errors.
  */

  إذا (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#إذا FOOTERS
    mstate fm = get_mstate_for(p);
    إذا (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      أرجع;
    }
#وإلا /* FOOTERS */
#وصف_ڪريو fm gm
#جيڪڏهن_پورو /* FOOTERS */
    إذا (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      إذا (RTCHECK(ok_پتو(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr ايندڙ = chunk_plus_وٿي(p, psize);
        إذا (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          إذا (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            إذا (CALL_MUNMAP((حرف*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            اذهب postaction;
          }
          وإلا {
            mchunkptr prev = chunk_minus_وٿي(p, prevsize);
            psize += prevsize;
            p = prev;
            إذا (RTCHECK(ok_پتو(fm, prev))) { /* consolidate backward */
              إذا (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              وإلا إذا ((ايندڙ->سر & INUSE_بٽ) == INUSE_بٽ) {
                fm->dvsize = psize;
                سيٽ_free_with_pinuse(p, psize, ايندڙ);
                اذهب postaction;
              }
            }
            وإلا
              اذهب erroraction;
          }
        }

        إذا (RTCHECK(ok_ايندڙ(p, ايندڙ) && ok_pinuse(ايندڙ))) {
          إذا (!cinuse(ايندڙ)) {  /* consolidate forward */
            إذا (ايندڙ == fm->چوٽي) {
              size_t tsize = fm->topsize += psize;
              fm->چوٽي = p;
              p->سر = tsize | PINUSE_ٻٽو_انگ;
              إذا (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              إذا (should_trim(fm, tsize))
                sys_trim(fm, 0);
              اذهب postaction;
            }
            وإلا إذا (ايندڙ == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              سيٽ_سائيز_and_pinuse_جو_free_chunk(p, dsize);
              اذهب postaction;
            }
            وإلا {
              size_t nsize = chunksize(ايندڙ);
              psize += nsize;
              unlink_chunk(fm, ايندڙ, nsize);
              سيٽ_سائيز_and_pinuse_جو_free_chunk(p, psize);
              إذا (p == fm->dv) {
                fm->dvsize = psize;
                اذهب postaction;
              }
            }
          }
          وإلا
            سيٽ_free_with_pinuse(p, psize, ايندڙ);

          إذا (is_small(psize)) {
            وجهو_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          وإلا {
            tchunkptr tp = (tchunkptr)p;
            وجهو_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            إذا (--fm->release_checks == 0)
              release_اڻ_استعمال_ٿيل_segments(fm);
          }
          اذهب postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
#إذا !FOOTERS
#وصف_رد fm
#جيڪڏهن_پورو /* FOOTERS */
}

فراغ* dlcalloc(size_t n_elements, size_t elem_سائيز) {
  فراغ* mem;
  size_t req = 0;
  إذا (n_elements != 0) {
    req = n_elements * elem_سائيز;
    إذا (((n_elements | elem_سائيز) & ~(size_t)0xffff) &&
        (req / n_elements != elem_سائيز))
      req = MAX_سائيز_T; /* force downstream failure on overflow */
  }
  mem = dlmalloc(req);
  إذا (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  أرجع mem;
}

#جيڪڏهن_پورو /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
ساكن mchunkptr try_realloc_chunk(mstate m, mchunkptr p, size_t nb,
                                   صحيح can_move) {
  mchunkptr newp = 0;
  size_t oldsize = chunksize(p);
  mchunkptr ايندڙ = chunk_plus_وٿي(p, oldsize);
  إذا (RTCHECK(ok_پتو(m, p) && ok_inuse(p) &&
              ok_ايندڙ(p, ايندڙ) && ok_pinuse(ايندڙ))) {
    إذا (is_mmapped(p)) {
      newp = mmap_resize(m, p, nb, can_move);
    }
    وإلا إذا (oldsize >= nb) {             /* already big enough */
      size_t rsize = oldsize - nb;
      إذا (rsize >= MIN_CHUNK_سائيز) {      /* split off remainder */
        mchunkptr r = chunk_plus_وٿي(p, nb);
        سيٽ_inuse(m, p, nb);
        سيٽ_inuse(m, r, rsize);
        dispose_chunk(m, r, rsize);
      }
      newp = p;
    }
    وإلا إذا (ايندڙ == m->چوٽي) {  /* extend into top */
      إذا (oldsize + m->topsize > nb) {
        size_t newsize = oldsize + m->topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_وٿي(p, nb);
        سيٽ_inuse(m, p, nb);
        newtop->سر = newtopsize |PINUSE_ٻٽو_انگ;
        m->چوٽي = newtop;
        m->topsize = newtopsize;
        newp = p;
      }
    }
    وإلا إذا (ايندڙ == m->dv) { /* extend into dv */
      size_t dvs = m->dvsize;
      إذا (oldsize + dvs >= nb) {
        size_t dsize = oldsize + dvs - nb;
        إذا (dsize >= MIN_CHUNK_سائيز) {
          mchunkptr r = chunk_plus_وٿي(p, nb);
          mchunkptr n = chunk_plus_وٿي(r, dsize);
          سيٽ_inuse(m, p, nb);
          سيٽ_سائيز_and_pinuse_جو_free_chunk(r, dsize);
          clear_pinuse(n);
          m->dvsize = dsize;
          m->dv = r;
        }
        وإلا { /* exhaust dv */
          size_t newsize = oldsize + dvs;
          سيٽ_inuse(m, p, newsize);
          m->dvsize = 0;
          m->dv = 0;
        }
        newp = p;
      }
    }
    وإلا إذا (!cinuse(ايندڙ)) { /* extend into next free chunk */
      size_t nextsize = chunksize(ايندڙ);
      إذا (oldsize + nextsize >= nb) {
        size_t rsize = oldsize + nextsize - nb;
        unlink_chunk(m, ايندڙ, nextsize);
        إذا (rsize < MIN_CHUNK_سائيز) {
          size_t newsize = oldsize + nextsize;
          سيٽ_inuse(m, p, newsize);
        }
        وإلا {
          mchunkptr r = chunk_plus_وٿي(p, nb);
          سيٽ_inuse(m, p, nb);
          سيٽ_inuse(m, r, rsize);
          dispose_chunk(m, r, rsize);
        }
        newp = p;
      }
    }
  }
  وإلا {
    USAGE_ERROR_ACTION(m, chunk2mem(p));
  }
  أرجع newp;
}

ساكن فراغ* اندروني_memalign(mstate m, size_t alignment, size_t بائيٽون) {
  فراغ* mem = 0;
  إذا (alignment <  MIN_CHUNK_سائيز) /* must be at least a minimum chunk size */
    alignment = MIN_CHUNK_سائيز;
  إذا ((alignment & (alignment-سائيز_T_ONE)) != 0) {/* Ensure a power of 2 */
    size_t a = MALLOC_ALIGNMENT << 1;
    طالما (a < alignment) a <<= 1;
    alignment = a;
  }
  إذا (بائيٽون >= MAX_REQUEST - alignment) {
    إذا (m != 0)  { /* Test isn't needed but avoids compiler warning */
      MALLOC_FAILURE_ACTION;
    }
  }
  وإلا {
    size_t nb = request2size(بائيٽون);
    size_t req = nb + alignment + MIN_CHUNK_سائيز - CHUNK_OVERHEAD;
    mem = اندروني_malloc(m, req);
    إذا (mem != 0) {
      mchunkptr p = mem2chunk(mem);
      إذا (PREACTION(m))
        أرجع 0;
      إذا ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
        حرف* br = (حرف*)mem2chunk((size_t)(((size_t)((حرف*)mem + alignment -
                                                       سائيز_T_ONE)) &
                                             -alignment));
        حرف* pos = ((size_t)(br - (حرف*)(p)) >= MIN_CHUNK_سائيز)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (حرف*)(p);
        size_t newsize = chunksize(p) - leadsize;

        إذا (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
          newp->prev_foot = p->prev_foot + leadsize;
          newp->سر = newsize;
        }
        وإلا { /* Otherwise, give back leader, use the rest */
          سيٽ_inuse(m, newp, newsize);
          سيٽ_inuse(m, p, leadsize);
          dispose_chunk(m, p, leadsize);
        }
        p = newp;
      }

      /* Give back spare room at the end */
      إذا (!is_mmapped(p)) {
        size_t سائيز = chunksize(p);
        إذا (سائيز > nb + MIN_CHUNK_سائيز) {
          size_t remainder_سائيز = سائيز - nb;
          mchunkptr remainder = chunk_plus_وٿي(p, nb);
          سيٽ_inuse(m, p, nb);
          سيٽ_inuse(m, remainder, remainder_سائيز);
          dispose_chunk(m, remainder, remainder_سائيز);
        }
      }

      mem = chunk2mem(p);
      أكد (chunksize(p) >= nb);
      أكد(((size_t)mem & (alignment - 1)) == 0);
      check_inuse_chunk(m, p);
      POSTACTION(m);
    }
  }
  أرجع mem;
}

/*
  Common support for independent_X routines, handling
    all of the combinations that can result.
  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/
ساكن فراغ** ialloc(mstate m,
                     size_t n_elements,
                     size_t* sizes,
                     صحيح opts,
                     فراغ* chunks[]) {

  size_t    element_سائيز;   /* chunksize of each element, if all same */
  size_t    contents_سائيز;  /* total size of elements */
  size_t    array_سائيز;     /* request size of pointer array */
  فراغ*     mem;            /* malloced aggregate space */
  mchunkptr p;              /* corresponding chunk */
  size_t    remainder_سائيز; /* remaining bytes while splitting */
  فراغ**    marray;         /* either "chunks" or malloced ptr array */
  mchunkptr array_chunk;    /* chunk for malloced ptr array */
  flag_t    was_enabled;    /* to disable mmap */
  size_t    سائيز;
  size_t    i;

  ensure_initialization();
  /* compute array length, if needed */
  إذا (chunks != 0) {
    إذا (n_elements == 0)
      أرجع chunks; /* nothing to do */
    marray = chunks;
    array_سائيز = 0;
  }
  وإلا {
    /* if empty req, must still return chunk representing empty array */
    إذا (n_elements == 0)
      أرجع (فراغ**)اندروني_malloc(m, 0);
    marray = 0;
    array_سائيز = request2size(n_elements * (حجم(فراغ*)));
  }

  /* compute total element size */
  إذا (opts & 0x1) { /* all-same-size */
    element_سائيز = request2size(*sizes);
    contents_سائيز = n_elements * element_سائيز;
  }
  وإلا { /* add up all the sizes */
    element_سائيز = 0;
    contents_سائيز = 0;
    لكل (i = 0; i != n_elements; ++i)
      contents_سائيز += request2size(sizes[i]);
  }

  سائيز = contents_سائيز + array_سائيز;

  /*
     Allocate the aggregate chunk.  First disable direct-mmapping so
     malloc won't use it, since we would not be able to later
     free/realloc space internal to a segregated mmap region.
  */
  was_enabled = use_mmap(m);
  disable_mmap(m);
  mem = اندروني_malloc(m, سائيز - CHUNK_OVERHEAD);
  إذا (was_enabled)
    enable_mmap(m);
  إذا (mem == 0)
    أرجع 0;

  إذا (PREACTION(m)) أرجع 0;
  p = mem2chunk(mem);
  remainder_سائيز = chunksize(p);

  أكد(!is_mmapped(p));

  إذا (opts & 0x2) {       /* optionally clear the elements */
    memset((size_t*)mem, 0, remainder_سائيز - سائيز_T_سائيز - array_سائيز);
  }

  /* If not provided, allocate the pointer array as final part of chunk */
  إذا (marray == 0) {
    size_t  array_chunk_سائيز;
    array_chunk = chunk_plus_وٿي(p, contents_سائيز);
    array_chunk_سائيز = remainder_سائيز - contents_سائيز;
    marray = (فراغ**) (chunk2mem(array_chunk));
    سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, array_chunk, array_chunk_سائيز);
    remainder_سائيز = contents_سائيز;
  }

  /* split out elements */
  لكل (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    إذا (i != n_elements-1) {
      إذا (element_سائيز != 0)
        سائيز = element_سائيز;
      وإلا
        سائيز = request2size(sizes[i]);
      remainder_سائيز -= سائيز;
      سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, p, سائيز);
      p = chunk_plus_وٿي(p, سائيز);
    }
    وإلا { /* the final element absorbs any overallocation slop */
      سيٽ_سائيز_and_pinuse_جو_inuse_chunk(m, p, remainder_سائيز);
      اكسر;
    }
  }

#إذا DEBUG
  إذا (marray != chunks) {
    /* final element must have exactly exhausted chunk */
    إذا (element_سائيز != 0) {
      أكد(remainder_سائيز == element_سائيز);
    }
    وإلا {
      أكد(remainder_سائيز == request2size(sizes[i]));
    }
    check_inuse_chunk(m, mem2chunk(marray));
  }
  لكل (i = 0; i != n_elements; ++i)
    check_inuse_chunk(m, mem2chunk(marray[i]));

#جيڪڏهن_پورو /* DEBUG */

  POSTACTION(m);
  أرجع marray;
}

/* Try to free all pointers in the given array.
   Note: this could be made faster, by delaying consolidation,
   at the price of disabling some user integrity checks, We
   still optimize some consolidations by combining adjacent
   chunks before freeing, which will occur often if allocated
   with ialloc or the array is sorted.
*/
ساكن size_t اندروني_bulk_free(mstate m, فراغ* array[], size_t nelem) {
  size_t unfreed = 0;
  إذا (!PREACTION(m)) {
    فراغ** a;
    فراغ** fence = &(array[nelem]);
    لكل (a = array; a != fence; ++a) {
      فراغ* mem = *a;
      إذا (mem != 0) {
        mchunkptr p = mem2chunk(mem);
        size_t psize = chunksize(p);
#إذا FOOTERS
        إذا (get_mstate_for(p) != m) {
          ++unfreed;
          استمر;
        }
#جيڪڏهن_پورو
        check_inuse_chunk(m, p);
        *a = 0;
        إذا (RTCHECK(ok_پتو(m, p) && ok_inuse(p))) {
          فراغ ** b = a + 1; /* try to merge with next chunk */
          mchunkptr ايندڙ = ايندڙ_chunk(p);
          إذا (b != fence && *b == chunk2mem(ايندڙ)) {
            size_t newsize = chunksize(ايندڙ) + psize;
            سيٽ_inuse(m, p, newsize);
            *b = chunk2mem(p);
          }
          وإلا
            dispose_chunk(m, p, psize);
        }
        وإلا {
          CORRUPTION_ERROR_ACTION(m);
          اكسر;
        }
      }
    }
    إذا (should_trim(m, m->topsize))
      sys_trim(m, 0);
    POSTACTION(m);
  }
  أرجع unfreed;
}

/* Traversal */
#إذا MALLOC_INSPECT_ALL
ساكن فراغ اندروني_inspect_all(mstate m,
                                 فراغ(*handler)(فراغ *شروعات,
                                                فراغ *پڇاڙي,
                                                size_t استعمال_ٿيل_بائيٽون,
                                                فراغ* callback_arg),
                                 فراغ* arg) {
  إذا (is_initialized(m)) {
    mchunkptr چوٽي = m->چوٽي;
    msegmentptr s;
    لكل (s = &m->seg; s != 0; s = s->ايندڙ) {
      mchunkptr q = align_as_chunk(s->بنياد);
      طالما (segment_holds(s, q) && q->سر != FENCEPOST_سر) {
        mchunkptr ايندڙ = ايندڙ_chunk(q);
        size_t sz = chunksize(q);
        size_t استعمال_ٿيل;
        فراغ* شروعات;
        إذا (is_inuse(q)) {
          استعمال_ٿيل = sz - CHUNK_OVERHEAD; /* must not be mmapped */
          شروعات = chunk2mem(q);
        }
        وإلا {
          استعمال_ٿيل = 0;
          إذا (is_small(sz)) {     /* offset by possible bookkeeping */
            شروعات = (فراغ*)((حرف*)q + حجم(بنية malloc_chunk));
          }
          وإلا {
            شروعات = (فراغ*)((حرف*)q + حجم(بنية malloc_وڻ_chunk));
          }
        }
        إذا (شروعات < (فراغ*)ايندڙ)  /* skip if all space is bookkeeping */
          handler(شروعات, ايندڙ, استعمال_ٿيل, arg);
        إذا (q == چوٽي)
          اكسر;
        q = ايندڙ;
      }
    }
  }
}
#جيڪڏهن_پورو /* MALLOC_INSPECT_ALL */

/* ------------------ Exported realloc, memalign, etc -------------------- */

#إذا !ONLY_MSPACES

فراغ* dlrealloc(فراغ* oldmem, size_t بائيٽون) {
  فراغ* mem = 0;
  إذا (oldmem == 0) {
    mem = dlmalloc(بائيٽون);
  }
  وإلا إذا (بائيٽون >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#جيڪڏهن_وصف_ٿيل REALLOC_ZERO_بائيٽون_FREES
  وإلا إذا (بائيٽون == 0) {
    dlfree(oldmem);
  }
#جيڪڏهن_پورو /* REALLOC_ZERO_BYTES_FREES */
  وإلا {
    size_t nb = request2size(بائيٽون);
    mchunkptr oldp = mem2chunk(oldmem);
#إذا ! FOOTERS
    mstate m = gm;
#وإلا /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    إذا (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      أرجع 0;
    }
#جيڪڏهن_پورو /* FOOTERS */
    إذا (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      إذا (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      وإلا {
        mem = اندروني_malloc(m, بائيٽون);
        إذا (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          يادگيري_نقل(mem, oldmem, (oc < بائيٽون)? oc : بائيٽون);
          اندروني_free(m, oldmem);
        }
      }
    }
  }
  أرجع mem;
}

فراغ* dlrealloc_اندريون_رکو(فراغ* oldmem, size_t بائيٽون) {
  فراغ* mem = 0;
  إذا (oldmem != 0) {
    إذا (بائيٽون >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    وإلا {
      size_t nb = request2size(بائيٽون);
      mchunkptr oldp = mem2chunk(oldmem);
#إذا ! FOOTERS
      mstate m = gm;
#وإلا /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      إذا (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        أرجع 0;
      }
#جيڪڏهن_پورو /* FOOTERS */
      إذا (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        إذا (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  أرجع mem;
}

فراغ* dlmemalign(size_t alignment, size_t بائيٽون) {
  إذا (alignment <= MALLOC_ALIGNMENT) {
    أرجع dlmalloc(بائيٽون);
  }
  أرجع اندروني_memalign(gm, alignment, بائيٽون);
}

صحيح dlposix_memalign(فراغ** pp, size_t alignment, size_t بائيٽون) {
  فراغ* mem = 0;
  إذا (alignment == MALLOC_ALIGNMENT)
    mem = dlmalloc(بائيٽون);
  وإلا {
    size_t d = alignment / حجم(فراغ*);
    size_t r = alignment % حجم(فراغ*);
    إذا (r != 0 || d == 0 || (d & (d-سائيز_T_ONE)) != 0)
      أرجع EINVAL;
    وإلا إذا (بائيٽون <= MAX_REQUEST - alignment) {
      إذا (alignment <  MIN_CHUNK_سائيز)
        alignment = MIN_CHUNK_سائيز;
      mem = اندروني_memalign(gm, alignment, بائيٽون);
    }
  }
  إذا (mem == 0)
    أرجع ENOMEM;
  وإلا {
    *pp = mem;
    أرجع 0;
  }
}

فراغ* dlvalloc(size_t بائيٽون) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.صفحو_سائيز;
  أرجع dlmemalign(pagesz, بائيٽون);
}

فراغ* dlpvalloc(size_t بائيٽون) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.صفحو_سائيز;
  أرجع dlmemalign(pagesz, (بائيٽون + pagesz - سائيز_T_ONE) & ~(pagesz - سائيز_T_ONE));
}

فراغ** dlindependent_calloc(size_t n_elements, size_t elem_سائيز,
                            فراغ* chunks[]) {
  size_t sz = elem_سائيز; /* serves as 1-element array */
  أرجع ialloc(gm, n_elements, &sz, 3, chunks);
}

فراغ** dlindependent_comalloc(size_t n_elements, size_t sizes[],
                              فراغ* chunks[]) {
  أرجع ialloc(gm, n_elements, sizes, 0, chunks);
}

size_t dlbulk_free(فراغ* array[], size_t nelem) {
  أرجع اندروني_bulk_free(gm, array, nelem);
}

#إذا MALLOC_INSPECT_ALL
فراغ dlmalloc_inspect_all(فراغ(*handler)(فراغ *شروعات,
                                         فراغ *پڇاڙي,
                                         size_t استعمال_ٿيل_بائيٽون,
                                         فراغ* callback_arg),
                          فراغ* arg) {
  ensure_initialization();
  إذا (!PREACTION(gm)) {
    اندروني_inspect_all(gm, handler, arg);
    POSTACTION(gm);
  }
}
#جيڪڏهن_پورو /* MALLOC_INSPECT_ALL */

صحيح dlmalloc_trim(size_t pad) {
  صحيح نتيجو = 0;
  ensure_initialization();
  إذا (!PREACTION(gm)) {
    نتيجو = sys_trim(gm, pad);
    POSTACTION(gm);
  }
  أرجع نتيجو;
}

size_t dlmalloc_footprint(فراغ) {
  أرجع gm->footprint;
}

size_t dlmalloc_max_footprint(فراغ) {
  أرجع gm->max_footprint;
}

size_t dlmalloc_footprint_حد_بندي(فراغ) {
  size_t maf = gm->footprint_حد_بندي;
  أرجع maf == 0 ? MAX_سائيز_T : maf;
}

size_t dlmalloc_سيٽ_footprint_حد_بندي(size_t بائيٽون) {
  size_t نتيجو;  /* invert sense of 0 */
  إذا (بائيٽون == 0)
    نتيجو = granularity_align(1); /* Use minimal size */
  إذا (بائيٽون == MAX_سائيز_T)
    نتيجو = 0;                    /* disable */
  وإلا
    نتيجو = granularity_align(بائيٽون);
  أرجع gm->footprint_حد_بندي = نتيجو;
}

#إذا !NO_MALLINFO
بنية mallinfo dlmallinfo(فراغ) {
  أرجع اندروني_mallinfo(gm);
}
#جيڪڏهن_پورو /* NO_MALLINFO */

#إذا !NO_MALLOC_STATS
فراغ dlmalloc_stats(فراغ) {
  اندروني_malloc_stats(gm);
}
#جيڪڏهن_پورو /* NO_MALLOC_STATS */

صحيح dlmallopt(صحيح param_عدد, صحيح قدر) {
  أرجع change_mparam(param_عدد, قدر);
}

size_t dlmalloc_usable_سائيز(فراغ* mem) {
  إذا (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    إذا (is_inuse(p))
      أرجع chunksize(p) - overhead_for(p);
  }
  أرجع 0;
}

#جيڪڏهن_پورو /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#إذا MSPACES

ساكن mstate init_user_mstate(حرف* tbase, size_t tsize) {
  size_t msize = pad_request(حجم(بنية malloc_state));
  mchunkptr mn;
  mchunkptr msp = align_as_chunk(tbase);
  mstate m = (mstate)(chunk2mem(msp));
  memset(m, 0, msize);
  (فراغ)INITIAL_تالو(&m->mutex);
  msp->سر = (msize|INUSE_بٽ);
  m->seg.بنياد = m->گهٽ_کان_گهٽ_addr = tbase;
  m->seg.سائيز = m->footprint = m->max_footprint = tsize;
  m->magic = mparams.magic;
  m->release_checks = MAX_RELEASE_CHECK_شرح;
  m->mflags = mparams.default_mflags;
  m->extp = 0;
  m->exts = 0;
  disable_contiguous(m);
  init_bins(m);
  mn = ايندڙ_chunk(mem2chunk(m));
  init_چوٽي(m, mn, (size_t)((tbase + tsize) - (حرف*)mn) - چوٽي_FOOT_سائيز);
  check_چوٽي_chunk(m, m->چوٽي);
  أرجع m;
}

mspace create_mspace(size_t صلاحيت, صحيح locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(حجم(بنية malloc_state));
  إذا (صلاحيت < (size_t) -(msize + چوٽي_FOOT_سائيز + mparams.صفحو_سائيز)) {
    size_t rs = ((صلاحيت == 0)? mparams.granularity :
                 (صلاحيت + چوٽي_FOOT_سائيز + msize));
    size_t tsize = granularity_align(rs);
    حرف* tbase = (حرف*)(CALL_MMAP(tsize));
    إذا (tbase != CMFAIL) {
      m = init_user_mstate(tbase, tsize);
      سيٽ_segment_flags(&m->seg, USE_MMAP_ٻٽو_انگ);
      سيٽ_تالو(m, locked);
    }
  }
  أرجع (mspace)m;
}

mspace create_mspace_with_بنياد(فراغ* بنياد, size_t صلاحيت, صحيح locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(حجم(بنية malloc_state));
  إذا (صلاحيت > msize + چوٽي_FOOT_سائيز &&
      صلاحيت < (size_t) -(msize + چوٽي_FOOT_سائيز + mparams.صفحو_سائيز)) {
    m = init_user_mstate((حرف*)بنياد, صلاحيت);
    سيٽ_segment_flags(&m->seg, EXTERN_ٻٽو_انگ);
    سيٽ_تالو(m, locked);
  }
  أرجع (mspace)m;
}

صحيح mspace_track_large_chunks(mspace msp, صحيح enable) {
  صحيح ret = 0;
  mstate ms = (mstate)msp;
  إذا (!PREACTION(ms)) {
    إذا (!use_mmap(ms)) {
      ret = 1;
    }
    إذا (!enable) {
      enable_mmap(ms);
    } وإلا {
      disable_mmap(ms);
    }
    POSTACTION(ms);
  }
  أرجع ret;
}

size_t destroy_mspace(mspace msp) {
  size_t freed = 0;
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    msegmentptr sp = &ms->seg;
    (فراغ)DESTROY_تالو(&ms->mutex); /* destroy before unmapped */
    طالما (sp != 0) {
      حرف* بنياد = sp->بنياد;
      size_t سائيز = sp->سائيز;
      flag_t flag = get_segment_flags(sp);
      (فراغ)بنياد; /* placate people compiling -Wunused-variable */
      sp = sp->ايندڙ;
      إذا ((flag & USE_MMAP_ٻٽو_انگ) && !(flag & EXTERN_ٻٽو_انگ) &&
          CALL_MUNMAP(بنياد, سائيز) == 0)
        freed += سائيز;
    }
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
  أرجع freed;
}

/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

فراغ* mspace_malloc(mspace msp, size_t بائيٽون) {
  mstate ms = (mstate)msp;
  إذا (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    أرجع 0;
  }
  إذا (!PREACTION(ms)) {
    فراغ* mem;
    size_t nb;
    إذا (بائيٽون <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (بائيٽون < MIN_REQUEST)? MIN_CHUNK_سائيز : pad_request(بائيٽون);
      idx = small_اشاريو(nb);
      smallbits = ms->smallmap >> idx;

      إذا ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(ms, idx);
        p = b->fd;
        أكد(chunksize(p) == small_index2size(idx));
        unlink_پهريون_small_chunk(ms, b, p, idx);
        سيٽ_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        اذهب postaction;
      }

      وإلا إذا (nb > ms->dvsize) {
        إذا (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & کاٻو_بٽ(idx2bit(idx));
          binmap_t leastbit = گهٽ_کان_گهٽ_ٻٽو_انگ(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b->fd;
          أكد(chunksize(p) == small_index2size(i));
          unlink_پهريون_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          إذا (سائيز_T_سائيز != 4 && rsize < MIN_CHUNK_سائيز)
            سيٽ_inuse_and_pinuse(ms, p, small_index2size(i));
          وإلا {
            سيٽ_سائيز_and_pinuse_جو_inuse_chunk(ms, p, nb);
            r = chunk_plus_وٿي(p, nb);
            سيٽ_سائيز_and_pinuse_جو_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          اذهب postaction;
        }

        وإلا إذا (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
          check_malloced_chunk(ms, mem, nb);
          اذهب postaction;
        }
      }
    }
    وإلا إذا (بائيٽون >= MAX_REQUEST)
      nb = MAX_سائيز_T; /* Too big to allocate. Force failure (in sys alloc) */
    وإلا {
      nb = pad_request(بائيٽون);
      إذا (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
        check_malloced_chunk(ms, mem, nb);
        اذهب postaction;
      }
    }

    إذا (nb <= ms->dvsize) {
      size_t rsize = ms->dvsize - nb;
      mchunkptr p = ms->dv;
      إذا (rsize >= MIN_CHUNK_سائيز) { /* split dv */
        mchunkptr r = ms->dv = chunk_plus_وٿي(p, nb);
        ms->dvsize = rsize;
        سيٽ_سائيز_and_pinuse_جو_free_chunk(r, rsize);
        سيٽ_سائيز_and_pinuse_جو_inuse_chunk(ms, p, nb);
      }
      وإلا { /* exhaust dv */
        size_t dvs = ms->dvsize;
        ms->dvsize = 0;
        ms->dv = 0;
        سيٽ_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      اذهب postaction;
    }

    وإلا إذا (nb < ms->topsize) { /* Split top */
      size_t rsize = ms->topsize -= nb;
      mchunkptr p = ms->چوٽي;
      mchunkptr r = ms->چوٽي = chunk_plus_وٿي(p, nb);
      r->سر = rsize | PINUSE_ٻٽو_انگ;
      سيٽ_سائيز_and_pinuse_جو_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_چوٽي_chunk(ms, ms->چوٽي);
      check_malloced_chunk(ms, mem, nb);
      اذهب postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
    POSTACTION(ms);
    أرجع mem;
  }

  أرجع 0;
}

فراغ mspace_free(mspace msp, فراغ* mem) {
  إذا (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#إذا FOOTERS
    mstate fm = get_mstate_for(p);
    (فراغ)msp; /* placate people compiling -Wunused */
#وإلا /* FOOTERS */
    mstate fm = (mstate)msp;
#جيڪڏهن_پورو /* FOOTERS */
    إذا (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      أرجع;
    }
    إذا (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      إذا (RTCHECK(ok_پتو(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr ايندڙ = chunk_plus_وٿي(p, psize);
        إذا (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          إذا (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            إذا (CALL_MUNMAP((حرف*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            اذهب postaction;
          }
          وإلا {
            mchunkptr prev = chunk_minus_وٿي(p, prevsize);
            psize += prevsize;
            p = prev;
            إذا (RTCHECK(ok_پتو(fm, prev))) { /* consolidate backward */
              إذا (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              وإلا إذا ((ايندڙ->سر & INUSE_بٽ) == INUSE_بٽ) {
                fm->dvsize = psize;
                سيٽ_free_with_pinuse(p, psize, ايندڙ);
                اذهب postaction;
              }
            }
            وإلا
              اذهب erroraction;
          }
        }

        إذا (RTCHECK(ok_ايندڙ(p, ايندڙ) && ok_pinuse(ايندڙ))) {
          إذا (!cinuse(ايندڙ)) {  /* consolidate forward */
            إذا (ايندڙ == fm->چوٽي) {
              size_t tsize = fm->topsize += psize;
              fm->چوٽي = p;
              p->سر = tsize | PINUSE_ٻٽو_انگ;
              إذا (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              إذا (should_trim(fm, tsize))
                sys_trim(fm, 0);
              اذهب postaction;
            }
            وإلا إذا (ايندڙ == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              سيٽ_سائيز_and_pinuse_جو_free_chunk(p, dsize);
              اذهب postaction;
            }
            وإلا {
              size_t nsize = chunksize(ايندڙ);
              psize += nsize;
              unlink_chunk(fm, ايندڙ, nsize);
              سيٽ_سائيز_and_pinuse_جو_free_chunk(p, psize);
              إذا (p == fm->dv) {
                fm->dvsize = psize;
                اذهب postaction;
              }
            }
          }
          وإلا
            سيٽ_free_with_pinuse(p, psize, ايندڙ);

          إذا (is_small(psize)) {
            وجهو_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          وإلا {
            tchunkptr tp = (tchunkptr)p;
            وجهو_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            إذا (--fm->release_checks == 0)
              release_اڻ_استعمال_ٿيل_segments(fm);
          }
          اذهب postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
}

فراغ* mspace_calloc(mspace msp, size_t n_elements, size_t elem_سائيز) {
  فراغ* mem;
  size_t req = 0;
  mstate ms = (mstate)msp;
  إذا (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    أرجع 0;
  }
  إذا (n_elements != 0) {
    req = n_elements * elem_سائيز;
    إذا (((n_elements | elem_سائيز) & ~(size_t)0xffff) &&
        (req / n_elements != elem_سائيز))
      req = MAX_سائيز_T; /* force downstream failure on overflow */
  }
  mem = اندروني_malloc(ms, req);
  إذا (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  أرجع mem;
}

فراغ* mspace_realloc(mspace msp, فراغ* oldmem, size_t بائيٽون) {
  فراغ* mem = 0;
  إذا (oldmem == 0) {
    mem = mspace_malloc(msp, بائيٽون);
  }
  وإلا إذا (بائيٽون >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#جيڪڏهن_وصف_ٿيل REALLOC_ZERO_بائيٽون_FREES
  وإلا إذا (بائيٽون == 0) {
    mspace_free(msp, oldmem);
  }
#جيڪڏهن_پورو /* REALLOC_ZERO_BYTES_FREES */
  وإلا {
    size_t nb = request2size(بائيٽون);
    mchunkptr oldp = mem2chunk(oldmem);
#إذا ! FOOTERS
    mstate m = (mstate)msp;
#وإلا /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    إذا (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      أرجع 0;
    }
#جيڪڏهن_پورو /* FOOTERS */
    إذا (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      إذا (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      وإلا {
        mem = mspace_malloc(m, بائيٽون);
        إذا (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          يادگيري_نقل(mem, oldmem, (oc < بائيٽون)? oc : بائيٽون);
          mspace_free(m, oldmem);
        }
      }
    }
  }
  أرجع mem;
}

فراغ* mspace_realloc_اندريون_رکو(mspace msp, فراغ* oldmem, size_t بائيٽون) {
  فراغ* mem = 0;
  إذا (oldmem != 0) {
    إذا (بائيٽون >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    وإلا {
      size_t nb = request2size(بائيٽون);
      mchunkptr oldp = mem2chunk(oldmem);
#إذا ! FOOTERS
      mstate m = (mstate)msp;
#وإلا /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      (فراغ)msp; /* placate people compiling -Wunused */
      إذا (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        أرجع 0;
      }
#جيڪڏهن_پورو /* FOOTERS */
      إذا (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        إذا (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  أرجع mem;
}

فراغ* mspace_memalign(mspace msp, size_t alignment, size_t بائيٽون) {
  mstate ms = (mstate)msp;
  إذا (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    أرجع 0;
  }
  إذا (alignment <= MALLOC_ALIGNMENT)
    أرجع mspace_malloc(msp, بائيٽون);
  أرجع اندروني_memalign(ms, alignment, بائيٽون);
}

فراغ** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_سائيز, فراغ* chunks[]) {
  size_t sz = elem_سائيز; /* serves as 1-element array */
  mstate ms = (mstate)msp;
  إذا (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    أرجع 0;
  }
  أرجع ialloc(ms, n_elements, &sz, 3, chunks);
}

فراغ** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], فراغ* chunks[]) {
  mstate ms = (mstate)msp;
  إذا (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    أرجع 0;
  }
  أرجع ialloc(ms, n_elements, sizes, 0, chunks);
}

size_t mspace_bulk_free(mspace msp, فراغ* array[], size_t nelem) {
  أرجع اندروني_bulk_free((mstate)msp, array, nelem);
}

#إذا MALLOC_INSPECT_ALL
فراغ mspace_inspect_all(mspace msp,
                        فراغ(*handler)(فراغ *شروعات,
                                       فراغ *پڇاڙي,
                                       size_t استعمال_ٿيل_بائيٽون,
                                       فراغ* callback_arg),
                        فراغ* arg) {
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    إذا (!PREACTION(ms)) {
      اندروني_inspect_all(ms, handler, arg);
      POSTACTION(ms);
    }
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#جيڪڏهن_پورو /* MALLOC_INSPECT_ALL */

صحيح mspace_trim(mspace msp, size_t pad) {
  صحيح نتيجو = 0;
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    إذا (!PREACTION(ms)) {
      نتيجو = sys_trim(ms, pad);
      POSTACTION(ms);
    }
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
  أرجع نتيجو;
}

#إذا !NO_MALLOC_STATS
فراغ mspace_malloc_stats(mspace msp) {
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    اندروني_malloc_stats(ms);
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#جيڪڏهن_پورو /* NO_MALLOC_STATS */

size_t mspace_footprint(mspace msp) {
  size_t نتيجو = 0;
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    نتيجو = ms->footprint;
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
  أرجع نتيجو;
}

size_t mspace_max_footprint(mspace msp) {
  size_t نتيجو = 0;
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    نتيجو = ms->max_footprint;
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
  أرجع نتيجو;
}

size_t mspace_footprint_حد_بندي(mspace msp) {
  size_t نتيجو = 0;
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    size_t maf = ms->footprint_حد_بندي;
    نتيجو = (maf == 0) ? MAX_سائيز_T : maf;
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
  أرجع نتيجو;
}

size_t mspace_سيٽ_footprint_حد_بندي(mspace msp, size_t بائيٽون) {
  size_t نتيجو = 0;
  mstate ms = (mstate)msp;
  إذا (ok_magic(ms)) {
    إذا (بائيٽون == 0)
      نتيجو = granularity_align(1); /* Use minimal size */
    إذا (بائيٽون == MAX_سائيز_T)
      نتيجو = 0;                    /* disable */
    وإلا
      نتيجو = granularity_align(بائيٽون);
    ms->footprint_حد_بندي = نتيجو;
  }
  وإلا {
    USAGE_ERROR_ACTION(ms,ms);
  }
  أرجع نتيجو;
}

#إذا !NO_MALLINFO
بنية mallinfo mspace_mallinfo(mspace msp) {
  mstate ms = (mstate)msp;
  إذا (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
  }
  أرجع اندروني_mallinfo(ms);
}
#جيڪڏهن_پورو /* NO_MALLINFO */

size_t mspace_usable_سائيز(ثابت فراغ* mem) {
  إذا (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    إذا (is_inuse(p))
      أرجع chunksize(p) - overhead_for(p);
  }
  أرجع 0;
}

صحيح mspace_mallopt(صحيح param_عدد, صحيح قدر) {
  أرجع change_mparam(param_عدد, قدر);
}

#جيڪڏهن_پورو /* MSPACES */


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
