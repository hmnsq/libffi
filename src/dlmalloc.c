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
#اگر_غیر_تعریف_شدہ DLMALLOC_نسخہ
#تعریف_کرو DLMALLOC_نسخہ 20806
#اگر_ختم /* DLMALLOC_VERSION */

#اگر_غیر_تعریف_شدہ DLMALLOC_EXPORT
#تعریف_کرو DLMALLOC_EXPORT خارجی
#اگر_ختم

#اگر defined __linux__ && !defined GNU_ماخذ
/* mremap() on Linux requires this via sys/mman.h */
#تعریف_کرو GNU_ماخذ 1
#اگر_ختم

#اگر_غیر_تعریف_شدہ WIN32
#اگر_تعریف_شدہ _WIN32
#تعریف_کرو WIN32 1
#اگر_ختم  /* _WIN32 */
#اگر_تعریف_شدہ _WIN32_WCE
#تعریف_کرو LACKS_FCNTL_H
#تعریف_کرو WIN32 1
#اگر_ختم /* _WIN32_WCE */
#اگر_ختم  /* WIN32 */
#اگر_تعریف_شدہ WIN32
#تعریف_کرو WIN32_LEAN_AND_اوسط
#شامل <windows.h>
#شامل <tchar.h>
#تعریف_کرو HAVE_MMAP 1
#تعریف_کرو HAVE_MORECORE 0
#تعریف_کرو LACKS_UNISTD_H
#تعریف_کرو LACKS_SYS_PARAM_H
#تعریف_کرو LACKS_SYS_MMAN_H
#تعریف_کرو LACKS_STRING_H
#تعریف_کرو LACKS_لڑیاں_H
#تعریف_کرو LACKS_SYS_TYPES_H
#تعریف_کرو LACKS_ERRNO_H
#تعریف_کرو LACKS_SCHED_H
#اگر_غیر_تعریف_شدہ MALLOC_FAILURE_ACTION
#تعریف_کرو MALLOC_FAILURE_ACTION
#اگر_ختم /* MALLOC_FAILURE_ACTION */
#اگر_غیر_تعریف_شدہ MMAP_CLEARS
#اگر_تعریف_شدہ _WIN32_WCE /* WINCE reportedly does not clear */
#تعریف_کرو MMAP_CLEARS 0
#ورنہ
#تعریف_کرو MMAP_CLEARS 1
#اگر_ختم /* _WIN32_WCE */
#اگر_ختم /*MMAP_CLEARS */
#اگر_ختم  /* WIN32 */

#اگر_تعریف_شدہ __OS2__
#تعریف_کرو INCL_DOS
#شامل <os2.h>
#تعریف_کرو HAVE_MMAP 1
#تعریف_کرو HAVE_MORECORE 0
#تعریف_کرو LACKS_SYS_MMAN_H
#اگر_ختم  /* __OS2__ */

#اگر defined(DARWIN) || defined(_DARWIN)
/* Mac OSX docs advise not to use sbrk; it seems better to use mmap */
#اگر_غیر_تعریف_شدہ HAVE_MORECORE
#تعریف_کرو HAVE_MORECORE 0
#تعریف_کرو HAVE_MMAP 1
/* OSX allocators provide 16 byte alignment */
#اگر_غیر_تعریف_شدہ MALLOC_ALIGNMENT
#تعریف_کرو MALLOC_ALIGNMENT ((size_t)16U)
#اگر_ختم
#اگر_ختم  /* HAVE_MORECORE */
#اگر_ختم  /* DARWIN */

#اگر_غیر_تعریف_شدہ LACKS_SYS_TYPES_H
#شامل <sys/types.h>  /* For size_t */
#اگر_ختم  /* LACKS_SYS_TYPES_H */

/* The maximum possible size_t value has all bits set */
#تعریف_کرو MAX_سائز_T           (~(size_t)0)

#اگر_غیر_تعریف_شدہ USE_LOCKS /* ensure true if spin or recursive locks set */
#تعریف_کرو USE_LOCKS  ((defined(USE_SPIN_LOCKS) && USE_SPIN_LOCKS != 0) || \
                    (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0))
#اگر_ختم /* USE_LOCKS */

#اگر USE_LOCKS /* Spin locks for gcc >= 4.1, older gcc on x86, MSC >= 1310 */
#اگر ((defined(__GNUC__) &&                                              \
      ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) ||      \
       defined(__i386__) || defined(__x86_64__))) ||                    \
     (defined(_MSC_VER) && _MSC_VER>=1310))
#اگر_غیر_تعریف_شدہ USE_SPIN_LOCKS
#تعریف_کرو USE_SPIN_LOCKS 1
#اگر_ختم /* USE_SPIN_LOCKS */
#وگرنہ_اگر USE_SPIN_LOCKS
#غلطی "USE_SPIN_LOCKS defined without implementation"
#اگر_ختم /* ... locks available... */
#وگرنہ_اگر !defined(USE_SPIN_LOCKS)
#تعریف_کرو USE_SPIN_LOCKS 0
#اگر_ختم /* USE_LOCKS */

#اگر_غیر_تعریف_شدہ ONLY_MSPACES
#تعریف_کرو ONLY_MSPACES 0
#اگر_ختم  /* ONLY_MSPACES */
#اگر_غیر_تعریف_شدہ MSPACES
#اگر ONLY_MSPACES
#تعریف_کرو MSPACES 1
#ورنہ   /* ONLY_MSPACES */
#تعریف_کرو MSPACES 0
#اگر_ختم  /* ONLY_MSPACES */
#اگر_ختم  /* MSPACES */
#اگر_غیر_تعریف_شدہ MALLOC_ALIGNMENT
#تعریف_کرو MALLOC_ALIGNMENT ((size_t)(2 * حجم(خالی *)))
#اگر_ختم  /* MALLOC_ALIGNMENT */
#اگر_غیر_تعریف_شدہ FOOTERS
#تعریف_کرو FOOTERS 0
#اگر_ختم  /* FOOTERS */
#اگر_غیر_تعریف_شدہ ABORT
#تعریف_کرو ABORT  abort()
#اگر_ختم  /* ABORT */
#اگر_غیر_تعریف_شدہ ABORT_ON_ASSERT_FAILURE
#تعریف_کرو ABORT_ON_ASSERT_FAILURE 1
#اگر_ختم  /* ABORT_ON_ASSERT_FAILURE */
#اگر_غیر_تعریف_شدہ PROCEED_ON_ERROR
#تعریف_کرو PROCEED_ON_ERROR 0
#اگر_ختم  /* PROCEED_ON_ERROR */

#اگر_غیر_تعریف_شدہ INSECURE
#تعریف_کرو INSECURE 0
#اگر_ختم  /* INSECURE */
#اگر_غیر_تعریف_شدہ MALLOC_INSPECT_ALL
#تعریف_کرو MALLOC_INSPECT_ALL 0
#اگر_ختم  /* MALLOC_INSPECT_ALL */
#اگر_غیر_تعریف_شدہ HAVE_MMAP
#تعریف_کرو HAVE_MMAP 1
#اگر_ختم  /* HAVE_MMAP */
#اگر_غیر_تعریف_شدہ MMAP_CLEARS
#تعریف_کرو MMAP_CLEARS 1
#اگر_ختم  /* MMAP_CLEARS */
#اگر_غیر_تعریف_شدہ HAVE_MREMAP
#اگر_تعریف_شدہ linux
#تعریف_کرو HAVE_MREMAP 1
#تعریف_کرو GNU_ماخذ /* Turns on mremap() definition */
#ورنہ   /* linux */
#تعریف_کرو HAVE_MREMAP 0
#اگر_ختم  /* linux */
#اگر_ختم  /* HAVE_MREMAP */
#اگر_غیر_تعریف_شدہ MALLOC_FAILURE_ACTION
#تعریف_کرو MALLOC_FAILURE_ACTION  errno = ENOMEM;
#اگر_ختم  /* MALLOC_FAILURE_ACTION */
#اگر_غیر_تعریف_شدہ HAVE_MORECORE
#اگر ONLY_MSPACES
#تعریف_کرو HAVE_MORECORE 0
#ورنہ   /* ONLY_MSPACES */
#تعریف_کرو HAVE_MORECORE 1
#اگر_ختم  /* ONLY_MSPACES */
#اگر_ختم  /* HAVE_MORECORE */
#اگر !HAVE_MORECORE
#تعریف_کرو MORECORE_CONTIGUOUS 0
#ورنہ   /* !HAVE_MORECORE */
#تعریف_کرو MORECORE_DEFAULT sbrk
#اگر_غیر_تعریف_شدہ MORECORE_CONTIGUOUS
#تعریف_کرو MORECORE_CONTIGUOUS 1
#اگر_ختم  /* MORECORE_CONTIGUOUS */
#اگر_ختم  /* HAVE_MORECORE */
#اگر_غیر_تعریف_شدہ DEFAULT_GRANULARITY
#اگر (MORECORE_CONTIGUOUS || defined(WIN32))
#تعریف_کرو DEFAULT_GRANULARITY (0)  /* 0 means to compute in init_mparams */
#ورنہ   /* MORECORE_CONTIGUOUS */
#تعریف_کرو DEFAULT_GRANULARITY ((size_t)64U * (size_t)1024U)
#اگر_ختم  /* MORECORE_CONTIGUOUS */
#اگر_ختم  /* DEFAULT_GRANULARITY */
#اگر_غیر_تعریف_شدہ DEFAULT_TRIM_حد
#اگر_غیر_تعریف_شدہ MORECORE_CANNOT_TRIM
#تعریف_کرو DEFAULT_TRIM_حد ((size_t)2U * (size_t)1024U * (size_t)1024U)
#ورنہ   /* MORECORE_CANNOT_TRIM */
#تعریف_کرو DEFAULT_TRIM_حد MAX_سائز_T
#اگر_ختم  /* MORECORE_CANNOT_TRIM */
#اگر_ختم  /* DEFAULT_TRIM_THRESHOLD */
#اگر_غیر_تعریف_شدہ DEFAULT_MMAP_حد
#اگر HAVE_MMAP
#تعریف_کرو DEFAULT_MMAP_حد ((size_t)256U * (size_t)1024U)
#ورنہ   /* HAVE_MMAP */
#تعریف_کرو DEFAULT_MMAP_حد MAX_سائز_T
#اگر_ختم  /* HAVE_MMAP */
#اگر_ختم  /* DEFAULT_MMAP_THRESHOLD */
#اگر_غیر_تعریف_شدہ MAX_RELEASE_CHECK_شرح
#اگر HAVE_MMAP
#تعریف_کرو MAX_RELEASE_CHECK_شرح 4095
#ورنہ
#تعریف_کرو MAX_RELEASE_CHECK_شرح MAX_سائز_T
#اگر_ختم /* HAVE_MMAP */
#اگر_ختم /* MAX_RELEASE_CHECK_RATE */
#اگر_غیر_تعریف_شدہ USE_BUILTIN_FFS
#تعریف_کرو USE_BUILTIN_FFS 0
#اگر_ختم  /* USE_BUILTIN_FFS */
#اگر_غیر_تعریف_شدہ USE_DEV_RANDOM
#تعریف_کرو USE_DEV_RANDOM 0
#اگر_ختم  /* USE_DEV_RANDOM */
#اگر_غیر_تعریف_شدہ NO_MALLINFO
#تعریف_کرو NO_MALLINFO 0
#اگر_ختم  /* NO_MALLINFO */
#اگر_غیر_تعریف_شدہ MALLINFO_میدان_TYPE
#تعریف_کرو MALLINFO_میدان_TYPE size_t
#اگر_ختم  /* MALLINFO_FIELD_TYPE */
#اگر_غیر_تعریف_شدہ NO_MALLOC_STATS
#تعریف_کرو NO_MALLOC_STATS 0
#اگر_ختم  /* NO_MALLOC_STATS */
#اگر_غیر_تعریف_شدہ NO_SEGMENT_TRAVERSAL
#تعریف_کرو NO_SEGMENT_TRAVERSAL 0
#اگر_ختم /* NO_SEGMENT_TRAVERSAL */

/*
  mallopt tuning options.  SVID/XPG defines four standard parameter
  numbers for mallopt, normally defined in malloc.h.  None of these
  are used in this malloc, so setting them has no effect. But this
  malloc does support the following options.
*/

/* The system's malloc.h may have conflicting defines. */
#تعریف_منسوخ M_TRIM_حد
#تعریف_منسوخ M_GRANULARITY
#تعریف_منسوخ M_MMAP_حد

#تعریف_کرو M_TRIM_حد     (-1)
#تعریف_کرو M_GRANULARITY        (-2)
#تعریف_کرو M_MMAP_حد     (-3)

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

#اگر_تعریف_شدہ HAVE_USR_INCLUDE_MALLOC_H
#شامل "/usr/include/malloc.h"
#ورنہ /* HAVE_USR_INCLUDE_MALLOC_H */
#اگر_غیر_تعریف_شدہ STRUCT_MALLINFO_DECLARED
/* HP-UX (and others?) redefines mallinfo unless _STRUCT_MALLINFO is defined */
#تعریف_کرو _STRUCT_MALLINFO
#تعریف_کرو STRUCT_MALLINFO_DECLARED 1
ساخت mallinfo {
  MALLINFO_میدان_TYPE arena;    /* non-mmapped space allocated from system */
  MALLINFO_میدان_TYPE ordblks;  /* number of free chunks */
  MALLINFO_میدان_TYPE smblks;   /* always 0 */
  MALLINFO_میدان_TYPE hblks;    /* always 0 */
  MALLINFO_میدان_TYPE hblkhd;   /* space in mmapped regions */
  MALLINFO_میدان_TYPE usmblks;  /* maximum total allocated space */
  MALLINFO_میدان_TYPE fsmblks;  /* always 0 */
  MALLINFO_میدان_TYPE uordblks; /* total allocated space */
  MALLINFO_میدان_TYPE fordblks; /* total free space */
  MALLINFO_میدان_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#اگر_ختم /* STRUCT_MALLINFO_DECLARED */
#اگر_ختم /* HAVE_USR_INCLUDE_MALLOC_H */
#اگر_ختم /* NO_MALLINFO */

/*
  Try to persuade compilers to inline. The most critical functions for
  inlining are defined as macros, so these aren't used for them.
*/

#اگر_غیر_تعریف_شدہ FORCEINLINE
  #اگر defined(__GNUC__)
#تعریف_کرو FORCEINLINE __inline __attribute__ ((always_inline))
  #وگرنہ_اگر defined(_MSC_VER)
    #تعریف_کرو FORCEINLINE __forceinline
  #اگر_ختم
#اگر_ختم
#اگر_غیر_تعریف_شدہ NOINLINE
  #اگر defined(__GNUC__)
    #تعریف_کرو NOINLINE __attribute__ ((noinline))
  #وگرنہ_اگر defined(_MSC_VER)
    #تعریف_کرو NOINLINE __declspec(noinline)
  #ورنہ
    #تعریف_کرو NOINLINE
  #اگر_ختم
#اگر_ختم

#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_غیر_تعریف_شدہ FORCEINLINE
 #تعریف_کرو FORCEINLINE اندرونی
#اگر_ختم
#اگر_ختم /* __cplusplus */
#اگر_غیر_تعریف_شدہ FORCEINLINE
 #تعریف_کرو FORCEINLINE
#اگر_ختم

#اگر !ONLY_MSPACES

/* ------------------- Declarations of public routines ------------------- */

#اگر_غیر_تعریف_شدہ USE_DL_سابقہ
#تعریف_کرو dlcalloc               صفر_حافظہ_دو
#تعریف_کرو dlfree                 آزاد
#تعریف_کرو dlmalloc               حافظہ_دو
#تعریف_کرو dlmemalign             memalign
#تعریف_کرو dlposix_memalign       posix_memalign
#تعریف_کرو dlrealloc              realloc
#تعریف_کرو dlrealloc_اندر_رکھو     realloc_اندر_رکھو
#تعریف_کرو dlvalloc               valloc
#تعریف_کرو dlpvalloc              pvalloc
#تعریف_کرو dlmallinfo             mallinfo
#تعریف_کرو dlmallopt              mallopt
#تعریف_کرو dlmalloc_trim          malloc_trim
#تعریف_کرو dlmalloc_stats         malloc_stats
#تعریف_کرو dlmalloc_usable_سائز   malloc_usable_سائز
#تعریف_کرو dlmalloc_footprint     malloc_footprint
#تعریف_کرو dlmalloc_max_footprint malloc_max_footprint
#تعریف_کرو dlmalloc_footprint_حد_بندی malloc_footprint_حد_بندی
#تعریف_کرو dlmalloc_سیٹ_footprint_حد_بندی malloc_سیٹ_footprint_حد_بندی
#تعریف_کرو dlmalloc_inspect_all   malloc_inspect_all
#تعریف_کرو dlindependent_calloc   independent_calloc
#تعریف_کرو dlindependent_comalloc independent_comalloc
#تعریف_کرو dlbulk_free            bulk_free
#اگر_ختم /* USE_DL_PREFIX */

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
DLMALLOC_EXPORT خالی* dlmalloc(size_t);

/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
DLMALLOC_EXPORT خالی  dlfree(خالی*);

/*
  calloc(size_t n_elements, size_t element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
DLMALLOC_EXPORT خالی* dlcalloc(size_t, size_t);

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
DLMALLOC_EXPORT خالی* dlrealloc(خالی*, size_t);

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
DLMALLOC_EXPORT خالی* dlrealloc_اندر_رکھو(خالی*, size_t);

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
DLMALLOC_EXPORT خالی* dlmemalign(size_t, size_t);

/*
  int posix_memalign(void** pp, size_t alignment, size_t n);
  Allocates a chunk of n bytes, aligned in accord with the alignment
  argument. Differs from memalign only in that it (1) assigns the
  allocated memory to *pp rather than returning it, (2) fails and
  returns EINVAL if the alignment is not a power of two (3) fails and
  returns ENOMEM if memory cannot be allocated.
*/
DLMALLOC_EXPORT صحیح dlposix_memalign(خالی**, size_t, size_t);

/*
  valloc(size_t n);
  Equivalent to memalign(pagesize, n), where pagesize is the page
  size of the system. If the pagesize is unknown, 4096 is used.
*/
DLMALLOC_EXPORT خالی* dlvalloc(size_t);

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
DLMALLOC_EXPORT size_t dlmalloc_footprint(خالی);

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
DLMALLOC_EXPORT size_t dlmalloc_max_footprint(خالی);

/*
  malloc_footprint_limit();
  Returns the number of bytes that the heap is allowed to obtain from
  the system, returning the last value returned by
  malloc_set_footprint_limit, or the maximum size_t value if
  never set. The returned value reflects a permission. There is no
  guarantee that this number of bytes can actually be obtained from
  the system.
*/
DLMALLOC_EXPORT size_t dlmalloc_footprint_حد_بندی();

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
DLMALLOC_EXPORT size_t dlmalloc_سیٹ_footprint_حد_بندی(size_t بائٹیں);

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
DLMALLOC_EXPORT خالی dlmalloc_inspect_all(خالی(*handler)(خالی*, خالی *, size_t, خالی*),
                           خالی* arg);

#اگر_ختم /* MALLOC_INSPECT_ALL */

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
DLMALLOC_EXPORT ساخت mallinfo dlmallinfo(خالی);
#اگر_ختم /* NO_MALLINFO */

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
DLMALLOC_EXPORT خالی** dlindependent_calloc(size_t, size_t, خالی**);

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
DLMALLOC_EXPORT خالی** dlindependent_comalloc(size_t, size_t*, خالی**);

/*
  bulk_free(void* array[], size_t n_elements)
  Frees and clears (sets to null) each non-null pointer in the given
  array.  This is likely to be faster than freeing them one-by-one.
  If footers are used, pointers that have been allocated in different
  mspaces are not freed or cleared, and the count of all such pointers
  is returned.  For large arrays of pointers with poor locality, it
  may be worthwhile to sort this array before calling bulk_free.
*/
DLMALLOC_EXPORT size_t  dlbulk_free(خالی**, size_t n_elements);

/*
  pvalloc(size_t n);
  Equivalent to valloc(minimum-page-that-holds(n)), that is,
  round up n to nearest pagesize.
 */
DLMALLOC_EXPORT خالی*  dlpvalloc(size_t);

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
DLMALLOC_EXPORT خالی  dlmalloc_stats(خالی);

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
size_t dlmalloc_usable_سائز(خالی*);

#اگر_ختم /* ONLY_MSPACES */

#اگر MSPACES

/*
  mspace is an opaque type representing an independent
  region of space that supports mspace_malloc, etc.
*/
قسمتعریف خالی* mspace;

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
DLMALLOC_EXPORT mspace create_mspace(size_t استعداد, صحیح locked);

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
DLMALLOC_EXPORT mspace create_mspace_with_بنیاد(خالی* بنیاد, size_t استعداد, صحیح locked);

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
DLMALLOC_EXPORT خالی* mspace_malloc(mspace msp, size_t بائٹیں);

/*
  mspace_free behaves as free, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_free is not actually needed.
  free may be called instead of mspace_free because freed chunks from
  any space are handled by their originating spaces.
*/
DLMALLOC_EXPORT خالی mspace_free(mspace msp, خالی* mem);

/*
  mspace_realloc behaves as realloc, but operates within
  the given space.

  If compiled with FOOTERS==1, mspace_realloc is not actually
  needed.  realloc may be called instead of mspace_realloc because
  realloced chunks from any space are handled by their originating
  spaces.
*/
DLMALLOC_EXPORT خالی* mspace_realloc(mspace msp, خالی* mem, size_t newsize);

/*
  mspace_calloc behaves as calloc, but operates within
  the given space.
*/
DLMALLOC_EXPORT خالی* mspace_calloc(mspace msp, size_t n_elements, size_t elem_سائز);

/*
  mspace_memalign behaves as memalign, but operates within
  the given space.
*/
DLMALLOC_EXPORT خالی* mspace_memalign(mspace msp, size_t alignment, size_t بائٹیں);

/*
  mspace_independent_calloc behaves as independent_calloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT خالی** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_سائز, خالی* chunks[]);

/*
  mspace_independent_comalloc behaves as independent_comalloc, but
  operates within the given space.
*/
DLMALLOC_EXPORT خالی** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], خالی* chunks[]);

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
DLMALLOC_EXPORT ساخت mallinfo mspace_mallinfo(mspace msp);
#اگر_ختم /* NO_MALLINFO */

/*
  malloc_usable_size(void* p) behaves the same as malloc_usable_size;
*/
DLMALLOC_EXPORT size_t mspace_usable_سائز(ثابت خالی* mem);

/*
  mspace_malloc_stats behaves as malloc_stats, but reports
  properties of the given space.
*/
DLMALLOC_EXPORT خالی mspace_malloc_stats(mspace msp);

/*
  mspace_trim behaves as malloc_trim, but
  operates within the given space.
*/
DLMALLOC_EXPORT صحیح mspace_trim(mspace msp, size_t pad);

/*
  An alias for mallopt.
*/
DLMALLOC_EXPORT صحیح mspace_mallopt(صحیح, صحیح);

#اگر_ختم /* MSPACES */

#اگر_تعریف_شدہ __cplusplus
}  /* end of extern "C" */
#اگر_ختم /* __cplusplus */

/*
  ========================================================================
  To make a fully customizable malloc.h header file, cut everything
  above this line, put into file malloc.h, edit to suit, and #include it
  on the next line, as well as in programs that use this malloc.
  ========================================================================
*/

/* #include "malloc.h" */

/*------------------------------ internal #includes ---------------------- */

#اگر_تعریف_شدہ _MSC_VER
#ہدایت warning( disable : 4146 ) /* no "unsigned" warnings */
#اگر_ختم /* _MSC_VER */
#اگر !NO_MALLOC_STATS
#شامل <stdio.h>       /* for printing in malloc_stats */
#اگر_ختم /* NO_MALLOC_STATS */
#اگر_غیر_تعریف_شدہ LACKS_ERRNO_H
#شامل <errno.h>       /* for MALLOC_FAILURE_ACTION */
#اگر_ختم /* LACKS_ERRNO_H */
#اگر_تعریف_شدہ DEBUG
#اگر ABORT_ON_ASSERT_FAILURE
#تعریف_منسوخ أكد
#تعریف_کرو أكد(x) اگر(!(x)) ABORT
#ورنہ /* ABORT_ON_ASSERT_FAILURE */
#شامل <assert.h>
#اگر_ختم /* ABORT_ON_ASSERT_FAILURE */
#ورنہ  /* DEBUG */
#اگر_غیر_تعریف_شدہ أكد
#تعریف_کرو أكد(x)
#اگر_ختم
#تعریف_کرو DEBUG 0
#اگر_ختم /* DEBUG */
#اگر !defined(WIN32) && !defined(LACKS_وقت_H)
#شامل <time.h>        /* for magic initialization */
#اگر_ختم /* WIN32 */
#اگر_غیر_تعریف_شدہ LACKS_STDLIB_H
#شامل <stdlib.h>      /* for abort() */
#اگر_ختم /* LACKS_STDLIB_H */
#اگر_غیر_تعریف_شدہ LACKS_STRING_H
#شامل <string.h>      /* for memset etc */
#اگر_ختم  /* LACKS_STRING_H */
#اگر USE_BUILTIN_FFS
#اگر_غیر_تعریف_شدہ LACKS_لڑیاں_H
#شامل <strings.h>     /* for ffs */
#اگر_ختم /* LACKS_STRINGS_H */
#اگر_ختم /* USE_BUILTIN_FFS */
#اگر HAVE_MMAP
#اگر_غیر_تعریف_شدہ LACKS_SYS_MMAN_H
/* On some versions of linux, mremap decl in mman.h needs __USE_GNU set */
#اگر (defined(linux) && !defined(__USE_GNU))
#تعریف_کرو __USE_GNU 1
#شامل <sys/mman.h>    /* for mmap */
#تعریف_منسوخ __USE_GNU
#ورنہ
#شامل <sys/mman.h>    /* for mmap */
#اگر_ختم /* linux */
#اگر_ختم /* LACKS_SYS_MMAN_H */
#اگر_غیر_تعریف_شدہ LACKS_FCNTL_H
#شامل <fcntl.h>
#اگر_ختم /* LACKS_FCNTL_H */
#اگر_ختم /* HAVE_MMAP */
#اگر_غیر_تعریف_شدہ LACKS_UNISTD_H
#شامل <unistd.h>     /* for sbrk, sysconf */
#ورنہ /* LACKS_UNISTD_H */
#اگر !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
خارجی خالی*     sbrk(ptrdiff_t);
#اگر_ختم /* FreeBSD etc */
#اگر_ختم /* LACKS_UNISTD_H */

/* Declarations for locking */
#اگر USE_LOCKS
#اگر_غیر_تعریف_شدہ WIN32
#اگر defined (__SVR4) && defined (__sun)  /* solaris */
#شامل <thread.h>
#وگرنہ_اگر !defined(LACKS_SCHED_H)
#شامل <sched.h>
#اگر_ختم /* solaris or LACKS_SCHED_H */
#اگر (defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0) || !USE_SPIN_LOCKS
#شامل <pthread.h>
#اگر_ختم /* USE_RECURSIVE_LOCKS ... */
#وگرنہ_اگر defined(_MSC_VER)
#اگر_غیر_تعریف_شدہ _M_AMD64
/* These are already defined on AMD64 builds */
#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم /* __cplusplus */
LONG __cdecl _InterlockedCompareExchange(LONG متلون *Dest, LONG Exchange, LONG Comp);
LONG __cdecl _InterlockedExchange(LONG متلون *ہدف, LONG قدر_2);
#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم /* __cplusplus */
#اگر_ختم /* _M_AMD64 */
#ہدایت intrinsic (_InterlockedCompareExchange)
#ہدایت intrinsic (_InterlockedExchange)
#تعریف_کرو interlockedcompareexchange _InterlockedCompareExchange
#تعریف_کرو interlockedexchange _InterlockedExchange
#وگرنہ_اگر defined(WIN32) && defined(__GNUC__)
#تعریف_کرو interlockedcompareexchange(a, b, c) __sync_val_compare_and_swap(a, c, b)
#تعریف_کرو interlockedexchange __sync_lock_test_and_set
#اگر_ختم /* Win32 */
#ورنہ /* USE_LOCKS */
#اگر_ختم /* USE_LOCKS */

#اگر_غیر_تعریف_شدہ تالا_AT_FORK
#تعریف_کرو تالا_AT_FORK 0
#اگر_ختم

/* Declarations for bit scanning on win32 */
#اگر defined(_MSC_VER) && _MSC_VER>=1300
#اگر_غیر_تعریف_شدہ ثنائی_ہندسہ_جائزہ_Forward /* Try to avoid pulling in WinNT.h */
#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم /* __cplusplus */
غیرعلامتی حرف ثنائی_ہندسہ_جائزہ_Forward_2(غیرعلامتی طویل *اشاریہ, غیرعلامتی طویل نقاب);
غیرعلامتی حرف ثنائی_ہندسہ_جائزہ_الٹ(غیرعلامتی طویل *اشاریہ, غیرعلامتی طویل نقاب);
#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم /* __cplusplus */

#تعریف_کرو ثنائی_ہندسہ_جائزہ_Forward ثنائی_ہندسہ_جائزہ_Forward_2
#تعریف_کرو ثنائی_ہندسہ_جائزہ_الٹ_2 ثنائی_ہندسہ_جائزہ_الٹ
#ہدایت intrinsic(ثنائی_ہندسہ_جائزہ_Forward_2)
#ہدایت intrinsic(ثنائی_ہندسہ_جائزہ_الٹ)
#اگر_ختم /* BitScanForward */
#اگر_ختم /* defined(_MSC_VER) && _MSC_VER>=1300 */

#اگر_غیر_تعریف_شدہ WIN32
#اگر_غیر_تعریف_شدہ malloc_getpagesize
#  ifdef _SC_PAGESIZE         /* some SVR4 systems omit an underscore */
#    ifndef SC_صفحہ_سائز
#      define SC_صفحہ_سائز _SC_PAGESIZE
#    endif
#  endif
#  ifdef SC_صفحہ_سائز
#    define malloc_getpagesize sysconf(SC_صفحہ_سائز)
#  ورنہ
#    اگر defined(BSD) || defined(DGUX) || defined(HAVE_GETPAGESIZE)
       خارجی size_t getpagesize();
#      define malloc_getpagesize getpagesize()
#    ورنہ
#      ifdef WIN32 /* use supplied emulation of getpagesize */
#        define malloc_getpagesize getpagesize()
#      ورنہ
#        ifndef LACKS_SYS_PARAM_H
#          include <sys/param.h>
#        endif
#        ifdef EXEC_PAGESIZE
#          define malloc_getpagesize EXEC_PAGESIZE
#        ورنہ
#          ifdef NBPG
#            ifndef CLSIZE
#              define malloc_getpagesize NBPG
#            ورنہ
#              define malloc_getpagesize (NBPG * CLSIZE)
#            endif
#          ورنہ
#            ifdef NBPC
#              define malloc_getpagesize NBPC
#            ورنہ
#              ifdef PAGESIZE
#                define malloc_getpagesize PAGESIZE
#              ورنہ /* just guess */
#                define malloc_getpagesize ((size_t)4096U)
#              endif
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#اگر_ختم
#اگر_ختم

/* libffi: the upstream malloc_getpagesize definition above is gated on
   !WIN32 (WIN32 builds use GetSystemInfo in init_mparams instead).  Guarantee
   the symbol is always defined so the !WIN32 init_mparams branch still compiles
   on toolchains where it is not preprocessed out as expected (e.g. MSVC).  */
#اگر_غیر_تعریف_شدہ malloc_getpagesize
#تعریف_کرو malloc_getpagesize ((size_t)4096U)
#اگر_ختم

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#تعریف_کرو سائز_T_سائز         (حجم(size_t))
#تعریف_کرو سائز_T_BITSIZE      (حجم(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#تعریف_کرو سائز_T_ZERO         ((size_t)0)
#تعریف_کرو سائز_T_ONE          ((size_t)1)
#تعریف_کرو سائز_T_TWO          ((size_t)2)
#تعریف_کرو سائز_T_FOUR         ((size_t)4)
#تعریف_کرو TWO_سائز_T_SIZES    (سائز_T_سائز<<1)
#تعریف_کرو FOUR_سائز_T_SIZES   (سائز_T_سائز<<2)
#تعریف_کرو SIX_سائز_T_SIZES    (FOUR_سائز_T_SIZES+TWO_سائز_T_SIZES)
#تعریف_کرو HALF_MAX_سائز_T     (MAX_سائز_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#تعریف_کرو CHUNK_ALIGN_نقاب    (MALLOC_ALIGNMENT - سائز_T_ONE)

/* True if address a has acceptable alignment */
#تعریف_کرو is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_نقاب)) == 0)

/* the number of bytes to offset an address to align it */
#تعریف_کرو align_انحراف(A)\
 ((((size_t)(A) & CHUNK_ALIGN_نقاب) == 0)? 0 :\
  ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_نقاب)) & CHUNK_ALIGN_نقاب))

/* -------------------------- MMAP preliminaries ------------------------- */

/*
   If HAVE_MORECORE or HAVE_MMAP are false, we just define calls and
   checks to fail so compiler optimizer can delete code rather than
   using so many "#if"s.
*/


/* MORECORE and MMAP must return MFAIL on failure */
#تعریف_کرو MFAIL                ((خالی*)(MAX_سائز_T))
#تعریف_کرو CMFAIL               ((حرف*)(MFAIL)) /* defined for convenience */

#اگر HAVE_MMAP

#اگر !defined(WIN32) && !defined (__OS2__)
#تعریف_کرو MUNMAP_DEFAULT(a, s)  munmap((a), (s))
#تعریف_کرو MMAP_PROT            (PROT_READ|PROT_WRITE)
#اگر !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#تعریف_کرو MAP_ANONYMOUS        MAP_ANON
#اگر_ختم /* MAP_ANON */
#اگر_تعریف_شدہ MAP_ANONYMOUS
#تعریف_کرو MMAP_FLAGS           (MAP_نجی|MAP_ANONYMOUS)
#تعریف_کرو MMAP_DEFAULT(s)       mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#ورنہ /* MAP_ANONYMOUS */
/*
   Nearly all versions of mmap support MAP_ANONYMOUS, so the following
   is unlikely to be needed, but is supplied just in case.
*/
#تعریف_کرو MMAP_FLAGS           (MAP_نجی)
ساکن صحیح dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#تعریف_کرو MMAP_DEFAULT(s) ((dev_zero_fd < 0) ? \
           (dev_zero_fd = open("/dev/zero", O_RDWR), \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) : \
            mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#اگر_ختم /* MAP_ANONYMOUS */

#تعریف_کرو براہ_راست_MMAP_DEFAULT(s) MMAP_DEFAULT(s)

#وگرنہ_اگر defined(__OS2__)

/* OS/2 MMAP via DosAllocMem */
ساکن خالی* os2mmap(size_t سائز) {
  خالی* ptr;
  اگر (DosAllocMem(&ptr, سائز, OBJ_ANY|PAG_COMMIT|PAG_READ|PAG_WRITE) &&
      DosAllocMem(&ptr, سائز, PAG_COMMIT|PAG_READ|PAG_WRITE))
    واپس MFAIL;
  واپس ptr;
}

#تعریف_کرو os2direct_mmap(n)     os2mmap(n)

/* This function supports releasing coalesed segments */
ساکن صحیح os2munmap(خالی* ptr, size_t سائز) {
  جب (سائز) {
    ULONG ul_سائز = سائز;
    ULONG ulFlags = 0;
    اگر (DosQueryMem(ptr, &ul_سائز, &ulFlags) != 0)
      واپس -1;
    اگر ((ulFlags & PAG_بنیاد) == 0 ||(ulFlags & PAG_COMMIT) == 0 ||
        ul_سائز > سائز)
      واپس -1;
    اگر (DosFreeMem(ptr) != 0)
      واپس -1;
    ptr = ( خالی * ) ( ( حرف * ) ptr + ul_سائز );
    سائز -= ul_سائز;
  }
  واپس 0;
}

#تعریف_کرو MMAP_DEFAULT(s)         os2mmap(s)
#تعریف_کرو MUNMAP_DEFAULT(a, s)    os2munmap((a), (s))
#تعریف_کرو براہ_راست_MMAP_DEFAULT(s)  os2direct_mmap(s)

#ورنہ /* WIN32 */

/* Win32 MMAP via VirtualAlloc */
/* libffi: plain static (not FORCEINLINE) -- mingw's <windows.h> defines
   FORCEINLINE with an 'extern' storage class, which clashes with 'static'.  */
ساکن خالی* win32mmap(size_t سائز) {
  خالی* ptr = VirtualAlloc(0, سائز, MEM_RESERVE|MEM_COMMIT, صفحہ_EXECUTE_READWRITE);
  واپس (ptr != 0)? ptr: MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
ساکن خالی* win32direct_mmap(size_t سائز) {
  خالی* ptr = VirtualAlloc(0, سائز, MEM_RESERVE|MEM_COMMIT|MEM_چوٹی_نیچے_کو,
                           صفحہ_EXECUTE_READWRITE);
  واپس (ptr != 0)? ptr: MFAIL;
}

/* This function supports releasing coalesed segments */
ساکن صحیح win32munmap(خالی* ptr, size_t سائز) {
  حافظہ_BASIC_INFORMATION minfo;
  حرف* cptr = (حرف*)ptr;
  جب (سائز) {
    اگر (VirtualQuery(cptr, &minfo, حجم(minfo)) == 0)
      واپس -1;
    اگر (minfo.بنیاد_پتہ != cptr || minfo.تخصیص_بنیاد != cptr ||
        minfo.State != MEM_COMMIT || minfo.Region_سائز > سائز)
      واپس -1;
    اگر (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
      واپس -1;
    cptr += minfo.Region_سائز;
    سائز -= minfo.Region_سائز;
  }
  واپس 0;
}

#تعریف_کرو MMAP_DEFAULT(s)             win32mmap(s)
#تعریف_کرو MUNMAP_DEFAULT(a, s)        win32munmap((a), (s))
#تعریف_کرو براہ_راست_MMAP_DEFAULT(s)      win32direct_mmap(s)
#اگر_ختم /* WIN32 */
#اگر_ختم /* HAVE_MMAP */

#اگر HAVE_MREMAP
#اگر_غیر_تعریف_شدہ WIN32
#تعریف_کرو MREMAP_DEFAULT(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#اگر_ختم /* WIN32 */
#اگر_ختم /* HAVE_MREMAP */

/**
 * Define CALL_MORECORE
 */
#اگر HAVE_MORECORE
    #اگر_تعریف_شدہ MORECORE
        #تعریف_کرو CALL_MORECORE(S)    MORECORE(S)
    #ورنہ  /* MORECORE */
        #تعریف_کرو CALL_MORECORE(S)    MORECORE_DEFAULT(S)
    #اگر_ختم /* MORECORE */
#ورنہ  /* HAVE_MORECORE */
    #تعریف_کرو CALL_MORECORE(S)        MFAIL
#اگر_ختم /* HAVE_MORECORE */

/**
 * Define CALL_MMAP/CALL_MUNMAP/CALL_DIRECT_MMAP
 */
#اگر HAVE_MMAP
    #تعریف_کرو USE_MMAP_ثنائی_ہندسہ            (سائز_T_ONE)

    #اگر_تعریف_شدہ MMAP
        #تعریف_کرو CALL_MMAP(s)        MMAP(s)
    #ورنہ /* MMAP */
        #تعریف_کرو CALL_MMAP(s)        MMAP_DEFAULT(s)
    #اگر_ختم /* MMAP */
    #اگر_تعریف_شدہ MUNMAP
        #تعریف_کرو CALL_MUNMAP(a, s)   MUNMAP((a), (s))
    #ورنہ /* MUNMAP */
        #تعریف_کرو CALL_MUNMAP(a, s)   MUNMAP_DEFAULT((a), (s))
    #اگر_ختم /* MUNMAP */
    #اگر_تعریف_شدہ براہ_راست_MMAP
        #تعریف_کرو CALL_براہ_راست_MMAP(s) براہ_راست_MMAP(s)
    #ورنہ /* DIRECT_MMAP */
        #تعریف_کرو CALL_براہ_راست_MMAP(s) براہ_راست_MMAP_DEFAULT(s)
    #اگر_ختم /* DIRECT_MMAP */
#ورنہ  /* HAVE_MMAP */
    #تعریف_کرو USE_MMAP_ثنائی_ہندسہ            (سائز_T_ZERO)

    #تعریف_کرو MMAP(s)                 MFAIL
    #تعریف_کرو MUNMAP(a, s)            (-1)
    #تعریف_کرو براہ_راست_MMAP(s)          MFAIL
    #تعریف_کرو CALL_براہ_راست_MMAP(s)     براہ_راست_MMAP(s)
    #تعریف_کرو CALL_MMAP(s)            MMAP(s)
    #تعریف_کرو CALL_MUNMAP(a, s)       MUNMAP((a), (s))
#اگر_ختم /* HAVE_MMAP */

/**
 * Define CALL_MREMAP
 */
#اگر HAVE_MMAP && HAVE_MREMAP
    #اگر_تعریف_شدہ MREMAP
        #تعریف_کرو CALL_MREMAP(addr, osz, nsz, mv) MREMAP((addr), (osz), (nsz), (mv))
    #ورنہ /* MREMAP */
        #تعریف_کرو CALL_MREMAP(addr, osz, nsz, mv) MREMAP_DEFAULT((addr), (osz), (nsz), (mv))
    #اگر_ختم /* MREMAP */
#ورنہ  /* HAVE_MMAP && HAVE_MREMAP */
    #تعریف_کرو CALL_MREMAP(addr, osz, nsz, mv)     MFAIL
#اگر_ختم /* HAVE_MMAP && HAVE_MREMAP */

/* mstate bit set if contiguous morecore disabled or failed */
#تعریف_کرو USE_NONCONTIGUOUS_ثنائی_ہندسہ (4U)

/* segment bit set in create_mspace_with_base */
#تعریف_کرو EXTERN_ثنائی_ہندسہ            (8U)


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
#تعریف_کرو USE_تالا_ثنائی_ہندسہ               (0U)
#تعریف_کرو INITIAL_تالا(l)            (0)
#تعریف_کرو DESTROY_تالا(l)            (0)
#تعریف_کرو ACQUIRE_MALLOC_GLOBAL_تالا()
#تعریف_کرو RELEASE_MALLOC_GLOBAL_تالا()

#ورنہ
#اگر USE_LOCKS > 1
/* -----------------------  User-defined locks ------------------------ */
/* Define your own lock implementation here */
/* #define INITIAL_LOCK(lk)  ... */
/* #define DESTROY_LOCK(lk)  ... */
/* #define ACQUIRE_LOCK(lk)  ... */
/* #define RELEASE_LOCK(lk)  ... */
/* #define TRY_LOCK(lk) ... */
/* static MLOCK_T malloc_global_mutex = ... */

#وگرنہ_اگر USE_SPIN_LOCKS

/* First, define CAS_LOCK and CLEAR_LOCK on ints */
/* Note CAS_LOCK defined to return 0 on success */

#اگر defined(__GNUC__)&& (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#تعریف_کرو CAS_تالا(sl)     __sync_lock_test_and_set(sl, 1)
#تعریف_کرو CLEAR_تالا(sl)   __sync_lock_release(sl)

#وگرنہ_اگر (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
/* Custom spin locks for older gcc on x86 */
ساکن FORCEINLINE صحیح x86_cas_تالا(صحیح *sl) {
  صحیح ret;
  صحیح val = 1;
  صحیح cmp = 0;
  __asm__ __volatile__  ("lock; cmpxchgl %1, %2"
                         : "=a" (ret)
                         : "r" (val), "m" (*(sl)), "0"(cmp)
                         : "memory", "cc");
  واپس ret;
}

ساکن FORCEINLINE خالی x86_clear_تالا(صحیح* sl) {
  أكد(*sl != 0);
  صحیح پچھلا = 0;
  صحیح ret;
  __asm__ __volatile__ ("lock; xchgl %0, %1"
                        : "=r" (ret)
                        : "m" (*(sl)), "0"(پچھلا)
                        : "memory");
}

#تعریف_کرو CAS_تالا(sl)     x86_cas_تالا(sl)
#تعریف_کرو CLEAR_تالا(sl)   x86_clear_تالا(sl)

#ورنہ /* Win32 MSC */
#تعریف_کرو CAS_تالا(sl)     interlockedexchange((LONG متلون *)(sl), (LONG)1)
#تعریف_کرو CLEAR_تالا(sl)   interlockedexchange ((LONG متلون *)(sl), (LONG)0)

#اگر_ختم /* ... gcc spins locks ... */

/* How to yield for a spin lock */
#تعریف_کرو SPINS_فی_YIELD       63
#اگر defined(_MSC_VER)
#تعریف_کرو SLEEP_EX_DURATION     50 /* delay for yield/sleep */
#تعریف_کرو SPIN_تالا_YIELD  SleepEx(SLEEP_EX_DURATION, FALSE)
#وگرنہ_اگر defined (__SVR4) && defined (__sun) /* solaris */
#تعریف_کرو SPIN_تالا_YIELD   thr_yield();
#وگرنہ_اگر !defined(LACKS_SCHED_H)
#تعریف_کرو SPIN_تالا_YIELD   sched_yield();
#ورنہ
#تعریف_کرو SPIN_تالا_YIELD
#اگر_ختم /* ... yield ... */

/* libffi: read the spin-lock word with a relaxed atomic load for the lock-free
   "is it held?" peek, so ThreadSanitizer does not flag it as racing with the
   atomic CLEAR_LOCK release on unlock.  Relaxed ordering is sufficient: the
   peek is only a hint to avoid a costly CAS, and CAS_LOCK still provides the
   acquire ordering on success.  Falls back to the original volatile read when
   atomic builtins are unavailable.  */
#اگر defined(__ATOMIC_RELAXED)
# define ffi_spin_peek(sl) __atomic_load_n((sl), __ATOMIC_RELAXED)
#وگرنہ_اگر defined(_MSC_VER)
# define ffi_spin_peek(sl) \
  ((صحیح)_InterlockedCompareExchange((LONG متلون *)(sl), (LONG)0, (LONG)0))
#ورنہ
# define ffi_spin_peek(sl) (*(متلون صحیح *)(sl))
#اگر_ختم

#اگر !defined(USE_RECURSIVE_LOCKS) || USE_RECURSIVE_LOCKS == 0
/* Plain spin locks use single word (embedded in malloc_states) */
ساکن صحیح spin_acquire_تالا(صحیح *sl) {
  صحیح spins = 0;
  جب (ffi_spin_peek(sl) != 0 || CAS_تالا(sl)) {
    اگر ((++spins & SPINS_فی_YIELD) == 0) {
      SPIN_تالا_YIELD;
    }
  }
  واپس 0;
}

#تعریف_کرو MLOCK_T               صحیح
#تعریف_کرو TRY_تالا(sl)          !CAS_تالا(sl)
#تعریف_کرو RELEASE_تالا(sl)      CLEAR_تالا(sl)
#تعریف_کرو ACQUIRE_تالا(sl)      (CAS_تالا(sl)? spin_acquire_تالا(sl) : 0)
#تعریف_کرو INITIAL_تالا(sl)      (*sl = 0)
#تعریف_کرو DESTROY_تالا(sl)      (0)
ساکن MLOCK_T malloc_global_mutex = 0;

#ورنہ /* USE_RECURSIVE_LOCKS */
/* types for lock owners */
#اگر_تعریف_شدہ WIN32
#تعریف_کرو THREAD_ID_T           DWORD
#تعریف_کرو CURRENT_THREAD        GetCurrentThreadId()
#تعریف_کرو EQ_OWNER(X,Y)         ((X) == (Y))
#ورنہ
/*
  Note: the following assume that pthread_t is a type that can be
  initialized to (casted) zero. If this is not the case, you will need to
  somehow redefine these or not use spin locks.
*/
#تعریف_کرو THREAD_ID_T           pthread_t
#تعریف_کرو CURRENT_THREAD        pthread_self()
#تعریف_کرو EQ_OWNER(X,Y)         pthread_equal(X, Y)
#اگر_ختم

ساخت malloc_recursive_تالا {
  صحیح sl;
  غیرعلامتی صحیح c;
  THREAD_ID_T threadid;
};

#تعریف_کرو MLOCK_T  ساخت malloc_recursive_تالا
ساکن MLOCK_T malloc_global_mutex = { 0, 0, (THREAD_ID_T)0};

ساکن FORCEINLINE خالی recursive_release_تالا(MLOCK_T *lk) {
  أكد(lk->sl != 0);
  اگر (--lk->c == 0) {
    CLEAR_تالا(&lk->sl);
  }
}

ساکن FORCEINLINE صحیح recursive_acquire_تالا(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  صحیح spins = 0;
  برائے (;;) {
    اگر (ffi_spin_peek(&lk->sl) == 0) {
      اگر (!CAS_تالا(&lk->sl)) {
        lk->threadid = mythreadid;
        lk->c = 1;
        واپس 0;
      }
    }
    ورنہ اگر (EQ_OWNER(lk->threadid, mythreadid)) {
      ++lk->c;
      واپس 0;
    }
    اگر ((++spins & SPINS_فی_YIELD) == 0) {
      SPIN_تالا_YIELD;
    }
  }
}

ساکن FORCEINLINE صحیح recursive_try_تالا(MLOCK_T *lk) {
  THREAD_ID_T mythreadid = CURRENT_THREAD;
  اگر (ffi_spin_peek(&lk->sl) == 0) {
    اگر (!CAS_تالا(&lk->sl)) {
      lk->threadid = mythreadid;
      lk->c = 1;
      واپس 1;
    }
  }
  ورنہ اگر (EQ_OWNER(lk->threadid, mythreadid)) {
    ++lk->c;
    واپس 1;
  }
  واپس 0;
}

#تعریف_کرو RELEASE_تالا(lk)      recursive_release_تالا(lk)
#تعریف_کرو TRY_تالا(lk)          recursive_try_تالا(lk)
#تعریف_کرو ACQUIRE_تالا(lk)      recursive_acquire_تالا(lk)
#تعریف_کرو INITIAL_تالا(lk)      ((lk)->threadid = (THREAD_ID_T)0, (lk)->sl = 0, (lk)->c = 0)
#تعریف_کرو DESTROY_تالا(lk)      (0)
#اگر_ختم /* USE_RECURSIVE_LOCKS */

#وگرنہ_اگر defined(WIN32) /* Win32 critical sections */
#تعریف_کرو MLOCK_T               CRITICAL_SECTION
#تعریف_کرو ACQUIRE_تالا(lk)      (EnterCriticalSection(lk), 0)
#تعریف_کرو RELEASE_تالا(lk)      LeaveCriticalSection(lk)
#تعریف_کرو TRY_تالا(lk)          TryEnterCriticalSection(lk)
#تعریف_کرو INITIAL_تالا(lk)      (!Initialize_Critical_Section_And_Spin_گنتی((lk), 0x80000000|4000))
#تعریف_کرو DESTROY_تالا(lk)      (DeleteCriticalSection(lk), 0)
#تعریف_کرو NEED_GLOBAL_تالا_INIT

ساکن MLOCK_T malloc_global_mutex;
ساکن متلون LONG malloc_global_mutex_حالت;

/* Use spin loop to initialize global lock */
ساکن خالی init_malloc_global_mutex() {
  برائے (;;) {
    طویل stat = malloc_global_mutex_حالت;
    اگر (stat > 0)
      واپس;
    /* transition to < 0 while initializing, then to > 0) */
    اگر (stat == 0 &&
        interlockedcompareexchange(&malloc_global_mutex_حالت, (LONG)-1, (LONG)0) == 0) {
      InitializeCriticalSection(&malloc_global_mutex);
      interlockedexchange(&malloc_global_mutex_حالت, (LONG)1);
      واپس;
    }
    SleepEx(0, FALSE);
  }
}

#ورنہ /* pthreads-based locks */
#تعریف_کرو MLOCK_T               pthread_mutex_t
#تعریف_کرو ACQUIRE_تالا(lk)      pthread_mutex_تالا(lk)
#تعریف_کرو RELEASE_تالا(lk)      pthread_mutex_unlock(lk)
#تعریف_کرو TRY_تالا(lk)          (!pthread_mutex_trylock(lk))
#تعریف_کرو INITIAL_تالا(lk)      pthread_init_تالا(lk)
#تعریف_کرو DESTROY_تالا(lk)      pthread_mutex_destroy(lk)

#اگر defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0 && defined(linux) && !defined(PTHREAD_MUTEX_RECURSIVE)
/* Cope with old-style linux recursive lock initialization by adding */
/* skipped internal declaration from pthread.h */
خارجی صحیح pthread_mutexattr_setkind_np __P ((pthread_mutexattr_t *__attr,
                                              صحیح __kind));
#تعریف_کرو PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#تعریف_کرو pthread_mutexattr_settype(x,y) pthread_mutexattr_setkind_np(x,y)
#اگر_ختم /* USE_RECURSIVE_LOCKS ... */

ساکن MLOCK_T malloc_global_mutex = PTHREAD_MUTEX_INITIALIZER;

ساکن صحیح pthread_init_تالا (MLOCK_T *lk) {
  pthread_mutexattr_t attr;
  اگر (pthread_mutexattr_init(&attr)) واپس 1;
#اگر defined(USE_RECURSIVE_LOCKS) && USE_RECURSIVE_LOCKS != 0
  اگر (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) واپس 1;
#اگر_ختم
  اگر (pthread_mutex_init(lk, &attr)) واپس 1;
  اگر (pthread_mutexattr_destroy(&attr)) واپس 1;
  واپس 0;
}

#اگر_ختم /* ... lock types ... */

/* Common code for all lock types */
#تعریف_کرو USE_تالا_ثنائی_ہندسہ               (2U)

#اگر_غیر_تعریف_شدہ ACQUIRE_MALLOC_GLOBAL_تالا
#تعریف_کرو ACQUIRE_MALLOC_GLOBAL_تالا()  ACQUIRE_تالا(&malloc_global_mutex);
#اگر_ختم

#اگر_غیر_تعریف_شدہ RELEASE_MALLOC_GLOBAL_تالا
#تعریف_کرو RELEASE_MALLOC_GLOBAL_تالا()  RELEASE_تالا(&malloc_global_mutex);
#اگر_ختم

#اگر_ختم /* USE_LOCKS */

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

ساخت malloc_chunk {
  size_t               پچھلا_foot;  /* Size of previous chunk (if free).  */
  size_t               سر;       /* Size and inuse bits. */
  ساخت malloc_chunk* fd;         /* double links -- used only if free. */
  ساخت malloc_chunk* bk;
};

قسمتعریف ساخت malloc_chunk  mchunk;
قسمتعریف ساخت malloc_chunk* mchunkptr;
قسمتعریف ساخت malloc_chunk* sbinptr;  /* The type of bins of chunks */
قسمتعریف غیرعلامتی صحیح bindex_t;         /* Described below */
قسمتعریف غیرعلامتی صحیح binmap_t;         /* Described below */
قسمتعریف غیرعلامتی صحیح flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#تعریف_کرو MCHUNK_سائز         (حجم(mchunk))

#اگر FOOTERS
#تعریف_کرو CHUNK_OVERHEAD      (TWO_سائز_T_SIZES)
#ورنہ /* FOOTERS */
#تعریف_کرو CHUNK_OVERHEAD      (سائز_T_سائز)
#اگر_ختم /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#تعریف_کرو MMAP_CHUNK_OVERHEAD (TWO_سائز_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#تعریف_کرو MMAP_FOOT_PAD       (FOUR_سائز_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#تعریف_کرو MIN_CHUNK_سائز\
  ((MCHUNK_سائز + CHUNK_ALIGN_نقاب) & ~CHUNK_ALIGN_نقاب)

/* conversion from malloc headers to user pointers, and back */
#تعریف_کرو chunk2mem(p)        ((خالی*)((حرف*)(p)       + TWO_سائز_T_SIZES))
#تعریف_کرو mem2chunk(mem)      ((mchunkptr)((حرف*)(mem) - TWO_سائز_T_SIZES))
/* chunk associated with aligned address A */
#تعریف_کرو align_as_chunk(A)   (mchunkptr)((A) + align_انحراف(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#تعریف_کرو MAX_REQUEST         ((-MIN_CHUNK_سائز) << 2)
#تعریف_کرو MIN_REQUEST         (MIN_CHUNK_سائز - CHUNK_OVERHEAD - سائز_T_ONE)

/* pad request bytes into a usable size */
#تعریف_کرو pad_request(req) \
   (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_نقاب) & ~CHUNK_ALIGN_نقاب)

/* pad request, checking for minimum (but not maximum) */
#تعریف_کرو request2size(req) \
  (((req) < MIN_REQUEST)? MIN_CHUNK_سائز : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

/*
  The head field of a chunk is or'ed with PINUSE_BIT when previous
  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
  use, unless mmapped, in which case both bits are cleared.

  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
*/

#تعریف_کرو PINUSE_ثنائی_ہندسہ          (سائز_T_ONE)
#تعریف_کرو CINUSE_ثنائی_ہندسہ          (سائز_T_TWO)
#تعریف_کرو FLAG4_ثنائی_ہندسہ           (سائز_T_FOUR)
#تعریف_کرو INUSE_بٹ          (PINUSE_ثنائی_ہندسہ|CINUSE_ثنائی_ہندسہ)
#تعریف_کرو FLAG_بٹ           (PINUSE_ثنائی_ہندسہ|CINUSE_ثنائی_ہندسہ|FLAG4_ثنائی_ہندسہ)

/* Head value for fenceposts */
#تعریف_کرو FENCEPOST_سر      (INUSE_بٹ|سائز_T_سائز)

/* extraction of fields from head words */
#تعریف_کرو cinuse(p)           ((p)->سر & CINUSE_ثنائی_ہندسہ)
#تعریف_کرو pinuse(p)           ((p)->سر & PINUSE_ثنائی_ہندسہ)
#تعریف_کرو flag4inuse(p)       ((p)->سر & FLAG4_ثنائی_ہندسہ)
#تعریف_کرو is_inuse(p)         (((p)->سر & INUSE_بٹ) != PINUSE_ثنائی_ہندسہ)
#تعریف_کرو is_mmapped(p)       (((p)->سر & INUSE_بٹ) == 0)

#تعریف_کرو chunksize(p)        ((p)->سر & ~(FLAG_بٹ))

#تعریف_کرو clear_pinuse(p)     ((p)->سر &= ~PINUSE_ثنائی_ہندسہ)
#تعریف_کرو سیٹ_flag4(p)        ((p)->سر |= FLAG4_ثنائی_ہندسہ)
#تعریف_کرو clear_flag4(p)      ((p)->سر &= ~FLAG4_ثنائی_ہندسہ)

/* Treat space at ptr +/- offset as a chunk */
#تعریف_کرو chunk_plus_انحراف(p, s)  ((mchunkptr)(((حرف*)(p)) + (s)))
#تعریف_کرو chunk_minus_انحراف(p, s) ((mchunkptr)(((حرف*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#تعریف_کرو اگلا_chunk(p) ((mchunkptr)( ((حرف*)(p)) + ((p)->سر & ~FLAG_بٹ)))
#تعریف_کرو پچھلا_chunk(p) ((mchunkptr)( ((حرف*)(p)) - ((p)->پچھلا_foot) ))

/* extract next chunk's pinuse bit */
#تعریف_کرو اگلا_pinuse(p)  ((اگلا_chunk(p)->سر) & PINUSE_ثنائی_ہندسہ)

/* Get/set size at footer */
#تعریف_کرو get_foot(p, s)  (((mchunkptr)((حرف*)(p) + (s)))->پچھلا_foot)
#تعریف_کرو سیٹ_foot(p, s)  (((mchunkptr)((حرف*)(p) + (s)))->پچھلا_foot = (s))

/* Set size, pinuse bit, and foot */
#تعریف_کرو سیٹ_سائز_and_pinuse_کا_free_chunk(p, s)\
  ((p)->سر = (s|PINUSE_ثنائی_ہندسہ), سیٹ_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#تعریف_کرو سیٹ_free_with_pinuse(p, s, n)\
  (clear_pinuse(n), سیٹ_سائز_and_pinuse_کا_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#تعریف_کرو overhead_for(p)\
 (is_mmapped(p)? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)

/* Return true if malloced space is not necessarily cleared */
#اگر MMAP_CLEARS
#تعریف_کرو calloc_must_clear(p) (!is_mmapped(p))
#ورنہ /* MMAP_CLEARS */
#تعریف_کرو calloc_must_clear(p) (1)
#اگر_ختم /* MMAP_CLEARS */

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

ساخت malloc_درخت_chunk {
  /* The first four fields must be compatible with malloc_chunk */
  size_t                    پچھلا_foot;
  size_t                    سر;
  ساخت malloc_درخت_chunk* fd;
  ساخت malloc_درخت_chunk* bk;

  ساخت malloc_درخت_chunk* بچہ[2];
  ساخت malloc_درخت_chunk* والد;
  bindex_t                  اشاریہ;
};

قسمتعریف ساخت malloc_درخت_chunk  tchunk;
قسمتعریف ساخت malloc_درخت_chunk* tchunkptr;
قسمتعریف ساخت malloc_درخت_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#تعریف_کرو leftmost_بچہ(t) ((t)->بچہ[0] != 0? (t)->بچہ[0] : (t)->بچہ[1])

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

ساخت malloc_segment {
  حرف*        بنیاد;             /* base address */
  size_t       سائز;             /* allocated size */
  ساخت malloc_segment* اگلا;   /* ptr to next segment */
#اگر FFI_MMAP_EXEC_WRIT
  /* The mmap magic is supposed to store the address of the executable
     segment at the very end of the requested block.  */

# define mmap_exec_انحراف(b,s) (*(ptrdiff_t*)((b)+(s)-حجم(ptrdiff_t)))

  /* We can only merge segments if their corresponding executable
     segments are at identical offsets.  */
# define check_segment_ملاؤ(S,b,s) \
  (mmap_exec_انحراف((b),(s)) == (S)->exec_انحراف)

# define جوڑو_segment_exec_انحراف(p,S) ((حرف*)(p) + (S)->exec_انحراف)
# define sub_segment_exec_انحراف(p,S) ((حرف*)(p) - (S)->exec_انحراف)

  /* The removal of sflags only works with HAVE_MORECORE == 0.  */

# define get_segment_flags(S)   (USE_MMAP_ثنائی_ہندسہ)
# define سیٹ_segment_flags(S,v) \
  (((v) != USE_MMAP_ثنائی_ہندسہ) ? (ABORT, (v)) :				\
   (((S)->exec_انحراف =							\
     mmap_exec_انحراف((S)->بنیاد, (S)->سائز)),				\
    (mmap_exec_انحراف((S)->بنیاد + (S)->exec_انحراف, (S)->سائز) !=	\
     (S)->exec_انحراف) ? (ABORT, (v)) :					\
   (mmap_exec_انحراف((S)->بنیاد, (S)->سائز) = 0), (v)))

  /* We use an offset here, instead of a pointer, because then, when
     base changes, we don't have to modify this.  On architectures
     with segmented addresses, this might not work.  */
  ptrdiff_t    exec_انحراف;
#ورنہ

# define get_segment_flags(S)   ((S)->sflags)
# define سیٹ_segment_flags(S,v) ((S)->sflags = (v))
# define check_segment_ملاؤ(S,b,s) (1)

  flag_t       sflags;           /* mmap and extern flag */
#اگر_ختم
};

#تعریف_کرو is_mmapped_segment(S)  (get_segment_flags(S) & USE_MMAP_ثنائی_ہندسہ)
#تعریف_کرو is_extern_segment(S)   (get_segment_flags(S) & EXTERN_ثنائی_ہندسہ)

قسمتعریف ساخت malloc_segment  msegment;
قسمتعریف ساخت malloc_segment* msegmentptr;

/* ---------------------------- malloc_state ----------------------------- */

/*
   A malloc_state holds all of the bookkeeping for a space.
   The main fields are:

چوٹی
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
#تعریف_کرو NSMALLBINS        (32U)
#تعریف_کرو NTREEBINS         (32U)
#تعریف_کرو SMALLBIN_سرکاؤ    (3U)
#تعریف_کرو SMALLBIN_چوڑائی    (سائز_T_ONE << SMALLBIN_سرکاؤ)
#تعریف_کرو TREEBIN_سرکاؤ     (8U)
#تعریف_کرو MIN_LARGE_سائز    (سائز_T_ONE << TREEBIN_سرکاؤ)
#تعریف_کرو MAX_SMALL_سائز    (MIN_LARGE_سائز - سائز_T_ONE)
#تعریف_کرو MAX_SMALL_REQUEST (MAX_SMALL_سائز - CHUNK_ALIGN_نقاب - CHUNK_OVERHEAD)

ساخت malloc_state {
  binmap_t   smallmap;
  binmap_t   treemap;
  size_t     dvsize;
  size_t     topsize;
  حرف*      کم_سے_کم_addr;
  mchunkptr  dv;
  mchunkptr  چوٹی;
  size_t     trim_check;
  size_t     release_checks;
  size_t     magic;
  mchunkptr  smallbins[(NSMALLBINS+1)*2];
  tbinptr    treebins[NTREEBINS];
  size_t     footprint;
  size_t     max_footprint;
  size_t     footprint_حد_بندی; /* zero means no limit */
  flag_t     mflags;
#اگر USE_LOCKS
  MLOCK_T    mutex;     /* locate lock among fields that rarely change */
#اگر_ختم /* USE_LOCKS */
  msegment   seg;
  خالی*      extp;      /* Unused but available for extensions */
  size_t     exts;
};

قسمتعریف ساخت malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

ساخت malloc_params {
  size_t magic;
  size_t صفحہ_سائز;
  size_t granularity;
  size_t mmap_حد;
  size_t trim_حد;
  flag_t default_mflags;
};

ساکن ساخت malloc_params mparams;

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
# define ffi_mparams_magic_بھرو()   __atomic_load_n(&mparams.magic, __ATOMIC_ACQUIRE)
# define ffi_mparams_magic_store(v) __atomic_store_n(&mparams.magic, (size_t)(v), __ATOMIC_RELEASE)
#وگرنہ_اگر USE_LOCKS && defined(_MSC_VER)
# اگر defined(_WIN64)
#  define ffi_mparams_magic_بھرو() \
  ((size_t)_InterlockedCompareExchange64((__int64 متلون *)&mparams.magic, \
                                         (__int64)0, (__int64)0))
#  define ffi_mparams_magic_store(v) \
  ((خالی)_InterlockedExchange64((__int64 متلون *)&mparams.magic, \
                                (__int64)(v)))
# ورنہ
#  define ffi_mparams_magic_بھرو() \
  ((size_t)_InterlockedCompareExchange((LONG متلون *)&mparams.magic, \
                                       (LONG)0, (LONG)0))
#  define ffi_mparams_magic_store(v) \
  ((خالی)_InterlockedExchange((LONG متلون *)&mparams.magic, (LONG)(v)))
# endif
#ورنہ
# define ffi_mparams_magic_بھرو()   (mparams.magic)
# define ffi_mparams_magic_store(v) (*(متلون size_t *)(&(mparams.magic)) = (size_t)(v))
#اگر_ختم

/* Ensure mparams initialized */
#تعریف_کرو ensure_initialization() (خالی)(ffi_mparams_magic_بھرو() != 0 || init_mparams())

#اگر !ONLY_MSPACES

/* The global malloc_state used for all non-"mspace" calls */
ساکن ساخت malloc_state _gm_;
#تعریف_کرو gm                 (&_gm_)
#تعریف_کرو is_global(M)       ((M) == &_gm_)

#اگر_ختم /* !ONLY_MSPACES */

#تعریف_کرو is_initialized(M)  ((M)->چوٹی != 0)

/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#تعریف_کرو use_تالا(M)           ((M)->mflags &   USE_تالا_ثنائی_ہندسہ)
#تعریف_کرو enable_تالا(M)        ((M)->mflags |=  USE_تالا_ثنائی_ہندسہ)
#اگر USE_LOCKS
#تعریف_کرو disable_تالا(M)       ((M)->mflags &= ~USE_تالا_ثنائی_ہندسہ)
#ورنہ
#تعریف_کرو disable_تالا(M)
#اگر_ختم

#تعریف_کرو use_mmap(M)           ((M)->mflags &   USE_MMAP_ثنائی_ہندسہ)
#تعریف_کرو enable_mmap(M)        ((M)->mflags |=  USE_MMAP_ثنائی_ہندسہ)
#اگر HAVE_MMAP
#تعریف_کرو disable_mmap(M)       ((M)->mflags &= ~USE_MMAP_ثنائی_ہندسہ)
#ورنہ
#تعریف_کرو disable_mmap(M)
#اگر_ختم

#تعریف_کرو use_noncontiguous(M)  ((M)->mflags &   USE_NONCONTIGUOUS_ثنائی_ہندسہ)
#تعریف_کرو disable_contiguous(M) ((M)->mflags |=  USE_NONCONTIGUOUS_ثنائی_ہندسہ)

#تعریف_کرو سیٹ_تالا(M,L)\
 ((M)->mflags = (L)?\
  ((M)->mflags | USE_تالا_ثنائی_ہندسہ) :\
  ((M)->mflags & ~USE_تالا_ثنائی_ہندسہ))

/* page-align a size */
#تعریف_کرو صفحہ_align(S)\
 (((S) + (mparams.صفحہ_سائز - سائز_T_ONE)) & ~(mparams.صفحہ_سائز - سائز_T_ONE))

/* granularity-align a size */
#تعریف_کرو granularity_align(S)\
  (((S) + (mparams.granularity - سائز_T_ONE))\
   & ~(mparams.granularity - سائز_T_ONE))


/* For mmap, use granularity alignment on windows, else page-align */
#اگر_تعریف_شدہ WIN32
#تعریف_کرو mmap_align(S) granularity_align(S)
#ورنہ
#تعریف_کرو mmap_align(S) صفحہ_align(S)
#اگر_ختم

/* For sys_alloc, enough padding to ensure can malloc request on success */
#تعریف_کرو SYS_ALLOC_PADDING (چوٹی_FOOT_سائز + MALLOC_ALIGNMENT)

#تعریف_کرو is_صفحہ_aligned(S)\
   (((size_t)(S) & (mparams.صفحہ_سائز - سائز_T_ONE)) == 0)
#تعریف_کرو is_granularity_aligned(S)\
   (((size_t)(S) & (mparams.granularity - سائز_T_ONE)) == 0)

/*  True if segment S holds address A */
#تعریف_کرو segment_holds(S, A)\
  ((حرف*)(A) >= S->بنیاد && (حرف*)(A) < S->بنیاد + S->سائز)

/* Return segment holding given address */
ساکن msegmentptr segment_holding(mstate m, حرف* addr) {
  msegmentptr sp = &m->seg;
  برائے (;;) {
    اگر (addr >= sp->بنیاد && addr < sp->بنیاد + sp->سائز)
      واپس sp;
    اگر ((sp = sp->اگلا) == 0)
      واپس 0;
  }
}

/* Return true if segment contains a segment link */
ساکن صحیح has_segment_link(mstate m, msegmentptr ss) {
  msegmentptr sp = &m->seg;
  برائے (;;) {
    اگر ((حرف*)sp >= ss->بنیاد && (حرف*)sp < ss->بنیاد + ss->سائز)
      واپس 1;
    اگر ((sp = sp->اگلا) == 0)
      واپس 0;
  }
}

#اگر_غیر_تعریف_شدہ MORECORE_CANNOT_TRIM
#تعریف_کرو should_trim(M,s)  ((s) > (M)->trim_check)
#ورنہ  /* MORECORE_CANNOT_TRIM */
#تعریف_کرو should_trim(M,s)  (0)
#اگر_ختم /* MORECORE_CANNOT_TRIM */

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#تعریف_کرو چوٹی_FOOT_سائز\
  (align_انحراف(chunk2mem(0))+pad_request(حجم(ساخت malloc_segment))+MIN_CHUNK_سائز)


/* -------------------------------  Hooks -------------------------------- */

/*
  PREACTION should be defined to return 0 on success, and nonzero on
  failure. If you are not using locking, you can redefine these to do
  anything you like.
*/

#اگر USE_LOCKS
#تعریف_کرو PREACTION(M)  ((use_تالا(M))? ACQUIRE_تالا(&(M)->mutex) : 0)
#تعریف_کرو POSTACTION(M) { اگر (use_تالا(M)) RELEASE_تالا(&(M)->mutex); }
#ورنہ /* USE_LOCKS */

#اگر_غیر_تعریف_شدہ PREACTION
#تعریف_کرو PREACTION(M) (0)
#اگر_ختم  /* PREACTION */

#اگر_غیر_تعریف_شدہ POSTACTION
#تعریف_کرو POSTACTION(M)
#اگر_ختم  /* POSTACTION */

#اگر_ختم /* USE_LOCKS */

/*
  CORRUPTION_ERROR_ACTION is triggered upon detected bad addresses.
  USAGE_ERROR_ACTION is triggered on detected bad frees and
  reallocs. The argument p is an address that might have triggered the
  fault. It is ignored by the two predefined actions, but might be
  useful in custom actions that try to help diagnose errors.
*/

#اگر PROCEED_ON_ERROR

/* A count of the number of corruption errors causing resets */
صحیح malloc_corruption_error_گنتی;

/* default corruption action */
ساکن خالی reset_on_error(mstate m);

#تعریف_کرو CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#تعریف_کرو USAGE_ERROR_ACTION(m, p)

#ورنہ /* PROCEED_ON_ERROR */

#اگر_غیر_تعریف_شدہ CORRUPTION_ERROR_ACTION
#تعریف_کرو CORRUPTION_ERROR_ACTION(m) ABORT
#اگر_ختم /* CORRUPTION_ERROR_ACTION */

#اگر_غیر_تعریف_شدہ USAGE_ERROR_ACTION
#تعریف_کرو USAGE_ERROR_ACTION(m,p) ABORT
#اگر_ختم /* USAGE_ERROR_ACTION */

#اگر_ختم /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#اگر ! DEBUG

#تعریف_کرو check_free_chunk(M,P)
#تعریف_کرو check_inuse_chunk(M,P)
#تعریف_کرو check_malloced_chunk(M,P,N)
#تعریف_کرو check_mmapped_chunk(M,P)
#تعریف_کرو check_malloc_state(M)
#تعریف_کرو check_چوٹی_chunk(M,P)

#ورنہ /* DEBUG */
#تعریف_کرو check_free_chunk(M,P)       do_check_free_chunk(M,P)
#تعریف_کرو check_inuse_chunk(M,P)      do_check_inuse_chunk(M,P)
#تعریف_کرو check_چوٹی_chunk(M,P)        do_check_چوٹی_chunk(M,P)
#تعریف_کرو check_malloced_chunk(M,P,N) do_check_malloced_chunk(M,P,N)
#تعریف_کرو check_mmapped_chunk(M,P)    do_check_mmapped_chunk(M,P)
#تعریف_کرو check_malloc_state(M)       do_check_malloc_state(M)

ساکن خالی   do_check_any_chunk(mstate m, mchunkptr p);
ساکن خالی   do_check_چوٹی_chunk(mstate m, mchunkptr p);
ساکن خالی   do_check_mmapped_chunk(mstate m, mchunkptr p);
ساکن خالی   do_check_inuse_chunk(mstate m, mchunkptr p);
ساکن خالی   do_check_free_chunk(mstate m, mchunkptr p);
ساکن خالی   do_check_malloced_chunk(mstate m, خالی* mem, size_t s);
ساکن خالی   do_check_درخت(mstate m, tchunkptr t);
ساکن خالی   do_check_treebin(mstate m, bindex_t i);
ساکن خالی   do_check_smallbin(mstate m, bindex_t i);
ساکن خالی   do_check_malloc_state(mstate m);
ساکن صحیح    bin_ڈھونڈو(mstate m, mchunkptr x);
ساکن size_t traverse_and_check(mstate m);
#اگر_ختم /* DEBUG */

/* ---------------------------- Indexing Bins ---------------------------- */

#تعریف_کرو is_small(s)         (((s) >> SMALLBIN_سرکاؤ) < NSMALLBINS)
#تعریف_کرو small_اشاریہ(s)      (bindex_t)((s)  >> SMALLBIN_سرکاؤ)
#تعریف_کرو small_index2size(i) ((i)  << SMALLBIN_سرکاؤ)
#تعریف_کرو MIN_SMALL_اشاریہ     (small_اشاریہ(MIN_CHUNK_سائز))

/* addressing by index. See above about smallbin repositioning */
#تعریف_کرو smallbin_at(M, i)   ((sbinptr)((حرف*)&((M)->smallbins[(i)<<1])))
#تعریف_کرو treebin_at(M,i)     (&((M)->treebins[i]))

/* assign tree index for size S to variable I.
   libffi: enable the __builtin_clz path on any GNU-compatible compiler, not
   only x86, so the log2 is an intrinsic rather than the generic C idiom that
   the compiler can't always recognize (libffi #754).  */
#اگر defined(__GNUC__)
#تعریف_کرو compute_درخت_اشاریہ(S, I)\
{\
  غیرعلامتی صحیح X = S >> TREEBIN_سرکاؤ;\
  اگر (X == 0)\
    I = 0;\
  ورنہ اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ورنہ {\
    غیرعلامتی صحیح K = (غیرعلامتی) حجم(X)*__CHAR_BIT__ - 1 - (غیرعلامتی) __builtin_clz(X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_سرکاؤ-1)) & 1)));\
  }\
}

#وگرنہ_اگر defined (__INTEL_COMPILER)
#تعریف_کرو compute_درخت_اشاریہ(S, I)\
{\
  size_t X = S >> TREEBIN_سرکاؤ;\
  اگر (X == 0)\
    I = 0;\
  ورنہ اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ورنہ {\
    غیرعلامتی صحیح K = ثنائی_ہندسہ_جائزہ_الٹ_3 (X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_سرکاؤ-1)) & 1)));\
  }\
}

#وگرنہ_اگر defined(_MSC_VER) && _MSC_VER>=1300
#تعریف_کرو compute_درخت_اشاریہ(S, I)\
{\
  size_t X = S >> TREEBIN_سرکاؤ;\
  اگر (X == 0)\
    I = 0;\
  ورنہ اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ورنہ {\
    غیرعلامتی صحیح K;\
    ثنائی_ہندسہ_جائزہ_الٹ((DWORD *) &K, (DWORD) X);\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_سرکاؤ-1)) & 1)));\
  }\
}

#ورنہ /* GNUC */
#تعریف_کرو compute_درخت_اشاریہ(S, I)\
{\
  size_t X = S >> TREEBIN_سرکاؤ;\
  اگر (X == 0)\
    I = 0;\
  ورنہ اگر (X > 0xFFFF)\
    I = NTREEBINS-1;\
  ورنہ {\
    غیرعلامتی صحیح Y = (غیرعلامتی صحیح)X;\
    غیرعلامتی صحیح N = ((Y - 0x100) >> 16) & 8;\
    غیرعلامتی صحیح K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_سرکاؤ-1)) & 1));\
  }\
}
#اگر_ختم /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#تعریف_کرو ثنائی_ہندسہ_for_درخت_اشاریہ(i) \
   (i == NTREEBINS-1)? (سائز_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_سرکاؤ - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#تعریف_کرو leftshift_for_درخت_اشاریہ(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((سائز_T_BITSIZE-سائز_T_ONE) - (((i) >> 1) + TREEBIN_سرکاؤ - 2)))

/* The size of the smallest chunk held in bin with index i */
#تعریف_کرو minsize_for_درخت_اشاریہ(i) \
   ((سائز_T_ONE << (((i) >> 1) + TREEBIN_سرکاؤ)) |  \
   (((size_t)((i) & سائز_T_ONE)) << (((i) >> 1) + TREEBIN_سرکاؤ - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#تعریف_کرو idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#تعریف_کرو mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#تعریف_کرو clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#تعریف_کرو smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#تعریف_کرو mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#تعریف_کرو clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#تعریف_کرو treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#تعریف_کرو کم_سے_کم_ثنائی_ہندسہ(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#تعریف_کرو بایاں_بٹ(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#تعریف_کرو same_or_بایاں_بٹ(x) ((x) | -(x))

/* index corresponding to given bit.
   libffi: use __builtin_ctz on any GNU-compatible compiler, not only x86
   (libffi #754).  */

#اگر defined(__GNUC__)
#تعریف_کرو compute_bit2idx(X, I)\
{\
  غیرعلامتی صحیح J;\
  J = __builtin_ctz(X); \
  I = (bindex_t)J;\
}

#وگرنہ_اگر defined (__INTEL_COMPILER)
#تعریف_کرو compute_bit2idx(X, I)\
{\
  غیرعلامتی صحیح J;\
  J = ثنائی_ہندسہ_جائزہ_forward (X); \
  I = (bindex_t)J;\
}

#وگرنہ_اگر defined(_MSC_VER) && _MSC_VER>=1300
#تعریف_کرو compute_bit2idx(X, I)\
{\
  غیرعلامتی صحیح J;\
  ثنائی_ہندسہ_جائزہ_Forward_2((DWORD *) &J, X);\
  I = (bindex_t)J;\
}

#وگرنہ_اگر USE_BUILTIN_FFS
#تعریف_کرو compute_bit2idx(X, I) I = ffs(X)-1

#ورنہ
#تعریف_کرو compute_bit2idx(X, I)\
{\
  غیرعلامتی صحیح Y = X - 1;\
  غیرعلامتی صحیح K = Y >> (16-4) & 16;\
  غیرعلامتی صحیح N = K;        Y >>= K;\
  N += K = Y >> (8-3) &  8;  Y >>= K;\
  N += K = Y >> (4-2) &  4;  Y >>= K;\
  N += K = Y >> (2-1) &  2;  Y >>= K;\
  N += K = Y >> (1-0) &  1;  Y >>= K;\
  I = (bindex_t)(N + Y);\
}
#اگر_ختم /* GNUC */


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
#تعریف_کرو ok_پتہ(M, a) ((حرف*)(a) >= (M)->کم_سے_کم_addr)
/* Check if address of next chunk n is higher than base chunk p */
#تعریف_کرو ok_اگلا(p, n)    ((حرف*)(p) < (حرف*)(n))
/* Check if p has inuse status */
#تعریف_کرو ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#تعریف_کرو ok_pinuse(p)     pinuse(p)

#ورنہ /* !INSECURE */
#تعریف_کرو ok_پتہ(M, a) (1)
#تعریف_کرو ok_اگلا(b, n)    (1)
#تعریف_کرو ok_inuse(p)      (1)
#تعریف_کرو ok_pinuse(p)     (1)
#اگر_ختم /* !INSECURE */

#اگر (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#تعریف_کرو ok_magic(M)      ((M)->magic == mparams.magic)
#ورنہ  /* (FOOTERS && !INSECURE) */
#تعریف_کرو ok_magic(M)      (1)
#اگر_ختم /* (FOOTERS && !INSECURE) */

/* In gcc, use __builtin_expect to minimize impact of checks */
#اگر !INSECURE
#اگر defined(__GNUC__) && __GNUC__ >= 3
#تعریف_کرو RTCHECK(e)  __builtin_expect(e, 1)
#ورنہ /* GNUC */
#تعریف_کرو RTCHECK(e)  (e)
#اگر_ختم /* GNUC */
#ورنہ /* !INSECURE */
#تعریف_کرو RTCHECK(e)  (1)
#اگر_ختم /* !INSECURE */

/* macros to set up inuse chunks with or without footers */

#اگر !FOOTERS

#تعریف_کرو mark_inuse_foot(M,p,s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#تعریف_کرو سیٹ_inuse(M,p,s)\
  ((p)->سر = (((p)->سر & PINUSE_ثنائی_ہندسہ)|s|CINUSE_ثنائی_ہندسہ),\
  ((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ثنائی_ہندسہ)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#تعریف_کرو سیٹ_inuse_and_pinuse(M,p,s)\
  ((p)->سر = (s|PINUSE_ثنائی_ہندسہ|CINUSE_ثنائی_ہندسہ),\
  ((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ثنائی_ہندسہ)

/* Set size, cinuse and pinuse bit of this chunk */
#تعریف_کرو سیٹ_سائز_and_pinuse_کا_inuse_chunk(M, p, s)\
  ((p)->سر = (s|PINUSE_ثنائی_ہندسہ|CINUSE_ثنائی_ہندسہ))

#ورنہ /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#تعریف_کرو mark_inuse_foot(M,p,s)\
  (((mchunkptr)((حرف*)(p) + (s)))->پچھلا_foot = ((size_t)(M) ^ mparams.magic))

#تعریف_کرو get_mstate_for(p)\
  ((mstate)(((mchunkptr)((حرف*)(p) +\
    (chunksize(p))))->پچھلا_foot ^ mparams.magic))

#تعریف_کرو سیٹ_inuse(M,p,s)\
  ((p)->سر = (((p)->سر & PINUSE_ثنائی_ہندسہ)|s|CINUSE_ثنائی_ہندسہ),\
  (((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ثنائی_ہندسہ), \
  mark_inuse_foot(M,p,s))

#تعریف_کرو سیٹ_inuse_and_pinuse(M,p,s)\
  ((p)->سر = (s|PINUSE_ثنائی_ہندسہ|CINUSE_ثنائی_ہندسہ),\
  (((mchunkptr)(((حرف*)(p)) + (s)))->سر |= PINUSE_ثنائی_ہندسہ),\
 mark_inuse_foot(M,p,s))

#تعریف_کرو سیٹ_سائز_and_pinuse_کا_inuse_chunk(M, p, s)\
  ((p)->سر = (s|PINUSE_ثنائی_ہندسہ|CINUSE_ثنائی_ہندسہ),\
  mark_inuse_foot(M, p, s))

#اگر_ختم /* !FOOTERS */

/* ---------------------------- setting mparams -------------------------- */

#اگر تالا_AT_FORK
ساکن خالی pre_fork(خالی)         { ACQUIRE_تالا(&(gm)->mutex); }
ساکن خالی post_fork_والد(خالی) { RELEASE_تالا(&(gm)->mutex); }
ساکن خالی post_fork_بچہ(خالی)  { INITIAL_تالا(&(gm)->mutex); }
#اگر_ختم /* LOCK_AT_FORK */

/* Initialize mparams */
ساکن صحیح init_mparams(خالی) {
#اگر_تعریف_شدہ NEED_GLOBAL_تالا_INIT
  اگر (malloc_global_mutex_حالت <= 0)
    init_malloc_global_mutex();
#اگر_ختم

  ACQUIRE_MALLOC_GLOBAL_تالا();
  اگر (mparams.magic == 0) {
    size_t magic;
    size_t psize;
    size_t gsize;

#اگر !defined(WIN32) && !defined(__OS2__)
    psize = malloc_getpagesize;
    gsize = ((DEFAULT_GRANULARITY != 0)? DEFAULT_GRANULARITY : psize);
#وگرنہ_اگر defined (__OS2__)
    /* if low-memory is used, os2munmap() would break
       if it were anything other than 64k */
    psize = 4096u;
    gsize = 65536u;
#ورنہ /* WIN32 */
    {
      SYSTEM_INFO system_info;
      GetSystemInfo(&system_info);
      psize = system_info.dw_صفحہ_سائز;
      gsize = ((DEFAULT_GRANULARITY != 0)?
               DEFAULT_GRANULARITY : system_info.dw_تخصیص_Granularity);
    }
#اگر_ختم /* WIN32 */

    /* Sanity-check configuration:
       size_t must be unsigned and as wide as pointer type.
       ints must be at least 4 bytes.
       alignment must be at least 8.
       Alignment, min chunk size, and page size must all be powers of 2.
    */
    اگر ((حجم(size_t) != حجم(حرف*)) ||
        (MAX_سائز_T < MIN_CHUNK_سائز)  ||
        (حجم(صحیح) < 4)  ||
        (MALLOC_ALIGNMENT < (size_t)8U) ||
        ((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-سائز_T_ONE)) != 0) ||
        ((MCHUNK_سائز      & (MCHUNK_سائز-سائز_T_ONE))      != 0) ||
        ((gsize            & (gsize-سائز_T_ONE))            != 0) ||
        ((psize            & (psize-سائز_T_ONE))            != 0))
      ABORT;
    mparams.granularity = gsize;
    mparams.صفحہ_سائز = psize;
    mparams.mmap_حد = DEFAULT_MMAP_حد;
    mparams.trim_حد = DEFAULT_TRIM_حد;
#اگر MORECORE_CONTIGUOUS
    mparams.default_mflags = USE_تالا_ثنائی_ہندسہ|USE_MMAP_ثنائی_ہندسہ;
#ورنہ  /* MORECORE_CONTIGUOUS */
    mparams.default_mflags = USE_تالا_ثنائی_ہندسہ|USE_MMAP_ثنائی_ہندسہ|USE_NONCONTIGUOUS_ثنائی_ہندسہ;
#اگر_ختم /* MORECORE_CONTIGUOUS */

#اگر !ONLY_MSPACES
    /* Set up lock for main malloc area */
    gm->mflags = mparams.default_mflags;
    (خالی)INITIAL_تالا(&gm->mutex);
#اگر_ختم
#اگر تالا_AT_FORK
    pthread_atfork(&pre_fork, &post_fork_والد, &post_fork_بچہ);
#اگر_ختم

    {
#اگر USE_DEV_RANDOM
      صحیح fd;
      غیرعلامتی حرف buf[حجم(size_t)];
      /* Try to use /dev/urandom, else fall back on using time */
      اگر ((fd = open("/dev/urandom", O_RDONLY)) >= 0 &&
          اقرأ(fd, buf, حجم(buf)) == حجم(buf)) {
        magic = *((size_t *) buf);
        close(fd);
      }
      ورنہ
#اگر_ختم /* USE_DEV_RANDOM */
#اگر_تعریف_شدہ WIN32
      magic = (size_t)(Get_Tick_گنتی() ^ (size_t)0x55555555U);
#وگرنہ_اگر defined(LACKS_وقت_H)
      magic = (size_t)&magic ^ (size_t)0x55555555U;
#ورنہ
      magic = (size_t)(time(0) ^ (size_t)0x55555555U);
#اگر_ختم
      magic |= (size_t)8U;    /* ensure nonzero */
      magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
      /* libffi: release-store so any thread that observes magic via the
         acquire load in ensure_initialization() sees the writes above (#873).  */
      ffi_mparams_magic_store(magic);
    }
  }

  RELEASE_MALLOC_GLOBAL_تالا();
  واپس 1;
}

/* support for mallopt */
ساکن صحیح change_mparam(صحیح param_نمبر, صحیح قدر) {
  size_t val;
  ensure_initialization();
  val = (قدر == -1)? MAX_سائز_T : (size_t)قدر;
  انتخاب(param_نمبر) {
  حالت M_TRIM_حد:
    mparams.trim_حد = val;
    واپس 1;
  حالت M_GRANULARITY:
    اگر (val >= mparams.صفحہ_سائز && ((val & (val-1)) == 0)) {
      mparams.granularity = val;
      واپس 1;
    }
    ورنہ
      واپس 0;
  حالت M_MMAP_حد:
    mparams.mmap_حد = val;
    واپس 1;
  طےشدہ:
    واپس 0;
  }
}

#اگر DEBUG
/* ------------------------- Debugging Support --------------------------- */

/* Check properties of any chunk, whether free, inuse, mmapped etc  */
ساکن خالی do_check_any_chunk(mstate m, mchunkptr p) {
  أكد((is_aligned(chunk2mem(p))) || (p->سر == FENCEPOST_سر));
  أكد(ok_پتہ(m, p));
}

/* Check properties of top chunk */
ساکن خالی do_check_چوٹی_chunk(mstate m, mchunkptr p) {
  msegmentptr sp = segment_holding(m, (حرف*)p);
  size_t  sz = p->سر & ~INUSE_بٹ; /* third-lowest bit can be set! */
  أكد(sp != 0);
  أكد((is_aligned(chunk2mem(p))) || (p->سر == FENCEPOST_سر));
  أكد(ok_پتہ(m, p));
  أكد(sz == m->topsize);
  أكد(sz > 0);
  أكد(sz == ((sp->بنیاد + sp->سائز) - (حرف*)p) - چوٹی_FOOT_سائز);
  أكد(pinuse(p));
  أكد(!pinuse(chunk_plus_انحراف(p, sz)));
}

/* Check properties of (inuse) mmapped chunks */
ساکن خالی do_check_mmapped_chunk(mstate m, mchunkptr p) {
  size_t  sz = chunksize(p);
  size_t len = (sz + (p->پچھلا_foot) + MMAP_FOOT_PAD);
  أكد(is_mmapped(p));
  أكد(use_mmap(m));
  أكد((is_aligned(chunk2mem(p))) || (p->سر == FENCEPOST_سر));
  أكد(ok_پتہ(m, p));
  أكد(!is_small(sz));
  أكد((len & (mparams.صفحہ_سائز-سائز_T_ONE)) == 0);
  أكد(chunk_plus_انحراف(p, sz)->سر == FENCEPOST_سر);
  أكد(chunk_plus_انحراف(p, sz+سائز_T_سائز)->سر == 0);
}

/* Check properties of inuse chunks */
ساکن خالی do_check_inuse_chunk(mstate m, mchunkptr p) {
  do_check_any_chunk(m, p);
  أكد(is_inuse(p));
  أكد(اگلا_pinuse(p));
  /* If not pinuse and not mmapped, previous chunk has OK offset */
  أكد(is_mmapped(p) || pinuse(p) || اگلا_chunk(پچھلا_chunk(p)) == p);
  اگر (is_mmapped(p))
    do_check_mmapped_chunk(m, p);
}

/* Check properties of free chunks */
ساکن خالی do_check_free_chunk(mstate m, mchunkptr p) {
  size_t sz = chunksize(p);
  mchunkptr اگلا = chunk_plus_انحراف(p, sz);
  do_check_any_chunk(m, p);
  أكد(!is_inuse(p));
  أكد(!اگلا_pinuse(p));
  أكد (!is_mmapped(p));
  اگر (p != m->dv && p != m->چوٹی) {
    اگر (sz >= MIN_CHUNK_سائز) {
      أكد((sz & CHUNK_ALIGN_نقاب) == 0);
      أكد(is_aligned(chunk2mem(p)));
      أكد(اگلا->پچھلا_foot == sz);
      أكد(pinuse(p));
      أكد (اگلا == m->چوٹی || is_inuse(اگلا));
      أكد(p->fd->bk == p);
      أكد(p->bk->fd == p);
    }
    ورنہ  /* markers are always of size SIZE_T_SIZE */
      أكد(sz == سائز_T_سائز);
  }
}

/* Check properties of malloced chunks at the point they are malloced */
ساکن خالی do_check_malloced_chunk(mstate m, خالی* mem, size_t s) {
  اگر (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    size_t sz = p->سر & ~INUSE_بٹ;
    do_check_inuse_chunk(m, p);
    أكد((sz & CHUNK_ALIGN_نقاب) == 0);
    أكد(sz >= MIN_CHUNK_سائز);
    أكد(sz >= s);
    /* unless mmapped, size is less than MIN_CHUNK_SIZE more than request */
    أكد(is_mmapped(p) || sz < (s + MIN_CHUNK_سائز));
  }
}

/* Check a tree and its subtrees.  */
ساکن خالی do_check_درخت(mstate m, tchunkptr t) {
  tchunkptr سر = 0;
  tchunkptr u = t;
  bindex_t tindex = t->اشاریہ;
  size_t tsize = chunksize(t);
  bindex_t idx;
  compute_درخت_اشاریہ(tsize, idx);
  أكد(tindex == idx);
  أكد(tsize >= MIN_LARGE_سائز);
  أكد(tsize >= minsize_for_درخت_اشاریہ(idx));
  أكد((idx == NTREEBINS-1) || (tsize < minsize_for_درخت_اشاریہ((idx+1))));

  کرو { /* traverse through chain of same-sized nodes */
    do_check_any_chunk(m, ((mchunkptr)u));
    أكد(u->اشاریہ == tindex);
    أكد(chunksize(u) == tsize);
    أكد(!is_inuse(u));
    أكد(!اگلا_pinuse(u));
    أكد(u->fd->bk == u);
    أكد(u->bk->fd == u);
    اگر (u->والد == 0) {
      أكد(u->بچہ[0] == 0);
      أكد(u->بچہ[1] == 0);
    }
    ورنہ {
      أكد(سر == 0); /* only one node on chain has parent */
      سر = u;
      أكد(u->والد != u);
      أكد (u->والد->بچہ[0] == u ||
              u->والد->بچہ[1] == u ||
              *((tbinptr*)(u->والد)) == u);
      اگر (u->بچہ[0] != 0) {
        أكد(u->بچہ[0]->والد == u);
        أكد(u->بچہ[0] != u);
        do_check_درخت(m, u->بچہ[0]);
      }
      اگر (u->بچہ[1] != 0) {
        أكد(u->بچہ[1]->والد == u);
        أكد(u->بچہ[1] != u);
        do_check_درخت(m, u->بچہ[1]);
      }
      اگر (u->بچہ[0] != 0 && u->بچہ[1] != 0) {
        أكد(chunksize(u->بچہ[0]) < chunksize(u->بچہ[1]));
      }
    }
    u = u->fd;
  } جب (u != t);
  أكد(سر != 0);
}

/*  Check all the chunks in a treebin.  */
ساکن خالی do_check_treebin(mstate m, bindex_t i) {
  tbinptr* tb = treebin_at(m, i);
  tchunkptr t = *tb;
  صحیح خالی_خانے = (m->treemap & (1U << i)) == 0;
  اگر (t == 0)
    أكد(خالی_خانے);
  اگر (!خالی_خانے)
    do_check_درخت(m, t);
}

/*  Check all the chunks in a smallbin.  */
ساکن خالی do_check_smallbin(mstate m, bindex_t i) {
  sbinptr b = smallbin_at(m, i);
  mchunkptr p = b->bk;
  غیرعلامتی صحیح خالی_خانے = (m->smallmap & (1U << i)) == 0;
  اگر (p == b)
    أكد(خالی_خانے);
  اگر (!خالی_خانے) {
    برائے (; p != b; p = p->bk) {
      size_t سائز = chunksize(p);
      mchunkptr q;
      /* each chunk claims to be free */
      do_check_free_chunk(m, p);
      /* chunk belongs in bin */
      أكد(small_اشاریہ(سائز) == i);
      أكد(p->bk == b || chunksize(p->bk) == chunksize(p));
      /* chunk is followed by an inuse chunk */
      q = اگلا_chunk(p);
      اگر (q->سر != FENCEPOST_سر)
        do_check_inuse_chunk(m, q);
    }
  }
}

/* Find x in a bin. Used in other check functions. */
ساکن صحیح bin_ڈھونڈو(mstate m, mchunkptr x) {
  size_t سائز = chunksize(x);
  اگر (is_small(سائز)) {
    bindex_t sidx = small_اشاریہ(سائز);
    sbinptr b = smallbin_at(m, sidx);
    اگر (smallmap_is_marked(m, sidx)) {
      mchunkptr p = b;
      کرو {
        اگر (p == x)
          واپس 1;
      } جب ((p = p->fd) != b);
    }
  }
  ورنہ {
    bindex_t tidx;
    compute_درخت_اشاریہ(سائز, tidx);
    اگر (treemap_is_marked(m, tidx)) {
      tchunkptr t = *treebin_at(m, tidx);
      size_t sizebits = سائز << leftshift_for_درخت_اشاریہ(tidx);
      جب (t != 0 && chunksize(t) != سائز) {
        t = t->بچہ[(sizebits >> (سائز_T_BITSIZE-سائز_T_ONE)) & 1];
        sizebits <<= 1;
      }
      اگر (t != 0) {
        tchunkptr u = t;
        کرو {
          اگر (u == (tchunkptr)x)
            واپس 1;
        } جب ((u = u->fd) != t);
      }
    }
  }
  واپس 0;
}

/* Traverse each chunk and check it; return total */
ساکن size_t traverse_and_check(mstate m) {
  size_t مجموع = 0;
  اگر (is_initialized(m)) {
    msegmentptr s = &m->seg;
    مجموع += m->topsize + چوٹی_FOOT_سائز;
    جب (s != 0) {
      mchunkptr q = align_as_chunk(s->بنیاد);
      mchunkptr lastq = 0;
      أكد(pinuse(q));
      جب (segment_holds(s, q) &&
             q != m->چوٹی && q->سر != FENCEPOST_سر) {
        مجموع += chunksize(q);
        اگر (is_inuse(q)) {
          أكد(!bin_ڈھونڈو(m, q));
          do_check_inuse_chunk(m, q);
        }
        ورنہ {
          أكد(q == m->dv || bin_ڈھونڈو(m, q));
          أكد(lastq == 0 || is_inuse(lastq)); /* Not 2 consecutive free */
          do_check_free_chunk(m, q);
        }
        lastq = q;
        q = اگلا_chunk(q);
      }
      s = s->اگلا;
    }
  }
  واپس مجموع;
}


/* Check all properties of malloc_state. */
ساکن خالی do_check_malloc_state(mstate m) {
  bindex_t i;
  size_t کل;
  /* check bins */
  برائے (i = 0; i < NSMALLBINS; ++i)
    do_check_smallbin(m, i);
  برائے (i = 0; i < NTREEBINS; ++i)
    do_check_treebin(m, i);

  اگر (m->dvsize != 0) { /* check dv chunk */
    do_check_any_chunk(m, m->dv);
    أكد(m->dvsize == chunksize(m->dv));
    أكد(m->dvsize >= MIN_CHUNK_سائز);
    أكد(bin_ڈھونڈو(m, m->dv) == 0);
  }

  اگر (m->چوٹی != 0) {   /* check top chunk */
    do_check_چوٹی_chunk(m, m->چوٹی);
    /*assert(m->topsize == chunksize(m->top)); redundant */
    أكد(m->topsize > 0);
    أكد(bin_ڈھونڈو(m, m->چوٹی) == 0);
  }

  کل = traverse_and_check(m);
  أكد(کل <= m->footprint);
  أكد(m->footprint <= m->max_footprint);
}
#اگر_ختم /* DEBUG */

/* ----------------------------- statistics ------------------------------ */

#اگر !NO_MALLINFO
ساکن ساخت mallinfo اندرونی_mallinfo(mstate m) {
  ساخت mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  ensure_initialization();
  اگر (!PREACTION(m)) {
    check_malloc_state(m);
    اگر (is_initialized(m)) {
      size_t nfree = سائز_T_ONE; /* top always free */
      size_t mfree = m->topsize + چوٹی_FOOT_سائز;
      size_t مجموع = mfree;
      msegmentptr s = &m->seg;
      جب (s != 0) {
        mchunkptr q = align_as_chunk(s->بنیاد);
        جب (segment_holds(s, q) &&
               q != m->چوٹی && q->سر != FENCEPOST_سر) {
          size_t sz = chunksize(q);
          مجموع += sz;
          اگر (!is_inuse(q)) {
            mfree += sz;
            ++nfree;
          }
          q = اگلا_chunk(q);
        }
        s = s->اگلا;
      }

      nm.arena    = مجموع;
      nm.ordblks  = nfree;
      nm.hblkhd   = m->footprint - مجموع;
      nm.usmblks  = m->max_footprint;
      nm.uordblks = m->footprint - mfree;
      nm.fordblks = mfree;
      nm.keepcost = m->topsize;
    }

    POSTACTION(m);
  }
  واپس nm;
}
#اگر_ختم /* !NO_MALLINFO */

#اگر !NO_MALLOC_STATS
ساکن خالی اندرونی_malloc_stats(mstate m) {
  ensure_initialization();
  اگر (!PREACTION(m)) {
    size_t maxfp = 0;
    size_t fp = 0;
    size_t مستعمل = 0;
    check_malloc_state(m);
    اگر (is_initialized(m)) {
      msegmentptr s = &m->seg;
      maxfp = m->max_footprint;
      fp = m->footprint;
      مستعمل = fp - (m->topsize + چوٹی_FOOT_سائز);

      جب (s != 0) {
        mchunkptr q = align_as_chunk(s->بنیاد);
        جب (segment_holds(s, q) &&
               q != m->چوٹی && q->سر != FENCEPOST_سر) {
          اگر (!is_inuse(q))
            مستعمل -= chunksize(q);
          q = اگلا_chunk(q);
        }
        s = s->اگلا;
      }
    }
    POSTACTION(m); /* drop lock */
    fprintf(stderr, "max system bytes = %10lu\n", (غیرعلامتی طویل)(maxfp));
    fprintf(stderr, "system bytes     = %10lu\n", (غیرعلامتی طویل)(fp));
    fprintf(stderr, "in use bytes     = %10lu\n", (غیرعلامتی طویل)(مستعمل));
  }
}
#اگر_ختم /* NO_MALLOC_STATS */

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#تعریف_کرو داخل_کرو_small_chunk(M, P, S) {\
  bindex_t I  = small_اشاریہ(S);\
  mchunkptr B = smallbin_at(M, I);\
  mchunkptr F = B;\
  أكد(S >= MIN_CHUNK_سائز);\
  اگر (!smallmap_is_marked(M, I))\
    mark_smallmap(M, I);\
  ورنہ اگر (RTCHECK(ok_پتہ(M, B->fd)))\
    F = B->fd;\
  ورنہ {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
  B->fd = P;\
  F->bk = P;\
  P->fd = F;\
  P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#تعریف_کرو unlink_small_chunk(M, P, S) {\
  mchunkptr F = P->fd;\
  mchunkptr B = P->bk;\
  bindex_t I = small_اشاریہ(S);\
  أكد(P != B);\
  أكد(P != F);\
  أكد(chunksize(P) == small_index2size(I));\
  اگر (RTCHECK(F == smallbin_at(M,I) || (ok_پتہ(M, F) && F->bk == P))) { \
    اگر (B == F) {\
      clear_smallmap(M, I);\
    }\
    ورنہ اگر (RTCHECK(B == smallbin_at(M,I) ||\
                     (ok_پتہ(M, B) && B->fd == P))) {\
      F->bk = B;\
      B->fd = F;\
    }\
    ورنہ {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  ورنہ {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Unlink the first chunk from a smallbin */
#تعریف_کرو unlink_پہلا_small_chunk(M, B, P, I) {\
  mchunkptr F = P->fd;\
  أكد(P != B);\
  أكد(P != F);\
  أكد(chunksize(P) == small_index2size(I));\
  اگر (B == F) {\
    clear_smallmap(M, I);\
  }\
  ورنہ اگر (RTCHECK(ok_پتہ(M, F) && F->bk == P)) {\
    F->bk = B;\
    B->fd = F;\
  }\
  ورنہ {\
    CORRUPTION_ERROR_ACTION(M);\
  }\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#تعریف_کرو replace_dv(M, P, S) {\
  size_t DVS = M->dvsize;\
  أكد(is_small(DVS));\
  اگر (DVS != 0) {\
    mchunkptr DV = M->dv;\
    داخل_کرو_small_chunk(M, DV, DVS);\
  }\
  M->dvsize = S;\
  M->dv = P;\
}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#تعریف_کرو داخل_کرو_large_chunk(M, X, S) {\
  tbinptr* H;\
  bindex_t I;\
  compute_درخت_اشاریہ(S, I);\
  H = treebin_at(M, I);\
  X->اشاریہ = I;\
  X->بچہ[0] = X->بچہ[1] = 0;\
  اگر (!treemap_is_marked(M, I)) {\
    mark_treemap(M, I);\
    *H = X;\
    X->والد = (tchunkptr)H;\
    X->fd = X->bk = X;\
  }\
  ورنہ {\
    tchunkptr T = *H;\
    size_t K = S << leftshift_for_درخت_اشاریہ(I);\
    برائے (;;) {\
      اگر (chunksize(T) != S) {\
        tchunkptr* C = &(T->بچہ[(K >> (سائز_T_BITSIZE-سائز_T_ONE)) & 1]);\
        K <<= 1;\
        اگر (*C != 0)\
          T = *C;\
        ورنہ اگر (RTCHECK(ok_پتہ(M, C))) {\
          *C = X;\
          X->والد = T;\
          X->fd = X->bk = X;\
          توڑو;\
        }\
        ورنہ {\
          CORRUPTION_ERROR_ACTION(M);\
          توڑو;\
        }\
      }\
      ورنہ {\
        tchunkptr F = T->fd;\
        اگر (RTCHECK(ok_پتہ(M, T) && ok_پتہ(M, F))) {\
          T->fd = F->bk = X;\
          X->fd = F;\
          X->bk = T;\
          X->والد = 0;\
          توڑو;\
        }\
        ورنہ {\
          CORRUPTION_ERROR_ACTION(M);\
          توڑو;\
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

#تعریف_کرو unlink_large_chunk(M, X) {\
  tchunkptr XP = X->والد;\
  tchunkptr R;\
  اگر (X->bk != X) {\
    tchunkptr F = X->fd;\
    R = X->bk;\
    اگر (RTCHECK(ok_پتہ(M, F) && F->bk == X && R->fd == X)) {\
      F->bk = R;\
      R->fd = F;\
    }\
    ورنہ {\
      CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
  ورنہ {\
    tchunkptr* RP;\
    اگر (((R = *(RP = &(X->بچہ[1]))) != 0) ||\
        ((R = *(RP = &(X->بچہ[0]))) != 0)) {\
      tchunkptr* CP;\
      جب ((*(CP = &(R->بچہ[1])) != 0) ||\
             (*(CP = &(R->بچہ[0])) != 0)) {\
        R = *(RP = CP);\
      }\
      اگر (RTCHECK(ok_پتہ(M, RP)))\
        *RP = 0;\
      ورنہ {\
        CORRUPTION_ERROR_ACTION(M);\
      }\
    }\
  }\
  اگر (XP != 0) {\
    tbinptr* H = treebin_at(M, X->اشاریہ);\
    اگر (X == *H) {\
      اگر ((*H = R) == 0) \
        clear_treemap(M, X->اشاریہ);\
    }\
    ورنہ اگر (RTCHECK(ok_پتہ(M, XP))) {\
      اگر (XP->بچہ[0] == X) \
        XP->بچہ[0] = R;\
      ورنہ \
        XP->بچہ[1] = R;\
    }\
    ورنہ\
      CORRUPTION_ERROR_ACTION(M);\
    اگر (R != 0) {\
      اگر (RTCHECK(ok_پتہ(M, R))) {\
        tchunkptr C0, C1;\
        R->والد = XP;\
        اگر ((C0 = X->بچہ[0]) != 0) {\
          اگر (RTCHECK(ok_پتہ(M, C0))) {\
            R->بچہ[0] = C0;\
            C0->والد = R;\
          }\
          ورنہ\
            CORRUPTION_ERROR_ACTION(M);\
        }\
        اگر ((C1 = X->بچہ[1]) != 0) {\
          اگر (RTCHECK(ok_پتہ(M, C1))) {\
            R->بچہ[1] = C1;\
            C1->والد = R;\
          }\
          ورنہ\
            CORRUPTION_ERROR_ACTION(M);\
        }\
      }\
      ورنہ\
        CORRUPTION_ERROR_ACTION(M);\
    }\
  }\
}

/* Relays to large vs small bin operations */

#تعریف_کرو داخل_کرو_chunk(M, P, S)\
  اگر (is_small(S)) داخل_کرو_small_chunk(M, P, S)\
  ورنہ { tchunkptr TP = (tchunkptr)(P); داخل_کرو_large_chunk(M, TP, S); }

#تعریف_کرو unlink_chunk(M, P, S)\
  اگر (is_small(S)) unlink_small_chunk(M, P, S)\
  ورنہ { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#اگر ONLY_MSPACES
#تعریف_کرو اندرونی_malloc(m, b) mspace_malloc(m, b)
#تعریف_کرو اندرونی_free(m, mem) mspace_free(m,mem);
#ورنہ /* ONLY_MSPACES */
#اگر MSPACES
#تعریف_کرو اندرونی_malloc(m, b)\
  ((m == gm)? dlmalloc(b) : mspace_malloc(m, b))
#تعریف_کرو اندرونی_free(m, mem)\
   اگر (m == gm) dlfree(mem); ورنہ mspace_free(m,mem);
#ورنہ /* MSPACES */
#تعریف_کرو اندرونی_malloc(m, b) dlmalloc(b)
#تعریف_کرو اندرونی_free(m, mem) dlfree(mem)
#اگر_ختم /* MSPACES */
#اگر_ختم /* ONLY_MSPACES */

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
ساکن خالی* mmap_alloc(mstate m, size_t nb) {
  size_t mmsize = mmap_align(nb + SIX_سائز_T_SIZES + CHUNK_ALIGN_نقاب);
  اگر (m->footprint_حد_بندی != 0) {
    size_t fp = m->footprint + mmsize;
    اگر (fp <= m->footprint || fp > m->footprint_حد_بندی)
      واپس 0;
  }
  اگر (mmsize > nb) {     /* Check for wrap around 0 */
    حرف* mm = (حرف*)(CALL_براہ_راست_MMAP(mmsize));
    اگر (mm != CMFAIL) {
      size_t offset = align_انحراف(chunk2mem(mm));
      size_t psize = mmsize - offset - MMAP_FOOT_PAD;
      mchunkptr p = (mchunkptr)(mm + offset);
      p->پچھلا_foot = offset;
      p->سر = psize;
      mark_inuse_foot(m, p, psize);
      chunk_plus_انحراف(p, psize)->سر = FENCEPOST_سر;
      chunk_plus_انحراف(p, psize+سائز_T_سائز)->سر = 0;

      اگر (m->کم_سے_کم_addr == 0 || mm < m->کم_سے_کم_addr)
        m->کم_سے_کم_addr = mm;
      اگر ((m->footprint += mmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      أكد(is_aligned(chunk2mem(p)));
      check_mmapped_chunk(m, p);
      واپس chunk2mem(p);
    }
  }
  واپس 0;
}

/* Realloc using mmap */
ساکن mchunkptr mmap_resize(mstate m, mchunkptr oldp, size_t nb, صحیح flags) {
  size_t oldsize = chunksize(oldp);
  (خالی)flags; /* placate people compiling -Wunused */
  اگر (is_small(nb)) /* Can't shrink mmap regions below small size */
    واپس 0;
  /* Keep old chunk if big enough but not too big */
  اگر (oldsize >= nb + سائز_T_سائز &&
      (oldsize - nb) <= (mparams.granularity << 1))
    واپس oldp;
  ورنہ {
    size_t offset = oldp->پچھلا_foot;
    size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
    size_t newmmsize = mmap_align(nb + SIX_سائز_T_SIZES + CHUNK_ALIGN_نقاب);
    حرف* cp = (حرف*)CALL_MREMAP((حرف*)oldp - offset,
                                  oldmmsize, newmmsize, flags);
    اگر (cp != CMFAIL) {
      mchunkptr newp = (mchunkptr)(cp + offset);
      size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
      newp->سر = psize;
      mark_inuse_foot(m, newp, psize);
      chunk_plus_انحراف(newp, psize)->سر = FENCEPOST_سر;
      chunk_plus_انحراف(newp, psize+سائز_T_سائز)->سر = 0;

      اگر (cp < m->کم_سے_کم_addr)
        m->کم_سے_کم_addr = cp;
      اگر ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
        m->max_footprint = m->footprint;
      check_mmapped_chunk(m, newp);
      واپس newp;
    }
  }
  واپس 0;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
ساکن خالی init_چوٹی(mstate m, mchunkptr p, size_t psize) {
  /* Ensure alignment */
  size_t offset = align_انحراف(chunk2mem(p));
  p = (mchunkptr)((حرف*)p + offset);
  psize -= offset;

  m->چوٹی = p;
  m->topsize = psize;
  p->سر = psize | PINUSE_ثنائی_ہندسہ;
  /* set size of fake trailing chunk holding overhead space only once */
  chunk_plus_انحراف(p, psize)->سر = چوٹی_FOOT_سائز;
  m->trim_check = mparams.trim_حد; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
ساکن خالی init_bins(mstate m) {
  /* Establish circular links for smallbins */
  bindex_t i;
  برائے (i = 0; i < NSMALLBINS; ++i) {
    sbinptr bin = smallbin_at(m,i);
    bin->fd = bin->bk = bin;
  }
}

#اگر PROCEED_ON_ERROR

/* default corruption action */
ساکن خالی reset_on_error(mstate m) {
  صحیح i;
  ++malloc_corruption_error_گنتی;
  /* Reinitialize fields to forget about all memory */
  m->smallmap = m->treemap = 0;
  m->dvsize = m->topsize = 0;
  m->seg.بنیاد = 0;
  m->seg.سائز = 0;
  m->seg.اگلا = 0;
  m->چوٹی = m->dv = 0;
  برائے (i = 0; i < NTREEBINS; ++i)
    *treebin_at(m, i) = 0;
  init_bins(m);
}
#اگر_ختم /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
ساکن خالی* prepend_alloc(mstate m, حرف* newbase, حرف* oldbase,
                           size_t nb) {
  mchunkptr p = align_as_chunk(newbase);
  mchunkptr oldfirst = align_as_chunk(oldbase);
  size_t psize = (حرف*)oldfirst - (حرف*)p;
  mchunkptr q = chunk_plus_انحراف(p, nb);
  size_t qsize = psize - nb;
  سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, p, nb);

  أكد((حرف*)oldfirst > (حرف*)q);
  أكد(pinuse(oldfirst));
  أكد(qsize >= MIN_CHUNK_سائز);

  /* consolidate remainder with first chunk of old base */
  اگر (oldfirst == m->چوٹی) {
    size_t tsize = m->topsize += qsize;
    m->چوٹی = q;
    q->سر = tsize | PINUSE_ثنائی_ہندسہ;
    check_چوٹی_chunk(m, q);
  }
  ورنہ اگر (oldfirst == m->dv) {
    size_t dsize = m->dvsize += qsize;
    m->dv = q;
    سیٹ_سائز_and_pinuse_کا_free_chunk(q, dsize);
  }
  ورنہ {
    اگر (!is_inuse(oldfirst)) {
      size_t nsize = chunksize(oldfirst);
      unlink_chunk(m, oldfirst, nsize);
      oldfirst = chunk_plus_انحراف(oldfirst, nsize);
      qsize += nsize;
    }
    سیٹ_free_with_pinuse(q, qsize, oldfirst);
    داخل_کرو_chunk(m, q, qsize);
    check_free_chunk(m, q);
  }

  check_malloced_chunk(m, chunk2mem(p), nb);
  واپس chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
ساکن خالی جوڑو_segment(mstate m, حرف* tbase, size_t tsize, flag_t mmapped) {
  /* Determine locations and sizes of segment, fenceposts, old top */
  حرف* old_چوٹی = (حرف*)m->چوٹی;
  msegmentptr oldsp = segment_holding(m, old_چوٹی);
  حرف* old_انتہا = oldsp->بنیاد + oldsp->سائز;
  size_t ssize = pad_request(حجم(ساخت malloc_segment));
  حرف* rawsp = old_انتہا - (ssize + FOUR_سائز_T_SIZES + CHUNK_ALIGN_نقاب);
  size_t offset = align_انحراف(chunk2mem(rawsp));
  حرف* asp = rawsp + offset;
  حرف* csp = (asp < (old_چوٹی + MIN_CHUNK_سائز))? old_چوٹی : asp;
  mchunkptr sp = (mchunkptr)csp;
  msegmentptr ss = (msegmentptr)(chunk2mem(sp));
  mchunkptr tnext = chunk_plus_انحراف(sp, ssize);
  mchunkptr p = tnext;
  صحیح nfences = 0;
  (خالی)nfences; /* Suppress unused variable warning */

  /* reset top to new space */
  init_چوٹی(m, (mchunkptr)tbase, tsize - چوٹی_FOOT_سائز);

  /* Set up segment record */
  أكد(is_aligned(ss));
  سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, sp, ssize);
  *ss = m->seg; /* Push current record */
  m->seg.بنیاد = tbase;
  m->seg.سائز = tsize;
  (خالی)سیٹ_segment_flags(&m->seg, mmapped);
  m->seg.اگلا = ss;

  /* Insert trailing fenceposts */
  برائے (;;) {
    mchunkptr nextp = chunk_plus_انحراف(p, سائز_T_سائز);
    p->سر = FENCEPOST_سر;
    ++nfences;
    اگر ((حرف*)(&(nextp->سر)) < old_انتہا)
      p = nextp;
    ورنہ
      توڑو;
  }
  أكد(nfences >= 2);

  /* Insert the rest of old top into a bin as an ordinary free chunk */
  اگر (csp != old_چوٹی) {
    mchunkptr q = (mchunkptr)old_چوٹی;
    size_t psize = csp - old_چوٹی;
    mchunkptr tn = chunk_plus_انحراف(q, psize);
    سیٹ_free_with_pinuse(q, psize, tn);
    داخل_کرو_chunk(m, q, psize);
  }

  check_چوٹی_chunk(m, m->چوٹی);
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
ساکن خالی* sys_alloc(mstate m, size_t nb) {
  حرف* tbase = CMFAIL;
  size_t tsize = 0;
  flag_t mmap_flag = 0;
  size_t asize; /* allocation size */

  ensure_initialization();

  /* Directly map large chunks, but only if already initialized */
  اگر (use_mmap(m) && nb >= mparams.mmap_حد && m->topsize != 0) {
    خالی* mem = mmap_alloc(m, nb);
    اگر (mem != 0)
      واپس mem;
  }

  asize = granularity_align(nb + SYS_ALLOC_PADDING);
  اگر (asize <= nb)
    واپس 0; /* wraparound */
  اگر (m->footprint_حد_بندی != 0) {
    size_t fp = m->footprint + asize;
    اگر (fp <= m->footprint || fp > m->footprint_حد_بندی)
      واپس 0;
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
    حرف* br = CMFAIL;
    size_t ssize = asize; /* sbrk call size */
    msegmentptr ss = (m->چوٹی == 0)? 0 : segment_holding(m, (حرف*)m->چوٹی);
    ACQUIRE_MALLOC_GLOBAL_تالا();

    اگر (ss == 0) {  /* First time through or recovery */
      حرف* بنیاد = (حرف*)CALL_MORECORE(0);
      اگر (بنیاد != CMFAIL) {
        size_t fp;
        /* Adjust to end on a page boundary */
        اگر (!is_صفحہ_aligned(بنیاد))
          ssize += (صفحہ_align((size_t)بنیاد) - (size_t)بنیاد);
        fp = m->footprint + ssize; /* recheck limits */
        اگر (ssize > nb && ssize < HALF_MAX_سائز_T &&
            (m->footprint_حد_بندی == 0 ||
             (fp > m->footprint && fp <= m->footprint_حد_بندی)) &&
            (br = (حرف*)(CALL_MORECORE(ssize))) == بنیاد) {
          tbase = بنیاد;
          tsize = ssize;
        }
      }
    }
    ورنہ {
      /* Subtract out existing available top space from MORECORE request. */
      ssize = granularity_align(nb - m->topsize + SYS_ALLOC_PADDING);
      /* Use mem here only if it did continuously extend old space */
      اگر (ssize < HALF_MAX_سائز_T &&
          (br = (حرف*)(CALL_MORECORE(ssize))) == ss->بنیاد+ss->سائز) {
        tbase = br;
        tsize = ssize;
      }
    }

    اگر (tbase == CMFAIL) {    /* Cope with partial failure */
      اگر (br != CMFAIL) {    /* Try to use/extend the space we did get */
        اگر (ssize < HALF_MAX_سائز_T &&
            ssize < nb + SYS_ALLOC_PADDING) {
          size_t esize = granularity_align(nb + SYS_ALLOC_PADDING - ssize);
          اگر (esize < HALF_MAX_سائز_T) {
            حرف* انتہا = (حرف*)CALL_MORECORE(esize);
            اگر (انتہا != CMFAIL)
              ssize += esize;
            ورنہ {            /* Can't use; try to release */
              (خالی) CALL_MORECORE(-ssize);
              br = CMFAIL;
            }
          }
        }
      }
      اگر (br != CMFAIL) {    /* Use the space we did get */
        tbase = br;
        tsize = ssize;
      }
      ورنہ
        disable_contiguous(m); /* Don't try contiguous path in the future */
    }

    RELEASE_MALLOC_GLOBAL_تالا();
  }

  اگر (HAVE_MMAP && tbase == CMFAIL) {  /* Try MMAP */
    حرف* mp = (حرف*)(CALL_MMAP(asize));
    اگر (mp != CMFAIL) {
      tbase = mp;
      tsize = asize;
      mmap_flag = USE_MMAP_ثنائی_ہندسہ;
    }
  }

  اگر (HAVE_MORECORE && tbase == CMFAIL) { /* Try noncontiguous MORECORE */
    اگر (asize < HALF_MAX_سائز_T) {
      حرف* br = CMFAIL;
      حرف* انتہا = CMFAIL;
      ACQUIRE_MALLOC_GLOBAL_تالا();
      br = (حرف*)(CALL_MORECORE(asize));
      انتہا = (حرف*)(CALL_MORECORE(0));
      RELEASE_MALLOC_GLOBAL_تالا();
      اگر (br != CMFAIL && انتہا != CMFAIL && br < انتہا) {
        size_t ssize = انتہا - br;
        اگر (ssize > nb + چوٹی_FOOT_سائز) {
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
      اگر (m->کم_سے_کم_addr == 0 || tbase < m->کم_سے_کم_addr)
        m->کم_سے_کم_addr = tbase;
      m->seg.بنیاد = tbase;
      m->seg.سائز = tsize;
      (خالی)سیٹ_segment_flags(&m->seg, mmap_flag);
      m->magic = mparams.magic;
      m->release_checks = MAX_RELEASE_CHECK_شرح;
      init_bins(m);
#اگر !ONLY_MSPACES
      اگر (is_global(m))
        init_چوٹی(m, (mchunkptr)tbase, tsize - چوٹی_FOOT_سائز);
      ورنہ
#اگر_ختم
      {
        /* Offset top by embedded malloc_state */
        mchunkptr mn = اگلا_chunk(mem2chunk(m));
        init_چوٹی(m, mn, (size_t)((tbase + tsize) - (حرف*)mn) -چوٹی_FOOT_سائز);
      }
    }

    ورنہ {
      /* Try to merge with an existing segment */
      msegmentptr sp = &m->seg;
      /* Only consider most recent segment if traversal suppressed */
      جب (sp != 0 && tbase != sp->بنیاد + sp->سائز)
        sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->اگلا;
      اگر (sp != 0 &&
          !is_extern_segment(sp) &&
          check_segment_ملاؤ(sp, tbase, tsize) &&
          (get_segment_flags(sp) & USE_MMAP_ثنائی_ہندسہ) == mmap_flag &&
          segment_holds(sp, m->چوٹی)) { /* append */
        sp->سائز += tsize;
        init_چوٹی(m, m->چوٹی, m->topsize + tsize);
      }
      ورنہ {
        اگر (tbase < m->کم_سے_کم_addr)
          m->کم_سے_کم_addr = tbase;
        sp = &m->seg;
        جب (sp != 0 && sp->بنیاد != tbase + tsize)
          sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->اگلا;
        اگر (sp != 0 &&
            !is_extern_segment(sp) &&
            check_segment_ملاؤ(sp, tbase, tsize) &&
            (get_segment_flags(sp) & USE_MMAP_ثنائی_ہندسہ) == mmap_flag) {
          حرف* oldbase = sp->بنیاد;
          sp->بنیاد = tbase;
          sp->سائز += tsize;
          واپس prepend_alloc(m, tbase, oldbase, nb);
        }
        ورنہ
          جوڑو_segment(m, tbase, tsize, mmap_flag);
      }
    }

    اگر (nb < m->topsize) { /* Allocate from new or extended top space */
      size_t rsize = m->topsize -= nb;
      mchunkptr p = m->چوٹی;
      mchunkptr r = m->چوٹی = chunk_plus_انحراف(p, nb);
      r->سر = rsize | PINUSE_ثنائی_ہندسہ;
      سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, p, nb);
      check_چوٹی_chunk(m, m->چوٹی);
      check_malloced_chunk(m, chunk2mem(p), nb);
      واپس chunk2mem(p);
    }
  }

  MALLOC_FAILURE_ACTION;
  واپس 0;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
ساکن size_t release_غیر_مستعمل_segments(mstate m) {
  size_t released = 0;
  صحیح nsegs = 0;
  msegmentptr pred = &m->seg;
  msegmentptr sp = pred->اگلا;
  جب (sp != 0) {
    حرف* بنیاد = sp->بنیاد;
    size_t سائز = sp->سائز;
    msegmentptr اگلا = sp->اگلا;
    ++nsegs;
    اگر (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
      mchunkptr p = align_as_chunk(بنیاد);
      size_t psize = chunksize(p);
      /* Can unmap if first chunk holds entire segment and not pinned */
      اگر (!is_inuse(p) && (حرف*)p + psize >= بنیاد + سائز - چوٹی_FOOT_سائز) {
        tchunkptr tp = (tchunkptr)p;
        أكد(segment_holds(sp, (حرف*)sp));
        اگر (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        ورنہ {
          unlink_large_chunk(m, tp);
        }
        اگر (CALL_MUNMAP(بنیاد, سائز) == 0) {
          released += سائز;
          m->footprint -= سائز;
          /* unlink obsoleted record */
          sp = pred;
          sp->اگلا = اگلا;
        }
        ورنہ { /* back out if cannot unmap */
          داخل_کرو_large_chunk(m, tp, psize);
        }
      }
    }
    اگر (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
      توڑو;
    pred = sp;
    sp = اگلا;
  }
  /* Reset check counter */
  m->release_checks = (((size_t) nsegs > (size_t) MAX_RELEASE_CHECK_شرح)?
                       (size_t) nsegs : (size_t) MAX_RELEASE_CHECK_شرح);
  واپس released;
}

ساکن صحیح sys_trim(mstate m, size_t pad) {
  size_t released = 0;
  ensure_initialization();
  اگر (pad < MAX_REQUEST && is_initialized(m)) {
    pad += چوٹی_FOOT_سائز; /* ensure enough room for segment overhead */

    اگر (m->topsize > pad) {
      /* Shrink top space in granularity-size units, keeping at least one */
      size_t unit = mparams.granularity;
      size_t extra = ((m->topsize - pad + (unit - سائز_T_ONE)) / unit -
                      سائز_T_ONE) * unit;
      msegmentptr sp = segment_holding(m, (حرف*)m->چوٹی);

      اگر (sp == 0)
        CORRUPTION_ERROR_ACTION(m);
      اگر (!is_extern_segment(sp)) {
        اگر (is_mmapped_segment(sp)) {
          اگر (HAVE_MMAP &&
              sp->سائز >= extra &&
              !has_segment_link(m, sp)) { /* can't shrink if pinned */
            size_t newsize = sp->سائز - extra;
            (خالی)newsize; /* placate people compiling -Wunused-variable */
            /* Prefer mremap, fall back to munmap */
            اگر ((CALL_MREMAP(sp->بنیاد, sp->سائز, newsize, 0) != MFAIL) ||
                (CALL_MUNMAP(sp->بنیاد + newsize, extra) == 0)) {
              released = extra;
            }
          }
        }
        ورنہ اگر (HAVE_MORECORE) {
          اگر (extra >= HALF_MAX_سائز_T) /* Avoid wrapping negative */
            extra = (HALF_MAX_سائز_T) + سائز_T_ONE - unit;
          ACQUIRE_MALLOC_GLOBAL_تالا();
          {
            /* Make sure end of memory is where we last set it. */
            حرف* old_br = (حرف*)(CALL_MORECORE(0));
            اگر (old_br == sp->بنیاد + sp->سائز) {
              حرف* rel_br = (حرف*)(CALL_MORECORE(-extra));
              حرف* new_br = (حرف*)(CALL_MORECORE(0));
              اگر (rel_br != CMFAIL && new_br < old_br)
                released = old_br - new_br;
            }
          }
          RELEASE_MALLOC_GLOBAL_تالا();
        }
      }

      اگر (released != 0) {
        sp->سائز -= released;
        m->footprint -= released;
        init_چوٹی(m, m->چوٹی, m->topsize - released);
        check_چوٹی_chunk(m, m->چوٹی);
      }
    }

    /* Unmap any unused mmapped segments */
    اگر (HAVE_MMAP)
      released += release_غیر_مستعمل_segments(m);

    /* On failure, disable autotrim to avoid repeated failed future calls */
    اگر (released == 0 && m->topsize > m->trim_check)
      m->trim_check = MAX_سائز_T;
  }

  واپس (released != 0)? 1 : 0;
}

/* Consolidate and bin a chunk. Differs from exported versions
   of free mainly in that the chunk need not be marked as inuse.
*/
ساکن خالی dispose_chunk(mstate m, mchunkptr p, size_t psize) {
  mchunkptr اگلا = chunk_plus_انحراف(p, psize);
  اگر (!pinuse(p)) {
    mchunkptr پچھلا;
    size_t prevsize = p->پچھلا_foot;
    اگر (is_mmapped(p)) {
      psize += prevsize + MMAP_FOOT_PAD;
      اگر (CALL_MUNMAP((حرف*)p - prevsize, psize) == 0)
        m->footprint -= psize;
      واپس;
    }
    پچھلا = chunk_minus_انحراف(p, prevsize);
    psize += prevsize;
    p = پچھلا;
    اگر (RTCHECK(ok_پتہ(m, پچھلا))) { /* consolidate backward */
      اگر (p != m->dv) {
        unlink_chunk(m, p, prevsize);
      }
      ورنہ اگر ((اگلا->سر & INUSE_بٹ) == INUSE_بٹ) {
        m->dvsize = psize;
        سیٹ_free_with_pinuse(p, psize, اگلا);
        واپس;
      }
    }
    ورنہ {
      CORRUPTION_ERROR_ACTION(m);
      واپس;
    }
  }
  اگر (RTCHECK(ok_پتہ(m, اگلا))) {
    اگر (!cinuse(اگلا)) {  /* consolidate forward */
      اگر (اگلا == m->چوٹی) {
        size_t tsize = m->topsize += psize;
        m->چوٹی = p;
        p->سر = tsize | PINUSE_ثنائی_ہندسہ;
        اگر (p == m->dv) {
          m->dv = 0;
          m->dvsize = 0;
        }
        واپس;
      }
      ورنہ اگر (اگلا == m->dv) {
        size_t dsize = m->dvsize += psize;
        m->dv = p;
        سیٹ_سائز_and_pinuse_کا_free_chunk(p, dsize);
        واپس;
      }
      ورنہ {
        size_t nsize = chunksize(اگلا);
        psize += nsize;
        unlink_chunk(m, اگلا, nsize);
        سیٹ_سائز_and_pinuse_کا_free_chunk(p, psize);
        اگر (p == m->dv) {
          m->dvsize = psize;
          واپس;
        }
      }
    }
    ورنہ {
      سیٹ_free_with_pinuse(p, psize, اگلا);
    }
    داخل_کرو_chunk(m, p, psize);
  }
  ورنہ {
    CORRUPTION_ERROR_ACTION(m);
  }
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
ساکن خالی* tmalloc_large(mstate m, size_t nb) {
  tchunkptr v = 0;
  size_t rsize = -nb; /* Unsigned negation */
  tchunkptr t;
  bindex_t idx;
  compute_درخت_اشاریہ(nb, idx);
  اگر ((t = *treebin_at(m, idx)) != 0) {
    /* Traverse tree for this bin looking for node with size == nb */
    size_t sizebits = nb << leftshift_for_درخت_اشاریہ(idx);
    tchunkptr rst = 0;  /* The deepest untaken right subtree */
    برائے (;;) {
      tchunkptr rt;
      size_t trem = chunksize(t) - nb;
      اگر (trem < rsize) {
        v = t;
        اگر ((rsize = trem) == 0)
          توڑو;
      }
      rt = t->بچہ[1];
      t = t->بچہ[(sizebits >> (سائز_T_BITSIZE-سائز_T_ONE)) & 1];
      اگر (rt != 0 && rt != t)
        rst = rt;
      اگر (t == 0) {
        t = rst; /* set t to least subtree holding sizes > nb */
        توڑو;
      }
      sizebits <<= 1;
    }
  }
  اگر (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
    binmap_t leftbits = بایاں_بٹ(idx2bit(idx)) & m->treemap;
    اگر (leftbits != 0) {
      bindex_t i;
      binmap_t leastbit = کم_سے_کم_ثنائی_ہندسہ(leftbits);
      compute_bit2idx(leastbit, i);
      t = *treebin_at(m, i);
    }
  }

  جب (t != 0) { /* find smallest of tree or subtree */
    size_t trem = chunksize(t) - nb;
    اگر (trem < rsize) {
      rsize = trem;
      v = t;
    }
    t = leftmost_بچہ(t);
  }

  /*  If dv is a better fit, return 0 so malloc will use it */
  اگر (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
    اگر (RTCHECK(ok_پتہ(m, v))) { /*بانٹو*/
      mchunkptr r = chunk_plus_انحراف(v, nb);
      أكد(chunksize(v) == rsize + nb);
      اگر (RTCHECK(ok_اگلا(v, r))) {
        unlink_large_chunk(m, v);
        اگر (rsize < MIN_CHUNK_سائز)
          سیٹ_inuse_and_pinuse(m, v, (rsize + nb));
        ورنہ {
          سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, v, nb);
          سیٹ_سائز_and_pinuse_کا_free_chunk(r, rsize);
          داخل_کرو_chunk(m, r, rsize);
        }
        واپس chunk2mem(v);
      }
    }
    CORRUPTION_ERROR_ACTION(m);
  }
  واپس 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
ساکن خالی* tmalloc_small(mstate m, size_t nb) {
  tchunkptr t, v;
  size_t rsize;
  bindex_t i;
  binmap_t leastbit = کم_سے_کم_ثنائی_ہندسہ(m->treemap);
  compute_bit2idx(leastbit, i);
  v = t = *treebin_at(m, i);
  rsize = chunksize(t) - nb;

  جب ((t = leftmost_بچہ(t)) != 0) {
    size_t trem = chunksize(t) - nb;
    اگر (trem < rsize) {
      rsize = trem;
      v = t;
    }
  }

  اگر (RTCHECK(ok_پتہ(m, v))) {
    mchunkptr r = chunk_plus_انحراف(v, nb);
    أكد(chunksize(v) == rsize + nb);
    اگر (RTCHECK(ok_اگلا(v, r))) {
      unlink_large_chunk(m, v);
      اگر (rsize < MIN_CHUNK_سائز)
        سیٹ_inuse_and_pinuse(m, v, (rsize + nb));
      ورنہ {
        سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, v, nb);
        سیٹ_سائز_and_pinuse_کا_free_chunk(r, rsize);
        replace_dv(m, r, rsize);
      }
      واپس chunk2mem(v);
    }
  }

  CORRUPTION_ERROR_ACTION(m);
  واپس 0;
}

#اگر !ONLY_MSPACES

خالی* dlmalloc(size_t بائٹیں) {
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
#اگر_ختم

  اگر (!PREACTION(gm)) {
    خالی* mem;
    size_t nb;
    اگر (بائٹیں <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (بائٹیں < MIN_REQUEST)? MIN_CHUNK_سائز : pad_request(بائٹیں);
      idx = small_اشاریہ(nb);
      smallbits = gm->smallmap >> idx;

      اگر ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(gm, idx);
        p = b->fd;
        أكد(chunksize(p) == small_index2size(idx));
        unlink_پہلا_small_chunk(gm, b, p, idx);
        سیٹ_inuse_and_pinuse(gm, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(gm, mem, nb);
        جاؤ postaction;
      }

      ورنہ اگر (nb > gm->dvsize) {
        اگر (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & بایاں_بٹ(idx2bit(idx));
          binmap_t leastbit = کم_سے_کم_ثنائی_ہندسہ(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(gm, i);
          p = b->fd;
          أكد(chunksize(p) == small_index2size(i));
          unlink_پہلا_small_chunk(gm, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          اگر (سائز_T_سائز != 4 && rsize < MIN_CHUNK_سائز)
            سیٹ_inuse_and_pinuse(gm, p, small_index2size(i));
          ورنہ {
            سیٹ_سائز_and_pinuse_کا_inuse_chunk(gm, p, nb);
            r = chunk_plus_انحراف(p, nb);
            سیٹ_سائز_and_pinuse_کا_free_chunk(r, rsize);
            replace_dv(gm, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(gm, mem, nb);
          جاؤ postaction;
        }

        ورنہ اگر (gm->treemap != 0 && (mem = tmalloc_small(gm, nb)) != 0) {
          check_malloced_chunk(gm, mem, nb);
          جاؤ postaction;
        }
      }
    }
    ورنہ اگر (بائٹیں >= MAX_REQUEST)
      nb = MAX_سائز_T; /* Too big to allocate. Force failure (in sys alloc) */
    ورنہ {
      nb = pad_request(بائٹیں);
      اگر (gm->treemap != 0 && (mem = tmalloc_large(gm, nb)) != 0) {
        check_malloced_chunk(gm, mem, nb);
        جاؤ postaction;
      }
    }

    اگر (nb <= gm->dvsize) {
      size_t rsize = gm->dvsize - nb;
      mchunkptr p = gm->dv;
      اگر (rsize >= MIN_CHUNK_سائز) { /* split dv */
        mchunkptr r = gm->dv = chunk_plus_انحراف(p, nb);
        gm->dvsize = rsize;
        سیٹ_سائز_and_pinuse_کا_free_chunk(r, rsize);
        سیٹ_سائز_and_pinuse_کا_inuse_chunk(gm, p, nb);
      }
      ورنہ { /* exhaust dv */
        size_t dvs = gm->dvsize;
        gm->dvsize = 0;
        gm->dv = 0;
        سیٹ_inuse_and_pinuse(gm, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(gm, mem, nb);
      جاؤ postaction;
    }

    ورنہ اگر (nb < gm->topsize) { /* Split top */
      size_t rsize = gm->topsize -= nb;
      mchunkptr p = gm->چوٹی;
      mchunkptr r = gm->چوٹی = chunk_plus_انحراف(p, nb);
      r->سر = rsize | PINUSE_ثنائی_ہندسہ;
      سیٹ_سائز_and_pinuse_کا_inuse_chunk(gm, p, nb);
      mem = chunk2mem(p);
      check_چوٹی_chunk(gm, gm->چوٹی);
      check_malloced_chunk(gm, mem, nb);
      جاؤ postaction;
    }

    mem = sys_alloc(gm, nb);

  postaction:
    POSTACTION(gm);
    واپس mem;
  }

  واپس 0;
}

/* ---------------------------- free --------------------------- */

خالی dlfree(خالی* mem) {
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
      واپس;
    }
#ورنہ /* FOOTERS */
#تعریف_کرو fm gm
#اگر_ختم /* FOOTERS */
    اگر (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      اگر (RTCHECK(ok_پتہ(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr اگلا = chunk_plus_انحراف(p, psize);
        اگر (!pinuse(p)) {
          size_t prevsize = p->پچھلا_foot;
          اگر (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            اگر (CALL_MUNMAP((حرف*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            جاؤ postaction;
          }
          ورنہ {
            mchunkptr پچھلا = chunk_minus_انحراف(p, prevsize);
            psize += prevsize;
            p = پچھلا;
            اگر (RTCHECK(ok_پتہ(fm, پچھلا))) { /* consolidate backward */
              اگر (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              ورنہ اگر ((اگلا->سر & INUSE_بٹ) == INUSE_بٹ) {
                fm->dvsize = psize;
                سیٹ_free_with_pinuse(p, psize, اگلا);
                جاؤ postaction;
              }
            }
            ورنہ
              جاؤ erroraction;
          }
        }

        اگر (RTCHECK(ok_اگلا(p, اگلا) && ok_pinuse(اگلا))) {
          اگر (!cinuse(اگلا)) {  /* consolidate forward */
            اگر (اگلا == fm->چوٹی) {
              size_t tsize = fm->topsize += psize;
              fm->چوٹی = p;
              p->سر = tsize | PINUSE_ثنائی_ہندسہ;
              اگر (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              اگر (should_trim(fm, tsize))
                sys_trim(fm, 0);
              جاؤ postaction;
            }
            ورنہ اگر (اگلا == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              سیٹ_سائز_and_pinuse_کا_free_chunk(p, dsize);
              جاؤ postaction;
            }
            ورنہ {
              size_t nsize = chunksize(اگلا);
              psize += nsize;
              unlink_chunk(fm, اگلا, nsize);
              سیٹ_سائز_and_pinuse_کا_free_chunk(p, psize);
              اگر (p == fm->dv) {
                fm->dvsize = psize;
                جاؤ postaction;
              }
            }
          }
          ورنہ
            سیٹ_free_with_pinuse(p, psize, اگلا);

          اگر (is_small(psize)) {
            داخل_کرو_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          ورنہ {
            tchunkptr tp = (tchunkptr)p;
            داخل_کرو_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            اگر (--fm->release_checks == 0)
              release_غیر_مستعمل_segments(fm);
          }
          جاؤ postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
#اگر !FOOTERS
#تعریف_منسوخ fm
#اگر_ختم /* FOOTERS */
}

خالی* dlcalloc(size_t n_elements, size_t elem_سائز) {
  خالی* mem;
  size_t req = 0;
  اگر (n_elements != 0) {
    req = n_elements * elem_سائز;
    اگر (((n_elements | elem_سائز) & ~(size_t)0xffff) &&
        (req / n_elements != elem_سائز))
      req = MAX_سائز_T; /* force downstream failure on overflow */
  }
  mem = dlmalloc(req);
  اگر (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  واپس mem;
}

#اگر_ختم /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
ساکن mchunkptr try_realloc_chunk(mstate m, mchunkptr p, size_t nb,
                                   صحیح can_move) {
  mchunkptr newp = 0;
  size_t oldsize = chunksize(p);
  mchunkptr اگلا = chunk_plus_انحراف(p, oldsize);
  اگر (RTCHECK(ok_پتہ(m, p) && ok_inuse(p) &&
              ok_اگلا(p, اگلا) && ok_pinuse(اگلا))) {
    اگر (is_mmapped(p)) {
      newp = mmap_resize(m, p, nb, can_move);
    }
    ورنہ اگر (oldsize >= nb) {             /* already big enough */
      size_t rsize = oldsize - nb;
      اگر (rsize >= MIN_CHUNK_سائز) {      /* split off remainder */
        mchunkptr r = chunk_plus_انحراف(p, nb);
        سیٹ_inuse(m, p, nb);
        سیٹ_inuse(m, r, rsize);
        dispose_chunk(m, r, rsize);
      }
      newp = p;
    }
    ورنہ اگر (اگلا == m->چوٹی) {  /* extend into top */
      اگر (oldsize + m->topsize > nb) {
        size_t newsize = oldsize + m->topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_انحراف(p, nb);
        سیٹ_inuse(m, p, nb);
        newtop->سر = newtopsize |PINUSE_ثنائی_ہندسہ;
        m->چوٹی = newtop;
        m->topsize = newtopsize;
        newp = p;
      }
    }
    ورنہ اگر (اگلا == m->dv) { /* extend into dv */
      size_t dvs = m->dvsize;
      اگر (oldsize + dvs >= nb) {
        size_t dsize = oldsize + dvs - nb;
        اگر (dsize >= MIN_CHUNK_سائز) {
          mchunkptr r = chunk_plus_انحراف(p, nb);
          mchunkptr n = chunk_plus_انحراف(r, dsize);
          سیٹ_inuse(m, p, nb);
          سیٹ_سائز_and_pinuse_کا_free_chunk(r, dsize);
          clear_pinuse(n);
          m->dvsize = dsize;
          m->dv = r;
        }
        ورنہ { /* exhaust dv */
          size_t newsize = oldsize + dvs;
          سیٹ_inuse(m, p, newsize);
          m->dvsize = 0;
          m->dv = 0;
        }
        newp = p;
      }
    }
    ورنہ اگر (!cinuse(اگلا)) { /* extend into next free chunk */
      size_t nextsize = chunksize(اگلا);
      اگر (oldsize + nextsize >= nb) {
        size_t rsize = oldsize + nextsize - nb;
        unlink_chunk(m, اگلا, nextsize);
        اگر (rsize < MIN_CHUNK_سائز) {
          size_t newsize = oldsize + nextsize;
          سیٹ_inuse(m, p, newsize);
        }
        ورنہ {
          mchunkptr r = chunk_plus_انحراف(p, nb);
          سیٹ_inuse(m, p, nb);
          سیٹ_inuse(m, r, rsize);
          dispose_chunk(m, r, rsize);
        }
        newp = p;
      }
    }
  }
  ورنہ {
    USAGE_ERROR_ACTION(m, chunk2mem(p));
  }
  واپس newp;
}

ساکن خالی* اندرونی_memalign(mstate m, size_t alignment, size_t بائٹیں) {
  خالی* mem = 0;
  اگر (alignment <  MIN_CHUNK_سائز) /* must be at least a minimum chunk size */
    alignment = MIN_CHUNK_سائز;
  اگر ((alignment & (alignment-سائز_T_ONE)) != 0) {/* Ensure a power of 2 */
    size_t a = MALLOC_ALIGNMENT << 1;
    جب (a < alignment) a <<= 1;
    alignment = a;
  }
  اگر (بائٹیں >= MAX_REQUEST - alignment) {
    اگر (m != 0)  { /* Test isn't needed but avoids compiler warning */
      MALLOC_FAILURE_ACTION;
    }
  }
  ورنہ {
    size_t nb = request2size(بائٹیں);
    size_t req = nb + alignment + MIN_CHUNK_سائز - CHUNK_OVERHEAD;
    mem = اندرونی_malloc(m, req);
    اگر (mem != 0) {
      mchunkptr p = mem2chunk(mem);
      اگر (PREACTION(m))
        واپس 0;
      اگر ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
        حرف* br = (حرف*)mem2chunk((size_t)(((size_t)((حرف*)mem + alignment -
                                                       سائز_T_ONE)) &
                                             -alignment));
        حرف* pos = ((size_t)(br - (حرف*)(p)) >= MIN_CHUNK_سائز)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (حرف*)(p);
        size_t newsize = chunksize(p) - leadsize;

        اگر (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
          newp->پچھلا_foot = p->پچھلا_foot + leadsize;
          newp->سر = newsize;
        }
        ورنہ { /* Otherwise, give back leader, use the rest */
          سیٹ_inuse(m, newp, newsize);
          سیٹ_inuse(m, p, leadsize);
          dispose_chunk(m, p, leadsize);
        }
        p = newp;
      }

      /* Give back spare room at the end */
      اگر (!is_mmapped(p)) {
        size_t سائز = chunksize(p);
        اگر (سائز > nb + MIN_CHUNK_سائز) {
          size_t remainder_سائز = سائز - nb;
          mchunkptr remainder = chunk_plus_انحراف(p, nb);
          سیٹ_inuse(m, p, nb);
          سیٹ_inuse(m, remainder, remainder_سائز);
          dispose_chunk(m, remainder, remainder_سائز);
        }
      }

      mem = chunk2mem(p);
      أكد (chunksize(p) >= nb);
      أكد(((size_t)mem & (alignment - 1)) == 0);
      check_inuse_chunk(m, p);
      POSTACTION(m);
    }
  }
  واپس mem;
}

/*
  Common support for independent_X routines, handling
    all of the combinations that can result.
  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/
ساکن خالی** ialloc(mstate m,
                     size_t n_elements,
                     size_t* sizes,
                     صحیح opts,
                     خالی* chunks[]) {

  size_t    element_سائز;   /* chunksize of each element, if all same */
  size_t    contents_سائز;  /* total size of elements */
  size_t    array_سائز;     /* request size of pointer array */
  خالی*     mem;            /* malloced aggregate space */
  mchunkptr p;              /* corresponding chunk */
  size_t    remainder_سائز; /* remaining bytes while splitting */
  خالی**    marray;         /* either "chunks" or malloced ptr array */
  mchunkptr array_chunk;    /* chunk for malloced ptr array */
  flag_t    was_enabled;    /* to disable mmap */
  size_t    سائز;
  size_t    i;

  ensure_initialization();
  /* compute array length, if needed */
  اگر (chunks != 0) {
    اگر (n_elements == 0)
      واپس chunks; /* nothing to do */
    marray = chunks;
    array_سائز = 0;
  }
  ورنہ {
    /* if empty req, must still return chunk representing empty array */
    اگر (n_elements == 0)
      واپس (خالی**)اندرونی_malloc(m, 0);
    marray = 0;
    array_سائز = request2size(n_elements * (حجم(خالی*)));
  }

  /* compute total element size */
  اگر (opts & 0x1) { /* all-same-size */
    element_سائز = request2size(*sizes);
    contents_سائز = n_elements * element_سائز;
  }
  ورنہ { /* add up all the sizes */
    element_سائز = 0;
    contents_سائز = 0;
    برائے (i = 0; i != n_elements; ++i)
      contents_سائز += request2size(sizes[i]);
  }

  سائز = contents_سائز + array_سائز;

  /*
     Allocate the aggregate chunk.  First disable direct-mmapping so
     malloc won't use it, since we would not be able to later
     free/realloc space internal to a segregated mmap region.
  */
  was_enabled = use_mmap(m);
  disable_mmap(m);
  mem = اندرونی_malloc(m, سائز - CHUNK_OVERHEAD);
  اگر (was_enabled)
    enable_mmap(m);
  اگر (mem == 0)
    واپس 0;

  اگر (PREACTION(m)) واپس 0;
  p = mem2chunk(mem);
  remainder_سائز = chunksize(p);

  أكد(!is_mmapped(p));

  اگر (opts & 0x2) {       /* optionally clear the elements */
    memset((size_t*)mem, 0, remainder_سائز - سائز_T_سائز - array_سائز);
  }

  /* If not provided, allocate the pointer array as final part of chunk */
  اگر (marray == 0) {
    size_t  array_chunk_سائز;
    array_chunk = chunk_plus_انحراف(p, contents_سائز);
    array_chunk_سائز = remainder_سائز - contents_سائز;
    marray = (خالی**) (chunk2mem(array_chunk));
    سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, array_chunk, array_chunk_سائز);
    remainder_سائز = contents_سائز;
  }

  /* split out elements */
  برائے (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    اگر (i != n_elements-1) {
      اگر (element_سائز != 0)
        سائز = element_سائز;
      ورنہ
        سائز = request2size(sizes[i]);
      remainder_سائز -= سائز;
      سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, p, سائز);
      p = chunk_plus_انحراف(p, سائز);
    }
    ورنہ { /* the final element absorbs any overallocation slop */
      سیٹ_سائز_and_pinuse_کا_inuse_chunk(m, p, remainder_سائز);
      توڑو;
    }
  }

#اگر DEBUG
  اگر (marray != chunks) {
    /* final element must have exactly exhausted chunk */
    اگر (element_سائز != 0) {
      أكد(remainder_سائز == element_سائز);
    }
    ورنہ {
      أكد(remainder_سائز == request2size(sizes[i]));
    }
    check_inuse_chunk(m, mem2chunk(marray));
  }
  برائے (i = 0; i != n_elements; ++i)
    check_inuse_chunk(m, mem2chunk(marray[i]));

#اگر_ختم /* DEBUG */

  POSTACTION(m);
  واپس marray;
}

/* Try to free all pointers in the given array.
   Note: this could be made faster, by delaying consolidation,
   at the price of disabling some user integrity checks, We
   still optimize some consolidations by combining adjacent
   chunks before freeing, which will occur often if allocated
   with ialloc or the array is sorted.
*/
ساکن size_t اندرونی_bulk_free(mstate m, خالی* array[], size_t nelem) {
  size_t unfreed = 0;
  اگر (!PREACTION(m)) {
    خالی** a;
    خالی** fence = &(array[nelem]);
    برائے (a = array; a != fence; ++a) {
      خالی* mem = *a;
      اگر (mem != 0) {
        mchunkptr p = mem2chunk(mem);
        size_t psize = chunksize(p);
#اگر FOOTERS
        اگر (get_mstate_for(p) != m) {
          ++unfreed;
          جاری;
        }
#اگر_ختم
        check_inuse_chunk(m, p);
        *a = 0;
        اگر (RTCHECK(ok_پتہ(m, p) && ok_inuse(p))) {
          خالی ** b = a + 1; /* try to merge with next chunk */
          mchunkptr اگلا = اگلا_chunk(p);
          اگر (b != fence && *b == chunk2mem(اگلا)) {
            size_t newsize = chunksize(اگلا) + psize;
            سیٹ_inuse(m, p, newsize);
            *b = chunk2mem(p);
          }
          ورنہ
            dispose_chunk(m, p, psize);
        }
        ورنہ {
          CORRUPTION_ERROR_ACTION(m);
          توڑو;
        }
      }
    }
    اگر (should_trim(m, m->topsize))
      sys_trim(m, 0);
    POSTACTION(m);
  }
  واپس unfreed;
}

/* Traversal */
#اگر MALLOC_INSPECT_ALL
ساکن خالی اندرونی_inspect_all(mstate m,
                                 خالی(*handler)(خالی *آغاز,
                                                خالی *انتہا,
                                                size_t مستعمل_بائٹیں,
                                                خالی* callback_arg),
                                 خالی* arg) {
  اگر (is_initialized(m)) {
    mchunkptr چوٹی = m->چوٹی;
    msegmentptr s;
    برائے (s = &m->seg; s != 0; s = s->اگلا) {
      mchunkptr q = align_as_chunk(s->بنیاد);
      جب (segment_holds(s, q) && q->سر != FENCEPOST_سر) {
        mchunkptr اگلا = اگلا_chunk(q);
        size_t sz = chunksize(q);
        size_t مستعمل;
        خالی* آغاز;
        اگر (is_inuse(q)) {
          مستعمل = sz - CHUNK_OVERHEAD; /* must not be mmapped */
          آغاز = chunk2mem(q);
        }
        ورنہ {
          مستعمل = 0;
          اگر (is_small(sz)) {     /* offset by possible bookkeeping */
            آغاز = (خالی*)((حرف*)q + حجم(ساخت malloc_chunk));
          }
          ورنہ {
            آغاز = (خالی*)((حرف*)q + حجم(ساخت malloc_درخت_chunk));
          }
        }
        اگر (آغاز < (خالی*)اگلا)  /* skip if all space is bookkeeping */
          handler(آغاز, اگلا, مستعمل, arg);
        اگر (q == چوٹی)
          توڑو;
        q = اگلا;
      }
    }
  }
}
#اگر_ختم /* MALLOC_INSPECT_ALL */

/* ------------------ Exported realloc, memalign, etc -------------------- */

#اگر !ONLY_MSPACES

خالی* dlrealloc(خالی* oldmem, size_t بائٹیں) {
  خالی* mem = 0;
  اگر (oldmem == 0) {
    mem = dlmalloc(بائٹیں);
  }
  ورنہ اگر (بائٹیں >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#اگر_تعریف_شدہ REALLOC_ZERO_بائٹیں_FREES
  ورنہ اگر (بائٹیں == 0) {
    dlfree(oldmem);
  }
#اگر_ختم /* REALLOC_ZERO_BYTES_FREES */
  ورنہ {
    size_t nb = request2size(بائٹیں);
    mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
    mstate m = gm;
#ورنہ /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    اگر (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      واپس 0;
    }
#اگر_ختم /* FOOTERS */
    اگر (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      اگر (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      ورنہ {
        mem = اندرونی_malloc(m, بائٹیں);
        اگر (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          حافظہ_نقل(mem, oldmem, (oc < بائٹیں)? oc : بائٹیں);
          اندرونی_free(m, oldmem);
        }
      }
    }
  }
  واپس mem;
}

خالی* dlrealloc_اندر_رکھو(خالی* oldmem, size_t بائٹیں) {
  خالی* mem = 0;
  اگر (oldmem != 0) {
    اگر (بائٹیں >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    ورنہ {
      size_t nb = request2size(بائٹیں);
      mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
      mstate m = gm;
#ورنہ /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      اگر (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        واپس 0;
      }
#اگر_ختم /* FOOTERS */
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
  واپس mem;
}

خالی* dlmemalign(size_t alignment, size_t بائٹیں) {
  اگر (alignment <= MALLOC_ALIGNMENT) {
    واپس dlmalloc(بائٹیں);
  }
  واپس اندرونی_memalign(gm, alignment, بائٹیں);
}

صحیح dlposix_memalign(خالی** pp, size_t alignment, size_t بائٹیں) {
  خالی* mem = 0;
  اگر (alignment == MALLOC_ALIGNMENT)
    mem = dlmalloc(بائٹیں);
  ورنہ {
    size_t d = alignment / حجم(خالی*);
    size_t r = alignment % حجم(خالی*);
    اگر (r != 0 || d == 0 || (d & (d-سائز_T_ONE)) != 0)
      واپس EINVAL;
    ورنہ اگر (بائٹیں <= MAX_REQUEST - alignment) {
      اگر (alignment <  MIN_CHUNK_سائز)
        alignment = MIN_CHUNK_سائز;
      mem = اندرونی_memalign(gm, alignment, بائٹیں);
    }
  }
  اگر (mem == 0)
    واپس ENOMEM;
  ورنہ {
    *pp = mem;
    واپس 0;
  }
}

خالی* dlvalloc(size_t بائٹیں) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.صفحہ_سائز;
  واپس dlmemalign(pagesz, بائٹیں);
}

خالی* dlpvalloc(size_t بائٹیں) {
  size_t pagesz;
  ensure_initialization();
  pagesz = mparams.صفحہ_سائز;
  واپس dlmemalign(pagesz, (بائٹیں + pagesz - سائز_T_ONE) & ~(pagesz - سائز_T_ONE));
}

خالی** dlindependent_calloc(size_t n_elements, size_t elem_سائز,
                            خالی* chunks[]) {
  size_t sz = elem_سائز; /* serves as 1-element array */
  واپس ialloc(gm, n_elements, &sz, 3, chunks);
}

خالی** dlindependent_comalloc(size_t n_elements, size_t sizes[],
                              خالی* chunks[]) {
  واپس ialloc(gm, n_elements, sizes, 0, chunks);
}

size_t dlbulk_free(خالی* array[], size_t nelem) {
  واپس اندرونی_bulk_free(gm, array, nelem);
}

#اگر MALLOC_INSPECT_ALL
خالی dlmalloc_inspect_all(خالی(*handler)(خالی *آغاز,
                                         خالی *انتہا,
                                         size_t مستعمل_بائٹیں,
                                         خالی* callback_arg),
                          خالی* arg) {
  ensure_initialization();
  اگر (!PREACTION(gm)) {
    اندرونی_inspect_all(gm, handler, arg);
    POSTACTION(gm);
  }
}
#اگر_ختم /* MALLOC_INSPECT_ALL */

صحیح dlmalloc_trim(size_t pad) {
  صحیح نتیجہ = 0;
  ensure_initialization();
  اگر (!PREACTION(gm)) {
    نتیجہ = sys_trim(gm, pad);
    POSTACTION(gm);
  }
  واپس نتیجہ;
}

size_t dlmalloc_footprint(خالی) {
  واپس gm->footprint;
}

size_t dlmalloc_max_footprint(خالی) {
  واپس gm->max_footprint;
}

size_t dlmalloc_footprint_حد_بندی(خالی) {
  size_t maf = gm->footprint_حد_بندی;
  واپس maf == 0 ? MAX_سائز_T : maf;
}

size_t dlmalloc_سیٹ_footprint_حد_بندی(size_t بائٹیں) {
  size_t نتیجہ;  /* invert sense of 0 */
  اگر (بائٹیں == 0)
    نتیجہ = granularity_align(1); /* Use minimal size */
  اگر (بائٹیں == MAX_سائز_T)
    نتیجہ = 0;                    /* disable */
  ورنہ
    نتیجہ = granularity_align(بائٹیں);
  واپس gm->footprint_حد_بندی = نتیجہ;
}

#اگر !NO_MALLINFO
ساخت mallinfo dlmallinfo(خالی) {
  واپس اندرونی_mallinfo(gm);
}
#اگر_ختم /* NO_MALLINFO */

#اگر !NO_MALLOC_STATS
خالی dlmalloc_stats(خالی) {
  اندرونی_malloc_stats(gm);
}
#اگر_ختم /* NO_MALLOC_STATS */

صحیح dlmallopt(صحیح param_نمبر, صحیح قدر) {
  واپس change_mparam(param_نمبر, قدر);
}

size_t dlmalloc_usable_سائز(خالی* mem) {
  اگر (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    اگر (is_inuse(p))
      واپس chunksize(p) - overhead_for(p);
  }
  واپس 0;
}

#اگر_ختم /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#اگر MSPACES

ساکن mstate init_user_mstate(حرف* tbase, size_t tsize) {
  size_t msize = pad_request(حجم(ساخت malloc_state));
  mchunkptr mn;
  mchunkptr msp = align_as_chunk(tbase);
  mstate m = (mstate)(chunk2mem(msp));
  memset(m, 0, msize);
  (خالی)INITIAL_تالا(&m->mutex);
  msp->سر = (msize|INUSE_بٹ);
  m->seg.بنیاد = m->کم_سے_کم_addr = tbase;
  m->seg.سائز = m->footprint = m->max_footprint = tsize;
  m->magic = mparams.magic;
  m->release_checks = MAX_RELEASE_CHECK_شرح;
  m->mflags = mparams.default_mflags;
  m->extp = 0;
  m->exts = 0;
  disable_contiguous(m);
  init_bins(m);
  mn = اگلا_chunk(mem2chunk(m));
  init_چوٹی(m, mn, (size_t)((tbase + tsize) - (حرف*)mn) - چوٹی_FOOT_سائز);
  check_چوٹی_chunk(m, m->چوٹی);
  واپس m;
}

mspace create_mspace(size_t استعداد, صحیح locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(حجم(ساخت malloc_state));
  اگر (استعداد < (size_t) -(msize + چوٹی_FOOT_سائز + mparams.صفحہ_سائز)) {
    size_t rs = ((استعداد == 0)? mparams.granularity :
                 (استعداد + چوٹی_FOOT_سائز + msize));
    size_t tsize = granularity_align(rs);
    حرف* tbase = (حرف*)(CALL_MMAP(tsize));
    اگر (tbase != CMFAIL) {
      m = init_user_mstate(tbase, tsize);
      سیٹ_segment_flags(&m->seg, USE_MMAP_ثنائی_ہندسہ);
      سیٹ_تالا(m, locked);
    }
  }
  واپس (mspace)m;
}

mspace create_mspace_with_بنیاد(خالی* بنیاد, size_t استعداد, صحیح locked) {
  mstate m = 0;
  size_t msize;
  ensure_initialization();
  msize = pad_request(حجم(ساخت malloc_state));
  اگر (استعداد > msize + چوٹی_FOOT_سائز &&
      استعداد < (size_t) -(msize + چوٹی_FOOT_سائز + mparams.صفحہ_سائز)) {
    m = init_user_mstate((حرف*)بنیاد, استعداد);
    سیٹ_segment_flags(&m->seg, EXTERN_ثنائی_ہندسہ);
    سیٹ_تالا(m, locked);
  }
  واپس (mspace)m;
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
    } ورنہ {
      disable_mmap(ms);
    }
    POSTACTION(ms);
  }
  واپس ret;
}

size_t destroy_mspace(mspace msp) {
  size_t freed = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    msegmentptr sp = &ms->seg;
    (خالی)DESTROY_تالا(&ms->mutex); /* destroy before unmapped */
    جب (sp != 0) {
      حرف* بنیاد = sp->بنیاد;
      size_t سائز = sp->سائز;
      flag_t flag = get_segment_flags(sp);
      (خالی)بنیاد; /* placate people compiling -Wunused-variable */
      sp = sp->اگلا;
      اگر ((flag & USE_MMAP_ثنائی_ہندسہ) && !(flag & EXTERN_ثنائی_ہندسہ) &&
          CALL_MUNMAP(بنیاد, سائز) == 0)
        freed += سائز;
    }
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  واپس freed;
}

/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

خالی* mspace_malloc(mspace msp, size_t بائٹیں) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    واپس 0;
  }
  اگر (!PREACTION(ms)) {
    خالی* mem;
    size_t nb;
    اگر (بائٹیں <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (بائٹیں < MIN_REQUEST)? MIN_CHUNK_سائز : pad_request(بائٹیں);
      idx = small_اشاریہ(nb);
      smallbits = ms->smallmap >> idx;

      اگر ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(ms, idx);
        p = b->fd;
        أكد(chunksize(p) == small_index2size(idx));
        unlink_پہلا_small_chunk(ms, b, p, idx);
        سیٹ_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        جاؤ postaction;
      }

      ورنہ اگر (nb > ms->dvsize) {
        اگر (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & بایاں_بٹ(idx2bit(idx));
          binmap_t leastbit = کم_سے_کم_ثنائی_ہندسہ(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b->fd;
          أكد(chunksize(p) == small_index2size(i));
          unlink_پہلا_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          اگر (سائز_T_سائز != 4 && rsize < MIN_CHUNK_سائز)
            سیٹ_inuse_and_pinuse(ms, p, small_index2size(i));
          ورنہ {
            سیٹ_سائز_and_pinuse_کا_inuse_chunk(ms, p, nb);
            r = chunk_plus_انحراف(p, nb);
            سیٹ_سائز_and_pinuse_کا_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          جاؤ postaction;
        }

        ورنہ اگر (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
          check_malloced_chunk(ms, mem, nb);
          جاؤ postaction;
        }
      }
    }
    ورنہ اگر (بائٹیں >= MAX_REQUEST)
      nb = MAX_سائز_T; /* Too big to allocate. Force failure (in sys alloc) */
    ورنہ {
      nb = pad_request(بائٹیں);
      اگر (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
        check_malloced_chunk(ms, mem, nb);
        جاؤ postaction;
      }
    }

    اگر (nb <= ms->dvsize) {
      size_t rsize = ms->dvsize - nb;
      mchunkptr p = ms->dv;
      اگر (rsize >= MIN_CHUNK_سائز) { /* split dv */
        mchunkptr r = ms->dv = chunk_plus_انحراف(p, nb);
        ms->dvsize = rsize;
        سیٹ_سائز_and_pinuse_کا_free_chunk(r, rsize);
        سیٹ_سائز_and_pinuse_کا_inuse_chunk(ms, p, nb);
      }
      ورنہ { /* exhaust dv */
        size_t dvs = ms->dvsize;
        ms->dvsize = 0;
        ms->dv = 0;
        سیٹ_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      جاؤ postaction;
    }

    ورنہ اگر (nb < ms->topsize) { /* Split top */
      size_t rsize = ms->topsize -= nb;
      mchunkptr p = ms->چوٹی;
      mchunkptr r = ms->چوٹی = chunk_plus_انحراف(p, nb);
      r->سر = rsize | PINUSE_ثنائی_ہندسہ;
      سیٹ_سائز_and_pinuse_کا_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_چوٹی_chunk(ms, ms->چوٹی);
      check_malloced_chunk(ms, mem, nb);
      جاؤ postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
    POSTACTION(ms);
    واپس mem;
  }

  واپس 0;
}

خالی mspace_free(mspace msp, خالی* mem) {
  اگر (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#اگر FOOTERS
    mstate fm = get_mstate_for(p);
    (خالی)msp; /* placate people compiling -Wunused */
#ورنہ /* FOOTERS */
    mstate fm = (mstate)msp;
#اگر_ختم /* FOOTERS */
    اگر (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      واپس;
    }
    اگر (!PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      اگر (RTCHECK(ok_پتہ(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr اگلا = chunk_plus_انحراف(p, psize);
        اگر (!pinuse(p)) {
          size_t prevsize = p->پچھلا_foot;
          اگر (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            اگر (CALL_MUNMAP((حرف*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            جاؤ postaction;
          }
          ورنہ {
            mchunkptr پچھلا = chunk_minus_انحراف(p, prevsize);
            psize += prevsize;
            p = پچھلا;
            اگر (RTCHECK(ok_پتہ(fm, پچھلا))) { /* consolidate backward */
              اگر (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              ورنہ اگر ((اگلا->سر & INUSE_بٹ) == INUSE_بٹ) {
                fm->dvsize = psize;
                سیٹ_free_with_pinuse(p, psize, اگلا);
                جاؤ postaction;
              }
            }
            ورنہ
              جاؤ erroraction;
          }
        }

        اگر (RTCHECK(ok_اگلا(p, اگلا) && ok_pinuse(اگلا))) {
          اگر (!cinuse(اگلا)) {  /* consolidate forward */
            اگر (اگلا == fm->چوٹی) {
              size_t tsize = fm->topsize += psize;
              fm->چوٹی = p;
              p->سر = tsize | PINUSE_ثنائی_ہندسہ;
              اگر (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              اگر (should_trim(fm, tsize))
                sys_trim(fm, 0);
              جاؤ postaction;
            }
            ورنہ اگر (اگلا == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              سیٹ_سائز_and_pinuse_کا_free_chunk(p, dsize);
              جاؤ postaction;
            }
            ورنہ {
              size_t nsize = chunksize(اگلا);
              psize += nsize;
              unlink_chunk(fm, اگلا, nsize);
              سیٹ_سائز_and_pinuse_کا_free_chunk(p, psize);
              اگر (p == fm->dv) {
                fm->dvsize = psize;
                جاؤ postaction;
              }
            }
          }
          ورنہ
            سیٹ_free_with_pinuse(p, psize, اگلا);

          اگر (is_small(psize)) {
            داخل_کرو_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          ورنہ {
            tchunkptr tp = (tchunkptr)p;
            داخل_کرو_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            اگر (--fm->release_checks == 0)
              release_غیر_مستعمل_segments(fm);
          }
          جاؤ postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      POSTACTION(fm);
    }
  }
}

خالی* mspace_calloc(mspace msp, size_t n_elements, size_t elem_سائز) {
  خالی* mem;
  size_t req = 0;
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    واپس 0;
  }
  اگر (n_elements != 0) {
    req = n_elements * elem_سائز;
    اگر (((n_elements | elem_سائز) & ~(size_t)0xffff) &&
        (req / n_elements != elem_سائز))
      req = MAX_سائز_T; /* force downstream failure on overflow */
  }
  mem = اندرونی_malloc(ms, req);
  اگر (mem != 0 && calloc_must_clear(mem2chunk(mem)))
    memset(mem, 0, req);
  واپس mem;
}

خالی* mspace_realloc(mspace msp, خالی* oldmem, size_t بائٹیں) {
  خالی* mem = 0;
  اگر (oldmem == 0) {
    mem = mspace_malloc(msp, بائٹیں);
  }
  ورنہ اگر (بائٹیں >= MAX_REQUEST) {
    MALLOC_FAILURE_ACTION;
  }
#اگر_تعریف_شدہ REALLOC_ZERO_بائٹیں_FREES
  ورنہ اگر (بائٹیں == 0) {
    mspace_free(msp, oldmem);
  }
#اگر_ختم /* REALLOC_ZERO_BYTES_FREES */
  ورنہ {
    size_t nb = request2size(بائٹیں);
    mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
    mstate m = (mstate)msp;
#ورنہ /* FOOTERS */
    mstate m = get_mstate_for(oldp);
    اگر (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m, oldmem);
      واپس 0;
    }
#اگر_ختم /* FOOTERS */
    اگر (!PREACTION(m)) {
      mchunkptr newp = try_realloc_chunk(m, oldp, nb, 1);
      POSTACTION(m);
      اگر (newp != 0) {
        check_inuse_chunk(m, newp);
        mem = chunk2mem(newp);
      }
      ورنہ {
        mem = mspace_malloc(m, بائٹیں);
        اگر (mem != 0) {
          size_t oc = chunksize(oldp) - overhead_for(oldp);
          حافظہ_نقل(mem, oldmem, (oc < بائٹیں)? oc : بائٹیں);
          mspace_free(m, oldmem);
        }
      }
    }
  }
  واپس mem;
}

خالی* mspace_realloc_اندر_رکھو(mspace msp, خالی* oldmem, size_t بائٹیں) {
  خالی* mem = 0;
  اگر (oldmem != 0) {
    اگر (بائٹیں >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
    }
    ورنہ {
      size_t nb = request2size(بائٹیں);
      mchunkptr oldp = mem2chunk(oldmem);
#اگر ! FOOTERS
      mstate m = (mstate)msp;
#ورنہ /* FOOTERS */
      mstate m = get_mstate_for(oldp);
      (خالی)msp; /* placate people compiling -Wunused */
      اگر (!ok_magic(m)) {
        USAGE_ERROR_ACTION(m, oldmem);
        واپس 0;
      }
#اگر_ختم /* FOOTERS */
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
  واپس mem;
}

خالی* mspace_memalign(mspace msp, size_t alignment, size_t بائٹیں) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    واپس 0;
  }
  اگر (alignment <= MALLOC_ALIGNMENT)
    واپس mspace_malloc(msp, بائٹیں);
  واپس اندرونی_memalign(ms, alignment, بائٹیں);
}

خالی** mspace_independent_calloc(mspace msp, size_t n_elements,
                                 size_t elem_سائز, خالی* chunks[]) {
  size_t sz = elem_سائز; /* serves as 1-element array */
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    واپس 0;
  }
  واپس ialloc(ms, n_elements, &sz, 3, chunks);
}

خالی** mspace_independent_comalloc(mspace msp, size_t n_elements,
                                   size_t sizes[], خالی* chunks[]) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    واپس 0;
  }
  واپس ialloc(ms, n_elements, sizes, 0, chunks);
}

size_t mspace_bulk_free(mspace msp, خالی* array[], size_t nelem) {
  واپس اندرونی_bulk_free((mstate)msp, array, nelem);
}

#اگر MALLOC_INSPECT_ALL
خالی mspace_inspect_all(mspace msp,
                        خالی(*handler)(خالی *آغاز,
                                       خالی *انتہا,
                                       size_t مستعمل_بائٹیں,
                                       خالی* callback_arg),
                        خالی* arg) {
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    اگر (!PREACTION(ms)) {
      اندرونی_inspect_all(ms, handler, arg);
      POSTACTION(ms);
    }
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#اگر_ختم /* MALLOC_INSPECT_ALL */

صحیح mspace_trim(mspace msp, size_t pad) {
  صحیح نتیجہ = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    اگر (!PREACTION(ms)) {
      نتیجہ = sys_trim(ms, pad);
      POSTACTION(ms);
    }
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  واپس نتیجہ;
}

#اگر !NO_MALLOC_STATS
خالی mspace_malloc_stats(mspace msp) {
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    اندرونی_malloc_stats(ms);
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
}
#اگر_ختم /* NO_MALLOC_STATS */

size_t mspace_footprint(mspace msp) {
  size_t نتیجہ = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    نتیجہ = ms->footprint;
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  واپس نتیجہ;
}

size_t mspace_max_footprint(mspace msp) {
  size_t نتیجہ = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    نتیجہ = ms->max_footprint;
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  واپس نتیجہ;
}

size_t mspace_footprint_حد_بندی(mspace msp) {
  size_t نتیجہ = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    size_t maf = ms->footprint_حد_بندی;
    نتیجہ = (maf == 0) ? MAX_سائز_T : maf;
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  واپس نتیجہ;
}

size_t mspace_سیٹ_footprint_حد_بندی(mspace msp, size_t بائٹیں) {
  size_t نتیجہ = 0;
  mstate ms = (mstate)msp;
  اگر (ok_magic(ms)) {
    اگر (بائٹیں == 0)
      نتیجہ = granularity_align(1); /* Use minimal size */
    اگر (بائٹیں == MAX_سائز_T)
      نتیجہ = 0;                    /* disable */
    ورنہ
      نتیجہ = granularity_align(بائٹیں);
    ms->footprint_حد_بندی = نتیجہ;
  }
  ورنہ {
    USAGE_ERROR_ACTION(ms,ms);
  }
  واپس نتیجہ;
}

#اگر !NO_MALLINFO
ساخت mallinfo mspace_mallinfo(mspace msp) {
  mstate ms = (mstate)msp;
  اگر (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
  }
  واپس اندرونی_mallinfo(ms);
}
#اگر_ختم /* NO_MALLINFO */

size_t mspace_usable_سائز(ثابت خالی* mem) {
  اگر (mem != 0) {
    mchunkptr p = mem2chunk(mem);
    اگر (is_inuse(p))
      واپس chunksize(p) - overhead_for(p);
  }
  واپس 0;
}

صحیح mspace_mallopt(صحیح param_نمبر, صحیح قدر) {
  واپس change_mparam(param_نمبر, قدر);
}

#اگر_ختم /* MSPACES */


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
