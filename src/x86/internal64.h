#నిర్వచించు UNIX64_RET_VOID		0
#నిర్వచించు UNIX64_RET_UINT8	1
#నిర్వచించు UNIX64_RET_UINT16	2
#నిర్వచించు UNIX64_RET_UINT32	3
#నిర్వచించు UNIX64_RET_SINT8	4
#నిర్వచించు UNIX64_RET_SINT16	5
#నిర్వచించు UNIX64_RET_SINT32	6
#నిర్వచించు UNIX64_RET_INT64	7
#నిర్వచించు UNIX64_RET_XMM32	8
#నిర్వచించు UNIX64_RET_XMM64	9
#నిర్వచించు UNIX64_RET_X87		10
#నిర్వచించు UNIX64_RET_X87_2	11
#నిర్వచించు UNIX64_RET_ST_XMM0_RAX	12
#నిర్వచించు UNIX64_RET_ST_RAX_XMM0	13
#నిర్వచించు UNIX64_RET_ST_XMM0_XMM1	14
#నిర్వచించు UNIX64_RET_ST_RAX_RDX	15
#నిర్వచించు UNIX64_RET_XMM128	16	/* binary128 long double in %xmm0 */

#నిర్వచించు UNIX64_RET_చివరి		16

#నిర్వచించు UNIX64_FLAG_RET_లోపల_MEM	(1 << 10)
#నిర్వచించు UNIX64_FLAG_XMM_వాదనలు	(1 << 11)
#నిర్వచించు UNIX64_కొలత_జరుపు	12

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 4K (base page size)
 * is chosen.
 */
#నిర్వచించు UNIX64_TRAMP_MAP_జరుపు	12
#నిర్వచించు UNIX64_TRAMP_MAP_కొలత	(1 << UNIX64_TRAMP_MAP_జరుపు)
#నిర్వచితమైతే ENDBR_PRESENT
#నిర్వచించు UNIX64_TRAMP_కొలత	40
#లేకపోతే
#నిర్వచించు UNIX64_TRAMP_కొలత	32
#అయితే_ముగింపు
#అయితే_ముగింపు
