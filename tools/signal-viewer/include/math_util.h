/**
 * @file math_util.h
 * @brief Math Utilities.
 *
 */




#ifndef MATH_UTIL_H
#define MATH_UTIL_H




#include <math.h>




/**
 * @brief Math macro: min(a,b).
 *
 */
#define m_min(a,b) ((a)<(b)?(a):(b))


/**
 * @brief Math macro: max(a,b).
 *
 */
#define m_max(a,b) ((a)>(b)?(a):(b))


/**
 * @brief Math macro: abs(x).
 *
 */
#define m_abs(x) ((x)>0?(x):-(x))


/**
 * @brief Math macro: constrain(amount, low, high).
 *
 */
#define m_constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


/**
 * @brief Math macro: radians(degrees).
 *
 */
#define m_radians(deg) ((deg)*(M_PI/180.0))


/**
 * @brief Math macro: degrees(radians).
 *
 */
#define m_degrees(rad) ((rad)*(180.0/M_PI))


/**
 * @brief Math macro: square(x).
 *
 */
#define m_sq(x) ((x)*(x))




#endif	/* MATH_UTIL_H */
