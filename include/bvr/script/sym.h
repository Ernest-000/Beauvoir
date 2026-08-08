#pragma once

#include <bvr/config.h>
#include <bvr/common.h>
#include <bvr/bvr.h>

#include <libtcc.h>

#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifndef BVR_ADD_SYM
    #define BVR_ADD_SYM(state, key) if(key != NULL) { tcc_add_symbol(state, #key, key); }
#endif

/**
 * @brief load all the core header for the scripting to work
 * this function tries to add the following modules to a TCC state. 
 * 
 * - assert.h
 * 
 * - ctype.h
 * 
 * - math.f
 * 
 * - stdio.h
 * 
 * - string.h
 * 
 * - time.h
 * @param _s a pointer to the tinycc state.
 */
BVR_H_FUNC void bvr_load_default_sym(void* _s)
{


	// assert.h
	BVR_ADD_SYM(((TCCState*)_s), bvri_wmessage);
	BVR_ADD_SYM(((TCCState*)_s), bvri_wassert);

	// ctype.h
	BVR_ADD_SYM(((TCCState*)_s), isalnum);
	BVR_ADD_SYM(((TCCState*)_s), isalpha);
	BVR_ADD_SYM(((TCCState*)_s), isblank);
	BVR_ADD_SYM(((TCCState*)_s), iscntrl);
	BVR_ADD_SYM(((TCCState*)_s), isdigit);
	BVR_ADD_SYM(((TCCState*)_s), isgraph);
	BVR_ADD_SYM(((TCCState*)_s), islower);
	BVR_ADD_SYM(((TCCState*)_s), isprint);
	BVR_ADD_SYM(((TCCState*)_s), ispunct);
	BVR_ADD_SYM(((TCCState*)_s), isspace);
	BVR_ADD_SYM(((TCCState*)_s), isupper);
	BVR_ADD_SYM(((TCCState*)_s), isxdigit);
	BVR_ADD_SYM(((TCCState*)_s), tolower);
	BVR_ADD_SYM(((TCCState*)_s), toupper);

	// float.h

	// lbvr.h
	BVR_ADD_SYM(((TCCState*)_s), bvr_get_frame);
	BVR_ADD_SYM(((TCCState*)_s), bvr_get_delta_time);

	// limits.h

	// math.h
	BVR_ADD_SYM(((TCCState*)_s), acos);
	BVR_ADD_SYM(((TCCState*)_s), acosf);
	BVR_ADD_SYM(((TCCState*)_s), acosl);
	BVR_ADD_SYM(((TCCState*)_s), asin);
	BVR_ADD_SYM(((TCCState*)_s), asinf);
	BVR_ADD_SYM(((TCCState*)_s), asinl);
	BVR_ADD_SYM(((TCCState*)_s), atan);
	BVR_ADD_SYM(((TCCState*)_s), atanf);
	BVR_ADD_SYM(((TCCState*)_s), atanl);
	BVR_ADD_SYM(((TCCState*)_s), atan2);
	BVR_ADD_SYM(((TCCState*)_s), atan2f);
	BVR_ADD_SYM(((TCCState*)_s), atan2l);
	BVR_ADD_SYM(((TCCState*)_s), cos);
	BVR_ADD_SYM(((TCCState*)_s), cosf);
	BVR_ADD_SYM(((TCCState*)_s), cosl);
	BVR_ADD_SYM(((TCCState*)_s), sin);
	BVR_ADD_SYM(((TCCState*)_s), sinf);
	BVR_ADD_SYM(((TCCState*)_s), sinl);
	BVR_ADD_SYM(((TCCState*)_s), tan);
	BVR_ADD_SYM(((TCCState*)_s), tanf);
	BVR_ADD_SYM(((TCCState*)_s), tanl);
	BVR_ADD_SYM(((TCCState*)_s), acosh);
	BVR_ADD_SYM(((TCCState*)_s), acoshf);
	BVR_ADD_SYM(((TCCState*)_s), acoshl);
	BVR_ADD_SYM(((TCCState*)_s), asinh);
	BVR_ADD_SYM(((TCCState*)_s), asinhf);
	BVR_ADD_SYM(((TCCState*)_s), asinhl);
	BVR_ADD_SYM(((TCCState*)_s), atanh);
	BVR_ADD_SYM(((TCCState*)_s), atanhf);
	BVR_ADD_SYM(((TCCState*)_s), atanhl);
	BVR_ADD_SYM(((TCCState*)_s), cosh);
	BVR_ADD_SYM(((TCCState*)_s), coshf);
	BVR_ADD_SYM(((TCCState*)_s), coshl);
	BVR_ADD_SYM(((TCCState*)_s), sinh);
	BVR_ADD_SYM(((TCCState*)_s), sinhf);
	BVR_ADD_SYM(((TCCState*)_s), sinhl);
	BVR_ADD_SYM(((TCCState*)_s), tanh);
	BVR_ADD_SYM(((TCCState*)_s), tanhf);
	BVR_ADD_SYM(((TCCState*)_s), tanhl);
	BVR_ADD_SYM(((TCCState*)_s), exp);
	BVR_ADD_SYM(((TCCState*)_s), expf);
	BVR_ADD_SYM(((TCCState*)_s), expl);
	BVR_ADD_SYM(((TCCState*)_s), exp2);
	BVR_ADD_SYM(((TCCState*)_s), exp2f);
	BVR_ADD_SYM(((TCCState*)_s), exp2l);
	BVR_ADD_SYM(((TCCState*)_s), expm1);
	BVR_ADD_SYM(((TCCState*)_s), expm1f);
	BVR_ADD_SYM(((TCCState*)_s), expm1l);
	BVR_ADD_SYM(((TCCState*)_s), frexp);
	BVR_ADD_SYM(((TCCState*)_s), frexpf);
	BVR_ADD_SYM(((TCCState*)_s), frexpl);
	BVR_ADD_SYM(((TCCState*)_s), ilogb);
	BVR_ADD_SYM(((TCCState*)_s), ilogbf);
	BVR_ADD_SYM(((TCCState*)_s), ilogbl);
	BVR_ADD_SYM(((TCCState*)_s), ldexp);
	BVR_ADD_SYM(((TCCState*)_s), ldexpf);
	BVR_ADD_SYM(((TCCState*)_s), ldexpl);
	BVR_ADD_SYM(((TCCState*)_s), log);
	BVR_ADD_SYM(((TCCState*)_s), logf);
	BVR_ADD_SYM(((TCCState*)_s), logl);
	BVR_ADD_SYM(((TCCState*)_s), log10);
	BVR_ADD_SYM(((TCCState*)_s), log10f);
	BVR_ADD_SYM(((TCCState*)_s), log10l);
	BVR_ADD_SYM(((TCCState*)_s), log1p);
	BVR_ADD_SYM(((TCCState*)_s), log1pf);
	BVR_ADD_SYM(((TCCState*)_s), log1pl);
	BVR_ADD_SYM(((TCCState*)_s), log2);
	BVR_ADD_SYM(((TCCState*)_s), log2f);
	BVR_ADD_SYM(((TCCState*)_s), log2l);
	BVR_ADD_SYM(((TCCState*)_s), logb);
	BVR_ADD_SYM(((TCCState*)_s), logbf);
	BVR_ADD_SYM(((TCCState*)_s), logbl);
	BVR_ADD_SYM(((TCCState*)_s), modf);
	BVR_ADD_SYM(((TCCState*)_s), modff);
	BVR_ADD_SYM(((TCCState*)_s), modfl);
	BVR_ADD_SYM(((TCCState*)_s), scalbn);
	BVR_ADD_SYM(((TCCState*)_s), scalbnf);
	BVR_ADD_SYM(((TCCState*)_s), scalbnl);
	BVR_ADD_SYM(((TCCState*)_s), scalbln);
	BVR_ADD_SYM(((TCCState*)_s), scalblnf);
	BVR_ADD_SYM(((TCCState*)_s), scalblnl);
	BVR_ADD_SYM(((TCCState*)_s), cbrt);
	BVR_ADD_SYM(((TCCState*)_s), cbrtf);
	BVR_ADD_SYM(((TCCState*)_s), cbrtl);
	BVR_ADD_SYM(((TCCState*)_s), fabs);
	BVR_ADD_SYM(((TCCState*)_s), fabsf);
	BVR_ADD_SYM(((TCCState*)_s), fabsl);
	BVR_ADD_SYM(((TCCState*)_s), hypot);
	BVR_ADD_SYM(((TCCState*)_s), hypotf);
	BVR_ADD_SYM(((TCCState*)_s), hypotl);
	BVR_ADD_SYM(((TCCState*)_s), pow);
	BVR_ADD_SYM(((TCCState*)_s), powf);
	BVR_ADD_SYM(((TCCState*)_s), powl);
	BVR_ADD_SYM(((TCCState*)_s), sqrt);
	BVR_ADD_SYM(((TCCState*)_s), sqrtf);
	BVR_ADD_SYM(((TCCState*)_s), sqrtl);
	BVR_ADD_SYM(((TCCState*)_s), erf);
	BVR_ADD_SYM(((TCCState*)_s), erff);
	BVR_ADD_SYM(((TCCState*)_s), erfl);
	BVR_ADD_SYM(((TCCState*)_s), erfc);
	BVR_ADD_SYM(((TCCState*)_s), erfcf);
	BVR_ADD_SYM(((TCCState*)_s), erfcl);
	BVR_ADD_SYM(((TCCState*)_s), lgamma);
	BVR_ADD_SYM(((TCCState*)_s), lgammaf);
	BVR_ADD_SYM(((TCCState*)_s), lgammal);
	BVR_ADD_SYM(((TCCState*)_s), tgamma);
	BVR_ADD_SYM(((TCCState*)_s), tgammaf);
	BVR_ADD_SYM(((TCCState*)_s), tgammal);
	BVR_ADD_SYM(((TCCState*)_s), ceil);
	BVR_ADD_SYM(((TCCState*)_s), ceilf);
	BVR_ADD_SYM(((TCCState*)_s), ceill);
	BVR_ADD_SYM(((TCCState*)_s), floor);
	BVR_ADD_SYM(((TCCState*)_s), floorf);
	BVR_ADD_SYM(((TCCState*)_s), floorl);
	BVR_ADD_SYM(((TCCState*)_s), nearbyint);
	BVR_ADD_SYM(((TCCState*)_s), nearbyintf);
	BVR_ADD_SYM(((TCCState*)_s), nearbyintl);
	BVR_ADD_SYM(((TCCState*)_s), rint);
	BVR_ADD_SYM(((TCCState*)_s), rintf);
	BVR_ADD_SYM(((TCCState*)_s), rintl);
	BVR_ADD_SYM(((TCCState*)_s), lrint);
	BVR_ADD_SYM(((TCCState*)_s), lrintf);
	BVR_ADD_SYM(((TCCState*)_s), lrintl);
	BVR_ADD_SYM(((TCCState*)_s), llrint);
	BVR_ADD_SYM(((TCCState*)_s), llrintf);
	BVR_ADD_SYM(((TCCState*)_s), llrintl);
	BVR_ADD_SYM(((TCCState*)_s), round);
	BVR_ADD_SYM(((TCCState*)_s), roundf);
	BVR_ADD_SYM(((TCCState*)_s), roundl);
	BVR_ADD_SYM(((TCCState*)_s), lround);
	BVR_ADD_SYM(((TCCState*)_s), lroundf);
	BVR_ADD_SYM(((TCCState*)_s), lroundl);
	BVR_ADD_SYM(((TCCState*)_s), llround);
	BVR_ADD_SYM(((TCCState*)_s), llroundf);
	BVR_ADD_SYM(((TCCState*)_s), llroundl);
	BVR_ADD_SYM(((TCCState*)_s), trunc);
	BVR_ADD_SYM(((TCCState*)_s), truncf);
	BVR_ADD_SYM(((TCCState*)_s), truncl);
	BVR_ADD_SYM(((TCCState*)_s), fmod);
	BVR_ADD_SYM(((TCCState*)_s), fmodf);
	BVR_ADD_SYM(((TCCState*)_s), fmodl);
	BVR_ADD_SYM(((TCCState*)_s), remainder);
	BVR_ADD_SYM(((TCCState*)_s), remainderf);
	BVR_ADD_SYM(((TCCState*)_s), remainderl);
	BVR_ADD_SYM(((TCCState*)_s), remquo);
	BVR_ADD_SYM(((TCCState*)_s), remquof);
	BVR_ADD_SYM(((TCCState*)_s), remquol);
	BVR_ADD_SYM(((TCCState*)_s), copysign);
	BVR_ADD_SYM(((TCCState*)_s), copysignf);
	BVR_ADD_SYM(((TCCState*)_s), copysignl);
	BVR_ADD_SYM(((TCCState*)_s), nan);
	BVR_ADD_SYM(((TCCState*)_s), nanf);
	BVR_ADD_SYM(((TCCState*)_s), nanl);
	BVR_ADD_SYM(((TCCState*)_s), nextafter);
	BVR_ADD_SYM(((TCCState*)_s), nextafterf);
	BVR_ADD_SYM(((TCCState*)_s), nextafterl);
	BVR_ADD_SYM(((TCCState*)_s), nexttoward);
	BVR_ADD_SYM(((TCCState*)_s), nexttowardf);
	BVR_ADD_SYM(((TCCState*)_s), nexttowardl);
	BVR_ADD_SYM(((TCCState*)_s), fdim);
	BVR_ADD_SYM(((TCCState*)_s), fdimf);
	BVR_ADD_SYM(((TCCState*)_s), fdiml);
	BVR_ADD_SYM(((TCCState*)_s), fmax);
	BVR_ADD_SYM(((TCCState*)_s), fmaxf);
	BVR_ADD_SYM(((TCCState*)_s), fmaxl);
	BVR_ADD_SYM(((TCCState*)_s), fmin);
	BVR_ADD_SYM(((TCCState*)_s), fminf);
	BVR_ADD_SYM(((TCCState*)_s), fminl);
	BVR_ADD_SYM(((TCCState*)_s), fma);
	BVR_ADD_SYM(((TCCState*)_s), fmaf);
	BVR_ADD_SYM(((TCCState*)_s), fmal);

	// stdarg.h

	// stddef.h

	// stdint.h

	// stdio.h
	BVR_ADD_SYM(((TCCState*)_s), remove);
	BVR_ADD_SYM(((TCCState*)_s), rename);
	BVR_ADD_SYM(((TCCState*)_s), tmpfile);
	BVR_ADD_SYM(((TCCState*)_s), tmpnam);
	BVR_ADD_SYM(((TCCState*)_s), fclose);
	BVR_ADD_SYM(((TCCState*)_s), fflush);
	BVR_ADD_SYM(((TCCState*)_s), fopen);
	BVR_ADD_SYM(((TCCState*)_s), freopen);
	BVR_ADD_SYM(((TCCState*)_s), setbuf);
	BVR_ADD_SYM(((TCCState*)_s), setvbuf);
	BVR_ADD_SYM(((TCCState*)_s), fprintf);
	BVR_ADD_SYM(((TCCState*)_s), fscanf);
	BVR_ADD_SYM(((TCCState*)_s), printf);
	BVR_ADD_SYM(((TCCState*)_s), scanf);
	BVR_ADD_SYM(((TCCState*)_s), snprintf);
	BVR_ADD_SYM(((TCCState*)_s), sprintf);
	BVR_ADD_SYM(((TCCState*)_s), sscanf);
	BVR_ADD_SYM(((TCCState*)_s), vfprintf);
	BVR_ADD_SYM(((TCCState*)_s), vfscanf);
	BVR_ADD_SYM(((TCCState*)_s), vprintf);
	BVR_ADD_SYM(((TCCState*)_s), vscanf);
	BVR_ADD_SYM(((TCCState*)_s), vsnprintf);
	BVR_ADD_SYM(((TCCState*)_s), vsprintf);
	BVR_ADD_SYM(((TCCState*)_s), vsscanf);
	BVR_ADD_SYM(((TCCState*)_s), fgetc);
	BVR_ADD_SYM(((TCCState*)_s), fgets);
	BVR_ADD_SYM(((TCCState*)_s), fputc);
	BVR_ADD_SYM(((TCCState*)_s), fputs);
	BVR_ADD_SYM(((TCCState*)_s), getc);
	BVR_ADD_SYM(((TCCState*)_s), getchar);
	BVR_ADD_SYM(((TCCState*)_s), putc);
	BVR_ADD_SYM(((TCCState*)_s), putchar);
	BVR_ADD_SYM(((TCCState*)_s), puts);
	BVR_ADD_SYM(((TCCState*)_s), ungetc);
	BVR_ADD_SYM(((TCCState*)_s), fread);
	BVR_ADD_SYM(((TCCState*)_s), fwrite);
	BVR_ADD_SYM(((TCCState*)_s), fgetpos);
	BVR_ADD_SYM(((TCCState*)_s), fseek);
	BVR_ADD_SYM(((TCCState*)_s), fsetpos);
	BVR_ADD_SYM(((TCCState*)_s), ftell);
	BVR_ADD_SYM(((TCCState*)_s), rewind);
	BVR_ADD_SYM(((TCCState*)_s), clearerr);
	BVR_ADD_SYM(((TCCState*)_s), feof);
	BVR_ADD_SYM(((TCCState*)_s), ferror);
	BVR_ADD_SYM(((TCCState*)_s), perror);

	// string.h
	BVR_ADD_SYM(((TCCState*)_s), memcpy);
	BVR_ADD_SYM(((TCCState*)_s), memmove);
	BVR_ADD_SYM(((TCCState*)_s), strcpy);
	BVR_ADD_SYM(((TCCState*)_s), strncpy);
	BVR_ADD_SYM(((TCCState*)_s), strcat);
	BVR_ADD_SYM(((TCCState*)_s), strncat);
	BVR_ADD_SYM(((TCCState*)_s), memcmp);
	BVR_ADD_SYM(((TCCState*)_s), strcmp);
	BVR_ADD_SYM(((TCCState*)_s), strcoll);
	BVR_ADD_SYM(((TCCState*)_s), strncmp);
	BVR_ADD_SYM(((TCCState*)_s), strxfrm);
	BVR_ADD_SYM(((TCCState*)_s), memchr);
	BVR_ADD_SYM(((TCCState*)_s), strchr);
	BVR_ADD_SYM(((TCCState*)_s), strcspn);
	BVR_ADD_SYM(((TCCState*)_s), strpbrk);
	BVR_ADD_SYM(((TCCState*)_s), strrchr);
	BVR_ADD_SYM(((TCCState*)_s), strspn);
	BVR_ADD_SYM(((TCCState*)_s), strstr);
	BVR_ADD_SYM(((TCCState*)_s), strtok);
	BVR_ADD_SYM(((TCCState*)_s), memset);
	BVR_ADD_SYM(((TCCState*)_s), strerror);
	BVR_ADD_SYM(((TCCState*)_s), strlen);

	// time.h
	BVR_ADD_SYM(((TCCState*)_s), clock);
	BVR_ADD_SYM(((TCCState*)_s), difftime);
	BVR_ADD_SYM(((TCCState*)_s), mktime);
	BVR_ADD_SYM(((TCCState*)_s), time);
	BVR_ADD_SYM(((TCCState*)_s), asctime);
	BVR_ADD_SYM(((TCCState*)_s), ctime);
	BVR_ADD_SYM(((TCCState*)_s), gmtime);
	BVR_ADD_SYM(((TCCState*)_s), localtime);
	BVR_ADD_SYM(((TCCState*)_s), strftime);

	// user
	tcc_add_symbol(((TCCState*)_s), "stdin", &stdin);
	tcc_add_symbol(((TCCState*)_s), "stderr", &stderr);
	tcc_add_symbol(((TCCState*)_s), "stdout", &stdout);

}