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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DLMALLOC_ਸੰਸਕਰਣ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DLMALLOC_ਸੰਸਕਰਣ 20806
#ਜੇ_ਅੰਤ /* DLMALLOC_VERSION */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DLMALLOC_EXPORT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DLMALLOC_EXPORT ਬਾਹਰੀ
#ਜੇ_ਅੰਤ

#ਜੇ defined __linux__ && !defined GNU_ਸਰੋਤ
/* mremap() on Linux requires this via sys/mman.h */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GNU_ਸਰੋਤ 1
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ WIN32
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN32 1
#ਜੇ_ਅੰਤ  /* _WIN32 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32_WCE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_FCNTL_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN32 1
#ਜੇ_ਅੰਤ /* _WIN32_WCE */
#ਜੇ_ਅੰਤ  /* WIN32 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN32_LEAN_AND_ਔਸਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tchar.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MMAP 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MORECORE 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_UNISTD_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_SYS_PARAM_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_SYS_MMAN_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_STRING_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_ਲੜੀਆਂ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_SYS_TYPES_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_ERRNO_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_SCHED_H
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MALLOC_FAILURE_ACTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_FAILURE_ACTION
#ਜੇ_ਅੰਤ /* MALLOC_FAILURE_ACTION */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MMAP_CLEARS
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32_WCE /* WINCE reportedly does not clear */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_CLEARS 0
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_CLEARS 1
#ਜੇ_ਅੰਤ /* _WIN32_WCE */
#ਜੇ_ਅੰਤ /*MMAP_CLEARS */
#ਜੇ_ਅੰਤ  /* WIN32 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __OS2__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INCL_DOS
#ਅਨ੍ਤਰ੍ਭਾਵ <os2.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MMAP 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MORECORE 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_SYS_MMAN_H
#ਜੇ_ਅੰਤ  /* __OS2__ */

#ਜੇ defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_MORECORE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MORECORE 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MALLOC_ALIGNMENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_ALIGNMENT ((size_t)16U)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ  /* HAVE_MORECORE */
#ਜੇ_ਅੰਤ  /* DARWIN */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_SYS_TYPES_H
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>  /* For size_t */
#ਜੇ_ਅੰਤ  /* LACKS_SYS_TYPES_H */

/* The maximum possible size_t value has all bits set */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਮਾਪ_T           (~(size_t)0)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_LOCKS /* ensure true if spin or recursive locks set */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_LOCKS  ((defined(USE_SPIN_LOCKS) && USE_SPIN_LOCKS != 0) || \
                    (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0))
#ਜੇ_ਅੰਤ /* USE_LOCKS */

#ਜੇ USE_LOCKS /* Spin locks for gcc >= 4.1, older gcc on x86, MSC >= 1310 */
#ਜੇ ((defined(__GNUC__) &&                                              \
      ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) ||      \
       defined(__i386__) || defined(__x86_64__))) ||                    \
     (defined(_MSC_VER) && _MSC_VER>=1310))
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_SPIN_LOCKS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_SPIN_LOCKS 1
#ਜੇ_ਅੰਤ /* USE_SPIN_LOCKS */
#ਜਾਂਜੇ USE_SPIN_LOCKS
#ਗਲਤੀ "USE_SPIN_LOCKS defined without implementation"
#ਜੇ_ਅੰਤ /* ... locks available... */
#ਜਾਂਜੇ !defined(USE_SPIN_LOCKS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_SPIN_LOCKS 0
#ਜੇ_ਅੰਤ /* USE_LOCKS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ONLY_MSPACES
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONLY_MSPACES 0
#ਜੇ_ਅੰਤ  /* ONLY_MSPACES */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MSPACES
#ਜੇ ONLY_MSPACES
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSPACES 1
#ਨਹੀਂਤਾਂ   /* ONLY_MSPACES */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSPACES 0
#ਜੇ_ਅੰਤ  /* ONLY_MSPACES */
#ਜੇ_ਅੰਤ  /* MSPACES */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MALLOC_ALIGNMENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_ALIGNMENT ((size_t)(2 * ਆਕਾਰ(ਖਾਲੀ *)))
#ਜੇ_ਅੰਤ  /* MALLOC_ALIGNMENT */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FOOTERS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOOTERS 0
#ਜੇ_ਅੰਤ  /* FOOTERS */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ABORT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ABORT  abort()
#ਜੇ_ਅੰਤ  /* ABORT */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ABORT_ON_ASSERT_FAILURE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ABORT_ON_ASSERT_FAILURE 1
#ਜੇ_ਅੰਤ  /* ABORT_ON_ASSERT_FAILURE */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ PROCEED_ON_ERROR
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROCEED_ON_ERROR 0
#ਜੇ_ਅੰਤ  /* PROCEED_ON_ERROR */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INSECURE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INSECURE 0
#ਜੇ_ਅੰਤ  /* INSECURE */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MALLOC_INSPECT_ALL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_INSPECT_ALL 0
#ਜੇ_ਅੰਤ  /* MALLOC_INSPECT_ALL */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_MMAP
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MMAP 1
#ਜੇ_ਅੰਤ  /* HAVE_MMAP */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MMAP_CLEARS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_CLEARS 1
#ਜੇ_ਅੰਤ  /* MMAP_CLEARS */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_MREMAP
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ linux
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MREMAP 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GNU_ਸਰੋਤ /* Turns on mremap() definition */
#ਨਹੀਂਤਾਂ   /* linux */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MREMAP 0
#ਜੇ_ਅੰਤ  /* linux */
#ਜੇ_ਅੰਤ  /* HAVE_MREMAP */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MALLOC_FAILURE_ACTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_FAILURE_ACTION  errno = ENOMEM;
#ਜੇ_ਅੰਤ  /* MALLOC_FAILURE_ACTION */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_MORECORE
#ਜੇ ONLY_MSPACES
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MORECORE 0
#ਨਹੀਂਤਾਂ   /* ONLY_MSPACES */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MORECORE 1
#ਜੇ_ਅੰਤ  /* ONLY_MSPACES */
#ਜੇ_ਅੰਤ  /* HAVE_MORECORE */
#ਜੇ !HAVE_MORECORE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MORECORE_CONTIGUOUS 0
#ਨਹੀਂਤਾਂ   /* !HAVE_MORECORE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MORECORE_DEFAULT sbrk
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MORECORE_CONTIGUOUS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MORECORE_CONTIGUOUS 1
#ਜੇ_ਅੰਤ  /* MORECORE_CONTIGUOUS */
#ਜੇ_ਅੰਤ  /* HAVE_MORECORE */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEFAULT_GRANULARITY
#ਜੇ (MORECORE_CONTIGUOUS || defined(WIN32))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#ਨਹੀਂਤਾਂ   /* MORECORE_CONTIGUOUS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#ਜੇ_ਅੰਤ  /* MORECORE_CONTIGUOUS */
#ਜੇ_ਅੰਤ  /* DEFAULT_GRANULARITY */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEFAULT_TRIM_ਹੱਦ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MORECORE_CANNOT_TRIM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_TRIM_ਹੱਦ ((size_t)2U * (size_t)1024U * (size_t)1024U)
#ਨਹੀਂਤਾਂ   /* MORECORE_CANNOT_TRIM */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_TRIM_ਹੱਦ MAX_ਮਾਪ_T
#ਜੇ_ਅੰਤ  /* MORECORE_CANNOT_TRIM */
#ਜੇ_ਅੰਤ  /* DEFAULT_TRIM_THRESHOLD */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEFAULT_MMAP_ਹੱਦ
#ਜੇ HAVE_MMAP
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_MMAP_ਹੱਦ ((size_t)256U * (size_t)1024U)
#ਨਹੀਂਤਾਂ   /* HAVE_MMAP */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_MMAP_ਹੱਦ MAX_ਮਾਪ_T
#ਜੇ_ਅੰਤ  /* HAVE_MMAP */
#ਜੇ_ਅੰਤ  /* DEFAULT_MMAP_THRESHOLD */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAX_RELEASE_CHECK_ਦਰ
#ਜੇ HAVE_MMAP
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_RELEASE_CHECK_ਦਰ 4095
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_RELEASE_CHECK_ਦਰ MAX_ਮਾਪ_T
#ਜੇ_ਅੰਤ /* HAVE_MMAP */
#ਜੇ_ਅੰਤ /* MAX_RELEASE_CHECK_RATE */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_BUILTIN_FFS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_BUILTIN_FFS 0
#ਜੇ_ਅੰਤ  /* USE_BUILTIN_FFS */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_DEV_RANDOM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_DEV_RANDOM 0
#ਜੇ_ਅੰਤ  /* USE_DEV_RANDOM */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_MALLINFO
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NO_MALLINFO 0
#ਜੇ_ਅੰਤ  /* NO_MALLINFO */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MALLINFO_ਖੇਤਰ_TYPE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLINFO_ਖੇਤਰ_TYPE size_t
#ਜੇ_ਅੰਤ  /* MALLINFO_FIELD_TYPE */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_MALLOC_STATS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NO_MALLOC_STATS 0
#ਜੇ_ਅੰਤ  /* NO_MALLOC_STATS */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_SEGMENT_TRAVERSAL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NO_SEGMENT_TRAVERSAL 0
#ਜੇ_ਅੰਤ /* NO_SEGMENT_TRAVERSAL */

/*
  mallopt tuning options.  SVID/XPG defines four standard parameter
  numbers for mallopt, normally defined in malloc.h.  None of these
  are used in this malloc, so setting them has no effect. But this
  malloc does support the following options.
*/

/* The system's malloc.h may have conflicting defines. */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ M_TRIM_ਹੱਦ
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ M_GRANULARITY
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ M_MMAP_ਹੱਦ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_TRIM_ਹੱਦ     (-1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_GRANULARITY        (-2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M_MMAP_ਹੱਦ     (-3)

/* ------------------------ Mallinfo declarations ------------------------ */

#ਜੇ !NO_MALLINFO
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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_USR_INCLUDE_MALLOC_H
#ਅਨ੍ਤਰ੍ਭਾਵ "/usr/include/malloc.h"
#ਨਹੀਂਤਾਂ /* HAVE_USR_INCLUDE_MALLOC_H */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STRUCT_MALLINFO_DECLARED
/* HP-UX (and others?) redefines mallinfo unless _STRUCT_MALLINFO is defined */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _STRUCT_MALLINFO
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRUCT_MALLINFO_DECLARED 1
ਬਣਤਰ mallinfo {
  MALLINFO_ਖੇਤਰ_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_ਖੇਤਰ_TYPE ordblks;  /* number of free chunks */
  MALLINFO_ਖੇਤਰ_TYPE smblks;   /* always 0 */
  MALLINFO_ਖੇਤਰ_TYPE hblks;    /* always 0 */
  MALLINFO_ਖੇਤਰ_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_ਖੇਤਰ_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_ਖੇਤਰ_TYPE fsmblks;  /* always 0 */
  MALLINFO_ਖੇਤਰ_TYPE uordblks; /* total allocated space */
  MALLINFO_ਖੇਤਰ_TYPE fordblks; /* total free space */
  MALLINFO_ਖੇਤਰ_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#ਜੇ_ਅੰਤ /* STRUCT_MALLINFO_DECLARED */
#ਜੇ_ਅੰਤ /* HAVE_USR_INCLUDE_MALLOC_H */
#ਜੇ_ਅੰਤ /* NO_MALLINFO */

/*
  Try to persuade compilers to inline. The most critical functions for
  inlining are defined as macros, so these aren't used for them.
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FORCEINLINE
  #ਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FORCEINLINE __inline __attribute__ ((always_inline))
  #ਜਾਂਜੇ defined(_MSC_VER)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FORCEINLINE __forceinline
  #ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NOINLINE
  #ਜੇ defined(__GNUC__)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NOINLINE __attribute__ ((noinline))
  #ਜਾਂਜੇ defined(_MSC_VER)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NOINLINE __declspec(noinline)
  #ਨਹੀਂਤਾਂ
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NOINLINE
  #ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FORCEINLINE
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FORCEINLINE ਸਤਰਅੰਦਰ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* __cplusplus */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FORCEINLINE
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FORCEINLINE
#ਜੇ_ਅੰਤ

#ਜੇ !ONLY_MSPACES

/* ------------------- Declarations of public routines ------------------- */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE_DL_ਅਗੇਤਰ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlcalloc               ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlfree                 ਮੁਕਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc               ਸਮ੍ਰਿਤੀ_ਦਿਓ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmemalign             memalign
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlposix_memalign       posix_memalign
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlrealloc              realloc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlrealloc_ਵਿੱਚ_ਰੱਖੋ     realloc_ਵਿੱਚ_ਰੱਖੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlvalloc               valloc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlpvalloc              pvalloc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmallinfo             mallinfo
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmallopt              mallopt
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_trim          malloc_trim
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_stats         malloc_stats
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_usable_ਮਾਪ   malloc_usable_ਮਾਪ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_footprint     malloc_footprint
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_max_footprint malloc_max_footprint
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_footprint_ਹੱਦਬੰਦੀ malloc_footprint_ਹੱਦਬੰਦੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_ਸੈੱਟ_footprint_ਹੱਦਬੰਦੀ malloc_ਸੈੱਟ_footprint_ਹੱਦਬੰਦੀ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlmalloc_inspect_all   malloc_inspect_all
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlindependent_calloc   independent_calloc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlindependent_comalloc independent_comalloc
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dlbulk_free            bulk_free
#ਜੇ_ਅੰਤ /* USE_DL_PREFIX */

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
DLMALLOC_EXPORT ਖਾਲੀ* dlmalloc(size_t);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
DLMALLOC_EXPORT ਖਾਲੀ  dlfree(ਖਾਲੀ*);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
DLMALLOC_EXPORT ਖਾਲੀ* dlcalloc(size_t, size_t);

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
DLMALLOC_EXPORT ਖਾਲੀ* dlrealloc(ਖਾਲੀ*, size_t);

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
DLMALLOC_EXPORT ਖਾਲੀ* dlrealloc_ਵਿੱਚ_ਰੱਖੋ(ਖਾਲੀ*, size_t);

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
DLMALLOC_EXPORT ਖਾਲੀ* dlmemalign(size_t, size_t);

/*
  int posix_memalign(void** pp, size_t alignment, size_t n);
  Allocates a chunk of n bytes, aligned in accord with the alignment
  argument. Differs from memalign only in that it (1) assigns the
  allocated memory to *pp rather than returning it, (2) fails and
  returns EINVAL if the alignment is not a power of two (3) fails and
  returns ENOMEM if memory cannot be allocated.
*/
DLMALLOC_EXPORT ਪੂਰਨ dlposix_memalign(ਖਾਲੀ**, size_t, size_t);

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
DLMALLOC_EXPORT ਖਾਲੀ* dlvalloc(size_t);

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
DLMALLOC_EXPORT ਪੂਰਨ dlmallopt(ਪੂਰਨ, ਪੂਰਨ);

/*
  malloc_footprint();
  Returns the number of bytes obtained from the system.  The total
  number of bytes allocated by malloc, realloc etc., is less than this
  value. Unlike mallinfo, this function returns only a precomputed
  result, so can be called frequently to monitor memory consumption.
  Even if locks are otherwise defined, this function does not use them,
  so results might not be up to date.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint(ਖਾਲੀ);

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
DLMALLOC_EXPORT size_t dlmalloc_max_footprint(ਖਾਲੀ);

/*
  malloc_footprint_limit();
  Returns the number of bytes that the heap is allowed to obtain from
  the system, returning the last value returned by
  malloc_set_footprint_limit, or the maximum size_t value if
  never set. The returned value reflects a permission. There is no
  guarantee that this number of bytes can actually be obtained from
  the system.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint_ਹੱਦਬੰਦੀ();

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
DLMALLOC_EXPORT size_t dlmalloc_ਸੈੱਟ_footprint_ਹੱਦਬੰਦੀ(size_t ਬਾਈਟਾਂ);

#ਜੇ MALLOC_INSPECT_ALL
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
DLMALLOC_EXPORT ਖਾਲੀ dlmalloc_inspect_all(ਖਾਲੀ(*handler)(ਖਾਲੀ*, ਖਾਲੀ *, size_t, ਖਾਲੀ*),
                           ਖਾਲੀ* arg);

#ਜੇ_ਅੰਤ /* MALLOC_INSPECT_ALL */

#ਜੇ !NO_MALLINFO
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
DLMALLOC_EXPORT ਬਣਤਰ mallinfo dlmallinfo(ਖਾਲੀ);
#ਜੇ_ਅੰਤ /* NO_MALLINFO */

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
DLMALLOC_EXPORT ਖਾਲੀ** dlindependent_calloc(size_t, size_t, ਖਾਲੀ**);

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
DLMALLOC_EXPORT ਖਾਲੀ** dlindependent_comalloc(size_t, size_t*, ਖਾਲੀ**);

/*
  bulk_free(void* array[], size_t n_elements)
  Frees and clears (sets to null) each non-null pointer in the given
  array.  This is likely to be faster than freeing them one-by-one.
  If footers are used, pointers that have been allocated in different
  mspaces are not freed or cleared, and the count of all such pointers
  is returned.  For large arrays of pointers with poor locality, it
  may be worthwhile to sort this array before calling bulk_free.
*/
DLMALLOC_EXPORT size_t  dlbulk_free(ਖਾਲੀ**, size_t n_elements);

/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
DLMALLOC_EXPORT ਖਾਲੀ*  dlpvalloc(size_t);

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
DLMALLOC_EXPORT ਪੂਰਨ  dlmalloc_trim(size_t);

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
DLMALLOC_EXPORT ਖਾਲੀ  dlmalloc_stats(ਖਾਲੀ);

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
size_t dlmalloc_usable_ਮਾਪ(ਖਾਲੀ*);

#ਜੇ_ਅੰਤ /* ONLY_MSPACES */

#ਜੇ MSPACES

/*
  mspace is an opaque type representing an independent
  region of space that supports mspace_malloc, etc.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ* mspace;

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
DLMALLOC_EXPORT mspace create_mspace(size_t ਸਮਰੱਥਾ, ਪੂਰਨ locked);

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
DLMALLOC_EXPORT mspace create_mspace_with_ਅਧਾਰ(ਖਾਲੀ* ਅਧਾਰ, size_t ਸਮਰੱਥਾ, ਪੂਰਨ locked);

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
DLMALLOC_EXPORT ਪੂਰਨ mspace_track_large_chunks(mspace msp, ਪੂਰਨ enable);


/*
  mspace_malloc behaves as malloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT ਖਾਲੀ* mspace_malloc(mspace msp, size_t ਬਾਈਟਾਂ);

/*
  mspace_free behaves as free, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_free is not actually needed.
  free may be called instead of mspace_free because freed chunks from
  any space are handled by their originating spaces.
*/
DLMALLOC_EXPORT ਖਾਲੀ mspace_free(mspace msp, ਖਾਲੀ* mem);

/*
  mspace_realloc behaves as realloc, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_realloc is not actually
  needed.  realloc may be called instead of mspace_realloc because
  realloced chunks from any space are handled by their originating
  spaces.
*/
DLMALLOC_EXPORT ਖਾਲੀ* mspace_realloc(mspace msp, ਖਾਲੀ* mem, size_t newsize);

/*
  mspace_calloc behaves as calloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT ਖਾਲੀ* mspace_calloc(mspace msp, size_t n_elements, size_t elem_ਮਾਪ);

/*
  mspace_memalign behaves as memalign, but operates within
  the given space.
*/
DLMALLOC_EXPORT ਖਾਲੀ* mspace_memalign(mspace msp, size_t alignment, size_t ਬਾਈਟਾਂ);

/*
  mspace_independent_calloc behaves as independent_calloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT ਖਾਲੀ** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_ਮਾਪ, ਖਾਲੀ* chunks[]);

/*
  mspace_independent_comalloc behaves as independent_comalloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT ਖਾਲੀ** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], ਖਾਲੀ* chunks[]);

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


#ਜੇ !NO_MALLINFO
/*
  mspace_mallinfo behaves as mallinfo, but reports properties of
  the given space.
*/
DLMALLOC_EXPORT ਬਣਤਰ mallinfo mspace_mallinfo(mspace msp);
#ਜੇ_ਅੰਤ /* NO_MALLINFO */

/*
  malloc_usable_size(void* p) behaves the same as malloc_usable_size;
*/
DLMALLOC_EXPORT size_t mspace_usable_ਮਾਪ(ਅਚਲ ਖਾਲੀ* mem);

/*
  mspace_malloc_stats behaves as malloc_stats, but reports
  properties of the given space.
*/
DLMALLOC_EXPORT ਖਾਲੀ mspace_malloc_stats(mspace msp);

/*
  mspace_trim behaves as malloc_trim, but
  operates within the given space.
*/
DLMALLOC_EXPORT ਪੂਰਨ mspace_trim(mspace msp, size_t pad);

/*
  An alias for mallopt.
*/
DLMALLOC_EXPORT ਪੂਰਨ mspace_mallopt(ਪੂਰਨ, ਪੂਰਨ);

#ਜੇ_ਅੰਤ /* MSPACES */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}  /* end of extern "C" */
#ਜੇ_ਅੰਤ /* __cplusplus */

/*
  ========================================================================
  To make a fully customizable malloc.h header file, cut everything
  above this line, put into file malloc.h, edit to suit, and #include it
  on the next line, as well as in programs that use this malloc.
  ========================================================================
*/

/* #include "malloc.h" */

/*------------------------------ internal #includes ---------------------- */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਹਦਾਇਤ warning( disable : 4146 ) /* no "unsigned" warnings */
#ਜੇ_ਅੰਤ /* _MSC_VER */
#ਜੇ !NO_MALLOC_STATS
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>       /* for printing in malloc_stats */
#ਜੇ_ਅੰਤ /* NO_MALLOC_STATS */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_ERRNO_H
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>       /* for MALLOC_FAILURE_ACTION */
#ਜੇ_ਅੰਤ /* LACKS_ERRNO_H */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
#ਜੇ ABORT_ON_ASSERT_FAILURE
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ਪ੍ਰਤਿਜ੍ਞਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪ੍ਰਤਿਜ੍ਞਾ(x) ਜੇ(!(x)) ABORT
#ਨਹੀਂਤਾਂ /* ABORT_ON_ASSERT_FAILURE */
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਜੇ_ਅੰਤ /* ABORT_ON_ASSERT_FAILURE */
#ਨਹੀਂਤਾਂ  /* DEBUG */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ਪ੍ਰਤਿਜ੍ਞਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪ੍ਰਤਿਜ੍ਞਾ(x)
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG 0
#ਜੇ_ਅੰਤ /* DEBUG */
#ਜੇ !defined(WIN32) && !defined(LACKS_ਸਮਾਂ_H)
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>        /* for magic initialization */
#ਜੇ_ਅੰਤ /* WIN32 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_STDLIB_H
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>      /* for abort() */
#ਜੇ_ਅੰਤ /* LACKS_STDLIB_H */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_STRING_H
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>      /* for memset etc */
#ਜੇ_ਅੰਤ  /* LACKS_STRING_H */
#ਜੇ USE_BUILTIN_FFS
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_ਲੜੀਆਂ_H
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>     /* for ffs */
#ਜੇ_ਅੰਤ /* LACKS_STRINGS_H */
#ਜੇ_ਅੰਤ /* USE_BUILTIN_FFS */
#ਜੇ HAVE_MMAP
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_SYS_MMAN_H
/* On some versions of linux, mremap decl in mman.h needs __USE_GNU set */
#ਜੇ (defined(linux) && !defined(__USE_GNU))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USE_GNU 1
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>    /* for mmap */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ __USE_GNU
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>    /* for mmap */
#ਜੇ_ਅੰਤ /* linux */
#ਜੇ_ਅੰਤ /* LACKS_SYS_MMAN_H */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_FCNTL_H
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਜੇ_ਅੰਤ /* LACKS_FCNTL_H */
#ਜੇ_ਅੰਤ /* HAVE_MMAP */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LACKS_UNISTD_H
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>     /* for sbrk, sysconf */
#ਨਹੀਂਤਾਂ /* LACKS_UNISTD_H */
#ਜੇ !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
ਬਾਹਰੀ ਖਾਲੀ*     sbrk(ptrdiff_t);
#ਜੇ_ਅੰਤ /* FreeBSD etc */
#ਜੇ_ਅੰਤ /* LACKS_UNISTD_H */

/* Declarations for locking */
#ਜੇ USE_LOCKS
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ WIN32
#ਜੇ defined (__SVR4) && defined (__sun)  /* solaris */
#ਅਨ੍ਤਰ੍ਭਾਵ <thread.h>
#ਜਾਂਜੇ !defined(LACKS_SCHED_H)
#ਅਨ੍ਤਰ੍ਭਾਵ <sched.h>
#ਜੇ_ਅੰਤ /* solaris or LACKS_SCHED_H */
#ਜੇ (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0) || !USE_SPIN_LOCKS
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਜੇ_ਅੰਤ /* USE_RECURSIVE_LOCKS ... */
#ਜਾਂਜੇ defined(_MSC_VER)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _M_AMD64
/* These are already defined on AMD64 builds */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ /* __cplusplus */
LONG __cdecl _InterlockedCompareExchange(LONG ਅਸਥਿਰ *Dest, LONG Exchange, LONG Comp);
LONG __cdecl _InterlockedExchange(LONG ਅਸਥਿਰ *ਨਿਸ਼ਾਨਾ, LONG ਮੁੱਲ_2);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ /* __cplusplus */
#ਜੇ_ਅੰਤ /* _M_AMD64 */
#ਹਦਾਇਤ intrinsic (_InterlockedCompareExchange)
#ਹਦਾਇਤ intrinsic (_InterlockedExchange)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ interlockedcompareexchange _InterlockedCompareExchange
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ interlockedexchange _InterlockedExchange
#ਜਾਂਜੇ defined(WIN32) && defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ interlockedcompareexchange(a, b, c) __sync_val_compare_and_swap(a, c, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ interlockedexchange __sync_lock_test_and_set
#ਜੇ_ਅੰਤ /* Win32 */
#ਨਹੀਂਤਾਂ /* USE_LOCKS */
#ਜੇ_ਅੰਤ /* USE_LOCKS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ਤਾਲਾ_AT_FORK
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਤਾਲਾ_AT_FORK 0
#ਜੇ_ਅੰਤ

/* Declarations for bit scanning on win32 */
#ਜੇ defined(_MSC_VER) && _MSC_VER>=1300
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_Forward /* Try to avoid pulling in WinNT.h */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ /* __cplusplus */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_Forward_2(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਸੂਚਕਾਂਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਨਕਾਬ);
ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਸੂਚਕਾਂਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਨਕਾਬ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ /* __cplusplus */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_Forward ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_Forward_2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ_2 ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ
#ਹਦਾਇਤ intrinsic(ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_Forward_2)
#ਹਦਾਇਤ intrinsic(ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ)
#ਜੇ_ਅੰਤ /* BitScanForward */
#ਜੇ_ਅੰਤ /* defined(_MSC_VER) && _MSC_VER>=1300 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ WIN32
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef SC_ਪੰਨਾ_ਮਾਪ
#      define SC_ਪੰਨਾ_ਮਾਪ _SC_PAGESIZE
#    endif
#  endif
#  ifdef SC_ਪੰਨਾ_ਮਾਪ
#    define malloc_getpagesize sysconf(SC_ਪੰਨਾ_ਮਾਪ)
#  ਨਹੀਂਤਾਂ
#    ਜੇ defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       ਬਾਹਰੀ size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    ਨਹੀਂਤਾਂ
#      ifdef WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize()
#      ਨਹੀਂਤਾਂ
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        ਨਹੀਂਤਾਂ
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            ਨਹੀਂਤਾਂ
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          ਨਹੀਂਤਾਂ
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            ਨਹੀਂਤਾਂ
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              ਨਹੀਂਤਾਂ /* just guess */
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

/* libffi: the upstream malloc_getpagesize definition above is gated on
   !WIN32 (WIN32 builds use GetSystemInfo in init_mparams instead).  Guarantee
   the symbol is always defined so the !WIN32 init_mparams branch still compiles
   on toolchains where it is not preprocessed out as expected (e.g. MSVC).  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ malloc_getpagesize
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ malloc_getpagesize ((size_t)4096U)
#ਜੇ_ਅੰਤ

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_T_ਮਾਪ         (ਆਕਾਰ(size_t))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_T_BITSIZE      (ਆਕਾਰ(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_T_ZERO         ((size_t)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_T_ONE          ((size_t)1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_T_TWO          ((size_t)2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_T_FOUR         ((size_t)4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TWO_ਮਾਪ_T_SIZES    (ਮਾਪ_T_ਮਾਪ<<1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOUR_ਮਾਪ_T_SIZES   (ਮਾਪ_T_ਮਾਪ<<2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIX_ਮਾਪ_T_SIZES    (FOUR_ਮਾਪ_T_SIZES+TWO_ਮਾਪ_T_SIZES)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HALF_MAX_ਮਾਪ_T     (MAX_ਮਾਪ_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK_ALIGN_ਨਕਾਬ    (MALLOC_ALIGNMENT - ਮਾਪ_T_ONE)

/* True if address a has acceptable alignment */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_ਨਕਾਬ)) == 0)

/* the number of bytes to offset an address to align it */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ align_ਵਿੱਥ(A)\
 ((((size_t)(A) & CHUNK_ALIGN_ਨਕਾਬ) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_ਨਕਾਬ)) & CHUNK_ALIGN_ਨਕਾਬ))

/* -------------------------- MMAP preliminaries ------------------------- */

/*
   If HAVE_MORECORE or HAVE_MMAP are false, we just define calls and
   checks to fail so compiler optimizer can delete code rather than
   using so many "#if"s.
*/


/* MORECORE and MMAP must return MFAIL on failure */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFAIL                ((ਖਾਲੀ*)(MAX_ਮਾਪ_T))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMFAIL               ((ਅੱਖਰ*)(MFAIL)) /* defined for convenience */

#ਜੇ HAVE_MMAP

#ਜੇ !defined(WIN32) && !defined (__OS2__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUNMAP_DEFAULT(a, s)  munmap((a), (s))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_PROT            (PROT_READ|PROT_WRITE)
#ਜੇ !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAP_ANONYMOUS        MAP_ANON
#ਜੇ_ਅੰਤ /* MAP_ANON */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAP_ANONYMOUS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_FLAGS           (MAP_ਨਿੱਜੀ|MAP_ANONYMOUS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_DEFAULT(s)       mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#ਨਹੀਂਤਾਂ /* MAP_ANONYMOUS */
/*
   Nearly all versions of mmap support MAP_ANONYMOUS, so the following
   is unlikely to be needed, but is supplied just in case.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_FLAGS           (MAP_ਨਿੱਜੀ)
ਸਥਿਰ ਪੂਰਨ dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_DEFAULT(s) ((dev_zero_fd < 0) ? \
           (dev_zero_fd = open("/dev/zero", O_RDWR), \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#ਜੇ_ਅੰਤ /* MAP_ANONYMOUS */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿੱਧਾ_MMAP_DEFAULT(s) MMAP_DEFAULT(s)

#ਜਾਂਜੇ defined(__OS2__)

/* OS/2 MMAP via DosAllocMem */
ਸਥਿਰ ਖਾਲੀ* os2mmap(size_t ਮਾਪ) {
  ਖਾਲੀ* ptr;
  ਜੇ (DosAllocMem(&ptr, ਮਾਪ, OBJ_ANY|PAG_COMMIT|PAG_READ|PAG_WRITE) &&
      DosAllocMem(&ptr, ਮਾਪ, PAG_COMMIT|PAG_READ|PAG_WRITE))
    ਵਾਪਸ MFAIL;
  ਵਾਪਸ ptr;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os2direct_mmap(n)     os2mmap(n)

/* This function supports releasing coalesed segments */
ਸਥਿਰ ਪੂਰਨ os2munmap(ਖਾਲੀ* ptr, size_t ਮਾਪ) {
  ਜਦੋਂ (ਮਾਪ) {
    ULONG ul_ਮਾਪ = ਮਾਪ;
    ULONG ulFlags = 0;
    ਜੇ (DosQueryMem(ptr, &ul_ਮਾਪ, &ulFlags) != 0)
      ਵਾਪਸ -1;
    ਜੇ ((ulFlags & PAG_ਅਧਾਰ) == 0 ||(ulFlags & PAG_COMMIT) == 0 ||
        ul_ਮਾਪ > ਮਾਪ)
      ਵਾਪਸ -1;
    ਜੇ (DosFreeMem(ptr) != 0)
      ਵਾਪਸ -1;
    ptr = ( ਖਾਲੀ * ) ( ( ਅੱਖਰ * ) ptr + ul_ਮਾਪ );
    ਮਾਪ -= ul_ਮਾਪ;
  }
  ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_DEFAULT(s)         os2mmap(s)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUNMAP_DEFAULT(a, s)    os2munmap((a), (s))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿੱਧਾ_MMAP_DEFAULT(s)  os2direct_mmap(s)

#ਨਹੀਂਤਾਂ /* WIN32 */

/* Win32 MMAP via VirtualAlloc */
/* libffi: plain static (not FORCEINLINE) -- mingw's <windows.h> defines
   FORCEINLINE with an 'extern' storage class, which clashes with 'static'.  */
ਸਥਿਰ ਖਾਲੀ* win32mmap(size_t ਮਾਪ) {
  ਖਾਲੀ* ptr = VirtualAlloc(0, ਮਾਪ, MEM_RESERVE|MEM_COMMIT, ਪੰਨਾ_EXECUTE_READWRITE);
  ਵਾਪਸ (ptr != 0)? ptr: MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
ਸਥਿਰ ਖਾਲੀ* win32direct_mmap(size_t ਮਾਪ) {
  ਖਾਲੀ* ptr = VirtualAlloc(0, ਮਾਪ, MEM_RESERVE|MEM_COMMIT|MEM_ਸਿਖਰ_ਥੱਲੇ,
                           ਪੰਨਾ_EXECUTE_READWRITE);
  ਵਾਪਸ (ptr != 0)? ptr: MFAIL;
}

/* This function supports releasing coalesed segments */
ਸਥਿਰ ਪੂਰਨ win32munmap(ਖਾਲੀ* ptr, size_t ਮਾਪ) {
  ਸਮ੍ਰਿਤੀ_BASIC_INFORMATION minfo;
  ਅੱਖਰ* cptr = (ਅੱਖਰ*)ptr;
  ਜਦੋਂ (ਮਾਪ) {
    ਜੇ (VirtualQuery(cptr, &minfo, ਆਕਾਰ(minfo)) == 0)
      ਵਾਪਸ -1;
    ਜੇ (minfo.ਅਧਾਰ_ਪਤਾ != cptr || minfo.ਵੰਡ_ਅਧਾਰ != cptr ||
        minfo.State != MEM_COMMIT || minfo.Region_ਮਾਪ > ਮਾਪ)
      ਵਾਪਸ -1;
    ਜੇ (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
      ਵਾਪਸ -1;
    cptr += minfo.Region_ਮਾਪ;
    ਮਾਪ -= minfo.Region_ਮਾਪ;
  }
  ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_DEFAULT(s)             win32mmap(s)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿੱਧਾ_MMAP_DEFAULT(s)      win32direct_mmap(s)
#ਜੇ_ਅੰਤ /* WIN32 */
#ਜੇ_ਅੰਤ /* HAVE_MMAP */

#ਜੇ HAVE_MREMAP
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#ਜੇ_ਅੰਤ /* WIN32 */
#ਜੇ_ਅੰਤ /* HAVE_MREMAP */

/**
 * Define CALL_MORECORE
 */
#ਜੇ HAVE_MORECORE
    #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MORECORE
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MORECORE(S)    MORECORE(S)
    #ਨਹੀਂਤਾਂ  /* MORECORE */
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MORECORE(S)    MORECORE_DEFAULT(S)
    #ਜੇ_ਅੰਤ /* MORECORE */
#ਨਹੀਂਤਾਂ  /* HAVE_MORECORE */
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MORECORE(S)        MFAIL
#ਜੇ_ਅੰਤ /* HAVE_MORECORE */

/**
 * Define CALL_MMAP/CALL_MUNMAP/CALL_DIRECT_MMAP
 */
#ਜੇ HAVE_MMAP
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ            (ਮਾਪ_T_ONE)

    #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MMAP
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MMAP(s)        MMAP(s)
    #ਨਹੀਂਤਾਂ /* MMAP */
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MMAP(s)        MMAP_DEFAULT(s)
    #ਜੇ_ਅੰਤ /* MMAP */
    #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MUNMAP
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MUNMAP(a, s)   MUNMAP((a), (s))
    #ਨਹੀਂਤਾਂ /* MUNMAP */
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MUNMAP(a, s)   MUNMAP_DEFAULT((a), (s))
    #ਜੇ_ਅੰਤ /* MUNMAP */
    #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸਿੱਧਾ_MMAP
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_ਸਿੱਧਾ_MMAP(s) ਸਿੱਧਾ_MMAP(s)
    #ਨਹੀਂਤਾਂ /* DIRECT_MMAP */
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_ਸਿੱਧਾ_MMAP(s) ਸਿੱਧਾ_MMAP_DEFAULT(s)
    #ਜੇ_ਅੰਤ /* DIRECT_MMAP */
#ਨਹੀਂਤਾਂ  /* HAVE_MMAP */
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ            (ਮਾਪ_T_ZERO)

    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP(s)                 MFAIL
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MUNMAP(a, s)            (-1)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿੱਧਾ_MMAP(s)          MFAIL
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_ਸਿੱਧਾ_MMAP(s)     ਸਿੱਧਾ_MMAP(s)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MMAP(s)            MMAP(s)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MUNMAP(a, s)       MUNMAP((a), (s))
#ਜੇ_ਅੰਤ /* HAVE_MMAP */

/**
 * Define CALL_MREMAP
 */
#ਜੇ HAVE_MMAP && HAVE_MREMAP
    #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MREMAP
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
    #ਨਹੀਂਤਾਂ /* MREMAP */
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MREMAP(addr, osz, nsz, mv) MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
    #ਜੇ_ਅੰਤ /* MREMAP */
#ਨਹੀਂਤਾਂ  /* HAVE_MMAP && HAVE_MREMAP */
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#ਜੇ_ਅੰਤ /* HAVE_MMAP && HAVE_MREMAP */

/* mstate bit set if contiguous morecore disabled or failed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_NONCONTIGUOUS_ਦੋਆਧਾਰੀ_ਅੰਕ (4U)

/* segment bit set in create_mspace_with_base */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXTERN_ਦੋਆਧਾਰੀ_ਅੰਕ            (8U)


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

#ਜੇ !USE_LOCKS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ               (0U)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITIAL_ਤਾਲਾ(l)            (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DESTROY_ਤਾਲਾ(l)            (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ACQUIRE_MALLOC_GLOBAL_ਤਾਲਾ()
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASE_MALLOC_GLOBAL_ਤਾਲਾ()

#ਨਹੀਂਤਾਂ
#ਜੇ USE_LOCKS > 1
/* -----------------------  User-defined locks ------------------------ */
/* Define your own lock implementation here */
/* #define INITIAL_LOCK(lk)  ... */
/* #define DESTROY_LOCK(lk)  ... */
/* #define ACQUIRE_LOCK(lk)  ... */
/* #define RELEASE_LOCK(lk)  ... */
/* #define TRY_LOCK(lk) ... */
/* static MLOCK_T malloc_global_mutex = ... */

#ਜਾਂਜੇ USE_SPIN_LOCKS

/* First, define CAS_LOCK and CLEAR_LOCK on ints */
/* Note CAS_LOCK defined to return 0 on success */

#ਜੇ defined(__GNUC__)&& (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAS_ਤਾਲਾ(sl)     __sync_lock_test_and_set(sl, 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CLEAR_ਤਾਲਾ(sl)   __sync_lock_release(sl)

#ਜਾਂਜੇ (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
/* Custom spin locks for older gcc on x86 */
ਸਥਿਰ FORCEINLINE ਪੂਰਨ x86_cas_ਤਾਲਾ(ਪੂਰਨ *sl) {
  ਪੂਰਨ ret;
  ਪੂਰਨ val = 1;
  ਪੂਰਨ cmp = 0;
  __asm__ __volatile__  ("lock; cmpxchgl %1, %2"
                         : "=a" (ret)
                         : "r" (val), "m" (*(sl)), "0"(cmp)
                         : "memory", "cc");
  ਵਾਪਸ ret;
}

ਸਥਿਰ FORCEINLINE ਖਾਲੀ x86_clear_ਤਾਲਾ(ਪੂਰਨ* sl) {
  ਪ੍ਰਤਿਜ੍ਞਾ(*sl != 0);
  ਪੂਰਨ prev = 0;
  ਪੂਰਨ ret;
  __asm__ __volatile__ ("lock; xchgl %0, %1"
                        : "=r" (ret)
                        : "m" (*(sl)), "0"(prev)
                        : "memory");
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAS_ਤਾਲਾ(sl)     x86_cas_ਤਾਲਾ(sl)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CLEAR_ਤਾਲਾ(sl)   x86_clear_ਤਾਲਾ(sl)

#ਨਹੀਂਤਾਂ /* Win32 MSC */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAS_ਤਾਲਾ(sl)     interlockedexchange((LONG ਅਸਥਿਰ *)(sl), (LONG)1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CLEAR_ਤਾਲਾ(sl)   interlockedexchange ((LONG ਅਸਥਿਰ *)(sl), (LONG)0)

#ਜੇ_ਅੰਤ /* ... gcc spins locks ... */

/* How to yield for a spin lock */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPINS_ਪ੍ਰਤੀ_YIELD       63
#ਜੇ defined(_MSC_VER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SLEEP_EX_DURATION     50 /* delay for yield/sleep */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPIN_ਤਾਲਾ_YIELD  SleepEx(SLEEP_EX_DURATION, FALSE)
#ਜਾਂਜੇ defined (__SVR4) && defined (__sun) /* solaris */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPIN_ਤਾਲਾ_YIELD   thr_yield();
#ਜਾਂਜੇ !defined(LACKS_SCHED_H)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPIN_ਤਾਲਾ_YIELD   sched_yield();
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPIN_ਤਾਲਾ_YIELD
#ਜੇ_ਅੰਤ /* ... yield ... */

/* libffi: read the spin-lock word with a relaxed atomic load for the lock-free
   "is it held?" peek, so ThreadSanitizer does not flag it as racing with the
   atomic CLEAR_LOCK release on unlock.  Relaxed ordering is sufficient: the
   peek is only a hint to avoid a costly CAS, and CAS_LOCK still provides the
   acquire ordering on success.  Falls back to the original volatile read when
   atomic builtins are unavailable.  */
#ਜੇ defined(__ATOMIC_RELAXED)
# define ffi_spin_peek(sl) __atomic_load_n((sl), __ATOMIC_RELAXED)
#ਜਾਂਜੇ defined(_MSC_VER)
# define ffi_spin_peek(sl) \
  ((ਪੂਰਨ)_InterlockedCompareExchange((LONG ਅਸਥਿਰ *)(sl), (LONG)0, (LONG)0))
#ਨਹੀਂਤਾਂ
# define ffi_spin_peek(sl) (*(ਅਸਥਿਰ ਪੂਰਨ *)(sl))
#ਜੇ_ਅੰਤ

#ਜੇ !defined(USE_RECURSIVE_LOCKS) || USE_RECURSIVE_LOCKS == 0
/* Plain spin locks use single word (embedded in malloc_states) */
ਸਥਿਰ ਪੂਰਨ spin_acquire_ਤਾਲਾ(ਪੂਰਨ *sl) {
  ਪੂਰਨ spins = 0;
  ਜਦੋਂ (ffi_spin_peek(sl) != 0 || CAS_ਤਾਲਾ(sl)) {
    ਜੇ ((++spins & SPINS_ਪ੍ਰਤੀ_YIELD) == 0) {
      SPIN_ਤਾਲਾ_YIELD;
    }
  }
  ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MLOCK_T               ਪੂਰਨ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRY_ਤਾਲਾ(sl)          !CAS_ਤਾਲਾ(sl)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASE_ਤਾਲਾ(sl)      CLEAR_ਤਾਲਾ(sl)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ACQUIRE_ਤਾਲਾ(sl)      (CAS_ਤਾਲਾ(sl)? spin_acquire_ਤਾਲਾ(sl) : 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITIAL_ਤਾਲਾ(sl)      (*sl = 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DESTROY_ਤਾਲਾ(sl)      (0)
ਸਥਿਰ MLOCK_T malloc_global_mutex = 0;

#ਨਹੀਂਤਾਂ /* USE_RECURSIVE_LOCKS */
/* types for lock owners */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THREAD_ID_T           DWORD
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CURRENT_THREAD        GetCurrentThreadId()
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EQ_OWNER(X,Y)         ((X) == (Y))
#ਨਹੀਂਤਾਂ
/*
  Note: the following assume that pthread_t is a type that can be
  initialized to (casted) zero. If this is not the case, you will need to
  somehow redefine these or not use spin locks.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ THREAD_ID_T           pthread_t
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CURRENT_THREAD        pthread_self()
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EQ_OWNER(X,Y)         pthread_equal(X, Y)
#ਜੇ_ਅੰਤ

ਬਣਤਰ malloc_recursive_ਤਾਲਾ {
  ਪੂਰਨ sl;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c;
  THREAD_ID_T threadid;
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MLOCK_T  ਬਣਤਰ malloc_recursive_ਤਾਲਾ
ਸਥਿਰ MLOCK_T malloc_global_mutex = { 0, 0, (THREAD_ID_T)0};

ਸਥਿਰ FORCEINLINE ਖਾਲੀ recursive_release_ਤਾਲਾ(MLOCK_T *lk) {
  ਪ੍ਰਤਿਜ੍ਞਾ(lk->sl != 0);
  ਜੇ (--lk->c == 0) {
    CLEAR_ਤਾਲਾ(&lk->sl);
  }
}

ਸਥਿਰ FORCEINLINE ਪੂਰਨ recursive_acquire_ਤਾਲਾ(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  ਪੂਰਨ spins = 0;
  ਲਈ (;;) {
    ਜੇ (ffi_spin_peek(&lk->sl) == 0) {
      ਜੇ (!CAS_ਤਾਲਾ(&lk->sl)) {
        lk->threadid = mythreadid;
        lk->c = 1;
        ਵਾਪਸ 0;
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ (EQ_OWNER(lk->threadid, mythreadid)) {
      ++lk->c;
      ਵਾਪਸ 0;
    }
    ਜੇ ((++spins & SPINS_ਪ੍ਰਤੀ_YIELD) == 0) {
      SPIN_ਤਾਲਾ_YIELD;
    }
  }
}

ਸਥਿਰ FORCEINLINE ਪੂਰਨ recursive_try_ਤਾਲਾ(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  ਜੇ (ffi_spin_peek(&lk->sl) == 0) {
    ਜੇ (!CAS_ਤਾਲਾ(&lk->sl)) {
      lk->threadid = mythreadid;
      lk->c = 1;
      ਵਾਪਸ 1;
    }
  }
  ਨਹੀਂਤਾਂ ਜੇ (EQ_OWNER(lk->threadid, mythreadid)) {
    ++lk->c;
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASE_ਤਾਲਾ(lk)      recursive_release_ਤਾਲਾ(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRY_ਤਾਲਾ(lk)          recursive_try_ਤਾਲਾ(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ACQUIRE_ਤਾਲਾ(lk)      recursive_acquire_ਤਾਲਾ(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITIAL_ਤਾਲਾ(lk)      ((lk)->threadid = (THREAD_ID_T)0, (lk)->sl = 0, (lk)->c = 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DESTROY_ਤਾਲਾ(lk)      (0)
#ਜੇ_ਅੰਤ /* USE_RECURSIVE_LOCKS */

#ਜਾਂਜੇ defined(WIN32) /* Win32 critical sections */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MLOCK_T               CRITICAL_SECTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ACQUIRE_ਤਾਲਾ(lk)      (EnterCriticalSection(lk), 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASE_ਤਾਲਾ(lk)      LeaveCriticalSection(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRY_ਤਾਲਾ(lk)          TryEnterCriticalSection(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITIAL_ਤਾਲਾ(lk)      (!Initialize_Critical_Section_And_Spin_ਗਿਣਤੀ((lk), 0x80000000|4000))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DESTROY_ਤਾਲਾ(lk)      (DeleteCriticalSection(lk), 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NEED_GLOBAL_ਤਾਲਾ_INIT

ਸਥਿਰ MLOCK_T malloc_global_mutex;
ਸਥਿਰ ਅਸਥਿਰ LONG malloc_global_mutex_ਸਥਿਤੀ;

/* Use spin loop to initialize global lock */
ਸਥਿਰ ਖਾਲੀ init_malloc_global_mutex() {
  ਲਈ (;;) {
    ਲੰਮਾ stat = malloc_global_mutex_ਸਥਿਤੀ;
    ਜੇ (stat > 0)
      ਵਾਪਸ;
    /* transition to < 0 while initializing, then to > 0) */
    ਜੇ (stat == 0 &&
        interlockedcompareexchange(&malloc_global_mutex_ਸਥਿਤੀ, (LONG)-1, (LONG)0) == 0) {
      InitializeCriticalSection(&malloc_global_mutex);
      interlockedexchange(&malloc_global_mutex_ਸਥਿਤੀ, (LONG)1);
      ਵਾਪਸ;
    }
    SleepEx(0, FALSE);
  }
}

#ਨਹੀਂਤਾਂ /* pthreads-based locks */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MLOCK_T               pthread_mutex_t
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ACQUIRE_ਤਾਲਾ(lk)      pthread_mutex_ਤਾਲਾ(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASE_ਤਾਲਾ(lk)      pthread_mutex_unlock(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRY_ਤਾਲਾ(lk)          (!pthread_mutex_trylock(lk))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITIAL_ਤਾਲਾ(lk)      pthread_init_ਤਾਲਾ(lk)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DESTROY_ਤਾਲਾ(lk)      pthread_mutex_destroy(lk)

#ਜੇ defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0 && defined(linux) && !defined(PTHREAD_MUTEX_RECURSIVE)
/* Cope with old-style linux recursive lock initialization by adding */
/* skipped internal declaration from pthread.h */
ਬਾਹਰੀ ਪੂਰਨ pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
                                              ਪੂਰਨ __kind));
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pthread_mutexattr_settype(x,y) pthread_mutexattr_setkind_np(x,y)
#ਜੇ_ਅੰਤ /* USE_RECURSIVE_LOCKS ... */

ਸਥਿਰ MLOCK_T malloc_global_mutex = PTHREAD_MUTEX_INITIALIZER;

ਸਥਿਰ ਪੂਰਨ pthread_init_ਤਾਲਾ (MLOCK_T *lk) {
  pthread_mutexattr_t attr;
  ਜੇ (pthread_mutexattr_init(&attr)) ਵਾਪਸ 1;
#ਜੇ defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0
  ਜੇ (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
  ਜੇ (pthread_mutex_init(lk, &attr)) ਵਾਪਸ 1;
  ਜੇ (pthread_mutexattr_destroy(&attr)) ਵਾਪਸ 1;
  ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* ... lock types ... */

/* Common code for all lock types */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ               (2U)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ACQUIRE_MALLOC_GLOBAL_ਤਾਲਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ACQUIRE_MALLOC_GLOBAL_ਤਾਲਾ()  ACQUIRE_ਤਾਲਾ(&malloc_global_mutex);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ RELEASE_MALLOC_GLOBAL_ਤਾਲਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RELEASE_MALLOC_GLOBAL_ਤਾਲਾ()  RELEASE_ਤਾਲਾ(&malloc_global_mutex);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* USE_LOCKS */

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

ਬਣਤਰ malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               ਸਿਰ;       /* Size and inuse bits. */
  ਬਣਤਰ malloc_chunk* fd;         /* double links -- used only if free. */
  ਬਣਤਰ malloc_chunk* bk;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_chunk  mchunk;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_chunk* mchunkptr;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_chunk* sbinptr;  /* The type of bins of chunks */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ bindex_t;         /* Described below */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ binmap_t;         /* Described below */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MCHUNK_ਮਾਪ         (ਆਕਾਰ(mchunk))

#ਜੇ FOOTERS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK_OVERHEAD      (TWO_ਮਾਪ_T_SIZES)
#ਨਹੀਂਤਾਂ /* FOOTERS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK_OVERHEAD      (ਮਾਪ_T_ਮਾਪ)
#ਜੇ_ਅੰਤ /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_CHUNK_OVERHEAD (TWO_ਮਾਪ_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MMAP_FOOT_PAD       (FOUR_ਮਾਪ_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_CHUNK_ਮਾਪ\
  ((MCHUNK_ਮਾਪ + CHUNK_ALIGN_ਨਕਾਬ) & ~CHUNK_ALIGN_ਨਕਾਬ)

/* conversion from malloc headers to user pointers, and back */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunk2mem(p)        ((ਖਾਲੀ*)((ਅੱਖਰ*)(p)       + TWO_ਮਾਪ_T_SIZES))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mem2chunk(mem)      ((mchunkptr)((ਅੱਖਰ*)(mem) - TWO_ਮਾਪ_T_SIZES))
/* chunk associated with aligned address A */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ align_as_chunk(A)   (mchunkptr)((A) + align_ਵਿੱਥ(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_REQUEST         ((-MIN_CHUNK_ਮਾਪ) << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_REQUEST         (MIN_CHUNK_ਮਾਪ - CHUNK_OVERHEAD - ਮਾਪ_T_ONE)

/* pad request bytes into a usable size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pad_request(req) \
   (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_ਨਕਾਬ) & ~CHUNK_ALIGN_ਨਕਾਬ)

/* pad request, checking for minimum (but not maximum) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ request2size(req) \
  (((req) < MIN_REQUEST)? MIN_CHUNK_ਮਾਪ : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ          (ਮਾਪ_T_ONE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ          (ਮਾਪ_T_TWO)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG4_ਦੋਆਧਾਰੀ_ਅੰਕ           (ਮਾਪ_T_FOUR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INUSE_ਬਿੱਟ          (PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ਬਿੱਟ           (PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ|FLAG4_ਦੋਆਧਾਰੀ_ਅੰਕ)

/* Head value for fenceposts */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FENCEPOST_ਸਿਰ      (INUSE_ਬਿੱਟ|ਮਾਪ_T_ਮਾਪ)

/* extraction of fields from head words */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cinuse(p)           ((p)->ਸਿਰ & CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pinuse(p)           ((p)->ਸਿਰ & PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ flag4inuse(p)       ((p)->ਸਿਰ & FLAG4_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_inuse(p)         (((p)->ਸਿਰ & INUSE_ਬਿੱਟ) != PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_mmapped(p)       (((p)->ਸਿਰ & INUSE_ਬਿੱਟ) == 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunksize(p)        ((p)->ਸਿਰ & ~(FLAG_ਬਿੱਟ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_pinuse(p)     ((p)->ਸਿਰ &= ~PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_flag4(p)        ((p)->ਸਿਰ |= FLAG4_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_flag4(p)      ((p)->ਸਿਰ &= ~FLAG4_ਦੋਆਧਾਰੀ_ਅੰਕ)

/* Treat space at ptr +/- offset as a chunk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunk_plus_ਵਿੱਥ(p, s)  ((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunk_minus_ਵਿੱਥ(p, s) ((mchunkptr)(((ਅੱਖਰ*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_chunk(p) ((mchunkptr)( ((ਅੱਖਰ*)(p)) + ((p)->ਸਿਰ & ~FLAG_ਬਿੱਟ)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ prev_chunk(p) ((mchunkptr)( ((ਅੱਖਰ*)(p)) - ((p)->prev_foot) ))

/* extract next chunk's pinuse bit */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_pinuse(p)  ((ਅਗਲਾ_chunk(p)->ਸਿਰ) & PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)

/* Get/set size at footer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get_foot(p, s)  (((mchunkptr)((ਅੱਖਰ*)(p) + (s)))->prev_foot)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_foot(p, s)  (((mchunkptr)((ਅੱਖਰ*)(p) + (s)))->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, s)\
  ((p)->ਸਿਰ = (s|PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ), ਸੈੱਟ_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ overhead_for(p)\
 (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)

/* Return true if malloced space is not necessarily cleared */
#ਜੇ MMAP_CLEARS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ calloc_must_clear(p) (!is_mmapped(p))
#ਨਹੀਂਤਾਂ /* MMAP_CLEARS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ calloc_must_clear(p) (1)
#ਜੇ_ਅੰਤ /* MMAP_CLEARS */

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

ਬਣਤਰ malloc_ਰੁੱਖ_chunk {
  /* The first four fields must be compatible with malloc_chunk */
  size_t                    prev_foot;
  size_t                    ਸਿਰ;
  ਬਣਤਰ malloc_ਰੁੱਖ_chunk* fd;
  ਬਣਤਰ malloc_ਰੁੱਖ_chunk* bk;

  ਬਣਤਰ malloc_ਰੁੱਖ_chunk* ਬੱਚਾ[2];
  ਬਣਤਰ malloc_ਰੁੱਖ_chunk* ਮਾਪਾ;
  bindex_t                  ਸੂਚਕਾਂਕ;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_ਰੁੱਖ_chunk  tchunk;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_ਰੁੱਖ_chunk* tchunkptr;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_ਰੁੱਖ_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ leftmost_ਬੱਚਾ(t) ((t)->ਬੱਚਾ[0] != 0? (t)->ਬੱਚਾ[0] : (t)->ਬੱਚਾ[1])

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

ਬਣਤਰ malloc_segment {
  ਅੱਖਰ*        ਅਧਾਰ;             /* base address */
  size_t       ਮਾਪ;             /* allocated size */
  ਬਣਤਰ malloc_segment* ਅਗਲਾ;   /* ptr to next segment */
#ਜੇ FFI_MMAP_EXEC_WRIT
  /* The mmap magic is supposed to store the address of the executable
     segment at the very end of the requested block.  */

# define mmap_exec_ਵਿੱਥ(b,s) (*(ptrdiff_t*)((b)+(s)-ਆਕਾਰ(ptrdiff_t)))

  /* We can only merge segments if their corresponding executable
     segments are at identical offsets.  */
# define check_segment_ਮਿਲਾਓ(S,b,s) \
  (mmap_exec_ਵਿੱਥ((b),(s)) == (S)->exec_ਵਿੱਥ)

# define ਜੋੜੋ_segment_exec_ਵਿੱਥ(p,S) ((ਅੱਖਰ*)(p) + (S)->exec_ਵਿੱਥ)
# define sub_segment_exec_ਵਿੱਥ(p,S) ((ਅੱਖਰ*)(p) - (S)->exec_ਵਿੱਥ)

  /* The removal of sflags only works with HAVE_MORECORE == 0.  */

# define get_segment_flags(S)   (USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ)
# define ਸੈੱਟ_segment_flags(S,v) \
  (((v) != USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ) ? (ABORT, (v)) :				\
   (((S)->exec_ਵਿੱਥ =							\
     mmap_exec_ਵਿੱਥ((S)->ਅਧਾਰ, (S)->ਮਾਪ)),				\
    (mmap_exec_ਵਿੱਥ((S)->ਅਧਾਰ + (S)->exec_ਵਿੱਥ, (S)->ਮਾਪ) !=	\
     (S)->exec_ਵਿੱਥ) ? (ABORT, (v)) :					\
   (mmap_exec_ਵਿੱਥ((S)->ਅਧਾਰ, (S)->ਮਾਪ) = 0), (v)))

  /* We use an offset here, instead of a pointer, because then, when
     base changes, we don't have to modify this.  On architectures
     with segmented addresses, this might not work.  */
  ptrdiff_t    exec_ਵਿੱਥ;
#ਨਹੀਂਤਾਂ

# define get_segment_flags(S)   ((S)->sflags)
# define ਸੈੱਟ_segment_flags(S,v) ((S)->sflags = (v))
# define check_segment_ਮਿਲਾਓ(S,b,s) (1)

  flag_t       sflags;           /* mmap and extern flag */
#ਜੇ_ਅੰਤ
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_mmapped_segment(S)  (get_segment_flags(S) & USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_extern_segment(S)   (get_segment_flags(S) & EXTERN_ਦੋਆਧਾਰੀ_ਅੰਕ)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_segment  msegment;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_segment* msegmentptr;

/* ---------------------------- malloc_state ----------------------------- */

/*
   A malloc_state holds all of the bookkeeping for a space.
   The main fields are:

ਸਿਖਰ
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NSMALLBINS        (32U)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NTREEBINS         (32U)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALLBIN_ਖਿਸਕਾਓ    (3U)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALLBIN_ਚੌੜਾਈ    (ਮਾਪ_T_ONE << SMALLBIN_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TREEBIN_ਖਿਸਕਾਓ     (8U)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_LARGE_ਮਾਪ    (ਮਾਪ_T_ONE << TREEBIN_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SMALL_ਮਾਪ    (MIN_LARGE_ਮਾਪ - ਮਾਪ_T_ONE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SMALL_REQUEST (MAX_SMALL_ਮਾਪ - CHUNK_ALIGN_ਨਕਾਬ - CHUNK_OVERHEAD)

ਬਣਤਰ malloc_state {
  binmap_t   smallmap;
  binmap_t   treemap;
  size_t     dvsize;
  size_t     topsize;
  ਅੱਖਰ*      ਘੱਟ_ਤੋਂ_ਘੱਟ_addr;
  mchunkptr  dv;
  mchunkptr  ਸਿਖਰ;
  size_t     trim_check;
  size_t     release_checks;
  size_t     magic;
  mchunkptr  smallbins[(NSMALLBINS+1)*2];
  tbinptr    treebins[NTREEBINS];
  size_t     footprint;
  size_t     max_footprint;
  size_t     footprint_ਹੱਦਬੰਦੀ; /* zero means no limit */
  flag_t     mflags;
#ਜੇ USE_LOCKS
  MLOCK_T    mutex;     /* locate lock among fields that rarely change */
#ਜੇ_ਅੰਤ /* USE_LOCKS */
  msegment   seg;
  ਖਾਲੀ*      extp;      /* Unused but available for extensions */
  size_t     exts;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

ਬਣਤਰ malloc_params {
  size_t magic;
  size_t ਪੰਨਾ_ਮਾਪ;
  size_t granularity;
  size_t mmap_ਹੱਦ;
  size_t trim_ਹੱਦ;
  flag_t default_mflags;
};

ਸਥਿਰ ਬਣਤਰ malloc_params mparams;

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
#ਜੇ USE_LOCKS && defined(__ATOMIC_ACQUIRE)
# define ffi_mparams_magic_ਲੱਦੋ()   __atomic_load_n(&mparams.magic, __ATOMIC_ACQUIRE)
# define ffi_mparams_magic_store(v) __atomic_store_n(&mparams.magic, (size_t)(v), __ATOMIC_RELEASE)
#ਜਾਂਜੇ USE_LOCKS && defined(_MSC_VER)
# ਜੇ defined(_WIN64)
#  define ffi_mparams_magic_ਲੱਦੋ() \
  ((size_t)_InterlockedCompareExchange64((__int64 ਅਸਥਿਰ *)&mparams.magic, \
                                         (__int64)0, (__int64)0))
#  define ffi_mparams_magic_store(v) \
  ((ਖਾਲੀ)_InterlockedExchange64((__int64 ਅਸਥਿਰ *)&mparams.magic, \
                                (__int64)(v)))
# ਨਹੀਂਤਾਂ
#  define ffi_mparams_magic_ਲੱਦੋ() \
  ((size_t)_InterlockedCompareExchange((LONG ਅਸਥਿਰ *)&mparams.magic, \
                                       (LONG)0, (LONG)0))
#  define ffi_mparams_magic_store(v) \
  ((ਖਾਲੀ)_InterlockedExchange((LONG ਅਸਥਿਰ *)&mparams.magic, (LONG)(v)))
# endif
#ਨਹੀਂਤਾਂ
# define ffi_mparams_magic_ਲੱਦੋ()   (mparams.magic)
# define ffi_mparams_magic_store(v) (*(ਅਸਥਿਰ size_t *)(&(mparams.magic)) = (size_t)(v))
#ਜੇ_ਅੰਤ

/* Ensure mparams initialized */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ensure_initialization() (ਖਾਲੀ)(ffi_mparams_magic_ਲੱਦੋ() != 0 || init_mparams())

#ਜੇ !ONLY_MSPACES

/* The global malloc_state used for all non-"mspace" calls */
ਸਥਿਰ ਬਣਤਰ malloc_state _gm_;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gm                 (&_gm_)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_global(M)       ((M) == &_gm_)

#ਜੇ_ਅੰਤ /* !ONLY_MSPACES */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_initialized(M)  ((M)->ਸਿਖਰ != 0)

/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ use_ਤਾਲਾ(M)           ((M)->mflags &   USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ enable_ਤਾਲਾ(M)        ((M)->mflags |=  USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਜੇ USE_LOCKS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ disable_ਤਾਲਾ(M)       ((M)->mflags &= ~USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ disable_ਤਾਲਾ(M)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ use_mmap(M)           ((M)->mflags &   USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ enable_mmap(M)        ((M)->mflags |=  USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਜੇ HAVE_MMAP
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ disable_mmap(M)       ((M)->mflags &= ~USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ disable_mmap(M)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ use_noncontiguous(M)  ((M)->mflags &   USE_NONCONTIGUOUS_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ disable_contiguous(M) ((M)->mflags |=  USE_NONCONTIGUOUS_ਦੋਆਧਾਰੀ_ਅੰਕ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਤਾਲਾ(M,L)\
 ((M)->mflags = (L)?\
  ((M)->mflags | USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ) :\
  ((M)->mflags & ~USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ))

/* page-align a size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੰਨਾ_align(S)\
 (((S) + (mparams.ਪੰਨਾ_ਮਾਪ - ਮਾਪ_T_ONE)) & ~(mparams.ਪੰਨਾ_ਮਾਪ - ਮਾਪ_T_ONE))

/* granularity-align a size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ granularity_align(S)\
  (((S) + (mparams.granularity - ਮਾਪ_T_ONE))\
   & ~(mparams.granularity - ਮਾਪ_T_ONE))


/* For mmap, use granularity alignment on windows, else page-align */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mmap_align(S) granularity_align(S)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mmap_align(S) ਪੰਨਾ_align(S)
#ਜੇ_ਅੰਤ

/* For sys_alloc, enough padding to ensure can malloc request on success */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYS_ALLOC_PADDING (ਸਿਖਰ_FOOT_ਮਾਪ + MALLOC_ALIGNMENT)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_ਪੰਨਾ_aligned(S)\
   (((size_t)(S) & (mparams.ਪੰਨਾ_ਮਾਪ - ਮਾਪ_T_ONE)) == 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_granularity_aligned(S)\
   (((size_t)(S) & (mparams.granularity - ਮਾਪ_T_ONE)) == 0)

/*  True if segment S holds address A */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ segment_holds(S, A)\
  ((ਅੱਖਰ*)(A) >= S->ਅਧਾਰ && (ਅੱਖਰ*)(A) < S->ਅਧਾਰ + S->ਮਾਪ)

/* Return segment holding given address */
ਸਥਿਰ msegmentptr segment_holding(mstate m, ਅੱਖਰ* addr) {
  msegmentptr sp = &m->seg;
  ਲਈ (;;) {
    ਜੇ (addr >= sp->ਅਧਾਰ && addr < sp->ਅਧਾਰ + sp->ਮਾਪ)
      ਵਾਪਸ sp;
    ਜੇ ((sp = sp->ਅਗਲਾ) == 0)
      ਵਾਪਸ 0;
  }
}

/* Return true if segment contains a segment link */
ਸਥਿਰ ਪੂਰਨ has_segment_link(mstate m, msegmentptr ss) {
  msegmentptr sp = &m->seg;
  ਲਈ (;;) {
    ਜੇ ((ਅੱਖਰ*)sp >= ss->ਅਧਾਰ && (ਅੱਖਰ*)sp < ss->ਅਧਾਰ + ss->ਮਾਪ)
      ਵਾਪਸ 1;
    ਜੇ ((sp = sp->ਅਗਲਾ) == 0)
      ਵਾਪਸ 0;
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MORECORE_CANNOT_TRIM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ should_trim(M,s)  ((s) > (M)->trim_check)
#ਨਹੀਂਤਾਂ  /* MORECORE_CANNOT_TRIM */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ should_trim(M,s)  (0)
#ਜੇ_ਅੰਤ /* MORECORE_CANNOT_TRIM */

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿਖਰ_FOOT_ਮਾਪ\
  (align_ਵਿੱਥ(chunk2mem(0))+pad_request(ਆਕਾਰ(ਬਣਤਰ malloc_segment))+MIN_CHUNK_ਮਾਪ)


/* -------------------------------  Hooks -------------------------------- */

/*
  PREACTION should be defined to return 0 on success, and nonzero on
  failure. If you are not using locking, you can redefine these to do
  anything you like.
*/

#ਜੇ USE_LOCKS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PREACTION(M)  ((use_ਤਾਲਾ(M))? ACQUIRE_ਤਾਲਾ(&(M)->mutex) : 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ POSTACTION(M) { ਜੇ (use_ਤਾਲਾ(M)) RELEASE_ਤਾਲਾ(&(M)->mutex); }
#ਨਹੀਂਤਾਂ /* USE_LOCKS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ PREACTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PREACTION(M) (0)
#ਜੇ_ਅੰਤ  /* PREACTION */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ POSTACTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ POSTACTION(M)
#ਜੇ_ਅੰਤ  /* POSTACTION */

#ਜੇ_ਅੰਤ /* USE_LOCKS */

/*
  CORRUPTION_ERROR_ACTION is triggered upon detected bad addresses.
  USAGE_ERROR_ACTION is triggered on detected bad frees and
  reallocs. The argument p is an address that might have triggered the
  fault. It is ignored by the two predefined actions, but might be
  useful in custom actions that try to help diagnose errors.
*/

#ਜੇ PROCEED_ON_ERROR

/* A count of the number of corruption errors causing resets */
ਪੂਰਨ malloc_corruption_error_ਗਿਣਤੀ;

/* default corruption action */
ਸਥਿਰ ਖਾਲੀ reset_on_error(mstate m);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USAGE_ERROR_ACTION(m, p)

#ਨਹੀਂਤਾਂ /* PROCEED_ON_ERROR */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CORRUPTION_ERROR_ACTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CORRUPTION_ERROR_ACTION(m) ABORT
#ਜੇ_ਅੰਤ /* CORRUPTION_ERROR_ACTION */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USAGE_ERROR_ACTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USAGE_ERROR_ACTION(m,p) ABORT
#ਜੇ_ਅੰਤ /* USAGE_ERROR_ACTION */

#ਜੇ_ਅੰਤ /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#ਜੇ ! DEBUG

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_free_chunk(M,P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_inuse_chunk(M,P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_malloced_chunk(M,P,N)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_mmapped_chunk(M,P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_malloc_state(M)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_ਸਿਖਰ_chunk(M,P)

#ਨਹੀਂਤਾਂ /* DEBUG */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_free_chunk(M,P)       do_check_free_chunk(M,P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_inuse_chunk(M,P)      do_check_inuse_chunk(M,P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_ਸਿਖਰ_chunk(M,P)        do_check_ਸਿਖਰ_chunk(M,P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_malloced_chunk(M,P,N) do_check_malloced_chunk(M,P,N)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_mmapped_chunk(M,P)    do_check_mmapped_chunk(M,P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_malloc_state(M)       do_check_malloc_state(M)

ਸਥਿਰ ਖਾਲੀ   do_check_any_chunk(mstate m, mchunkptr p);
ਸਥਿਰ ਖਾਲੀ   do_check_ਸਿਖਰ_chunk(mstate m, mchunkptr p);
ਸਥਿਰ ਖਾਲੀ   do_check_mmapped_chunk(mstate m, mchunkptr p);
ਸਥਿਰ ਖਾਲੀ   do_check_inuse_chunk(mstate m, mchunkptr p);
ਸਥਿਰ ਖਾਲੀ   do_check_free_chunk(mstate m, mchunkptr p);
ਸਥਿਰ ਖਾਲੀ   do_check_malloced_chunk(mstate m, ਖਾਲੀ* mem, size_t s);
ਸਥਿਰ ਖਾਲੀ   do_check_ਰੁੱਖ(mstate m, tchunkptr t);
ਸਥਿਰ ਖਾਲੀ   do_check_treebin(mstate m, bindex_t i);
ਸਥਿਰ ਖਾਲੀ   do_check_smallbin(mstate m, bindex_t i);
ਸਥਿਰ ਖਾਲੀ   do_check_malloc_state(mstate m);
ਸਥਿਰ ਪੂਰਨ    bin_ਲੱਭੋ(mstate m, mchunkptr x);
ਸਥਿਰ size_t traverse_and_check(mstate m);
#ਜੇ_ਅੰਤ /* DEBUG */

/* ---------------------------- Indexing Bins ---------------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_small(s)         (((s) >> SMALLBIN_ਖਿਸਕਾਓ) < NSMALLBINS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ small_ਸੂਚਕਾਂਕ(s)      (bindex_t)((s)  >> SMALLBIN_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ small_index2size(i) ((i)  << SMALLBIN_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_SMALL_ਸੂਚਕਾਂਕ     (small_ਸੂਚਕਾਂਕ(MIN_CHUNK_ਮਾਪ))

/* addressing by index. See above about smallbin repositioning */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ smallbin_at(M, i)   ((sbinptr)((ਅੱਖਰ*)&((M)->smallbins[(i)<<1])))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ treebin_at(M,i)     (&((M)->treebins[i]))

/* assign tree index for size S to variable I.
   libffi: enable the __builtin_clz path on any GNU-compatible compiler, not
   only x86, so the log2 is an intrinsic rather than the generic C idiom that
   the compiler can't always recognize (libffi #754).  */
#ਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(S, I)\
{\
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ X = S >> TREEBIN_ਖਿਸਕਾਓ;\
  ਜੇ (X == 0)\
    I = 0;\
  ਨਹੀਂਤਾਂ ਜੇ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ਨਹੀਂਤਾਂ {\
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ K = (ਬਿਨਾਨਿਸ਼ਾਨੀ) ਆਕਾਰ(X)*__CHAR_BIT__ - 1 - (ਬਿਨਾਨਿਸ਼ਾਨੀ) __builtin_clz(X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_ਖਿਸਕਾਓ-1)) & 1)));\
  }\
}

#ਜਾਂਜੇ defined (__INTEL_COMPILER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(S, I)\
{\
  size_t X = S >> TREEBIN_ਖਿਸਕਾਓ;\
  ਜੇ (X == 0)\
    I = 0;\
  ਨਹੀਂਤਾਂ ਜੇ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ਨਹੀਂਤਾਂ {\
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ K = ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ_3 (X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_ਖਿਸਕਾਓ-1)) & 1)));\
  }\
}

#ਜਾਂਜੇ defined(_MSC_VER) && _MSC_VER>=1300
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(S, I)\
{\
  size_t X = S >> TREEBIN_ਖਿਸਕਾਓ;\
  ਜੇ (X == 0)\
    I = 0;\
  ਨਹੀਂਤਾਂ ਜੇ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ਨਹੀਂਤਾਂ {\
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ K;\
    ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_ਉਲਟ((DWORD *) &K, (DWORD) X);\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_ਖਿਸਕਾਓ-1)) & 1)));\
  }\
}

#ਨਹੀਂਤਾਂ /* GNUC */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(S, I)\
{\
  size_t X = S >> TREEBIN_ਖਿਸਕਾਓ;\
  ਜੇ (X == 0)\
    I = 0;\
  ਨਹੀਂਤਾਂ ਜੇ (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ਨਹੀਂਤਾਂ {\
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Y = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)X;\
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ N = ((Y - 0x100) >> 16) & 8;\
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_ਖਿਸਕਾਓ-1)) & 1));\
  }\
}
#ਜੇ_ਅੰਤ /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਦੋਆਧਾਰੀ_ਅੰਕ_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ(i) \
   (i == NTREEBINS-1)? (ਮਾਪ_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_ਖਿਸਕਾਓ - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ leftshift_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((ਮਾਪ_T_BITSIZE-ਮਾਪ_T_ONE) - (((i) >> 1) + TREEBIN_ਖਿਸਕਾਓ - 2)))

/* The size of the smallest chunk held in bin with index i */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ minsize_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ(i) \
   ((ਮਾਪ_T_ONE << (((i) >> 1) + TREEBIN_ਖਿਸਕਾਓ)) |  \
   (((size_t)((i) & ਮਾਪ_T_ONE)) << (((i) >> 1) + TREEBIN_ਖਿਸਕਾਓ - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਘੱਟ_ਤੋਂ_ਘੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਖੱਬਾ_ਬਿੱਟ(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ same_or_ਖੱਬਾ_ਬਿੱਟ(x) ((x) | -(x))

/* index corresponding to given bit.
   libffi: use __builtin_ctz on any GNU-compatible compiler, not only x86
   (libffi #754).  */

#ਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_bit2idx(X, I)\
{\
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ J;\
  J = __builtin_ctz(X); \
  I = (bindex_t)J;\
}

#ਜਾਂਜੇ defined (__INTEL_COMPILER)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_bit2idx(X, I)\
{\
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ J;\
  J = ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_forward (X); \
  I = (bindex_t)J;\
}

#ਜਾਂਜੇ defined(_MSC_VER) && _MSC_VER>=1300
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_bit2idx(X, I)\
{\
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ J;\
  ਦੋਆਧਾਰੀ_ਅੰਕ_ਜਾਂਚ_Forward_2((DWORD *) &J, X);\
  I = (bindex_t)J;\
}

#ਜਾਂਜੇ USE_BUILTIN_FFS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_bit2idx(X, I) I = ffs(X)-1

#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_bit2idx(X, I)\
{\
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Y = X - 1;\
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ K = Y >> (16-4) & 16;\
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ N = K;        Y >>= K;\
  N += K = Y >> (8-3) &  8;  Y >>= K;\
  N += K = Y >> (4-2) &  4;  Y >>= K;\
  N += K = Y >> (2-1) &  2;  Y >>= K;\
  N += K = Y >> (1-0) &  1;  Y >>= K;\
  I = (bindex_t)(N + Y);\
}
#ਜੇ_ਅੰਤ /* GNUC */


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

#ਜੇ !INSECURE
/* Check if address a is at least as high as any from MORECORE or MMAP */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_ਪਤਾ(M, a) ((ਅੱਖਰ*)(a) >= (M)->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr)
/* Check if address of next chunk n is higher than base chunk p */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_ਅਗਲਾ(p, n)    ((ਅੱਖਰ*)(p) < (ਅੱਖਰ*)(n))
/* Check if p has inuse status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_pinuse(p)     pinuse(p)

#ਨਹੀਂਤਾਂ /* !INSECURE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_ਪਤਾ(M, a) (1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_ਅਗਲਾ(b, n)    (1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_inuse(p)      (1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_pinuse(p)     (1)
#ਜੇ_ਅੰਤ /* !INSECURE */

#ਜੇ (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_magic(M)      ((M)->magic == mparams.magic)
#ਨਹੀਂਤਾਂ  /* (FOOTERS && !INSECURE) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ok_magic(M)      (1)
#ਜੇ_ਅੰਤ /* (FOOTERS && !INSECURE) */

/* In gcc, use __builtin_expect to minimize impact of checks */
#ਜੇ !INSECURE
#ਜੇ defined(__GNUC__) && __GNUC__ >= 3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTCHECK(e)  __builtin_expect(e, 1)
#ਨਹੀਂਤਾਂ /* GNUC */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTCHECK(e)  (e)
#ਜੇ_ਅੰਤ /* GNUC */
#ਨਹੀਂਤਾਂ /* !INSECURE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RTCHECK(e)  (1)
#ਜੇ_ਅੰਤ /* !INSECURE */

/* macros to set up inuse chunks with or without footers */

#ਜੇ !FOOTERS

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mark_inuse_foot(M,p,s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_inuse(M,p,s)\
  ((p)->ਸਿਰ = (((p)->ਸਿਰ & PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)|s|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ),\
  ((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))->ਸਿਰ |= PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_inuse_and_pinuse(M,p,s)\
  ((p)->ਸਿਰ = (s|PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ),\
  ((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))->ਸਿਰ |= PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)

/* Set size, cinuse and pinuse bit of this chunk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(M, p, s)\
  ((p)->ਸਿਰ = (s|PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ))

#ਨਹੀਂਤਾਂ /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mark_inuse_foot(M,p,s)\
  (((mchunkptr)((ਅੱਖਰ*)(p) + (s)))->prev_foot = ((size_t)(M) ^ mparams.magic))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get_mstate_for(p)\
  ((mstate)(((mchunkptr)((ਅੱਖਰ*)(p) +\
    (chunksize(p))))->prev_foot ^ mparams.magic))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_inuse(M,p,s)\
  ((p)->ਸਿਰ = (((p)->ਸਿਰ & PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ)|s|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ),\
  (((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))->ਸਿਰ |= PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ), \
  mark_inuse_foot(M,p,s))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_inuse_and_pinuse(M,p,s)\
  ((p)->ਸਿਰ = (s|PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ),\
  (((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))->ਸਿਰ |= PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ),\
 mark_inuse_foot(M,p,s))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(M, p, s)\
  ((p)->ਸਿਰ = (s|PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ|CINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ),\
  mark_inuse_foot(M, p, s))

#ਜੇ_ਅੰਤ /* !FOOTERS */

/* ---------------------------- setting mparams -------------------------- */

#ਜੇ ਤਾਲਾ_AT_FORK
ਸਥਿਰ ਖਾਲੀ pre_fork(ਖਾਲੀ)         { ACQUIRE_ਤਾਲਾ(&(gm)->mutex); }
ਸਥਿਰ ਖਾਲੀ post_fork_ਮਾਪਾ(ਖਾਲੀ) { RELEASE_ਤਾਲਾ(&(gm)->mutex); }
ਸਥਿਰ ਖਾਲੀ post_fork_ਬੱਚਾ(ਖਾਲੀ)  { INITIAL_ਤਾਲਾ(&(gm)->mutex); }
#ਜੇ_ਅੰਤ /* LOCK_AT_FORK */

/* Initialize mparams */
ਸਥਿਰ ਪੂਰਨ init_mparams(ਖਾਲੀ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NEED_GLOBAL_ਤਾਲਾ_INIT
  ਜੇ (malloc_global_mutex_ਸਥਿਤੀ <= 0)
    init_malloc_global_mutex();
#ਜੇ_ਅੰਤ

  ACQUIRE_MALLOC_GLOBAL_ਤਾਲਾ();
  ਜੇ (mparams.magic == 0) {
    size_t magic;
    size_t psize;
    size_t gsize;

#ਜੇ !defined(WIN32) && !defined(__OS2__)
    psize = malloc_getpagesize;
    gsize = ((DEFAULT_GRANULARITY != 0)? DEFAULT_GRANULARITY : psize);
#ਜਾਂਜੇ defined (__OS2__)
    /* if low-memory is used, os2munmap() would break
       if it were anything other than 64k */
    psize = 4096u;
    gsize = 65536u;
#ਨਹੀਂਤਾਂ /* WIN32 */
    {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      psize = system_info.dw_ਪੰਨਾ_ਮਾਪ;
      gsize = ((DEFAULT_GRANULARITY != 0)?
               DEFAULT_GRANULARITY : system_info.dw_ਵੰਡ_Granularity);
    }
#ਜੇ_ਅੰਤ /* WIN32 */

    /* Sanity-check configuration:
       size_t must be unsigned and as wide as pointer type.
       ints must be at least 4 bytes.
       alignment must be at least 8.
       Alignment, min chunk size, and page size must all be powers of 2.
    */
    ਜੇ ((ਆਕਾਰ(size_t) != ਆਕਾਰ(ਅੱਖਰ*)) ||
        (MAX_ਮਾਪ_T < MIN_CHUNK_ਮਾਪ)  ||
        (ਆਕਾਰ(ਪੂਰਨ) < 4)  ||
        (MALLOC_ALIGNMENT < (size_t)8U) ||
        ((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-ਮਾਪ_T_ONE)) != 0) ||
        ((MCHUNK_ਮਾਪ      & (MCHUNK_ਮਾਪ-ਮਾਪ_T_ONE))      != 0) ||
        ((gsize            & (gsize-ਮਾਪ_T_ONE))            != 0) ||
        ((psize            & (psize-ਮਾਪ_T_ONE))            != 0))
      ABORT;
    mparams.granularity = gsize;
    mparams.ਪੰਨਾ_ਮਾਪ = psize;
    mparams.mmap_ਹੱਦ = DEFAULT_MMAP_ਹੱਦ;
    mparams.trim_ਹੱਦ = DEFAULT_TRIM_ਹੱਦ;
#ਜੇ MORECORE_CONTIGUOUS
    mparams.default_mflags = USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ|USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ;
#ਨਹੀਂਤਾਂ  /* MORECORE_CONTIGUOUS */
    mparams.default_mflags = USE_ਤਾਲਾ_ਦੋਆਧਾਰੀ_ਅੰਕ|USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ|USE_NONCONTIGUOUS_ਦੋਆਧਾਰੀ_ਅੰਕ;
#ਜੇ_ਅੰਤ /* MORECORE_CONTIGUOUS */

#ਜੇ !ONLY_MSPACES
    /* Set up lock for main malloc area */
    gm->mflags = mparams.default_mflags;
    (ਖਾਲੀ)INITIAL_ਤਾਲਾ(&gm->mutex);
#ਜੇ_ਅੰਤ
#ਜੇ ਤਾਲਾ_AT_FORK
    pthread_atfork(&pre_fork, &post_fork_ਮਾਪਾ, &post_fork_ਬੱਚਾ);
#ਜੇ_ਅੰਤ

    {
#ਜੇ USE_DEV_RANDOM
      ਪੂਰਨ fd;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[ਆਕਾਰ(size_t)];
      /* Try to use /dev/urandom, else fall back on using time */
      ਜੇ ((fd = open("/dev/urandom", O_RDONLY)) >= 0 &&
          ਪਠ(fd, buf, ਆਕਾਰ(buf)) == ਆਕਾਰ(buf)) {
        magic = *((size_t *) buf);
        close(fd);
      }
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* USE_DEV_RANDOM */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIN32
      magic = (size_t)(Get_Tick_ਗਿਣਤੀ() ^ (size_t)0x55555555U);
#ਜਾਂਜੇ defined(LACKS_ਸਮਾਂ_H)
      magic = (size_t)&magic ^ (size_t)0x55555555U;
#ਨਹੀਂਤਾਂ
      magic = (size_t)(time(0) ^ (size_t)0x55555555U);
#ਜੇ_ਅੰਤ
      magic |= (size_t)8U;    /* ensure nonzero */
      magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
      /* libffi: release-store so any thread that observes magic via the
         acquire load in ensure_initialization() sees the writes above (#873).  */
      ffi_mparams_magic_store(magic);
    }
  }

  RELEASE_MALLOC_GLOBAL_ਤਾਲਾ();
  ਵਾਪਸ 1;
}

/* support for mallopt */
ਸਥਿਰ ਪੂਰਨ change_mparam(ਪੂਰਨ param_ਸੰਖਿਆ, ਪੂਰਨ ਮੁੱਲ) {
  size_t val;
  ensure_initialization();
  val = (ਮੁੱਲ == -1)? MAX_ਮਾਪ_T : (size_t)ਮੁੱਲ;
  ਚੋਣ(param_ਸੰਖਿਆ) {
  ਹਾਲਤ M_TRIM_ਹੱਦ:
    mparams.trim_ਹੱਦ = val;
    ਵਾਪਸ 1;
  ਹਾਲਤ M_GRANULARITY:
    ਜੇ (val >= mparams.ਪੰਨਾ_ਮਾਪ && ((val & (val-1)) == 0)) {
      mparams.granularity = val;
      ਵਾਪਸ 1;
    }
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ 0;
  ਹਾਲਤ M_MMAP_ਹੱਦ:
    mparams.mmap_ਹੱਦ = val;
    ਵਾਪਸ 1;
  ਮੂਲ:
    ਵਾਪਸ 0;
  }
}

#ਜੇ DEBUG
/* ------------------------- Debugging Support --------------------------- */

/* Check properties of any chunk, whether free, inuse, mmapped etc  */
ਸਥਿਰ ਖਾਲੀ do_check_any_chunk(mstate m, mchunkptr p) {
  ਪ੍ਰਤਿਜ੍ਞਾ((is_aligned(chunk2mem(p))) || (p->ਸਿਰ == FENCEPOST_ਸਿਰ));
  ਪ੍ਰਤਿਜ੍ਞਾ(ok_ਪਤਾ(m, p));
}

/* Check properties of top chunk */
ਸਥਿਰ ਖਾਲੀ do_check_ਸਿਖਰ_chunk(mstate m, mchunkptr p) {
  msegmentptr sp = segment_holding(m, (ਅੱਖਰ*)p);
  size_t  sz = p->ਸਿਰ & ~INUSE_ਬਿੱਟ; /* third-lowest bit can be set! */
  ਪ੍ਰਤਿਜ੍ਞਾ(sp != 0);
  ਪ੍ਰਤਿਜ੍ਞਾ((is_aligned(chunk2mem(p))) || (p->ਸਿਰ == FENCEPOST_ਸਿਰ));
  ਪ੍ਰਤਿਜ੍ਞਾ(ok_ਪਤਾ(m, p));
  ਪ੍ਰਤਿਜ੍ਞਾ(sz == m->topsize);
  ਪ੍ਰਤਿਜ੍ਞਾ(sz > 0);
  ਪ੍ਰਤਿਜ੍ਞਾ(sz == ((sp->ਅਧਾਰ + sp->ਮਾਪ) - (ਅੱਖਰ*)p) - ਸਿਖਰ_FOOT_ਮਾਪ);
  ਪ੍ਰਤਿਜ੍ਞਾ(pinuse(p));
  ਪ੍ਰਤਿਜ੍ਞਾ(!pinuse(chunk_plus_ਵਿੱਥ(p, sz)));
}

/* Check properties of (inuse) mmapped chunks */
ਸਥਿਰ ਖਾਲੀ do_check_mmapped_chunk(mstate m, mchunkptr p) {
  size_t  sz = chunksize(p);
  size_t len = (sz + (p->prev_foot) + MMAP_FOOT_PAD);
  ਪ੍ਰਤਿਜ੍ਞਾ(is_mmapped(p));
  ਪ੍ਰਤਿਜ੍ਞਾ(use_mmap(m));
  ਪ੍ਰਤਿਜ੍ਞਾ((is_aligned(chunk2mem(p))) || (p->ਸਿਰ == FENCEPOST_ਸਿਰ));
  ਪ੍ਰਤਿਜ੍ਞਾ(ok_ਪਤਾ(m, p));
  ਪ੍ਰਤਿਜ੍ਞਾ(!is_small(sz));
  ਪ੍ਰਤਿਜ੍ਞਾ((len & (mparams.ਪੰਨਾ_ਮਾਪ-ਮਾਪ_T_ONE)) == 0);
  ਪ੍ਰਤਿਜ੍ਞਾ(chunk_plus_ਵਿੱਥ(p, sz)->ਸਿਰ == FENCEPOST_ਸਿਰ);
  ਪ੍ਰਤਿਜ੍ਞਾ(chunk_plus_ਵਿੱਥ(p, sz+ਮਾਪ_T_ਮਾਪ)->ਸਿਰ == 0);
}

/* Check properties of inuse chunks */
ਸਥਿਰ ਖਾਲੀ do_check_inuse_chunk(mstate m, mchunkptr p) {
  do_check_any_chunk(m, p);
  ਪ੍ਰਤਿਜ੍ਞਾ(is_inuse(p));
  ਪ੍ਰਤਿਜ੍ਞਾ(ਅਗਲਾ_pinuse(p));
  /* If not pinuse and not mmapped, previous chunk has OK offset */
  ਪ੍ਰਤਿਜ੍ਞਾ(is_mmapped(p) || pinuse(p) || ਅਗਲਾ_chunk(prev_chunk(p)) == p);
  ਜੇ (is_mmapped(p))
    do_check_mmapped_chunk(m, p);
}

/* Check properties of free chunks */
ਸਥਿਰ ਖਾਲੀ do_check_free_chunk(mstate m, mchunkptr p) {
  size_t sz = chunksize(p);
  mchunkptr ਅਗਲਾ = chunk_plus_ਵਿੱਥ(p, sz);
  do_check_any_chunk(m, p);
  ਪ੍ਰਤਿਜ੍ਞਾ(!is_inuse(p));
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਅਗਲਾ_pinuse(p));
  ਪ੍ਰਤਿਜ੍ਞਾ (!is_mmapped(p));
  ਜੇ (p != m->dv && p != m->ਸਿਖਰ) {
    ਜੇ (sz >= MIN_CHUNK_ਮਾਪ) {
      ਪ੍ਰਤਿਜ੍ਞਾ((sz & CHUNK_ALIGN_ਨਕਾਬ) == 0);
      ਪ੍ਰਤਿਜ੍ਞਾ(is_aligned(chunk2mem(p)));
      ਪ੍ਰਤਿਜ੍ਞਾ(ਅਗਲਾ->prev_foot == sz);
      ਪ੍ਰਤਿਜ੍ਞਾ(pinuse(p));
      ਪ੍ਰਤਿਜ੍ਞਾ (ਅਗਲਾ == m->ਸਿਖਰ || is_inuse(ਅਗਲਾ));
      ਪ੍ਰਤਿਜ੍ਞਾ(p->fd->bk == p);
      ਪ੍ਰਤਿਜ੍ਞਾ(p->bk->fd == p);
    }
    ਨਹੀਂਤਾਂ  /* markers are always of size SIZE_T_SIZE */
      ਪ੍ਰਤਿਜ੍ਞਾ(sz == ਮਾਪ_T_ਮਾਪ);
  }
}

/* Check properties of malloced chunks at the point they are malloced */
ਸਥਿਰ ਖਾਲੀ do_check_malloced_chunk(mstate m, ਖਾਲੀ* mem, size_t s) {
  ਜੇ (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    size_t sz = p->ਸਿਰ & ~INUSE_ਬਿੱਟ;
    do_check_inuse_chunk(m, p);
    ਪ੍ਰਤਿਜ੍ਞਾ((sz & CHUNK_ALIGN_ਨਕਾਬ) == 0);
    ਪ੍ਰਤਿਜ੍ਞਾ(sz >= MIN_CHUNK_ਮਾਪ);
    ਪ੍ਰਤਿਜ੍ਞਾ(sz >= s);
    /* unless mmapped, size is less than MIN_CHUNK_SIZE more than request */
    ਪ੍ਰਤਿਜ੍ਞਾ(is_mmapped(p) || sz < (s + MIN_CHUNK_ਮਾਪ));
  }
}

/* Check a tree and its subtrees.  */
ਸਥਿਰ ਖਾਲੀ do_check_ਰੁੱਖ(mstate m, tchunkptr t) {
  tchunkptr ਸਿਰ = 0;
  tchunkptr u = t;
  bindex_t tindex = t->ਸੂਚਕਾਂਕ;
  size_t tsize = chunksize(t);
  bindex_t idx;
  compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(tsize, idx);
  ਪ੍ਰਤਿਜ੍ਞਾ(tindex == idx);
  ਪ੍ਰਤਿਜ੍ਞਾ(tsize >= MIN_LARGE_ਮਾਪ);
  ਪ੍ਰਤਿਜ੍ਞਾ(tsize >= minsize_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ(idx));
  ਪ੍ਰਤਿਜ੍ਞਾ((idx == NTREEBINS-1) || (tsize < minsize_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ((idx+1))));

  ਕਰੋ { /* traverse through chain of same-sized nodes */
    do_check_any_chunk(m, ((mchunkptr)u));
    ਪ੍ਰਤਿਜ੍ਞਾ(u->ਸੂਚਕਾਂਕ == tindex);
    ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(u) == tsize);
    ਪ੍ਰਤਿਜ੍ਞਾ(!is_inuse(u));
    ਪ੍ਰਤਿਜ੍ਞਾ(!ਅਗਲਾ_pinuse(u));
    ਪ੍ਰਤਿਜ੍ਞਾ(u->fd->bk == u);
    ਪ੍ਰਤਿਜ੍ਞਾ(u->bk->fd == u);
    ਜੇ (u->ਮਾਪਾ == 0) {
      ਪ੍ਰਤਿਜ੍ਞਾ(u->ਬੱਚਾ[0] == 0);
      ਪ੍ਰਤਿਜ੍ਞਾ(u->ਬੱਚਾ[1] == 0);
    }
    ਨਹੀਂਤਾਂ {
      ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰ == 0); /* only one node on chain has parent */
      ਸਿਰ = u;
      ਪ੍ਰਤਿਜ੍ਞਾ(u->ਮਾਪਾ != u);
      ਪ੍ਰਤਿਜ੍ਞਾ (u->ਮਾਪਾ->ਬੱਚਾ[0] == u ||
              u->ਮਾਪਾ->ਬੱਚਾ[1] == u ||
              *((tbinptr*)(u->ਮਾਪਾ)) == u);
      ਜੇ (u->ਬੱਚਾ[0] != 0) {
        ਪ੍ਰਤਿਜ੍ਞਾ(u->ਬੱਚਾ[0]->ਮਾਪਾ == u);
        ਪ੍ਰਤਿਜ੍ਞਾ(u->ਬੱਚਾ[0] != u);
        do_check_ਰੁੱਖ(m, u->ਬੱਚਾ[0]);
      }
      ਜੇ (u->ਬੱਚਾ[1] != 0) {
        ਪ੍ਰਤਿਜ੍ਞਾ(u->ਬੱਚਾ[1]->ਮਾਪਾ == u);
        ਪ੍ਰਤਿਜ੍ਞਾ(u->ਬੱਚਾ[1] != u);
        do_check_ਰੁੱਖ(m, u->ਬੱਚਾ[1]);
      }
      ਜੇ (u->ਬੱਚਾ[0] != 0 && u->ਬੱਚਾ[1] != 0) {
        ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(u->ਬੱਚਾ[0]) < chunksize(u->ਬੱਚਾ[1]));
      }
    }
    u = u->fd;
  } ਜਦੋਂ (u != t);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਸਿਰ != 0);
}

/*  Check all the chunks in a treebin.  */
ਸਥਿਰ ਖਾਲੀ do_check_treebin(mstate m, bindex_t i) {
  tbinptr* tb = treebin_at(m, i);
  tchunkptr t = *tb;
  ਪੂਰਨ ਖਾਲੀ_ਥਾਵਾਂ = (m->treemap & (1U << i)) == 0;
  ਜੇ (t == 0)
    ਪ੍ਰਤਿਜ੍ਞਾ(ਖਾਲੀ_ਥਾਵਾਂ);
  ਜੇ (!ਖਾਲੀ_ਥਾਵਾਂ)
    do_check_ਰੁੱਖ(m, t);
}

/*  Check all the chunks in a smallbin.  */
ਸਥਿਰ ਖਾਲੀ do_check_smallbin(mstate m, bindex_t i) {
  sbinptr b = smallbin_at(m, i);
  mchunkptr p = b->bk;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਖਾਲੀ_ਥਾਵਾਂ = (m->smallmap & (1U << i)) == 0;
  ਜੇ (p == b)
    ਪ੍ਰਤਿਜ੍ਞਾ(ਖਾਲੀ_ਥਾਵਾਂ);
  ਜੇ (!ਖਾਲੀ_ਥਾਵਾਂ) {
    ਲਈ (; p != b; p = p->bk) {
      size_t ਮਾਪ = chunksize(p);
      mchunkptr q;
      /* each chunk claims to be free */
      do_check_free_chunk(m, p);
      /* chunk belongs in bin */
      ਪ੍ਰਤਿਜ੍ਞਾ(small_ਸੂਚਕਾਂਕ(ਮਾਪ) == i);
      ਪ੍ਰਤਿਜ੍ਞਾ(p->bk == b || chunksize(p->bk) == chunksize(p));
      /* chunk is followed by an inuse chunk */
      q = ਅਗਲਾ_chunk(p);
      ਜੇ (q->ਸਿਰ != FENCEPOST_ਸਿਰ)
        do_check_inuse_chunk(m, q);
    }
  }
}

/* Find x in a bin. Used in other check functions. */
ਸਥਿਰ ਪੂਰਨ bin_ਲੱਭੋ(mstate m, mchunkptr x) {
  size_t ਮਾਪ = chunksize(x);
  ਜੇ (is_small(ਮਾਪ)) {
    bindex_t sidx = small_ਸੂਚਕਾਂਕ(ਮਾਪ);
    sbinptr b = smallbin_at(m, sidx);
    ਜੇ (smallmap_is_marked(m, sidx)) {
      mchunkptr p = b;
      ਕਰੋ {
        ਜੇ (p == x)
          ਵਾਪਸ 1;
      } ਜਦੋਂ ((p = p->fd) != b);
    }
  }
  ਨਹੀਂਤਾਂ {
    bindex_t tidx;
    compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(ਮਾਪ, tidx);
    ਜੇ (treemap_is_marked(m, tidx)) {
      tchunkptr t = *treebin_at(m, tidx);
      size_t sizebits = ਮਾਪ << leftshift_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ(tidx);
      ਜਦੋਂ (t != 0 && chunksize(t) != ਮਾਪ) {
        t = t->ਬੱਚਾ[(sizebits >> (ਮਾਪ_T_BITSIZE-ਮਾਪ_T_ONE)) & 1];
        sizebits <<= 1;
      }
      ਜੇ (t != 0) {
        tchunkptr u = t;
        ਕਰੋ {
          ਜੇ (u == (tchunkptr)x)
            ਵਾਪਸ 1;
        } ਜਦੋਂ ((u = u->fd) != t);
      }
    }
  }
  ਵਾਪਸ 0;
}

/* Traverse each chunk and check it; return total */
ਸਥਿਰ size_t traverse_and_check(mstate m) {
  size_t ਜੋੜ = 0;
  ਜੇ (is_initialized(m)) {
    msegmentptr s = &m->seg;
    ਜੋੜ += m->topsize + ਸਿਖਰ_FOOT_ਮਾਪ;
    ਜਦੋਂ (s != 0) {
      mchunkptr q = align_as_chunk(s->ਅਧਾਰ);
      mchunkptr lastq = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ(pinuse(q));
      ਜਦੋਂ (segment_holds(s, q) &&
             q != m->ਸਿਖਰ && q->ਸਿਰ != FENCEPOST_ਸਿਰ) {
        ਜੋੜ += chunksize(q);
        ਜੇ (is_inuse(q)) {
          ਪ੍ਰਤਿਜ੍ਞਾ(!bin_ਲੱਭੋ(m, q));
          do_check_inuse_chunk(m, q);
        }
        ਨਹੀਂਤਾਂ {
          ਪ੍ਰਤਿਜ੍ਞਾ(q == m->dv || bin_ਲੱਭੋ(m, q));
          ਪ੍ਰਤਿਜ੍ਞਾ(lastq == 0 || is_inuse(lastq)); /* Not 2 consecutive free */
          do_check_free_chunk(m, q);
        }
        lastq = q;
        q = ਅਗਲਾ_chunk(q);
      }
      s = s->ਅਗਲਾ;
    }
  }
  ਵਾਪਸ ਜੋੜ;
}


/* Check all properties of malloc_state. */
ਸਥਿਰ ਖਾਲੀ do_check_malloc_state(mstate m) {
  bindex_t i;
  size_t ਕੁੱਲ;
  /* check bins */
  ਲਈ (i = 0; i < NSMALLBINS; ++i)
    do_check_smallbin(m, i);
  ਲਈ (i = 0; i < NTREEBINS; ++i)
    do_check_treebin(m, i);

  ਜੇ (m->dvsize != 0) { /* check dv chunk */
    do_check_any_chunk(m, m->dv);
    ਪ੍ਰਤਿਜ੍ਞਾ(m->dvsize == chunksize(m->dv));
    ਪ੍ਰਤਿਜ੍ਞਾ(m->dvsize >= MIN_CHUNK_ਮਾਪ);
    ਪ੍ਰਤਿਜ੍ਞਾ(bin_ਲੱਭੋ(m, m->dv) == 0);
  }

  ਜੇ (m->ਸਿਖਰ != 0) {   /* check top chunk */
    do_check_ਸਿਖਰ_chunk(m, m->ਸਿਖਰ);
    /*assert(m->topsize == chunksize(m->top)); redundant */
    ਪ੍ਰਤਿਜ੍ਞਾ(m->topsize > 0);
    ਪ੍ਰਤਿਜ੍ਞਾ(bin_ਲੱਭੋ(m, m->ਸਿਖਰ) == 0);
  }

  ਕੁੱਲ = traverse_and_check(m);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਕੁੱਲ <= m->footprint);
  ਪ੍ਰਤਿਜ੍ਞਾ(m->footprint <= m->max_footprint);
}
#ਜੇ_ਅੰਤ /* DEBUG */

/* ----------------------------- statistics ------------------------------ */

#ਜੇ !NO_MALLINFO
ਸਥਿਰ ਬਣਤਰ mallinfo ਅੰਦਰੂਨੀ_mallinfo(mstate m) {
  ਬਣਤਰ mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ensure_initialization();
  ਜੇ (!PREACTION(m)) {
    check_malloc_state(m);
    ਜੇ (is_initialized(m)) {
      size_t nfree = ਮਾਪ_T_ONE; /* top always free */
      size_t mfree = m->topsize + ਸਿਖਰ_FOOT_ਮਾਪ;
      size_t ਜੋੜ = mfree;
      msegmentptr s = &m->seg;
      ਜਦੋਂ (s != 0) {
        mchunkptr q = align_as_chunk(s->ਅਧਾਰ);
        ਜਦੋਂ (segment_holds(s, q) &&
               q != m->ਸਿਖਰ && q->ਸਿਰ != FENCEPOST_ਸਿਰ) {
          size_t sz = chunksize(q);
          ਜੋੜ += sz;
          ਜੇ (!is_inuse(q)) {
            mfree += sz;
            ++nfree;
          }
          q = ਅਗਲਾ_chunk(q);
        }
        s = s->ਅਗਲਾ;
      }

      nm.arena    = ਜੋੜ;
      nm.ordblks  = nfree;
      nm.hblkhd   = m->footprint - ਜੋੜ;
      nm.usmblks  = m->max_footprint;
      nm.uordblks = m->footprint - mfree;
      nm.fordblks = mfree;
      nm.keepcost = m->topsize;
    }

    POSTACTION(m);
  }
  ਵਾਪਸ nm;
}
#ਜੇ_ਅੰਤ /* !NO_MALLINFO */

#ਜੇ !NO_MALLOC_STATS
ਸਥਿਰ ਖਾਲੀ ਅੰਦਰੂਨੀ_malloc_stats(mstate m) {
  ensure_initialization();
  ਜੇ (!PREACTION(m)) {
    size_t maxfp = 0;
    size_t fp = 0;
    size_t ਵਰਤਿਆ = 0;
    check_malloc_state(m);
    ਜੇ (is_initialized(m)) {
      msegmentptr s = &m->seg;
      maxfp = m->max_footprint;
      fp = m->footprint;
      ਵਰਤਿਆ = fp - (m->topsize + ਸਿਖਰ_FOOT_ਮਾਪ);

      ਜਦੋਂ (s != 0) {
        mchunkptr q = align_as_chunk(s->ਅਧਾਰ);
        ਜਦੋਂ (segment_holds(s, q) &&
               q != m->ਸਿਖਰ && q->ਸਿਰ != FENCEPOST_ਸਿਰ) {
          ਜੇ (!is_inuse(q))
            ਵਰਤਿਆ -= chunksize(q);
          q = ਅਗਲਾ_chunk(q);
        }
        s = s->ਅਗਲਾ;
      }
    }
    POSTACTION(m); /* drop lock */
    fprintf(stderr, "max system bytes = %10lu\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(maxfp));
    fprintf(stderr, "system bytes     = %10lu\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(fp));
    fprintf(stderr, "in use bytes     = %10lu\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(ਵਰਤਿਆ));
  }
}
#ਜੇ_ਅੰਤ /* NO_MALLOC_STATS */

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਾਓ_small_chunk(M, P, S) {\
  bindex_t I  = small_ਸੂਚਕਾਂਕ(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  ਪ੍ਰਤਿਜ੍ਞਾ(S >= MIN_CHUNK_ਮਾਪ);\
  ਜੇ (!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  ਨਹੀਂਤਾਂ ਜੇ (RTCHECK(ok_ਪਤਾ(M, B->fd)))\
    F = B->fd;\
  ਨਹੀਂਤਾਂ {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  bindex_t I = small_ਸੂਚਕਾਂਕ(S);\
  ਪ੍ਰਤਿਜ੍ਞਾ(P != B);\
  ਪ੍ਰਤਿਜ੍ਞਾ(P != F);\
  ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(P) == small_index2size(I));\
  ਜੇ (RTCHECK(F == smallbin_at(M,I) || (ok_ਪਤਾ(M, F) && F->bk == P))) { \
    ਜੇ (B == F) {\
      clear_smallmap(M, I);\
    }\
    ਨਹੀਂਤਾਂ ਜੇ (RTCHECK(B == smallbin_at(M,I) ||\
                     (ok_ਪਤਾ(M, B) && B->fd == P))) {\
      F->bk = B;\
      B->fd = F;\
    }\
    ਨਹੀਂਤਾਂ {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  ਨਹੀਂਤਾਂ {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Unlink the first chunk from a smallbin */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unlink_ਪਹਿਲਾ_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  ਪ੍ਰਤਿਜ੍ਞਾ(P != B);\
  ਪ੍ਰਤਿਜ੍ਞਾ(P != F);\
  ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(P) == small_index2size(I));\
  ਜੇ (B == F) {\
    clear_smallmap(M, I);\
  }\
  ਨਹੀਂਤਾਂ ਜੇ (RTCHECK(ok_ਪਤਾ(M, F) && F->bk == P)) {\
    F->bk = B;\
    B->fd = F;\
  }\
  ਨਹੀਂਤਾਂ {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  ਪ੍ਰਤਿਜ੍ਞਾ(is_small(DVS));\
  ਜੇ (DVS != 0) {\
    mchunkptr DV = M->dv;\
    ਪਾਓ_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਾਓ_large_chunk(M, X, S) {\
  tbinptr* H;\
  bindex_t I;\
  compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(S, I);\
  H = treebin_at(M, I);\
  X->ਸੂਚਕਾਂਕ = I;\
  X->ਬੱਚਾ[0] = X->ਬੱਚਾ[1] = 0;\
  ਜੇ (!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->ਮਾਪਾ = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  ਨਹੀਂਤਾਂ {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ(I);\
    ਲਈ (;;) {\
      ਜੇ (chunksize(T) != S) {\
        tchunkptr* C = &(T->ਬੱਚਾ[(K >> (ਮਾਪ_T_BITSIZE-ਮਾਪ_T_ONE)) & 1]);\
        K <<= 1;\
        ਜੇ (*C != 0)\
          T = *C;\
        ਨਹੀਂਤਾਂ ਜੇ (RTCHECK(ok_ਪਤਾ(M, C))) {\
          *C = X;\
          X->ਮਾਪਾ = T;\
          X->fd = X->bk = X;\
          ਰੋਕੋ;\
        }\
        ਨਹੀਂਤਾਂ {\
          CORRUPTION_ERROR_ACTION(M);\
          ਰੋਕੋ;\
        }\
      }\
      ਨਹੀਂਤਾਂ {\
        tchunkptr F = T->fd;\
        ਜੇ (RTCHECK(ok_ਪਤਾ(M, T) && ok_ਪਤਾ(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->ਮਾਪਾ = 0;\
          ਰੋਕੋ;\
        }\
        ਨਹੀਂਤਾਂ {\
          CORRUPTION_ERROR_ACTION(M);\
          ਰੋਕੋ;\
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unlink_large_chunk(M, X) {\
  tchunkptr XP = X->ਮਾਪਾ;\
  tchunkptr R;\
  ਜੇ (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    ਜੇ (RTCHECK(ok_ਪਤਾ(M, F) && F->bk == X && R->fd == X)) {\
      F->bk = R;\
      R->fd = F;\
    }\
    ਨਹੀਂਤਾਂ {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  ਨਹੀਂਤਾਂ {\
    tchunkptr* RP;\
    ਜੇ (((R = *(RP = &(X->ਬੱਚਾ[1]))) != 0) ||\
        ((R = *(RP = &(X->ਬੱਚਾ[0]))) != 0)) {\
      tchunkptr* CP;\
      ਜਦੋਂ ((*(CP = &(R->ਬੱਚਾ[1])) != 0) ||\
             (*(CP = &(R->ਬੱਚਾ[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      ਜੇ (RTCHECK(ok_ਪਤਾ(M, RP)))\
        *RP = 0;\
      ਨਹੀਂਤਾਂ {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  ਜੇ (XP != 0) {\
    tbinptr* H = treebin_at(M, X->ਸੂਚਕਾਂਕ);\
    ਜੇ (X == *H) {\
      ਜੇ ((*H = R) == 0) \
        clear_treemap(M, X->ਸੂਚਕਾਂਕ);\
    }\
    ਨਹੀਂਤਾਂ ਜੇ (RTCHECK(ok_ਪਤਾ(M, XP))) {\
      ਜੇ (XP->ਬੱਚਾ[0] == X) \
        XP->ਬੱਚਾ[0] = R;\
      ਨਹੀਂਤਾਂ \
        XP->ਬੱਚਾ[1] = R;\
    }\
    ਨਹੀਂਤਾਂ\
      CORRUPTION_ERROR_ACTION(M);\
    ਜੇ (R != 0) {\
      ਜੇ (RTCHECK(ok_ਪਤਾ(M, R))) {\
        tchunkptr C0, C1;\
        R->ਮਾਪਾ = XP;\
        ਜੇ ((C0 = X->ਬੱਚਾ[0]) != 0) {\
          ਜੇ (RTCHECK(ok_ਪਤਾ(M, C0))) {\
            R->ਬੱਚਾ[0] = C0;\
            C0->ਮਾਪਾ = R;\
          }\
          ਨਹੀਂਤਾਂ\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        ਜੇ ((C1 = X->ਬੱਚਾ[1]) != 0) {\
          ਜੇ (RTCHECK(ok_ਪਤਾ(M, C1))) {\
            R->ਬੱਚਾ[1] = C1;\
            C1->ਮਾਪਾ = R;\
          }\
          ਨਹੀਂਤਾਂ\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      ਨਹੀਂਤਾਂ\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}

/* Relays to large vs small bin operations */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਾਓ_chunk(M, P, S)\
  ਜੇ (is_small(S)) ਪਾਓ_small_chunk(M, P, S)\
  ਨਹੀਂਤਾਂ { tchunkptr TP = (tchunkptr)(P); ਪਾਓ_large_chunk(M, TP, S); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unlink_chunk(M, P, S)\
  ਜੇ (is_small(S)) unlink_small_chunk(M, P, S)\
  ਨਹੀਂਤਾਂ { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#ਜੇ ONLY_MSPACES
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਦਰੂਨੀ_malloc(m, b) mspace_malloc(m, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਦਰੂਨੀ_free(m, mem) mspace_free(m,mem);
#ਨਹੀਂਤਾਂ /* ONLY_MSPACES */
#ਜੇ MSPACES
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਦਰੂਨੀ_malloc(m, b)\
  ((m == gm)? dlmalloc(b) : mspace_malloc(m, b))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਦਰੂਨੀ_free(m, mem)\
   ਜੇ (m == gm) dlfree(mem); ਨਹੀਂਤਾਂ mspace_free(m,mem);
#ਨਹੀਂਤਾਂ /* MSPACES */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਦਰੂਨੀ_malloc(m, b) dlmalloc(b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਦਰੂਨੀ_free(m, mem) dlfree(mem)
#ਜੇ_ਅੰਤ /* MSPACES */
#ਜੇ_ਅੰਤ /* ONLY_MSPACES */

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
ਸਥਿਰ ਖਾਲੀ* mmap_alloc(mstate m, size_t nb) {
  size_t mmsize = mmap_align(nb + SIX_ਮਾਪ_T_SIZES + CHUNK_ALIGN_ਨਕਾਬ);
  ਜੇ (m->footprint_ਹੱਦਬੰਦੀ != 0) {
    size_t fp = m->footprint + mmsize;
    ਜੇ (fp <= m->footprint || fp > m->footprint_ਹੱਦਬੰਦੀ)
      ਵਾਪਸ 0;
  }
  ਜੇ (mmsize > nb) {     /* Check for wrap around 0 */
    ਅੱਖਰ* mm = (ਅੱਖਰ*)(CALL_ਸਿੱਧਾ_MMAP(mmsize));
    ਜੇ (mm != CMFAIL) {
      size_t offset = align_ਵਿੱਥ(chunk2mem(mm));
      size_t psize = mmsize - offset - MMAP_FOOT_PAD;
      mchunkptr p = (mchunkptr)(mm + offset);
      p->prev_foot = offset;
      p->ਸਿਰ = psize;
      mark_inuse_foot(m, p, psize);
      chunk_plus_ਵਿੱਥ(p, psize)->ਸਿਰ = FENCEPOST_ਸਿਰ;
      chunk_plus_ਵਿੱਥ(p, psize+ਮਾਪ_T_ਮਾਪ)->ਸਿਰ = 0;

      ਜੇ (m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr == 0 || mm < m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr)
        m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr = mm;
      ਜੇ ((m->footprint += mmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      ਪ੍ਰਤਿਜ੍ਞਾ(is_aligned(chunk2mem(p)));
      check_mmapped_chunk(m, p);
      ਵਾਪਸ chunk2mem(p);
    }
  }
  ਵਾਪਸ 0;
}

/* Realloc using mmap */
ਸਥਿਰ mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb, ਪੂਰਨ flags) {
  size_t oldsize = chunksize(oldp);
  (ਖਾਲੀ)flags; /* placate people compiling -Wunused */
  ਜੇ (is_small(nb)) /* Can't shrink mmap regions below small size */
    ਵਾਪਸ 0;
  /* Keep old chunk if big enough but not too big */
  ਜੇ (oldsize >= nb + ਮਾਪ_T_ਮਾਪ &&
      (oldsize - nb) <= (mparams.granularity << 1))
    ਵਾਪਸ oldp;
  ਨਹੀਂਤਾਂ {
    size_t offset = oldp->prev_foot;
    size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
    size_t newmmsize = mmap_align(nb + SIX_ਮਾਪ_T_SIZES + CHUNK_ALIGN_ਨਕਾਬ);
    ਅੱਖਰ* cp = (ਅੱਖਰ*)CALL_MREMAP((ਅੱਖਰ*)oldp - offset,
                                  oldmmsize, newmmsize, flags);
    ਜੇ (cp != CMFAIL) {
      mchunkptr newp = (mchunkptr)(cp + offset);
      size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
      newp->ਸਿਰ = psize;
      mark_inuse_foot(m, newp, psize);
      chunk_plus_ਵਿੱਥ(newp, psize)->ਸਿਰ = FENCEPOST_ਸਿਰ;
      chunk_plus_ਵਿੱਥ(newp, psize+ਮਾਪ_T_ਮਾਪ)->ਸਿਰ = 0;

      ਜੇ (cp < m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr)
        m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr = cp;
      ਜੇ ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      check_mmapped_chunk(m, newp);
      ਵਾਪਸ newp;
    }
  }
  ਵਾਪਸ 0;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
ਸਥਿਰ ਖਾਲੀ init_ਸਿਖਰ(mstate m, mchunkptr p, size_t psize) {
  /* Ensure alignment */
  size_t offset = align_ਵਿੱਥ(chunk2mem(p));
  p = (mchunkptr)((ਅੱਖਰ*)p + offset);
  psize -= offset;

  m->ਸਿਖਰ = p;
  m->topsize = psize;
  p->ਸਿਰ = psize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
  /* set size of fake trailing chunk holding overhead space only once */
  chunk_plus_ਵਿੱਥ(p, psize)->ਸਿਰ = ਸਿਖਰ_FOOT_ਮਾਪ;
  m->trim_check = mparams.trim_ਹੱਦ; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
ਸਥਿਰ ਖਾਲੀ init_bins(mstate m) {
  /* Establish circular links for smallbins */
  bindex_t i;
  ਲਈ (i = 0; i < NSMALLBINS; ++i) {
    sbinptr bin = smallbin_at(m,i);
    bin->fd = bin->bk = bin;
  }
}

#ਜੇ PROCEED_ON_ERROR

/* default corruption action */
ਸਥਿਰ ਖਾਲੀ reset_on_error(mstate m) {
  ਪੂਰਨ i;
  ++malloc_corruption_error_ਗਿਣਤੀ;
  /* Reinitialize fields to forget about all memory */
  m->smallmap = m->treemap = 0;
  m->dvsize = m->topsize = 0;
  m->seg.ਅਧਾਰ = 0;
  m->seg.ਮਾਪ = 0;
  m->seg.ਅਗਲਾ = 0;
  m->ਸਿਖਰ = m->dv = 0;
  ਲਈ (i = 0; i < NTREEBINS; ++i)
    *treebin_at(m, i) = 0;
  init_bins(m);
}
#ਜੇ_ਅੰਤ /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
ਸਥਿਰ ਖਾਲੀ* prepend_alloc(mstate m, ਅੱਖਰ* newbase, ਅੱਖਰ* oldbase,
                           size_t nb) {
  mchunkptr p = align_as_chunk(newbase);
  mchunkptr oldfirst = align_as_chunk(oldbase);
  size_t psize = (ਅੱਖਰ*)oldfirst - (ਅੱਖਰ*)p;
  mchunkptr q = chunk_plus_ਵਿੱਥ(p, nb);
  size_t qsize = psize - nb;
  ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, p, nb);

  ਪ੍ਰਤਿਜ੍ਞਾ((ਅੱਖਰ*)oldfirst > (ਅੱਖਰ*)q);
  ਪ੍ਰਤਿਜ੍ਞਾ(pinuse(oldfirst));
  ਪ੍ਰਤਿਜ੍ਞਾ(qsize >= MIN_CHUNK_ਮਾਪ);

  /* consolidate remainder with first chunk of old base */
  ਜੇ (oldfirst == m->ਸਿਖਰ) {
    size_t tsize = m->topsize += qsize;
    m->ਸਿਖਰ = q;
    q->ਸਿਰ = tsize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
    check_ਸਿਖਰ_chunk(m, q);
  }
  ਨਹੀਂਤਾਂ ਜੇ (oldfirst == m->dv) {
    size_t dsize = m->dvsize += qsize;
    m->dv = q;
    ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(q, dsize);
  }
  ਨਹੀਂਤਾਂ {
    ਜੇ (!is_inuse(oldfirst)) {
      size_t nsize = chunksize(oldfirst);
      unlink_chunk(m, oldfirst, nsize);
      oldfirst = chunk_plus_ਵਿੱਥ(oldfirst, nsize);
      qsize += nsize;
    }
    ਸੈੱਟ_free_with_pinuse(q, qsize, oldfirst);
    ਪਾਓ_chunk(m, q, qsize);
    check_free_chunk(m, q);
  }

  check_malloced_chunk(m, chunk2mem(p), nb);
  ਵਾਪਸ chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_segment(mstate m, ਅੱਖਰ* tbase, size_t tsize, flag_t mmapped) {
  /* Determine locations and sizes of segment, fenceposts, old top */
  ਅੱਖਰ* old_ਸਿਖਰ = (ਅੱਖਰ*)m->ਸਿਖਰ;
  msegmentptr oldsp = segment_holding(m, old_ਸਿਖਰ);
  ਅੱਖਰ* old_ਸਿਰਾ = oldsp->ਅਧਾਰ + oldsp->ਮਾਪ;
  size_t ssize = pad_request(ਆਕਾਰ(ਬਣਤਰ malloc_segment));
  ਅੱਖਰ* rawsp = old_ਸਿਰਾ - (ssize + FOUR_ਮਾਪ_T_SIZES + CHUNK_ALIGN_ਨਕਾਬ);
  size_t offset = align_ਵਿੱਥ(chunk2mem(rawsp));
  ਅੱਖਰ* asp = rawsp + offset;
  ਅੱਖਰ* csp = (asp < (old_ਸਿਖਰ + MIN_CHUNK_ਮਾਪ))? old_ਸਿਖਰ : asp;
  mchunkptr sp = (mchunkptr)csp;
  msegmentptr ss = (msegmentptr)(chunk2mem(sp));
  mchunkptr tnext = chunk_plus_ਵਿੱਥ(sp, ssize);
  mchunkptr p = tnext;
  ਪੂਰਨ nfences = 0;
  (ਖਾਲੀ)nfences; /* Suppress unused variable warning */

  /* reset top to new space */
  init_ਸਿਖਰ(m, (mchunkptr)tbase, tsize - ਸਿਖਰ_FOOT_ਮਾਪ);

  /* Set up segment record */
  ਪ੍ਰਤਿਜ੍ਞਾ(is_aligned(ss));
  ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, sp, ssize);
  *ss = m->seg; /* Push current record */
  m->seg.ਅਧਾਰ = tbase;
  m->seg.ਮਾਪ = tsize;
  (ਖਾਲੀ)ਸੈੱਟ_segment_flags(&m->seg, mmapped);
  m->seg.ਅਗਲਾ = ss;

  /* Insert trailing fenceposts */
  ਲਈ (;;) {
    mchunkptr nextp = chunk_plus_ਵਿੱਥ(p, ਮਾਪ_T_ਮਾਪ);
    p->ਸਿਰ = FENCEPOST_ਸਿਰ;
    ++nfences;
    ਜੇ ((ਅੱਖਰ*)(&(nextp->ਸਿਰ)) < old_ਸਿਰਾ)
      p = nextp;
    ਨਹੀਂਤਾਂ
      ਰੋਕੋ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(nfences >= 2);

  /* Insert the rest of old top into a bin as an ordinary free chunk */
  ਜੇ (csp != old_ਸਿਖਰ) {
    mchunkptr q = (mchunkptr)old_ਸਿਖਰ;
    size_t psize = csp - old_ਸਿਖਰ;
    mchunkptr tn = chunk_plus_ਵਿੱਥ(q, psize);
    ਸੈੱਟ_free_with_pinuse(q, psize, tn);
    ਪਾਓ_chunk(m, q, psize);
  }

  check_ਸਿਖਰ_chunk(m, m->ਸਿਖਰ);
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
ਸਥਿਰ ਖਾਲੀ* sys_alloc(mstate m, size_t nb) {
  ਅੱਖਰ* tbase = CMFAIL;
  size_t tsize = 0;
  flag_t mmap_flag = 0;
  size_t asize; /* allocation size */

  ensure_initialization();

  /* Directly map large chunks, but only if already initialized */
  ਜੇ (use_mmap(m) && nb >= mparams.mmap_ਹੱਦ && m->topsize != 0) {
    ਖਾਲੀ* mem = mmap_alloc(m, nb);
    ਜੇ (mem != 0)
      ਵਾਪਸ mem;
  }

  asize = granularity_align(nb + SYS_ALLOC_PADDING);
  ਜੇ (asize <= nb)
    ਵਾਪਸ 0; /* wraparound */
  ਜੇ (m->footprint_ਹੱਦਬੰਦੀ != 0) {
    size_t fp = m->footprint + asize;
    ਜੇ (fp <= m->footprint || fp > m->footprint_ਹੱਦਬੰਦੀ)
      ਵਾਪਸ 0;
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

  ਜੇ (MORECORE_CONTIGUOUS && !use_noncontiguous(m)) {
    ਅੱਖਰ* br = CMFAIL;
    size_t ssize = asize; /* sbrk call size */
    msegmentptr ss = (m->ਸਿਖਰ == 0)? 0 : segment_holding(m, (ਅੱਖਰ*)m->ਸਿਖਰ);
    ACQUIRE_MALLOC_GLOBAL_ਤਾਲਾ();

    ਜੇ (ss == 0) {  /* First time through or recovery */
      ਅੱਖਰ* ਅਧਾਰ = (ਅੱਖਰ*)CALL_MORECORE(0);
      ਜੇ (ਅਧਾਰ != CMFAIL) {
        size_t fp;
        /* Adjust to end on a page boundary */
        ਜੇ (!is_ਪੰਨਾ_aligned(ਅਧਾਰ))
          ssize += (ਪੰਨਾ_align((size_t)ਅਧਾਰ) - (size_t)ਅਧਾਰ);
        fp = m->footprint + ssize; /* recheck limits */
        ਜੇ (ssize > nb && ssize < HALF_MAX_ਮਾਪ_T &&
            (m->footprint_ਹੱਦਬੰਦੀ == 0 ||
             (fp > m->footprint && fp <= m->footprint_ਹੱਦਬੰਦੀ)) &&
            (br = (ਅੱਖਰ*)(CALL_MORECORE(ssize))) == ਅਧਾਰ) {
          tbase = ਅਧਾਰ;
          tsize = ssize;
        }
      }
    }
    ਨਹੀਂਤਾਂ {
      /* Subtract out existing available top space from MORECORE request. */
      ssize = granularity_align(nb - m->topsize + SYS_ALLOC_PADDING);
      /* Use mem here only if it did continuously extend old space */
      ਜੇ (ssize < HALF_MAX_ਮਾਪ_T &&
          (br = (ਅੱਖਰ*)(CALL_MORECORE(ssize))) == ss->ਅਧਾਰ+ss->ਮਾਪ) {
        tbase = br;
        tsize = ssize;
      }
    }

    ਜੇ (tbase == CMFAIL) {    /* Cope with partial failure */
      ਜੇ (br != CMFAIL) {    /* Try to use/extend the space we did get */
        ਜੇ (ssize < HALF_MAX_ਮਾਪ_T &&
            ssize < nb + SYS_ALLOC_PADDING) {
          size_t esize = granularity_align(nb + SYS_ALLOC_PADDING - ssize);
          ਜੇ (esize < HALF_MAX_ਮਾਪ_T) {
            ਅੱਖਰ* ਸਿਰਾ = (ਅੱਖਰ*)CALL_MORECORE(esize);
            ਜੇ (ਸਿਰਾ != CMFAIL)
              ssize += esize;
            ਨਹੀਂਤਾਂ {            /* Can't use; try to release */
              (ਖਾਲੀ) CALL_MORECORE(-ssize);
              br = CMFAIL;
            }
          }
        }
      }
      ਜੇ (br != CMFAIL) {    /* Use the space we did get */
        tbase = br;
        tsize = ssize;
      }
      ਨਹੀਂਤਾਂ
        disable_contiguous(m); /* Don't try contiguous path in the future */
    }

    RELEASE_MALLOC_GLOBAL_ਤਾਲਾ();
  }

  ਜੇ (HAVE_MMAP && tbase == CMFAIL) {  /* Try MMAP */
    ਅੱਖਰ* mp = (ਅੱਖਰ*)(CALL_MMAP(asize));
    ਜੇ (mp != CMFAIL) {
      tbase = mp;
      tsize = asize;
      mmap_flag = USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ;
    }
  }

  ਜੇ (HAVE_MORECORE && tbase == CMFAIL) { /* Try noncontiguous MORECORE */
    ਜੇ (asize < HALF_MAX_ਮਾਪ_T) {
      ਅੱਖਰ* br = CMFAIL;
      ਅੱਖਰ* ਸਿਰਾ = CMFAIL;
      ACQUIRE_MALLOC_GLOBAL_ਤਾਲਾ();
      br = (ਅੱਖਰ*)(CALL_MORECORE(asize));
      ਸਿਰਾ = (ਅੱਖਰ*)(CALL_MORECORE(0));
      RELEASE_MALLOC_GLOBAL_ਤਾਲਾ();
      ਜੇ (br != CMFAIL && ਸਿਰਾ != CMFAIL && br < ਸਿਰਾ) {
        size_t ssize = ਸਿਰਾ - br;
        ਜੇ (ssize > nb + ਸਿਖਰ_FOOT_ਮਾਪ) {
          tbase = br;
          tsize = ssize;
        }
      }
    }
  }

  ਜੇ (tbase != CMFAIL) {

    ਜੇ ((m->footprint += tsize) > m->max_footprint)
      m->max_footprint = m->footprint;

    ਜੇ (!is_initialized(m)) { /* first-time initialization */
      ਜੇ (m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr == 0 || tbase < m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr)
        m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr = tbase;
      m->seg.ਅਧਾਰ = tbase;
      m->seg.ਮਾਪ = tsize;
      (ਖਾਲੀ)ਸੈੱਟ_segment_flags(&m->seg, mmap_flag);
      m->magic = mparams.magic;
      m->release_checks = MAX_RELEASE_CHECK_ਦਰ;
      init_bins(m);
#ਜੇ !ONLY_MSPACES
      ਜੇ (is_global(m))
        init_ਸਿਖਰ(m, (mchunkptr)tbase, tsize - ਸਿਖਰ_FOOT_ਮਾਪ);
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
      {
        /* Offset top by embedded malloc_state */
        mchunkptr mn = ਅਗਲਾ_chunk(mem2chunk(m));
        init_ਸਿਖਰ(m, mn, (size_t)((tbase + tsize) - (ਅੱਖਰ*)mn) -ਸਿਖਰ_FOOT_ਮਾਪ);
      }
    }

    ਨਹੀਂਤਾਂ {
      /* Try to merge with an existing segment */
      msegmentptr sp = &m->seg;
      /* Only consider most recent segment if traversal suppressed */
      ਜਦੋਂ (sp != 0 && tbase != sp->ਅਧਾਰ + sp->ਮਾਪ)
        sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->ਅਗਲਾ;
      ਜੇ (sp != 0 &&
          !is_extern_segment(sp) &&
          check_segment_ਮਿਲਾਓ(sp, tbase, tsize) &&
          (get_segment_flags(sp) & USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ) == mmap_flag &&
          segment_holds(sp, m->ਸਿਖਰ)) { /* append */
        sp->ਮਾਪ += tsize;
        init_ਸਿਖਰ(m, m->ਸਿਖਰ, m->topsize + tsize);
      }
      ਨਹੀਂਤਾਂ {
        ਜੇ (tbase < m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr)
          m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr = tbase;
        sp = &m->seg;
        ਜਦੋਂ (sp != 0 && sp->ਅਧਾਰ != tbase + tsize)
          sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->ਅਗਲਾ;
        ਜੇ (sp != 0 &&
            !is_extern_segment(sp) &&
            check_segment_ਮਿਲਾਓ(sp, tbase, tsize) &&
            (get_segment_flags(sp) & USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ) == mmap_flag) {
          ਅੱਖਰ* oldbase = sp->ਅਧਾਰ;
          sp->ਅਧਾਰ = tbase;
          sp->ਮਾਪ += tsize;
          ਵਾਪਸ prepend_alloc(m, tbase, oldbase, nb);
        }
        ਨਹੀਂਤਾਂ
          ਜੋੜੋ_segment(m, tbase, tsize, mmap_flag);
      }
    }

    ਜੇ (nb < m->topsize) { /* Allocate from new or extended top space */
      size_t rsize = m->topsize -= nb;
      mchunkptr p = m->ਸਿਖਰ;
      mchunkptr r = m->ਸਿਖਰ = chunk_plus_ਵਿੱਥ(p, nb);
      r->ਸਿਰ = rsize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
      ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, p, nb);
      check_ਸਿਖਰ_chunk(m, m->ਸਿਖਰ);
      check_malloced_chunk(m, chunk2mem(p), nb);
      ਵਾਪਸ chunk2mem(p);
    }
  }

  MALLOC_FAILURE_ACTION;
  ਵਾਪਸ 0;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
ਸਥਿਰ size_t release_ਅਣਵਰਤਿਆ_segments(mstate m) {
  size_t released = 0;
  ਪੂਰਨ nsegs = 0;
  msegmentptr pred = &m->seg;
  msegmentptr sp = pred->ਅਗਲਾ;
  ਜਦੋਂ (sp != 0) {
    ਅੱਖਰ* ਅਧਾਰ = sp->ਅਧਾਰ;
    size_t ਮਾਪ = sp->ਮਾਪ;
    msegmentptr ਅਗਲਾ = sp->ਅਗਲਾ;
    ++nsegs;
    ਜੇ (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
      mchunkptr p = align_as_chunk(ਅਧਾਰ);
      size_t psize = chunksize(p);
      /* Can unmap if first chunk holds entire segment and not pinned */
      ਜੇ (!is_inuse(p) && (ਅੱਖਰ*)p + psize >= ਅਧਾਰ + ਮਾਪ - ਸਿਖਰ_FOOT_ਮਾਪ) {
        tchunkptr tp = (tchunkptr)p;
        ਪ੍ਰਤਿਜ੍ਞਾ(segment_holds(sp, (ਅੱਖਰ*)sp));
        ਜੇ (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        ਨਹੀਂਤਾਂ {
          unlink_large_chunk(m, tp);
        }
        ਜੇ (CALL_MUNMAP(ਅਧਾਰ, ਮਾਪ) == 0) {
          released += ਮਾਪ;
          m->footprint -= ਮਾਪ;
          /* unlink obsoleted record */
          sp = pred;
          sp->ਅਗਲਾ = ਅਗਲਾ;
        }
        ਨਹੀਂਤਾਂ { /* back out if cannot unmap */
          ਪਾਓ_large_chunk(m, tp, psize);
        }
      }
    }
    ਜੇ (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
      ਰੋਕੋ;
    pred = sp;
    sp = ਅਗਲਾ;
  }
  /* Reset check counter */
  m->release_checks = (((size_t) nsegs > (size_t) MAX_RELEASE_CHECK_ਦਰ)?
                       (size_t) nsegs : (size_t) MAX_RELEASE_CHECK_ਦਰ);
  ਵਾਪਸ released;
}

ਸਥਿਰ ਪੂਰਨ sys_trim(mstate m, size_t pad) {
  size_t released = 0;
  ensure_initialization();
  ਜੇ (pad < MAX_REQUEST && is_initialized(m)) {
    pad += ਸਿਖਰ_FOOT_ਮਾਪ; /* ensure enough room for segment overhead */

    ਜੇ (m->topsize > pad) {
      /* Shrink top space in granularity-size units, keeping at least one */
      size_t unit = mparams.granularity;
      size_t extra = ((m->topsize - pad + (unit - ਮਾਪ_T_ONE)) / unit -
                      ਮਾਪ_T_ONE) * unit;
      msegmentptr sp = segment_holding(m, (ਅੱਖਰ*)m->ਸਿਖਰ);

      ਜੇ (sp == 0)
        CORRUPTION_ERROR_ACTION(m);
      ਜੇ (!is_extern_segment(sp)) {
        ਜੇ (is_mmapped_segment(sp)) {
          ਜੇ (HAVE_MMAP &&
              sp->ਮਾਪ >= extra &&
              !has_segment_link(m, sp)) { /* can't shrink if pinned */
            size_t newsize = sp->ਮਾਪ - extra;
            (ਖਾਲੀ)newsize; /* placate people compiling -Wunused-variable */
            /* Prefer mremap, fall back to munmap */
            ਜੇ ((CALL_MREMAP(sp->ਅਧਾਰ, sp->ਮਾਪ, newsize, 0) != MFAIL) ||
                (CALL_MUNMAP(sp->ਅਧਾਰ + newsize, extra) == 0)) {
              released = extra;
            }
          }
        }
        ਨਹੀਂਤਾਂ ਜੇ (HAVE_MORECORE) {
          ਜੇ (extra >= HALF_MAX_ਮਾਪ_T) /* Avoid wrapping negative */
            extra = (HALF_MAX_ਮਾਪ_T) + ਮਾਪ_T_ONE - unit;
          ACQUIRE_MALLOC_GLOBAL_ਤਾਲਾ();
          {
            /* Make sure end of memory is where we last set it. */
            ਅੱਖਰ* old_br = (ਅੱਖਰ*)(CALL_MORECORE(0));
            ਜੇ (old_br == sp->ਅਧਾਰ + sp->ਮਾਪ) {
              ਅੱਖਰ* rel_br = (ਅੱਖਰ*)(CALL_MORECORE(-extra));
              ਅੱਖਰ* new_br = (ਅੱਖਰ*)(CALL_MORECORE(0));
              ਜੇ (rel_br != CMFAIL && new_br < old_br)
                released = old_br - new_br;
            }
          }
          RELEASE_MALLOC_GLOBAL_ਤਾਲਾ();
        }
      }

      ਜੇ (released != 0) {
        sp->ਮਾਪ -= released;
        m->footprint -= released;
        init_ਸਿਖਰ(m, m->ਸਿਖਰ, m->topsize - released);
        check_ਸਿਖਰ_chunk(m, m->ਸਿਖਰ);
      }
    }

    /* Unmap any unused mmapped segments */
    ਜੇ (HAVE_MMAP)
      released += release_ਅਣਵਰਤਿਆ_segments(m);

    /* On failure, disable autotrim to avoid repeated failed future calls */
    ਜੇ (released == 0 && m->topsize > m->trim_check)
      m->trim_check = MAX_ਮਾਪ_T;
  }

  ਵਾਪਸ (released != 0)? 1 : 0;
}

/* Consolidate and bin a chunk. Differs from exported versions
   of free mainly in that the chunk need not be marked as inuse.
*/
ਸਥਿਰ ਖਾਲੀ dispose_chunk(mstate m, mchunkptr p, size_t psize) {
  mchunkptr ਅਗਲਾ = chunk_plus_ਵਿੱਥ(p, psize);
  ਜੇ (!pinuse(p)) {
    mchunkptr prev;
    size_t prevsize = p->prev_foot;
    ਜੇ (is_mmapped(p)) {
      psize += prevsize + MMAP_FOOT_PAD;
      ਜੇ (CALL_MUNMAP((ਅੱਖਰ*)p - prevsize, psize) == 0)
        m->footprint -= psize;
      ਵਾਪਸ;
    }
    prev = chunk_minus_ਵਿੱਥ(p, prevsize);
    psize += prevsize;
    p = prev;
    ਜੇ (RTCHECK(ok_ਪਤਾ(m, prev))) { /* consolidate backward */
      ਜੇ (p != m->dv) {
        unlink_chunk(m, p, prevsize);
      }
      ਨਹੀਂਤਾਂ ਜੇ ((ਅਗਲਾ->ਸਿਰ & INUSE_ਬਿੱਟ) == INUSE_ਬਿੱਟ) {
        m->dvsize = psize;
        ਸੈੱਟ_free_with_pinuse(p, psize, ਅਗਲਾ);
        ਵਾਪਸ;
      }
    }
    ਨਹੀਂਤਾਂ {
      CORRUPTION_ERROR_ACTION(m);
      ਵਾਪਸ;
    }
  }
  ਜੇ (RTCHECK(ok_ਪਤਾ(m, ਅਗਲਾ))) {
    ਜੇ (!cinuse(ਅਗਲਾ)) {  /* consolidate forward */
      ਜੇ (ਅਗਲਾ == m->ਸਿਖਰ) {
        size_t tsize = m->topsize += psize;
        m->ਸਿਖਰ = p;
        p->ਸਿਰ = tsize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
        ਜੇ (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        ਵਾਪਸ;
      }
      ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == m->dv) {
        size_t dsize = m->dvsize += psize;
        m->dv = p;
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, dsize);
        ਵਾਪਸ;
      }
      ਨਹੀਂਤਾਂ {
        size_t nsize = chunksize(ਅਗਲਾ);
        psize += nsize;
        unlink_chunk(m, ਅਗਲਾ, nsize);
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, psize);
        ਜੇ (p == m->dv) {
          m->dvsize = psize;
          ਵਾਪਸ;
        }
      }
    }
    ਨਹੀਂਤਾਂ {
      ਸੈੱਟ_free_with_pinuse(p, psize, ਅਗਲਾ);
    }
    ਪਾਓ_chunk(m, p, psize);
  }
  ਨਹੀਂਤਾਂ {
    CORRUPTION_ERROR_ACTION(m);
  }
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
ਸਥਿਰ ਖਾਲੀ* tmalloc_large(mstate m, size_t nb) {
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  bindex_t idx;
  compute_ਰੁੱਖ_ਸੂਚਕਾਂਕ(nb, idx);
  ਜੇ ((t = *treebin_at(m, idx)) != 0) {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_ਰੁੱਖ_ਸੂਚਕਾਂਕ(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    ਲਈ (;;) {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      ਜੇ (trem < rsize) {
        v = t;
        ਜੇ ((rsize = trem) == 0)
          ਰੋਕੋ;
      }
      rt = t->ਬੱਚਾ[1];
      t = t->ਬੱਚਾ[(sizebits >> (ਮਾਪ_T_BITSIZE-ਮਾਪ_T_ONE)) & 1];
      ਜੇ (rt != 0 && rt != t)
        rst = rt;
      ਜੇ (t == 0) {
        t = rst; /* set t to least subtree holding sizes > nb */
        ਰੋਕੋ;
      }
      sizebits <<= 1;
    }
  }
  ਜੇ (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
    binmap_t leftbits = ਖੱਬਾ_ਬਿੱਟ(idx2bit(idx)) & m->treemap;
    ਜੇ (leftbits != 0) {
      bindex_t i;
      binmap_t leastbit = ਘੱਟ_ਤੋਂ_ਘੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  ਜਦੋਂ (t != 0) { /* find smallest of tree or subtree */
    size_t trem = chunksize(t) - nb;
    ਜੇ (trem < rsize) {
      rsize = trem;
      v = t;
    }
    t = leftmost_ਬੱਚਾ(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  ਜੇ (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
    ਜੇ (RTCHECK(ok_ਪਤਾ(m, v))) { /*ਵੰਡੋ*/
      mchunkptr r = chunk_plus_ਵਿੱਥ(v, nb);
      ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(v) == rsize + nb);
      ਜੇ (RTCHECK(ok_ਅਗਲਾ(v, r))) {
        unlink_large_chunk(m, v);
        ਜੇ (rsize < MIN_CHUNK_ਮਾਪ)
          ਸੈੱਟ_inuse_and_pinuse(m, v, (rsize + nb));
        ਨਹੀਂਤਾਂ {
          ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, v, nb);
          ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(r, rsize);
          ਪਾਓ_chunk(m, r, rsize);
        }
        ਵਾਪਸ chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  ਵਾਪਸ 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
ਸਥਿਰ ਖਾਲੀ* tmalloc_small(mstate m, size_t nb) {
  tchunkptr t, v;
  size_t rsize;
  bindex_t i;
  binmap_t leastbit = ਘੱਟ_ਤੋਂ_ਘੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(m->treemap);
  compute_bit2idx(leastbit, i);
  v = t = *treebin_at(m, i);
  rsize = chunksize(t) - nb;

  ਜਦੋਂ ((t = leftmost_ਬੱਚਾ(t)) != 0) {
    size_t trem = chunksize(t) - nb;
    ਜੇ (trem < rsize) {
      rsize = trem;
      v = t;
    }
  }

  ਜੇ (RTCHECK(ok_ਪਤਾ(m, v))) {
    mchunkptr r = chunk_plus_ਵਿੱਥ(v, nb);
    ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(v) == rsize + nb);
    ਜੇ (RTCHECK(ok_ਅਗਲਾ(v, r))) {
      unlink_large_chunk(m, v);
      ਜੇ (rsize < MIN_CHUNK_ਮਾਪ)
        ਸੈੱਟ_inuse_and_pinuse(m, v, (rsize + nb));
      ਨਹੀਂਤਾਂ {
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, v, nb);
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(r, rsize);
        replace_dv(m, r, rsize);
      }
      ਵਾਪਸ chunk2mem(v);
    }
  }

  CORRUPTION_ERROR_ACTION(m);
  ਵਾਪਸ 0;
}

#ਜੇ !ONLY_MSPACES

ਖਾਲੀ* dlmalloc(size_t ਬਾਈਟਾਂ) {
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

#ਜੇ USE_LOCKS
  ensure_initialization(); /* initialize in sys_alloc if not using locks */
#ਜੇ_ਅੰਤ

  ਜੇ (!PREACTION(gm)) {
    ਖਾਲੀ* mem;
    size_t nb;
    ਜੇ (ਬਾਈਟਾਂ <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (ਬਾਈਟਾਂ < MIN_REQUEST)? MIN_CHUNK_ਮਾਪ : pad_request(ਬਾਈਟਾਂ);
      idx = small_ਸੂਚਕਾਂਕ(nb);
      smallbits = gm->smallmap >> idx;

      ਜੇ ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(gm, idx);
        p = b->fd;
        ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(p) == small_index2size(idx));
        unlink_ਪਹਿਲਾ_small_chunk(gm, b, p, idx);
        ਸੈੱਟ_inuse_and_pinuse(gm, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(gm, mem, nb);
        ਜਾਓ postaction;
      }

      ਨਹੀਂਤਾਂ ਜੇ (nb > gm->dvsize) {
        ਜੇ (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & ਖੱਬਾ_ਬਿੱਟ(idx2bit(idx));
          binmap_t leastbit = ਘੱਟ_ਤੋਂ_ਘੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(gm, i);
          p = b->fd;
          ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(p) == small_index2size(i));
          unlink_ਪਹਿਲਾ_small_chunk(gm, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          ਜੇ (ਮਾਪ_T_ਮਾਪ != 4 && rsize < MIN_CHUNK_ਮਾਪ)
            ਸੈੱਟ_inuse_and_pinuse(gm, p, small_index2size(i));
          ਨਹੀਂਤਾਂ {
            ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(gm, p, nb);
            r = chunk_plus_ਵਿੱਥ(p, nb);
            ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(r, rsize);
            replace_dv(gm, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(gm, mem, nb);
          ਜਾਓ postaction;
        }

        ਨਹੀਂਤਾਂ ਜੇ (gm->treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0) {
          check_malloced_chunk(gm, mem, nb);
          ਜਾਓ postaction;
        }
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ >= MAX_REQUEST)
      nb = MAX_ਮਾਪ_T; /* Too big to allocate. Force failure (in sys alloc) */
    ਨਹੀਂਤਾਂ {
      nb = pad_request(ਬਾਈਟਾਂ);
      ਜੇ (gm->treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0) {
        check_malloced_chunk(gm, mem, nb);
        ਜਾਓ postaction;
      }
    }

    ਜੇ (nb <= gm->dvsize) {
      size_t rsize = gm->dvsize - nb;
      mchunkptr p = gm->dv;
      ਜੇ (rsize >= MIN_CHUNK_ਮਾਪ) { /* split dv */
        mchunkptr r = gm->dv = chunk_plus_ਵਿੱਥ(p, nb);
        gm->dvsize = rsize;
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(r, rsize);
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(gm, p, nb);
      }
      ਨਹੀਂਤਾਂ { /* exhaust dv */
        size_t dvs = gm->dvsize;
        gm->dvsize = 0;
        gm->dv = 0;
        ਸੈੱਟ_inuse_and_pinuse(gm, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(gm, mem, nb);
      ਜਾਓ postaction;
    }

    ਨਹੀਂਤਾਂ ਜੇ (nb < gm->topsize) { /* Split top */
      size_t rsize = gm->topsize -= nb;
      mchunkptr p = gm->ਸਿਖਰ;
      mchunkptr r = gm->ਸਿਖਰ = chunk_plus_ਵਿੱਥ(p, nb);
      r->ਸਿਰ = rsize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
      ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(gm, p, nb);
      mem = chunk2mem(p);
      check_ਸਿਖਰ_chunk(gm, gm->ਸਿਖਰ);
      check_malloced_chunk(gm, mem, nb);
      ਜਾਓ postaction;
    }

    mem = sys_alloc(gm, nb);

  postaction:
    POSTACTION(gm);
    ਵਾਪਸ mem;
  }

  ਵਾਪਸ 0;
}

/* ---------------------------- free --------------------------- */

ਖਾਲੀ dlfree(ਖਾਲੀ* mem) {
  /*
     Consolidate freed chunks with preceding or succeeding bordering
     free chunks, if they exist, and then place in a bin.  Intermixed
     with special cases for top, dv, mmapped chunks, and usage errors.
  */

  ਜੇ (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#ਜੇ FOOTERS
    mstate fm = get_mstate_for(p);
    ਜੇ (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      ਵਾਪਸ;
    }
#ਨਹੀਂਤਾਂ /* FOOTERS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fm gm
#ਜੇ_ਅੰਤ /* FOOTERS */
    ਜੇ (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      ਜੇ (RTCHECK(ok_ਪਤਾ(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr ਅਗਲਾ = chunk_plus_ਵਿੱਥ(p, psize);
        ਜੇ (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          ਜੇ (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            ਜੇ (CALL_MUNMAP((ਅੱਖਰ*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            ਜਾਓ postaction;
          }
          ਨਹੀਂਤਾਂ {
            mchunkptr prev = chunk_minus_ਵਿੱਥ(p, prevsize);
            psize += prevsize;
            p = prev;
            ਜੇ (RTCHECK(ok_ਪਤਾ(fm, prev))) { /* consolidate backward */
              ਜੇ (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              ਨਹੀਂਤਾਂ ਜੇ ((ਅਗਲਾ->ਸਿਰ & INUSE_ਬਿੱਟ) == INUSE_ਬਿੱਟ) {
                fm->dvsize = psize;
                ਸੈੱਟ_free_with_pinuse(p, psize, ਅਗਲਾ);
                ਜਾਓ postaction;
              }
            }
            ਨਹੀਂਤਾਂ
              ਜਾਓ erroraction;
          }
        }

        ਜੇ (RTCHECK(ok_ਅਗਲਾ(p, ਅਗਲਾ) && ok_pinuse(ਅਗਲਾ))) {
          ਜੇ (!cinuse(ਅਗਲਾ)) {  /* consolidate forward */
            ਜੇ (ਅਗਲਾ == fm->ਸਿਖਰ) {
              size_t tsize = fm->topsize += psize;
              fm->ਸਿਖਰ = p;
              p->ਸਿਰ = tsize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
              ਜੇ (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              ਜੇ (should_trim(fm, tsize))
                sys_trim(fm, 0);
              ਜਾਓ postaction;
            }
            ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, dsize);
              ਜਾਓ postaction;
            }
            ਨਹੀਂਤਾਂ {
              size_t nsize = chunksize(ਅਗਲਾ);
              psize += nsize;
              unlink_chunk(fm, ਅਗਲਾ, nsize);
              ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, psize);
              ਜੇ (p == fm->dv) {
                fm->dvsize = psize;
                ਜਾਓ postaction;
              }
            }
          }
          ਨਹੀਂਤਾਂ
            ਸੈੱਟ_free_with_pinuse(p, psize, ਅਗਲਾ);

          ਜੇ (is_small(psize)) {
            ਪਾਓ_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          ਨਹੀਂਤਾਂ {
            tchunkptr tp = (tchunkptr)p;
            ਪਾਓ_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            ਜੇ (--fm->release_checks == 0)
              release_ਅਣਵਰਤਿਆ_segments(fm);
          }
          ਜਾਓ postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
#ਜੇ !FOOTERS
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ fm
#ਜੇ_ਅੰਤ /* FOOTERS */
}

ਖਾਲੀ* dlcalloc(size_t n_elements, size_t elem_ਮਾਪ) {
  ਖਾਲੀ* mem;
  size_t req = 0;
  ਜੇ (n_elements != 0) {
    req = n_elements * elem_ਮਾਪ;
    ਜੇ (((n_elements | elem_ਮਾਪ) & ~(size_t)0xffff) &&
        (req / n_elements != elem_ਮਾਪ))
      req = MAX_ਮਾਪ_T; /* force downstream failure on overflow */
  }
  mem = dlmalloc(req);
  ਜੇ (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  ਵਾਪਸ mem;
}

#ਜੇ_ਅੰਤ /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
ਸਥਿਰ mchunkptr try_realloc_chunk(mstate m, mchunkptr p, size_t nb,
                                   ਪੂਰਨ can_move) {
  mchunkptr newp = 0;
  size_t oldsize = chunksize(p);
  mchunkptr ਅਗਲਾ = chunk_plus_ਵਿੱਥ(p, oldsize);
  ਜੇ (RTCHECK(ok_ਪਤਾ(m, p) && ok_inuse(p) &&
              ok_ਅਗਲਾ(p, ਅਗਲਾ) && ok_pinuse(ਅਗਲਾ))) {
    ਜੇ (is_mmapped(p)) {
      newp = mmap_resize(m, p, nb, can_move);
    }
    ਨਹੀਂਤਾਂ ਜੇ (oldsize >= nb) {             /* already big enough */
      size_t rsize = oldsize - nb;
      ਜੇ (rsize >= MIN_CHUNK_ਮਾਪ) {      /* split off remainder */
        mchunkptr r = chunk_plus_ਵਿੱਥ(p, nb);
        ਸੈੱਟ_inuse(m, p, nb);
        ਸੈੱਟ_inuse(m, r, rsize);
        dispose_chunk(m, r, rsize);
      }
      newp = p;
    }
    ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == m->ਸਿਖਰ) {  /* extend into top */
      ਜੇ (oldsize + m->topsize > nb) {
        size_t newsize = oldsize + m->topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_ਵਿੱਥ(p, nb);
        ਸੈੱਟ_inuse(m, p, nb);
        newtop->ਸਿਰ = newtopsize |PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
        m->ਸਿਖਰ = newtop;
        m->topsize = newtopsize;
        newp = p;
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == m->dv) { /* extend into dv */
      size_t dvs = m->dvsize;
      ਜੇ (oldsize + dvs >= nb) {
        size_t dsize = oldsize + dvs - nb;
        ਜੇ (dsize >= MIN_CHUNK_ਮਾਪ) {
          mchunkptr r = chunk_plus_ਵਿੱਥ(p, nb);
          mchunkptr n = chunk_plus_ਵਿੱਥ(r, dsize);
          ਸੈੱਟ_inuse(m, p, nb);
          ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(r, dsize);
          clear_pinuse(n);
          m->dvsize = dsize;
          m->dv = r;
        }
        ਨਹੀਂਤਾਂ { /* exhaust dv */
          size_t newsize = oldsize + dvs;
          ਸੈੱਟ_inuse(m, p, newsize);
          m->dvsize = 0;
          m->dv = 0;
        }
        newp = p;
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ (!cinuse(ਅਗਲਾ)) { /* extend into next free chunk */
      size_t nextsize = chunksize(ਅਗਲਾ);
      ਜੇ (oldsize + nextsize >= nb) {
        size_t rsize = oldsize + nextsize - nb;
        unlink_chunk(m, ਅਗਲਾ, nextsize);
        ਜੇ (rsize < MIN_CHUNK_ਮਾਪ) {
          size_t newsize = oldsize + nextsize;
          ਸੈੱਟ_inuse(m, p, newsize);
        }
        ਨਹੀਂਤਾਂ {
          mchunkptr r = chunk_plus_ਵਿੱਥ(p, nb);
          ਸੈੱਟ_inuse(m, p, nb);
          ਸੈੱਟ_inuse(m, r, rsize);
          dispose_chunk(m, r, rsize);
        }
        newp = p;
      }
    }
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(m, chunk2mem(p));
  }
  ਵਾਪਸ newp;
}

ਸਥਿਰ ਖਾਲੀ* ਅੰਦਰੂਨੀ_memalign(mstate m, size_t alignment, size_t ਬਾਈਟਾਂ) {
  ਖਾਲੀ* mem = 0;
  ਜੇ (alignment <  MIN_CHUNK_ਮਾਪ) /* must be at least a minimum chunk size */
    alignment = MIN_CHUNK_ਮਾਪ;
  ਜੇ ((alignment & (alignment-ਮਾਪ_T_ONE)) != 0) {/* Ensure a power of 2 */
    size_t a = MALLOC_ALIGNMENT << 1;
    ਜਦੋਂ (a < alignment) a <<= 1;
    alignment = a;
  }
  ਜੇ (ਬਾਈਟਾਂ >= MAX_REQUEST - alignment) {
    ਜੇ (m != 0)  { /* Test isn't needed but avoids compiler warning */
      MALLOC_FAILURE_ACTION;
    }
  }
  ਨਹੀਂਤਾਂ {
    size_t nb = request2size(ਬਾਈਟਾਂ);
    size_t req = nb + alignment + MIN_CHUNK_ਮਾਪ - CHUNK_OVERHEAD;
    mem = ਅੰਦਰੂਨੀ_malloc(m, req);
    ਜੇ (mem != 0) {
      mchunkptr p = mem2chunk(mem);
      ਜੇ (PREACTION(m))
        ਵਾਪਸ 0;
      ਜੇ ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
        ਅੱਖਰ* br = (ਅੱਖਰ*)mem2chunk((size_t)(((size_t)((ਅੱਖਰ*)mem + alignment -
                                                       ਮਾਪ_T_ONE)) &
                                             -alignment));
        ਅੱਖਰ* pos = ((size_t)(br - (ਅੱਖਰ*)(p)) >= MIN_CHUNK_ਮਾਪ)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (ਅੱਖਰ*)(p);
        size_t newsize = chunksize(p) - leadsize;

        ਜੇ (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
          newp->prev_foot = p->prev_foot + leadsize;
          newp->ਸਿਰ = newsize;
        }
        ਨਹੀਂਤਾਂ { /* Otherwise, give back leader, use the rest */
          ਸੈੱਟ_inuse(m, newp, newsize);
          ਸੈੱਟ_inuse(m, p, leadsize);
          dispose_chunk(m, p, leadsize);
        }
        p = newp;
      }

      /* Give back spare room at the end */
      ਜੇ (!is_mmapped(p)) {
        size_t ਮਾਪ = chunksize(p);
        ਜੇ (ਮਾਪ > nb + MIN_CHUNK_ਮਾਪ) {
          size_t remainder_ਮਾਪ = ਮਾਪ - nb;
          mchunkptr remainder = chunk_plus_ਵਿੱਥ(p, nb);
          ਸੈੱਟ_inuse(m, p, nb);
          ਸੈੱਟ_inuse(m, remainder, remainder_ਮਾਪ);
          dispose_chunk(m, remainder, remainder_ਮਾਪ);
        }
      }

      mem = chunk2mem(p);
      ਪ੍ਰਤਿਜ੍ਞਾ (chunksize(p) >= nb);
      ਪ੍ਰਤਿਜ੍ਞਾ(((size_t)mem & (alignment - 1)) == 0);
      check_inuse_chunk(m, p);
      POSTACTION(m);
    }
  }
  ਵਾਪਸ mem;
}

/*
  Common support for independent_X routines, handling
    all of the combinations that can result.
  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/
ਸਥਿਰ ਖਾਲੀ** ialloc(mstate m,
                     size_t n_elements,
                     size_t* sizes,
                     ਪੂਰਨ opts,
                     ਖਾਲੀ* chunks[]) {

  size_t    element_ਮਾਪ;   /* chunksize of each element, if all same */
  size_t    contents_ਮਾਪ;  /* total size of elements */
  size_t    array_ਮਾਪ;     /* request size of pointer array */
  ਖਾਲੀ*     mem;            /* malloced aggregate space */
  mchunkptr p;              /* corresponding chunk */
  size_t    remainder_ਮਾਪ; /* remaining bytes while splitting */
  ਖਾਲੀ**    marray;         /* either "chunks" or malloced ptr array */
  mchunkptr array_chunk;    /* chunk for malloced ptr array */
  flag_t    was_enabled;    /* to disable mmap */
  size_t    ਮਾਪ;
  size_t    i;

  ensure_initialization();
  /* compute array length, if needed */
  ਜੇ (chunks != 0) {
    ਜੇ (n_elements == 0)
      ਵਾਪਸ chunks; /* nothing to do */
    marray = chunks;
    array_ਮਾਪ = 0;
  }
  ਨਹੀਂਤਾਂ {
    /* if empty req, must still return chunk representing empty array */
    ਜੇ (n_elements == 0)
      ਵਾਪਸ (ਖਾਲੀ**)ਅੰਦਰੂਨੀ_malloc(m, 0);
    marray = 0;
    array_ਮਾਪ = request2size(n_elements * (ਆਕਾਰ(ਖਾਲੀ*)));
  }

  /* compute total element size */
  ਜੇ (opts & 0x1) { /* all-same-size */
    element_ਮਾਪ = request2size(*sizes);
    contents_ਮਾਪ = n_elements * element_ਮਾਪ;
  }
  ਨਹੀਂਤਾਂ { /* add up all the sizes */
    element_ਮਾਪ = 0;
    contents_ਮਾਪ = 0;
    ਲਈ (i = 0; i != n_elements; ++i)
      contents_ਮਾਪ += request2size(sizes[i]);
  }

  ਮਾਪ = contents_ਮਾਪ + array_ਮਾਪ;

  /*
     Allocate the aggregate chunk.  First disable direct-mmapping so
     malloc won't use it, since we would not be able to later
     free/realloc space internal to a segregated mmap region.
  */
  was_enabled = use_mmap(m);
  disable_mmap(m);
  mem = ਅੰਦਰੂਨੀ_malloc(m, ਮਾਪ - CHUNK_OVERHEAD);
  ਜੇ (was_enabled)
    enable_mmap(m);
  ਜੇ (mem == 0)
    ਵਾਪਸ 0;

  ਜੇ (PREACTION(m)) ਵਾਪਸ 0;
  p = mem2chunk(mem);
  remainder_ਮਾਪ = chunksize(p);

  ਪ੍ਰਤਿਜ੍ਞਾ(!is_mmapped(p));

  ਜੇ (opts & 0x2) {       /* optionally clear the elements */
    memset((size_t*)mem, 0, remainder_ਮਾਪ - ਮਾਪ_T_ਮਾਪ - array_ਮਾਪ);
  }

  /* If not provided, allocate the pointer array as final part of chunk */
  ਜੇ (marray == 0) {
    size_t  array_chunk_ਮਾਪ;
    array_chunk = chunk_plus_ਵਿੱਥ(p, contents_ਮਾਪ);
    array_chunk_ਮਾਪ = remainder_ਮਾਪ - contents_ਮਾਪ;
    marray = (ਖਾਲੀ**) (chunk2mem(array_chunk));
    ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, array_chunk, array_chunk_ਮਾਪ);
    remainder_ਮਾਪ = contents_ਮਾਪ;
  }

  /* split out elements */
  ਲਈ (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    ਜੇ (i != n_elements-1) {
      ਜੇ (element_ਮਾਪ != 0)
        ਮਾਪ = element_ਮਾਪ;
      ਨਹੀਂਤਾਂ
        ਮਾਪ = request2size(sizes[i]);
      remainder_ਮਾਪ -= ਮਾਪ;
      ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, p, ਮਾਪ);
      p = chunk_plus_ਵਿੱਥ(p, ਮਾਪ);
    }
    ਨਹੀਂਤਾਂ { /* the final element absorbs any overallocation slop */
      ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(m, p, remainder_ਮਾਪ);
      ਰੋਕੋ;
    }
  }

#ਜੇ DEBUG
  ਜੇ (marray != chunks) {
    /* final element must have exactly exhausted chunk */
    ਜੇ (element_ਮਾਪ != 0) {
      ਪ੍ਰਤਿਜ੍ਞਾ(remainder_ਮਾਪ == element_ਮਾਪ);
    }
    ਨਹੀਂਤਾਂ {
      ਪ੍ਰਤਿਜ੍ਞਾ(remainder_ਮਾਪ == request2size(sizes[i]));
    }
    check_inuse_chunk(m, mem2chunk(marray));
  }
  ਲਈ (i = 0; i != n_elements; ++i)
    check_inuse_chunk(m, mem2chunk(marray[i]));

#ਜੇ_ਅੰਤ /* DEBUG */

  POSTACTION(m);
  ਵਾਪਸ marray;
}

/* Try to free all pointers in the given array.
   Note: this could be made faster, by delaying consolidation,
   at the price of disabling some user integrity checks, We
   still optimize some consolidations by combining adjacent
   chunks before freeing, which will occur often if allocated
   with ialloc or the array is sorted.
*/
ਸਥਿਰ size_t ਅੰਦਰੂਨੀ_bulk_free(mstate m, ਖਾਲੀ* array[], size_t nelem) {
  size_t unfreed = 0;
  ਜੇ (!PREACTION(m)) {
    ਖਾਲੀ** a;
    ਖਾਲੀ** fence = &(array[nelem]);
    ਲਈ (a = array; a != fence; ++a) {
      ਖਾਲੀ* mem = *a;
      ਜੇ (mem != 0) {
        mchunkptr p = mem2chunk(mem);
        size_t psize = chunksize(p);
#ਜੇ FOOTERS
        ਜੇ (get_mstate_for(p) != m) {
          ++unfreed;
          ਜਾਰੀ;
        }
#ਜੇ_ਅੰਤ
        check_inuse_chunk(m, p);
        *a = 0;
        ਜੇ (RTCHECK(ok_ਪਤਾ(m, p) && ok_inuse(p))) {
          ਖਾਲੀ ** b = a + 1; /* try to merge with next chunk */
          mchunkptr ਅਗਲਾ = ਅਗਲਾ_chunk(p);
          ਜੇ (b != fence && *b == chunk2mem(ਅਗਲਾ)) {
            size_t newsize = chunksize(ਅਗਲਾ) + psize;
            ਸੈੱਟ_inuse(m, p, newsize);
            *b = chunk2mem(p);
          }
          ਨਹੀਂਤਾਂ
            dispose_chunk(m, p, psize);
        }
        ਨਹੀਂਤਾਂ {
          CORRUPTION_ERROR_ACTION(m);
          ਰੋਕੋ;
        }
      }
    }
    ਜੇ (should_trim(m, m->topsize))
      sys_trim(m, 0);
    POSTACTION(m);
  }
  ਵਾਪਸ unfreed;
}

/* Traversal */
#ਜੇ MALLOC_INSPECT_ALL
ਸਥਿਰ ਖਾਲੀ ਅੰਦਰੂਨੀ_inspect_all(mstate m,
                                 ਖਾਲੀ(*handler)(ਖਾਲੀ *ਸ਼ੁਰੂ,
                                                ਖਾਲੀ *ਸਿਰਾ,
                                                size_t ਵਰਤਿਆ_ਬਾਈਟਾਂ,
                                                ਖਾਲੀ* callback_arg),
                                 ਖਾਲੀ* arg) {
  ਜੇ (is_initialized(m)) {
    mchunkptr ਸਿਖਰ = m->ਸਿਖਰ;
    msegmentptr s;
    ਲਈ (s = &m->seg; s != 0; s = s->ਅਗਲਾ) {
      mchunkptr q = align_as_chunk(s->ਅਧਾਰ);
      ਜਦੋਂ (segment_holds(s, q) && q->ਸਿਰ != FENCEPOST_ਸਿਰ) {
        mchunkptr ਅਗਲਾ = ਅਗਲਾ_chunk(q);
        size_t sz = chunksize(q);
        size_t ਵਰਤਿਆ;
        ਖਾਲੀ* ਸ਼ੁਰੂ;
        ਜੇ (is_inuse(q)) {
          ਵਰਤਿਆ = sz - CHUNK_OVERHEAD; /* must not be mmapped */
          ਸ਼ੁਰੂ = chunk2mem(q);
        }
        ਨਹੀਂਤਾਂ {
          ਵਰਤਿਆ = 0;
          ਜੇ (is_small(sz)) {     /* offset by possible bookkeeping */
            ਸ਼ੁਰੂ = (ਖਾਲੀ*)((ਅੱਖਰ*)q + ਆਕਾਰ(ਬਣਤਰ malloc_chunk));
          }
          ਨਹੀਂਤਾਂ {
            ਸ਼ੁਰੂ = (ਖਾਲੀ*)((ਅੱਖਰ*)q + ਆਕਾਰ(ਬਣਤਰ malloc_ਰੁੱਖ_chunk));
          }
        }
        ਜੇ (ਸ਼ੁਰੂ < (ਖਾਲੀ*)ਅਗਲਾ)  /* skip if all space is bookkeeping */
          handler(ਸ਼ੁਰੂ, ਅਗਲਾ, ਵਰਤਿਆ, arg);
        ਜੇ (q == ਸਿਖਰ)
          ਰੋਕੋ;
        q = ਅਗਲਾ;
      }
    }
  }
}
#ਜੇ_ਅੰਤ /* MALLOC_INSPECT_ALL */

/* ------------------ Exported realloc, memalign, etc -------------------- */

#ਜੇ !ONLY_MSPACES

ਖਾਲੀ* dlrealloc(ਖਾਲੀ* oldmem, size_t ਬਾਈਟਾਂ) {
  ਖਾਲੀ* mem = 0;
  ਜੇ (oldmem == 0) {
    mem = dlmalloc(ਬਾਈਟਾਂ);
  }
  ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ REALLOC_ZERO_ਬਾਈਟਾਂ_FREES
  ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ == 0) {
    dlfree(oldmem);
  }
#ਜੇ_ਅੰਤ /* REALLOC_ZERO_BYTES_FREES */
  ਨਹੀਂਤਾਂ {
    size_t nb = request2size(ਬਾਈਟਾਂ);
    mchunkptr oldp = mem2chunk(oldmem);
#ਜੇ ! FOOTERS
    mstate m = gm;
#ਨਹੀਂਤਾਂ /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    ਜੇ (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      ਵਾਪਸ 0;
    }
#ਜੇ_ਅੰਤ /* FOOTERS */
    ਜੇ (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      ਜੇ (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      ਨਹੀਂਤਾਂ {
        mem = ਅੰਦਰੂਨੀ_malloc(m, ਬਾਈਟਾਂ);
        ਜੇ (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(mem, oldmem, (oc < ਬਾਈਟਾਂ)? oc : ਬਾਈਟਾਂ);
          ਅੰਦਰੂਨੀ_free(m, oldmem);
        }
      }
    }
  }
  ਵਾਪਸ mem;
}

ਖਾਲੀ* dlrealloc_ਵਿੱਚ_ਰੱਖੋ(ਖਾਲੀ* oldmem, size_t ਬਾਈਟਾਂ) {
  ਖਾਲੀ* mem = 0;
  ਜੇ (oldmem != 0) {
    ਜੇ (ਬਾਈਟਾਂ >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    ਨਹੀਂਤਾਂ {
      size_t nb = request2size(ਬਾਈਟਾਂ);
      mchunkptr oldp = mem2chunk(oldmem);
#ਜੇ ! FOOTERS
      mstate m = gm;
#ਨਹੀਂਤਾਂ /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      ਜੇ (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        ਵਾਪਸ 0;
      }
#ਜੇ_ਅੰਤ /* FOOTERS */
      ਜੇ (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        ਜੇ (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  ਵਾਪਸ mem;
}

ਖਾਲੀ* dlmemalign(size_t alignment, size_t ਬਾਈਟਾਂ) {
  ਜੇ (alignment <= MALLOC_ALIGNMENT) {
    ਵਾਪਸ dlmalloc(ਬਾਈਟਾਂ);
  }
  ਵਾਪਸ ਅੰਦਰੂਨੀ_memalign(gm, alignment, ਬਾਈਟਾਂ);
}

ਪੂਰਨ dlposix_memalign(ਖਾਲੀ** pp, size_t alignment, size_t ਬਾਈਟਾਂ) {
  ਖਾਲੀ* mem = 0;
  ਜੇ (alignment == MALLOC_ALIGNMENT)
    mem = dlmalloc(ਬਾਈਟਾਂ);
  ਨਹੀਂਤਾਂ {
    size_t d = alignment / ਆਕਾਰ(ਖਾਲੀ*);
    size_t r = alignment % ਆਕਾਰ(ਖਾਲੀ*);
    ਜੇ (r != 0 || d == 0 || (d & (d-ਮਾਪ_T_ONE)) != 0)
      ਵਾਪਸ EINVAL;
    ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ <= MAX_REQUEST - alignment) {
      ਜੇ (alignment <  MIN_CHUNK_ਮਾਪ)
        alignment = MIN_CHUNK_ਮਾਪ;
      mem = ਅੰਦਰੂਨੀ_memalign(gm, alignment, ਬਾਈਟਾਂ);
    }
  }
  ਜੇ (mem == 0)
    ਵਾਪਸ ENOMEM;
  ਨਹੀਂਤਾਂ {
    *pp = mem;
    ਵਾਪਸ 0;
  }
}

ਖਾਲੀ* dlvalloc(size_t ਬਾਈਟਾਂ) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.ਪੰਨਾ_ਮਾਪ;
  ਵਾਪਸ dlmemalign(pagesz, ਬਾਈਟਾਂ);
}

ਖਾਲੀ* dlpvalloc(size_t ਬਾਈਟਾਂ) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.ਪੰਨਾ_ਮਾਪ;
  ਵਾਪਸ dlmemalign(pagesz, (ਬਾਈਟਾਂ + pagesz - ਮਾਪ_T_ONE) & ~(pagesz - ਮਾਪ_T_ONE));
}

ਖਾਲੀ** dlindependent_calloc(size_t n_elements, size_t elem_ਮਾਪ,
                            ਖਾਲੀ* chunks[]) {
  size_t sz = elem_ਮਾਪ; /* serves as 1-element array */
  ਵਾਪਸ ialloc(gm, n_elements, &sz, 3, chunks);
}

ਖਾਲੀ** dlindependent_comalloc(size_t n_elements, size_t sizes[],
                              ਖਾਲੀ* chunks[]) {
  ਵਾਪਸ ialloc(gm, n_elements, sizes, 0, chunks);
}

size_t dlbulk_free(ਖਾਲੀ* array[], size_t nelem) {
  ਵਾਪਸ ਅੰਦਰੂਨੀ_bulk_free(gm, array, nelem);
}

#ਜੇ MALLOC_INSPECT_ALL
ਖਾਲੀ dlmalloc_inspect_all(ਖਾਲੀ(*handler)(ਖਾਲੀ *ਸ਼ੁਰੂ,
                                         ਖਾਲੀ *ਸਿਰਾ,
                                         size_t ਵਰਤਿਆ_ਬਾਈਟਾਂ,
                                         ਖਾਲੀ* callback_arg),
                          ਖਾਲੀ* arg) {
  ensure_initialization();
  ਜੇ (!PREACTION(gm)) {
    ਅੰਦਰੂਨੀ_inspect_all(gm, handler, arg);
    POSTACTION(gm);
  }
}
#ਜੇ_ਅੰਤ /* MALLOC_INSPECT_ALL */

ਪੂਰਨ dlmalloc_trim(size_t pad) {
  ਪੂਰਨ ਨਤੀਜਾ = 0;
  ensure_initialization();
  ਜੇ (!PREACTION(gm)) {
    ਨਤੀਜਾ = sys_trim(gm, pad);
    POSTACTION(gm);
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

size_t dlmalloc_footprint(ਖਾਲੀ) {
  ਵਾਪਸ gm->footprint;
}

size_t dlmalloc_max_footprint(ਖਾਲੀ) {
  ਵਾਪਸ gm->max_footprint;
}

size_t dlmalloc_footprint_ਹੱਦਬੰਦੀ(ਖਾਲੀ) {
  size_t maf = gm->footprint_ਹੱਦਬੰਦੀ;
  ਵਾਪਸ maf == 0 ? MAX_ਮਾਪ_T : maf;
}

size_t dlmalloc_ਸੈੱਟ_footprint_ਹੱਦਬੰਦੀ(size_t ਬਾਈਟਾਂ) {
  size_t ਨਤੀਜਾ;  /* invert sense of 0 */
  ਜੇ (ਬਾਈਟਾਂ == 0)
    ਨਤੀਜਾ = granularity_align(1); /* Use minimal size */
  ਜੇ (ਬਾਈਟਾਂ == MAX_ਮਾਪ_T)
    ਨਤੀਜਾ = 0;                    /* disable */
  ਨਹੀਂਤਾਂ
    ਨਤੀਜਾ = granularity_align(ਬਾਈਟਾਂ);
  ਵਾਪਸ gm->footprint_ਹੱਦਬੰਦੀ = ਨਤੀਜਾ;
}

#ਜੇ !NO_MALLINFO
ਬਣਤਰ mallinfo dlmallinfo(ਖਾਲੀ) {
  ਵਾਪਸ ਅੰਦਰੂਨੀ_mallinfo(gm);
}
#ਜੇ_ਅੰਤ /* NO_MALLINFO */

#ਜੇ !NO_MALLOC_STATS
ਖਾਲੀ dlmalloc_stats(ਖਾਲੀ) {
  ਅੰਦਰੂਨੀ_malloc_stats(gm);
}
#ਜੇ_ਅੰਤ /* NO_MALLOC_STATS */

ਪੂਰਨ dlmallopt(ਪੂਰਨ param_ਸੰਖਿਆ, ਪੂਰਨ ਮੁੱਲ) {
  ਵਾਪਸ change_mparam(param_ਸੰਖਿਆ, ਮੁੱਲ);
}

size_t dlmalloc_usable_ਮਾਪ(ਖਾਲੀ* mem) {
  ਜੇ (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    ਜੇ (is_inuse(p))
      ਵਾਪਸ chunksize(p) - overhead_for(p);
  }
  ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#ਜੇ MSPACES

ਸਥਿਰ mstate init_user_mstate(ਅੱਖਰ* tbase, size_t tsize) {
  size_t msize = pad_request(ਆਕਾਰ(ਬਣਤਰ malloc_state));
  mchunkptr mn;
  mchunkptr msp = align_as_chunk(tbase);
  mstate m = (mstate)(chunk2mem(msp));
  memset(m, 0, msize);
  (ਖਾਲੀ)INITIAL_ਤਾਲਾ(&m->mutex);
  msp->ਸਿਰ = (msize|INUSE_ਬਿੱਟ);
  m->seg.ਅਧਾਰ = m->ਘੱਟ_ਤੋਂ_ਘੱਟ_addr = tbase;
  m->seg.ਮਾਪ = m->footprint = m->max_footprint = tsize;
  m->magic = mparams.magic;
  m->release_checks = MAX_RELEASE_CHECK_ਦਰ;
  m->mflags = mparams.default_mflags;
  m->extp = 0;
  m->exts = 0;
  disable_contiguous(m);
  init_bins(m);
  mn = ਅਗਲਾ_chunk(mem2chunk(m));
  init_ਸਿਖਰ(m, mn, (size_t)((tbase + tsize) - (ਅੱਖਰ*)mn) - ਸਿਖਰ_FOOT_ਮਾਪ);
  check_ਸਿਖਰ_chunk(m, m->ਸਿਖਰ);
  ਵਾਪਸ m;
}

mspace create_mspace(size_t ਸਮਰੱਥਾ, ਪੂਰਨ locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(ਆਕਾਰ(ਬਣਤਰ malloc_state));
  ਜੇ (ਸਮਰੱਥਾ < (size_t) -(msize + ਸਿਖਰ_FOOT_ਮਾਪ + mparams.ਪੰਨਾ_ਮਾਪ)) {
    size_t rs = ((ਸਮਰੱਥਾ == 0)? mparams.granularity :
                 (ਸਮਰੱਥਾ + ਸਿਖਰ_FOOT_ਮਾਪ + msize));
    size_t tsize = granularity_align(rs);
    ਅੱਖਰ* tbase = (ਅੱਖਰ*)(CALL_MMAP(tsize));
    ਜੇ (tbase != CMFAIL) {
      m = init_user_mstate(tbase, tsize);
      ਸੈੱਟ_segment_flags(&m->seg, USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ);
      ਸੈੱਟ_ਤਾਲਾ(m, locked);
    }
  }
  ਵਾਪਸ (mspace)m;
}

mspace create_mspace_with_ਅਧਾਰ(ਖਾਲੀ* ਅਧਾਰ, size_t ਸਮਰੱਥਾ, ਪੂਰਨ locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(ਆਕਾਰ(ਬਣਤਰ malloc_state));
  ਜੇ (ਸਮਰੱਥਾ > msize + ਸਿਖਰ_FOOT_ਮਾਪ &&
      ਸਮਰੱਥਾ < (size_t) -(msize + ਸਿਖਰ_FOOT_ਮਾਪ + mparams.ਪੰਨਾ_ਮਾਪ)) {
    m = init_user_mstate((ਅੱਖਰ*)ਅਧਾਰ, ਸਮਰੱਥਾ);
    ਸੈੱਟ_segment_flags(&m->seg, EXTERN_ਦੋਆਧਾਰੀ_ਅੰਕ);
    ਸੈੱਟ_ਤਾਲਾ(m, locked);
  }
  ਵਾਪਸ (mspace)m;
}

ਪੂਰਨ mspace_track_large_chunks(mspace msp, ਪੂਰਨ enable) {
  ਪੂਰਨ ret = 0;
  mstate ms = (mstate)msp;
  ਜੇ (!PREACTION(ms)) {
    ਜੇ (!use_mmap(ms)) {
      ret = 1;
    }
    ਜੇ (!enable) {
      enable_mmap(ms);
    } ਨਹੀਂਤਾਂ {
      disable_mmap(ms);
    }
    POSTACTION(ms);
  }
  ਵਾਪਸ ret;
}

size_t destroy_mspace(mspace msp) {
  size_t freed = 0;
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    msegmentptr sp = &ms->seg;
    (ਖਾਲੀ)DESTROY_ਤਾਲਾ(&ms->mutex); /* destroy before unmapped */
    ਜਦੋਂ (sp != 0) {
      ਅੱਖਰ* ਅਧਾਰ = sp->ਅਧਾਰ;
      size_t ਮਾਪ = sp->ਮਾਪ;
      flag_t flag = get_segment_flags(sp);
      (ਖਾਲੀ)ਅਧਾਰ; /* placate people compiling -Wunused-variable */
      sp = sp->ਅਗਲਾ;
      ਜੇ ((flag & USE_MMAP_ਦੋਆਧਾਰੀ_ਅੰਕ) && !(flag & EXTERN_ਦੋਆਧਾਰੀ_ਅੰਕ) &&
          CALL_MUNMAP(ਅਧਾਰ, ਮਾਪ) == 0)
        freed += ਮਾਪ;
    }
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  ਵਾਪਸ freed;
}

/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

ਖਾਲੀ* mspace_malloc(mspace msp, size_t ਬਾਈਟਾਂ) {
  mstate ms = (mstate)msp;
  ਜੇ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    ਵਾਪਸ 0;
  }
  ਜੇ (!PREACTION(ms)) {
    ਖਾਲੀ* mem;
    size_t nb;
    ਜੇ (ਬਾਈਟਾਂ <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (ਬਾਈਟਾਂ < MIN_REQUEST)? MIN_CHUNK_ਮਾਪ : pad_request(ਬਾਈਟਾਂ);
      idx = small_ਸੂਚਕਾਂਕ(nb);
      smallbits = ms->smallmap >> idx;

      ਜੇ ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(ms, idx);
        p = b->fd;
        ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(p) == small_index2size(idx));
        unlink_ਪਹਿਲਾ_small_chunk(ms, b, p, idx);
        ਸੈੱਟ_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        ਜਾਓ postaction;
      }

      ਨਹੀਂਤਾਂ ਜੇ (nb > ms->dvsize) {
        ਜੇ (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & ਖੱਬਾ_ਬਿੱਟ(idx2bit(idx));
          binmap_t leastbit = ਘੱਟ_ਤੋਂ_ਘੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b->fd;
          ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(p) == small_index2size(i));
          unlink_ਪਹਿਲਾ_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          ਜੇ (ਮਾਪ_T_ਮਾਪ != 4 && rsize < MIN_CHUNK_ਮਾਪ)
            ਸੈੱਟ_inuse_and_pinuse(ms, p, small_index2size(i));
          ਨਹੀਂਤਾਂ {
            ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(ms, p, nb);
            r = chunk_plus_ਵਿੱਥ(p, nb);
            ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          ਜਾਓ postaction;
        }

        ਨਹੀਂਤਾਂ ਜੇ (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
          check_malloced_chunk(ms, mem, nb);
          ਜਾਓ postaction;
        }
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ >= MAX_REQUEST)
      nb = MAX_ਮਾਪ_T; /* Too big to allocate. Force failure (in sys alloc) */
    ਨਹੀਂਤਾਂ {
      nb = pad_request(ਬਾਈਟਾਂ);
      ਜੇ (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
        check_malloced_chunk(ms, mem, nb);
        ਜਾਓ postaction;
      }
    }

    ਜੇ (nb <= ms->dvsize) {
      size_t rsize = ms->dvsize - nb;
      mchunkptr p = ms->dv;
      ਜੇ (rsize >= MIN_CHUNK_ਮਾਪ) { /* split dv */
        mchunkptr r = ms->dv = chunk_plus_ਵਿੱਥ(p, nb);
        ms->dvsize = rsize;
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(r, rsize);
        ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(ms, p, nb);
      }
      ਨਹੀਂਤਾਂ { /* exhaust dv */
        size_t dvs = ms->dvsize;
        ms->dvsize = 0;
        ms->dv = 0;
        ਸੈੱਟ_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      ਜਾਓ postaction;
    }

    ਨਹੀਂਤਾਂ ਜੇ (nb < ms->topsize) { /* Split top */
      size_t rsize = ms->topsize -= nb;
      mchunkptr p = ms->ਸਿਖਰ;
      mchunkptr r = ms->ਸਿਖਰ = chunk_plus_ਵਿੱਥ(p, nb);
      r->ਸਿਰ = rsize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
      ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_ਸਿਖਰ_chunk(ms, ms->ਸਿਖਰ);
      check_malloced_chunk(ms, mem, nb);
      ਜਾਓ postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
    POSTACTION(ms);
    ਵਾਪਸ mem;
  }

  ਵਾਪਸ 0;
}

ਖਾਲੀ mspace_free(mspace msp, ਖਾਲੀ* mem) {
  ਜੇ (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#ਜੇ FOOTERS
    mstate fm = get_mstate_for(p);
    (ਖਾਲੀ)msp; /* placate people compiling -Wunused */
#ਨਹੀਂਤਾਂ /* FOOTERS */
    mstate fm = (mstate)msp;
#ਜੇ_ਅੰਤ /* FOOTERS */
    ਜੇ (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      ਵਾਪਸ;
    }
    ਜੇ (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      ਜੇ (RTCHECK(ok_ਪਤਾ(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr ਅਗਲਾ = chunk_plus_ਵਿੱਥ(p, psize);
        ਜੇ (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          ਜੇ (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            ਜੇ (CALL_MUNMAP((ਅੱਖਰ*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            ਜਾਓ postaction;
          }
          ਨਹੀਂਤਾਂ {
            mchunkptr prev = chunk_minus_ਵਿੱਥ(p, prevsize);
            psize += prevsize;
            p = prev;
            ਜੇ (RTCHECK(ok_ਪਤਾ(fm, prev))) { /* consolidate backward */
              ਜੇ (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              ਨਹੀਂਤਾਂ ਜੇ ((ਅਗਲਾ->ਸਿਰ & INUSE_ਬਿੱਟ) == INUSE_ਬਿੱਟ) {
                fm->dvsize = psize;
                ਸੈੱਟ_free_with_pinuse(p, psize, ਅਗਲਾ);
                ਜਾਓ postaction;
              }
            }
            ਨਹੀਂਤਾਂ
              ਜਾਓ erroraction;
          }
        }

        ਜੇ (RTCHECK(ok_ਅਗਲਾ(p, ਅਗਲਾ) && ok_pinuse(ਅਗਲਾ))) {
          ਜੇ (!cinuse(ਅਗਲਾ)) {  /* consolidate forward */
            ਜੇ (ਅਗਲਾ == fm->ਸਿਖਰ) {
              size_t tsize = fm->topsize += psize;
              fm->ਸਿਖਰ = p;
              p->ਸਿਰ = tsize | PINUSE_ਦੋਆਧਾਰੀ_ਅੰਕ;
              ਜੇ (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              ਜੇ (should_trim(fm, tsize))
                sys_trim(fm, 0);
              ਜਾਓ postaction;
            }
            ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, dsize);
              ਜਾਓ postaction;
            }
            ਨਹੀਂਤਾਂ {
              size_t nsize = chunksize(ਅਗਲਾ);
              psize += nsize;
              unlink_chunk(fm, ਅਗਲਾ, nsize);
              ਸੈੱਟ_ਮਾਪ_and_pinuse_ਦਾ_free_chunk(p, psize);
              ਜੇ (p == fm->dv) {
                fm->dvsize = psize;
                ਜਾਓ postaction;
              }
            }
          }
          ਨਹੀਂਤਾਂ
            ਸੈੱਟ_free_with_pinuse(p, psize, ਅਗਲਾ);

          ਜੇ (is_small(psize)) {
            ਪਾਓ_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          ਨਹੀਂਤਾਂ {
            tchunkptr tp = (tchunkptr)p;
            ਪਾਓ_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            ਜੇ (--fm->release_checks == 0)
              release_ਅਣਵਰਤਿਆ_segments(fm);
          }
          ਜਾਓ postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
}

ਖਾਲੀ* mspace_calloc(mspace msp, size_t n_elements, size_t elem_ਮਾਪ) {
  ਖਾਲੀ* mem;
  size_t req = 0;
  mstate ms = (mstate)msp;
  ਜੇ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    ਵਾਪਸ 0;
  }
  ਜੇ (n_elements != 0) {
    req = n_elements * elem_ਮਾਪ;
    ਜੇ (((n_elements | elem_ਮਾਪ) & ~(size_t)0xffff) &&
        (req / n_elements != elem_ਮਾਪ))
      req = MAX_ਮਾਪ_T; /* force downstream failure on overflow */
  }
  mem = ਅੰਦਰੂਨੀ_malloc(ms, req);
  ਜੇ (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  ਵਾਪਸ mem;
}

ਖਾਲੀ* mspace_realloc(mspace msp, ਖਾਲੀ* oldmem, size_t ਬਾਈਟਾਂ) {
  ਖਾਲੀ* mem = 0;
  ਜੇ (oldmem == 0) {
    mem = mspace_malloc(msp, ਬਾਈਟਾਂ);
  }
  ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ REALLOC_ZERO_ਬਾਈਟਾਂ_FREES
  ਨਹੀਂਤਾਂ ਜੇ (ਬਾਈਟਾਂ == 0) {
    mspace_free(msp, oldmem);
  }
#ਜੇ_ਅੰਤ /* REALLOC_ZERO_BYTES_FREES */
  ਨਹੀਂਤਾਂ {
    size_t nb = request2size(ਬਾਈਟਾਂ);
    mchunkptr oldp = mem2chunk(oldmem);
#ਜੇ ! FOOTERS
    mstate m = (mstate)msp;
#ਨਹੀਂਤਾਂ /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    ਜੇ (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      ਵਾਪਸ 0;
    }
#ਜੇ_ਅੰਤ /* FOOTERS */
    ਜੇ (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      ਜੇ (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      ਨਹੀਂਤਾਂ {
        mem = mspace_malloc(m, ਬਾਈਟਾਂ);
        ਜੇ (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(mem, oldmem, (oc < ਬਾਈਟਾਂ)? oc : ਬਾਈਟਾਂ);
          mspace_free(m, oldmem);
        }
      }
    }
  }
  ਵਾਪਸ mem;
}

ਖਾਲੀ* mspace_realloc_ਵਿੱਚ_ਰੱਖੋ(mspace msp, ਖਾਲੀ* oldmem, size_t ਬਾਈਟਾਂ) {
  ਖਾਲੀ* mem = 0;
  ਜੇ (oldmem != 0) {
    ਜੇ (ਬਾਈਟਾਂ >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    ਨਹੀਂਤਾਂ {
      size_t nb = request2size(ਬਾਈਟਾਂ);
      mchunkptr oldp = mem2chunk(oldmem);
#ਜੇ ! FOOTERS
      mstate m = (mstate)msp;
#ਨਹੀਂਤਾਂ /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      (ਖਾਲੀ)msp; /* placate people compiling -Wunused */
      ਜੇ (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        ਵਾਪਸ 0;
      }
#ਜੇ_ਅੰਤ /* FOOTERS */
      ਜੇ (!PREACTION(m)) {
        mchunkptr newp = try_realloc_chunk(m, oldp, nb, 0);
        POSTACTION(m);
        ਜੇ (newp == oldp) {
          check_inuse_chunk(m, newp);
          mem = oldmem;
        }
      }
    }
  }
  ਵਾਪਸ mem;
}

ਖਾਲੀ* mspace_memalign(mspace msp, size_t alignment, size_t ਬਾਈਟਾਂ) {
  mstate ms = (mstate)msp;
  ਜੇ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    ਵਾਪਸ 0;
  }
  ਜੇ (alignment <= MALLOC_ALIGNMENT)
    ਵਾਪਸ mspace_malloc(msp, ਬਾਈਟਾਂ);
  ਵਾਪਸ ਅੰਦਰੂਨੀ_memalign(ms, alignment, ਬਾਈਟਾਂ);
}

ਖਾਲੀ** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_ਮਾਪ, ਖਾਲੀ* chunks[]) {
  size_t sz = elem_ਮਾਪ; /* serves as 1-element array */
  mstate ms = (mstate)msp;
  ਜੇ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ ialloc(ms, n_elements, &sz, 3, chunks);
}

ਖਾਲੀ** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], ਖਾਲੀ* chunks[]) {
  mstate ms = (mstate)msp;
  ਜੇ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ ialloc(ms, n_elements, sizes, 0, chunks);
}

size_t mspace_bulk_free(mspace msp, ਖਾਲੀ* array[], size_t nelem) {
  ਵਾਪਸ ਅੰਦਰੂਨੀ_bulk_free((mstate)msp, array, nelem);
}

#ਜੇ MALLOC_INSPECT_ALL
ਖਾਲੀ mspace_inspect_all(mspace msp,
                        ਖਾਲੀ(*handler)(ਖਾਲੀ *ਸ਼ੁਰੂ,
                                       ਖਾਲੀ *ਸਿਰਾ,
                                       size_t ਵਰਤਿਆ_ਬਾਈਟਾਂ,
                                       ਖਾਲੀ* callback_arg),
                        ਖਾਲੀ* arg) {
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    ਜੇ (!PREACTION(ms)) {
      ਅੰਦਰੂਨੀ_inspect_all(ms, handler, arg);
      POSTACTION(ms);
    }
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#ਜੇ_ਅੰਤ /* MALLOC_INSPECT_ALL */

ਪੂਰਨ mspace_trim(mspace msp, size_t pad) {
  ਪੂਰਨ ਨਤੀਜਾ = 0;
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    ਜੇ (!PREACTION(ms)) {
      ਨਤੀਜਾ = sys_trim(ms, pad);
      POSTACTION(ms);
    }
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

#ਜੇ !NO_MALLOC_STATS
ਖਾਲੀ mspace_malloc_stats(mspace msp) {
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    ਅੰਦਰੂਨੀ_malloc_stats(ms);
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#ਜੇ_ਅੰਤ /* NO_MALLOC_STATS */

size_t mspace_footprint(mspace msp) {
  size_t ਨਤੀਜਾ = 0;
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    ਨਤੀਜਾ = ms->footprint;
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

size_t mspace_max_footprint(mspace msp) {
  size_t ਨਤੀਜਾ = 0;
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    ਨਤੀਜਾ = ms->max_footprint;
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

size_t mspace_footprint_ਹੱਦਬੰਦੀ(mspace msp) {
  size_t ਨਤੀਜਾ = 0;
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    size_t maf = ms->footprint_ਹੱਦਬੰਦੀ;
    ਨਤੀਜਾ = (maf == 0) ? MAX_ਮਾਪ_T : maf;
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

size_t mspace_ਸੈੱਟ_footprint_ਹੱਦਬੰਦੀ(mspace msp, size_t ਬਾਈਟਾਂ) {
  size_t ਨਤੀਜਾ = 0;
  mstate ms = (mstate)msp;
  ਜੇ (ok_magic(ms)) {
    ਜੇ (ਬਾਈਟਾਂ == 0)
      ਨਤੀਜਾ = granularity_align(1); /* Use minimal size */
    ਜੇ (ਬਾਈਟਾਂ == MAX_ਮਾਪ_T)
      ਨਤੀਜਾ = 0;                    /* disable */
    ਨਹੀਂਤਾਂ
      ਨਤੀਜਾ = granularity_align(ਬਾਈਟਾਂ);
    ms->footprint_ਹੱਦਬੰਦੀ = ਨਤੀਜਾ;
  }
  ਨਹੀਂਤਾਂ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

#ਜੇ !NO_MALLINFO
ਬਣਤਰ mallinfo mspace_mallinfo(mspace msp) {
  mstate ms = (mstate)msp;
  ਜੇ (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
  }
  ਵਾਪਸ ਅੰਦਰੂਨੀ_mallinfo(ms);
}
#ਜੇ_ਅੰਤ /* NO_MALLINFO */

size_t mspace_usable_ਮਾਪ(ਅਚਲ ਖਾਲੀ* mem) {
  ਜੇ (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    ਜੇ (is_inuse(p))
      ਵਾਪਸ chunksize(p) - overhead_for(p);
  }
  ਵਾਪਸ 0;
}

ਪੂਰਨ mspace_mallopt(ਪੂਰਨ param_ਸੰਖਿਆ, ਪੂਰਨ ਮੁੱਲ) {
  ਵਾਪਸ change_mparam(param_ਸੰਖਿਆ, ਮੁੱਲ);
}

#ਜੇ_ਅੰਤ /* MSPACES */


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
