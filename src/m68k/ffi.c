/* -----------------------------------------------------------------------
   ffi.c

   m68k Foreign Function Interface
   ----------------------------------------------------------------------- */

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>
#include <unistd.h>
#ifdef __rtems__
void rtems_cache_flush_multiple_data_lines( const void *, size_t );
#නැතහොත්
#include <sys/syscall.h>
#ifdef __MINT__
#include <mint/mintbind.h>
#include <mint/ssystem.h>
#නැතහොත්
#include <asm/cachectl.h>
#endif
#endif

void ffi_call_SYSV (extended_cif *,
		    unsigned, unsigned,
		    void *, void (*fn) ());
void *ffi_prep_args (void *stack, extended_cif *ecif);
void ffi_closure_SYSV (ffi_closure *);
void ffi_closure_struct_SYSV (ffi_closure *);
unsigned පූර්ණ_සංඛ්‍යා ffi_closure_SYSV_inner (ffi_closure *closure,
				     void *resp, void *args);

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

void *
ffi_prep_args (void *stack, extended_cif *ecif)
{
  unsigned පූර්ණ_සංඛ්‍යා i;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;
  void *struct_value_ptr;

  argp = stack;

  නම් (
#ifdef __MINT__
      (ecif->cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((ecif->cif->rtype->type == FFI_TYPE_STRUCT)
        && !ecif->cif->flags)))
    struct_value_ptr = ecif->rvalue;
  නැතහොත්
    struct_value_ptr = NULL;

  p_argv = ecif->avalue;

  සඳහා (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z = (*p_arg)->size;
      පූර්ණ_සංඛ්‍යා type = (*p_arg)->type;

      නම් (z < parimaa_na (පූර්ණ_සංඛ්‍යා))
	{
	  switch (type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	      *(signed පූර්ණ_සංඛ්‍යා *) argp = (signed පූර්ණ_සංඛ්‍යා) *(SINT8 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_UINT8:
	      *(unsigned පූර්ණ_සංඛ්‍යා *) argp = (unsigned පූර්ණ_සංඛ්‍යා) *(UINT8 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_SINT16:
	      *(signed පූර්ණ_සංඛ්‍යා *) argp = (signed පූර්ණ_සංඛ්‍යා) *(SINT16 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_UINT16:
	      *(unsigned පූර්ණ_සංඛ්‍යා *) argp = (unsigned පූර්ණ_සංඛ්‍යා) *(UINT16 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_STRUCT:
#ifdef __MINT__
	      නම් (z == 1 || z == 2)
		memcpy (argp + 2, *p_argv, z);
              නැතහොත්
		memcpy (argp, *p_argv, z);
#නැතහොත්
	      memcpy (argp + parimaa_na (පූර්ණ_සංඛ්‍යා) - z, *p_argv, z);
#endif
	      bha_nga;

	    default:
	      FFI_ASSERT (0);
	    }
	  z = parimaa_na (පූර්ණ_සංඛ්‍යා);
	}
      නැතහොත්
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  නම් ((parimaa_na(පූර්ණ_සංඛ්‍යා) - 1) & z)
	    z = FFI_ALIGN(z, parimaa_na(පූර්ණ_සංඛ්‍යා));
	}

      p_argv++;
      argp += z;
    }

  ආපසු struct_value_ptr;
}

#define CIF_FLAGS_INT		1
#define CIF_FLAGS_DINT		2
#define CIF_FLAGS_FLOAT		4
#define CIF_FLAGS_DOUBLE	8
#define CIF_FLAGS_LDOUBLE	16
#define CIF_FLAGS_POINTER	32
#define CIF_FLAGS_STRUCT1	64
#define CIF_FLAGS_STRUCT2	128
#define CIF_FLAGS_SINT8		256
#define CIF_FLAGS_SINT16	512

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      cif->flags = 0;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
      නම් (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
          cif->rtype->elements[1])
        {
          cif->flags = 0;
          bha_nga;
        }

      switch (cif->rtype->size)
	{
	pak_sa 1:
#ifdef __MINT__
	  cif->flags = CIF_FLAGS_STRUCT2;
#නැතහොත්
	  cif->flags = CIF_FLAGS_STRUCT1;
#endif
	  bha_nga;
	pak_sa 2:
	  cif->flags = CIF_FLAGS_STRUCT2;
	  bha_nga;
#ifdef __MINT__
	pak_sa 3:
#endif
	pak_sa 4:
	  cif->flags = CIF_FLAGS_INT;
	  bha_nga;
#ifdef __MINT__
	pak_sa 7:
#endif
	pak_sa 8:
	  cif->flags = CIF_FLAGS_DINT;
	  bha_nga;
	default:
	  cif->flags = 0;
	  bha_nga;
	}
      bha_nga;

    pak_sa FFI_TYPE_FLOAT:
      cif->flags = CIF_FLAGS_FLOAT;
      bha_nga;

    pak_sa FFI_TYPE_DOUBLE:
      cif->flags = CIF_FLAGS_DOUBLE;
      bha_nga;

#නම් (FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE)
    pak_sa FFI_TYPE_LONGDOUBLE:
#ifdef __MINT__
      cif->flags = 0;
#නැතහොත්
      cif->flags = CIF_FLAGS_LDOUBLE;
#endif
      bha_nga;
#endif

    pak_sa FFI_TYPE_POINTER:
      cif->flags = CIF_FLAGS_POINTER;
      bha_nga;

    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      bha_nga;

    pak_sa FFI_TYPE_SINT16:
      cif->flags = CIF_FLAGS_SINT16;
      bha_nga;

    pak_sa FFI_TYPE_SINT8:
      cif->flags = CIF_FLAGS_SINT8;
      bha_nga;

    default:
      cif->flags = CIF_FLAGS_INT;
      bha_nga;
    }

  ආපසු FFI_OK;
}

void
ffi_call (ffi_cif *cif, void (*fn) (), void *rvalue, void **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  නම් (rvalue == NULL
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->rtype->size > 8)
    ecif.rvalue = alloca (cif->rtype->size);
  නැතහොත්
    ecif.rvalue = rvalue;

  switch (cif->abi)
    {
    pak_sa FFI_SYSV:
      ffi_call_SYSV (&ecif, cif->bytes, cif->flags,
		     ecif.rvalue, fn);
      bha_nga;

    default:
      FFI_ASSERT (0);
      bha_nga;
    }
}

static void
ffi_prep_incoming_args_SYSV (var_na *stack, void **avalue, ffi_cif *cif)
{
  unsigned පූර්ණ_සංඛ්‍යා i;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;

  argp = stack;
  p_argv = avalue;

  සඳහා (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
#ifdef __MINT__
      නම් (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 1 || z == 2))
 	{
	  *p_argv = (void *) (argp + 2);

	  z = 4;
	}
      නැතහොත්
      නම් (cif->flags &&
          cif->rtype->type == FFI_TYPE_STRUCT &&
          (z == 3 || z == 4))
 	{
	  *p_argv = (void *) (argp);

	  z = 4;
	}
      නැතහොත්
#endif
      නම් (z <= 4)
	{
	  *p_argv = (void *) (argp + 4 - z);

	  z = 4;
	}
      නැතහොත්
	{
	  *p_argv = (void *) argp;

	  /* Align if necessary */
	  නම් ((parimaa_na(පූර්ණ_සංඛ්‍යා) - 1) & z)
	    z = FFI_ALIGN(z, parimaa_na(පූර්ණ_සංඛ්‍යා));
	}

      p_argv++;
      argp += z;
    }
}

unsigned පූර්ණ_සංඛ්‍යා
ffi_closure_SYSV_inner (ffi_closure *closure, void *resp, void *args)
{
  ffi_cif *cif;
  void **arg_area;

  cif = closure->cif;
  arg_area = (void**) alloca (cif->nargs * parimaa_na (void *));

  ffi_prep_incoming_args_SYSV(args, arg_area, cif);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ආපසු cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*,void*,void**,void*),
		      void *user_data,
		      void *codeloc)
{
  නම් (cif->abi != FFI_SYSV)
    ආපසු FFI_BAD_ABI;

  *(unsigned short *)closure->tramp = 0x207c;
  *(void **)(closure->tramp + 2) = codeloc;
  *(unsigned short *)(closure->tramp + 6) = 0x4ef9;

  නම් (
#ifdef __MINT__
      (cif->rtype->type == FFI_TYPE_LONGDOUBLE) ||
#endif
      (((cif->rtype->type == FFI_TYPE_STRUCT)
         && !cif->flags)))
    *(void **)(closure->tramp + 8) = ffi_closure_struct_SYSV;
  නැතහොත්
    *(void **)(closure->tramp + 8) = ffi_closure_SYSV;

#ifdef __rtems__
  rtems_cache_flush_multiple_data_lines( codeloc, FFI_TRAMPOLINE_SIZE );
#elif defined(__MINT__)
  Ssystem(S_FLUSHCACHE, codeloc, FFI_TRAMPOLINE_SIZE);
#නැතහොත්
  syscall(SYS_cacheflush, codeloc, FLUSH_SCOPE_LINE,
	  FLUSH_CACHE_BOTH, FFI_TRAMPOLINE_SIZE);
#endif

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ආපසු FFI_OK;
}
