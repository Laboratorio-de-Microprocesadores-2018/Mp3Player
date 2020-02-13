/**
 * @file Assert.h
 * @brief Define assertion macro to halt the program execution.
 *
 * It halts the program and stores the filename and line where the assertion has
 * failed.
 */

#ifndef ASSERT_H_
#define ASSERT_H_


#if defined(_WIN64) || defined(_WIN32)
#define assert(expr)
#else
void __assert_func(const char *file, int line, const char *func, const char *failedExpr) __attribute__ ((noreturn));
#define assert(expr) \
		 if (expr){} \
		 else __assert_func(__FILE__, __LINE__, __FUNCTION__, #expr)
#endif

#endif
