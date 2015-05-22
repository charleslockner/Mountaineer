#ifndef __TEST_H__
#define __TEST_H__

#include <iostream>
#include <cmath>

#define PRINT_ERROR(file, line, exp, got) ( \
   std::cout << "[" << (file) << ":" << (line) << "]: " \
   "Expected[" << (exp) << "] Got[" << (got) << "]\n" \
)

inline void _boolCheck_(
   const char * file,
   int line,
   bool got,
   bool exp
) {
   if (got != exp)
      PRINT_ERROR(file, line, (exp ? "true" : "false"), (got ? "true" : "false"));
}

inline void _equalityIntCheck_(
   const char * file,
   int line,
   int got,
   int exp
) {
   if (got != exp)
      PRINT_ERROR(file, line, exp, got);
}

inline void _equalityFloatCheck_(
   const char * file,
   int line,
   double got,
   double exp,
   double tol
) {
   if ((got > exp && got - exp > tol) || (exp > got && exp - got > tol))
      PRINT_ERROR(file, line, exp, got);
}

inline void _nanCheck_(
   const char * file,
   int line,
   double got
) {
   if(!isnan(got) && !isinf(got))
      PRINT_ERROR(file, line, "nan", got);
}

#define boolCheck(got, exp) _boolCheck_(__FILE__, __LINE__, got, exp)
#define equalityIntCheck(got, exp) _equalityIntCheck_(__FILE__, __LINE__, got, exp)
#define equalityFloatCheck(got, exp, tol) _equalityFloatCheck_(__FILE__, __LINE__, got, exp, tol)
#define nanCheck(got) _nanCheck_(__FILE__, __LINE__, got)

void testGeometry();
void testModel();

#endif // __TEST_H__