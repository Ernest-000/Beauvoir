#pragma once

#include <bvr/config.h>
#include <bvr/common.h>

#include <libtcc.h>

#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifndef BVR_ADD_SYM
    #define BVR_ADD_SYM(state, key) if(key != NULL) { tcc_add_symbol(state, #key, key); }
    #define BVR_ADD_SYM_PTR(state, key) if(key != NULL) { tcc_add_symbol(state, #key, &key); }
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
    TCCState* state = (TCCState*)_s;

    // assert.h
    BVR_ADD_SYM(state, bvri_wmessage);
    BVR_ADD_SYM(state, bvri_wassert);

    // ctype.h
    BVR_ADD_SYM(state, isalnum);
    BVR_ADD_SYM(state, isalpha);
    BVR_ADD_SYM(state, isblank);
    BVR_ADD_SYM(state, iscntrl);
    BVR_ADD_SYM(state, isdigit);
    BVR_ADD_SYM(state, isgraph);
    BVR_ADD_SYM(state, islower);
    BVR_ADD_SYM(state, isprint);
    BVR_ADD_SYM(state, ispunct);
    BVR_ADD_SYM(state, isspace);
    BVR_ADD_SYM(state, isupper);
    BVR_ADD_SYM(state, isxdigit);
    BVR_ADD_SYM(state, tolower);
    BVR_ADD_SYM(state, toupper);

    // math.f
    BVR_ADD_SYM(state, acos);
    BVR_ADD_SYM(state, acosf);
    BVR_ADD_SYM(state, acosl);
    BVR_ADD_SYM(state, asin);
    BVR_ADD_SYM(state, asinf);
    BVR_ADD_SYM(state, asinl);
    BVR_ADD_SYM(state, atan);
    BVR_ADD_SYM(state, atanf);
    BVR_ADD_SYM(state, atanl);
    BVR_ADD_SYM(state, atan2);
    BVR_ADD_SYM(state, atan2f);
    BVR_ADD_SYM(state, atan2l);
    BVR_ADD_SYM(state, cos);
    BVR_ADD_SYM(state, cosf);
    BVR_ADD_SYM(state, cosl);
    BVR_ADD_SYM(state, sin);
    BVR_ADD_SYM(state, sinf);
    BVR_ADD_SYM(state, sinl);
    BVR_ADD_SYM(state, tan);
    BVR_ADD_SYM(state, tanf);
    BVR_ADD_SYM(state, tanl);
    BVR_ADD_SYM(state, acosh);
    BVR_ADD_SYM(state, acoshf);
    BVR_ADD_SYM(state, acoshl);
    BVR_ADD_SYM(state, asinh);
    BVR_ADD_SYM(state, asinhf);
    BVR_ADD_SYM(state, asinhl);
    BVR_ADD_SYM(state, atanh);
    BVR_ADD_SYM(state, atanhf);
    BVR_ADD_SYM(state, atanhl);
    BVR_ADD_SYM(state, cosh);
    BVR_ADD_SYM(state, coshf);
    BVR_ADD_SYM(state, coshl);
    BVR_ADD_SYM(state, sinh);
    BVR_ADD_SYM(state, sinhf);
    BVR_ADD_SYM(state, sinhl);
    BVR_ADD_SYM(state, tanh);
    BVR_ADD_SYM(state, tanhf);
    BVR_ADD_SYM(state, tanhl);
    BVR_ADD_SYM(state, exp);
    BVR_ADD_SYM(state, expf);
    BVR_ADD_SYM(state, expl);
    BVR_ADD_SYM(state, exp2);
    BVR_ADD_SYM(state, exp2f);
    BVR_ADD_SYM(state, exp2l);
    BVR_ADD_SYM(state, expm1);
    BVR_ADD_SYM(state, expm1f);
    BVR_ADD_SYM(state, expm1l);
    BVR_ADD_SYM(state, frexp);
    BVR_ADD_SYM(state, frexpf);
    BVR_ADD_SYM(state, frexpl);
    BVR_ADD_SYM(state, ilogb);
    BVR_ADD_SYM(state, ilogbf);
    BVR_ADD_SYM(state, ilogbl);
    BVR_ADD_SYM(state, ldexp);
    BVR_ADD_SYM(state, ldexpf);
    BVR_ADD_SYM(state, ldexpl);
    BVR_ADD_SYM(state, log);
    BVR_ADD_SYM(state, logf);
    BVR_ADD_SYM(state, logl);
    BVR_ADD_SYM(state, log10);
    BVR_ADD_SYM(state, log10f);
    BVR_ADD_SYM(state, log10l);
    BVR_ADD_SYM(state, log1p);
    BVR_ADD_SYM(state, log1pf);
    BVR_ADD_SYM(state, log1pl);
    BVR_ADD_SYM(state, log2);
    BVR_ADD_SYM(state, log2f);
    BVR_ADD_SYM(state, log2l);
    BVR_ADD_SYM(state, logb);
    BVR_ADD_SYM(state, logbf);
    BVR_ADD_SYM(state, logbl);
    BVR_ADD_SYM(state, modf);
    BVR_ADD_SYM(state, modff);
    BVR_ADD_SYM(state, modfl);
    BVR_ADD_SYM(state, scalbn);
    BVR_ADD_SYM(state, scalbnf);
    BVR_ADD_SYM(state, scalbnl);
    BVR_ADD_SYM(state, scalbln);
    BVR_ADD_SYM(state, scalblnf);
    BVR_ADD_SYM(state, scalblnl);
    BVR_ADD_SYM(state, cbrt);
    BVR_ADD_SYM(state, cbrtf);
    BVR_ADD_SYM(state, cbrtl);
    BVR_ADD_SYM(state, fabs);
    BVR_ADD_SYM(state, fabsf);
    BVR_ADD_SYM(state, fabsl);
    BVR_ADD_SYM(state, hypot);
    BVR_ADD_SYM(state, hypotf);
    BVR_ADD_SYM(state, hypotl);
    BVR_ADD_SYM(state, pow);
    BVR_ADD_SYM(state, powf);
    BVR_ADD_SYM(state, powl);
    BVR_ADD_SYM(state, sqrt);
    BVR_ADD_SYM(state, sqrtf);
    BVR_ADD_SYM(state, sqrtl);
    BVR_ADD_SYM(state, erf);
    BVR_ADD_SYM(state, erff);
    BVR_ADD_SYM(state, erfl);
    BVR_ADD_SYM(state, erfc);
    BVR_ADD_SYM(state, erfcf);
    BVR_ADD_SYM(state, erfcl);
    BVR_ADD_SYM(state, lgamma);
    BVR_ADD_SYM(state, lgammaf);
    BVR_ADD_SYM(state, lgammal);
    BVR_ADD_SYM(state, tgamma);
    BVR_ADD_SYM(state, tgammaf);
    BVR_ADD_SYM(state, tgammal);
    BVR_ADD_SYM(state, ceil);
    BVR_ADD_SYM(state, ceilf);
    BVR_ADD_SYM(state, ceill);
    BVR_ADD_SYM(state, floor);
    BVR_ADD_SYM(state, floorf);
    BVR_ADD_SYM(state, floorl);
    BVR_ADD_SYM(state, nearbyint);
    BVR_ADD_SYM(state, nearbyintf);
    BVR_ADD_SYM(state, nearbyintl);
    BVR_ADD_SYM(state, rint);
    BVR_ADD_SYM(state, rintf);
    BVR_ADD_SYM(state, rintl);
    BVR_ADD_SYM(state, lrint);
    BVR_ADD_SYM(state, lrintf);
    BVR_ADD_SYM(state, lrintl);
    BVR_ADD_SYM(state, llrint);
    BVR_ADD_SYM(state, llrintf);
    BVR_ADD_SYM(state, llrintl);
    BVR_ADD_SYM(state, round);
    BVR_ADD_SYM(state, roundf);
    BVR_ADD_SYM(state, roundl);
    BVR_ADD_SYM(state, lround);
    BVR_ADD_SYM(state, lroundf);
    BVR_ADD_SYM(state, lroundl);
    BVR_ADD_SYM(state, llround);
    BVR_ADD_SYM(state, llroundf);
    BVR_ADD_SYM(state, llroundl);
    BVR_ADD_SYM(state, trunc);
    BVR_ADD_SYM(state, truncf);
    BVR_ADD_SYM(state, truncl);
    BVR_ADD_SYM(state, fmod);
    BVR_ADD_SYM(state, fmodf);
    BVR_ADD_SYM(state, fmodl);
    BVR_ADD_SYM(state, remainder);
    BVR_ADD_SYM(state, remainderf);
    BVR_ADD_SYM(state, remainderl);
    BVR_ADD_SYM(state, remquo);
    BVR_ADD_SYM(state, remquof);
    BVR_ADD_SYM(state, remquol);
    BVR_ADD_SYM(state, copysign);
    BVR_ADD_SYM(state, copysignf);
    BVR_ADD_SYM(state, copysignl);
    BVR_ADD_SYM(state, nan);
    BVR_ADD_SYM(state, nanf);
    BVR_ADD_SYM(state, nanl);
    BVR_ADD_SYM(state, nextafter);
    BVR_ADD_SYM(state, nextafterf);
    BVR_ADD_SYM(state, nextafterl);
    BVR_ADD_SYM(state, nexttoward);
    BVR_ADD_SYM(state, nexttowardf);
    BVR_ADD_SYM(state, nexttowardl);
    BVR_ADD_SYM(state, fdim);
    BVR_ADD_SYM(state, fdimf);
    BVR_ADD_SYM(state, fdiml);
    BVR_ADD_SYM(state, fmax);
    BVR_ADD_SYM(state, fmaxf);
    BVR_ADD_SYM(state, fmaxl);
    BVR_ADD_SYM(state, fmin);
    BVR_ADD_SYM(state, fminf);
    BVR_ADD_SYM(state, fminl);
    BVR_ADD_SYM(state, fma);
    BVR_ADD_SYM(state, fmaf);
    BVR_ADD_SYM(state, fmal);

    // stdio.h
    BVR_ADD_SYM_PTR(state, stdout);
    BVR_ADD_SYM_PTR(state, stderr);
    BVR_ADD_SYM_PTR(state, stdin);
    
    BVR_ADD_SYM(state, remove);
    BVR_ADD_SYM(state, rename);
    BVR_ADD_SYM(state, tmpfile);
    // BVR_ADD_SYM(state, tmpnam); // depreciated
    BVR_ADD_SYM(state, fclose);
    BVR_ADD_SYM(state, fflush);
    BVR_ADD_SYM(state, fopen);
    BVR_ADD_SYM(state, freopen);
    BVR_ADD_SYM(state, setbuf);
    BVR_ADD_SYM(state, setvbuf);
    BVR_ADD_SYM(state, fprintf);
    BVR_ADD_SYM(state, fscanf);
    BVR_ADD_SYM(state, printf);
    BVR_ADD_SYM(state, scanf);
    BVR_ADD_SYM(state, snprintf);
    BVR_ADD_SYM(state, sprintf);
    BVR_ADD_SYM(state, sscanf);
    BVR_ADD_SYM(state, vfprintf);
    BVR_ADD_SYM(state, vfscanf);
    BVR_ADD_SYM(state, vprintf);
    BVR_ADD_SYM(state, vscanf);
    BVR_ADD_SYM(state, vsnprintf);
    BVR_ADD_SYM(state, vsprintf);
    BVR_ADD_SYM(state, vsscanf);
    BVR_ADD_SYM(state, fgetc);
    BVR_ADD_SYM(state, fgets);
    BVR_ADD_SYM(state, fputc);
    BVR_ADD_SYM(state, fputs);
    BVR_ADD_SYM(state, getc);
    BVR_ADD_SYM(state, getchar);
    BVR_ADD_SYM(state, putc);
    BVR_ADD_SYM(state, putchar);
    BVR_ADD_SYM(state, puts);
    BVR_ADD_SYM(state, ungetc);
    BVR_ADD_SYM(state, fread);
    BVR_ADD_SYM(state, fwrite);
    BVR_ADD_SYM(state, fgetpos);
    BVR_ADD_SYM(state, fseek);
    BVR_ADD_SYM(state, fsetpos);
    BVR_ADD_SYM(state, ftell);
    BVR_ADD_SYM(state, rewind);
    BVR_ADD_SYM(state, clearerr);
    BVR_ADD_SYM(state, feof);
    BVR_ADD_SYM(state, ferror);
    BVR_ADD_SYM(state, perror);

    // string.h
    BVR_ADD_SYM(state, memcpy);
    BVR_ADD_SYM(state, memmove);
    BVR_ADD_SYM(state, strcpy);
    BVR_ADD_SYM(state, strncpy);
    BVR_ADD_SYM(state, strcat);
    BVR_ADD_SYM(state, strncat);
    BVR_ADD_SYM(state, memcmp);
    BVR_ADD_SYM(state, strcmp);
    BVR_ADD_SYM(state, strcoll);
    BVR_ADD_SYM(state, strncmp);
    BVR_ADD_SYM(state, strxfrm);
    BVR_ADD_SYM(state, memchr);
    BVR_ADD_SYM(state, strchr);
    BVR_ADD_SYM(state, strcspn);
    BVR_ADD_SYM(state, strpbrk);
    BVR_ADD_SYM(state, strrchr);
    BVR_ADD_SYM(state, strspn);
    BVR_ADD_SYM(state, strstr);
    BVR_ADD_SYM(state, strtok);
    BVR_ADD_SYM(state, memset);
    BVR_ADD_SYM(state, strerror);
    BVR_ADD_SYM(state, strlen);

    // time.h
    BVR_ADD_SYM(state, clock);
    BVR_ADD_SYM(state, difftime);
    BVR_ADD_SYM(state, mktime);
    BVR_ADD_SYM(state, time);
    BVR_ADD_SYM(state, asctime);
    BVR_ADD_SYM(state, ctime);
    BVR_ADD_SYM(state, gmtime);
    BVR_ADD_SYM(state, localtime);
    BVR_ADD_SYM(state, strftime);
}