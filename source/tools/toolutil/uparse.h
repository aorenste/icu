/*
*******************************************************************************
*
*   Copyright (C) 2000-2002, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  uparse.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2000apr18
*   created by: Markus W. Scherer
*
*   This file provides a parser for files that are delimited by one single
*   character like ';' or TAB. Example: the Unicode Character Properties files
*   like UnicodeData.txt are semicolon-delimited.
*/

#ifndef __UPARSE_H__
#define __UPARSE_H__

#include "unicode/utypes.h"

U_CDECL_BEGIN

static U_INLINE const char *
u_skipWhitespace(const char *s) {
    while(*s==' ' || *s=='\t') {
        ++s;
    }
    return s;
}

/** Function type for u_parseDelimitedFile(). */
typedef void U_CALLCONV
UParseLineFn(void *context,
              char *fields[][2],
              int32_t fieldCount,
              UErrorCode *pErrorCode);

/**
 * Parser for files that are similar to UnicodeData.txt:
 * This function opens the file and reads it line by line. It skips empty lines
 * and comment lines that start with a '#'.
 * All other lines are separated into fields with one delimiter character
 * (semicolon for Unicode Properties files) between two fields. The last field in
 * a line does not need to be terminated with a delimiter.
 *
 * For each line, after segmenting it, a line function is called.
 * It gets passed the array of field start and limit pointers that is
 * passed into this parser and filled by it for each line.
 * For each field i of the line, the start pointer in fields[i][0]
 * points to the beginning of the field, while the limit pointer in fields[i][1]
 * points behind the field, i.e., to the delimiter or the line end.
 *
 * The context parameter of the line function is
 * the same as the one for the parse function.
 *
 * The line function may modify the contents of the fields including the
 * limit characters.
 *
 * If the file cannot be opened, or there is a parsing error or a field function
 * sets *pErrorCode, then the parser returns with *pErrorCode set to an error code.
 */
U_CAPI void U_EXPORT2
u_parseDelimitedFile(const char *filename, char delimiter,
                     char *fields[][2], int32_t fieldCount,
                     UParseLineFn *lineFn, void *context,
                     UErrorCode *pErrorCode);

/**
 * Parse a string of code points like 0061 0308 0300.
 * s must end with either ';' or NUL.
 *
 * @return Number of code points.
 */
U_CAPI int32_t U_EXPORT2
u_parseCodePoints(const char *s,
                  uint32_t *dest, int32_t destCapacity,
                  UErrorCode *pErrorCode);

/**
 * Parse a code point range like
 * 0085 or
 * 4E00..9FA5.
 *
 * s must contain such a range and end with either ';' or NUL.
 *
 * @return Length of code point range, end-start+1
 */
U_CAPI int32_t U_EXPORT2
u_parseCodePointRange(const char *s,
                      uint32_t *pStart, uint32_t *pEnd,
                      UErrorCode *pErrorCode);

U_CDECL_END

#endif
