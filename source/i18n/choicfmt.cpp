/*
*******************************************************************************
* Copyright (C) 1997-2001, International Business Machines Corporation and    *
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* File CHOICFMT.CPP
*
* Modification History:
*
*   Date        Name        Description
*   02/19/97    aliu        Converted from java.
*   03/20/97    helena      Finished first cut of implementation and got rid 
*                           of nextDouble/previousDouble and replaced with
*                           boolean array.
*   4/10/97     aliu        Clean up.  Modified to work on AIX.
*   06/04/97    helena      Fixed applyPattern(), toPattern() and not to include 
*                           wchar.h.
*   07/09/97    helena      Made ParsePosition into a class.
*   08/06/97    nos         removed overloaded constructor, fixed 'format(array)'
*   07/22/98    stephen     JDK 1.2 Sync - removed UBool array (doubleFlags)
*   02/22/99    stephen     Removed character literals for EBCDIC safety
********************************************************************************
*/

#include "cpputils.h"
#include "unicode/choicfmt.h"
#include "unicode/numfmt.h"
#include "unicode/locid.h"
#include "ustrfmt.h"

// *****************************************************************************
// class ChoiceFormat
// *****************************************************************************

U_NAMESPACE_BEGIN

const char        ChoiceFormat::fgClassID = 0; // Value is irrelevant

// Special characters used by ChoiceFormat.  There are two characters
// used interchangeably to indicate <=.  Either is parsed, but only
// LESS_EQUAL is generated by toPattern().
#define SINGLE_QUOTE ((UChar)0x0027)   /*'*/
#define LESS_THAN    ((UChar)0x003C)   /*<*/
#define LESS_EQUAL   ((UChar)0x0023)   /*#*/
#define LESS_EQUAL2  ((UChar)0x2264)
#define VERTICAL_BAR ((UChar)0x007C)   /*|*/
#define MINUS        ((UChar)0x002D)   /*-*/
#define INFINITY     ((UChar)0x221E)

const UChar ChoiceFormat::fgPositiveInfinity[] = {INFINITY, 0};
const UChar ChoiceFormat::fgNegativeInfinity[] = {MINUS, INFINITY, 0};
#define POSITIVE_INF_STRLEN 1
#define NEGATIVE_INF_STRLEN 2

// -------------------------------------
// Creates a ChoiceFormat instance based on the pattern.

ChoiceFormat::ChoiceFormat(const UnicodeString& newPattern,
                           UErrorCode& status)
: fChoiceLimits(0),
  fClosures(0),
  fChoiceFormats(0),
  fCount(0)
{
    applyPattern(newPattern, status);
}

// -------------------------------------
// Creates a ChoiceFormat instance with the limit array and 
// format strings for each limit.

ChoiceFormat::ChoiceFormat(const double* limits, 
                           const UnicodeString* formats, 
                           int32_t cnt )
: fChoiceLimits(0),
  fClosures(0),
  fChoiceFormats(0),
  fCount(0)
{
    setChoices(limits, formats, cnt );
}

// -------------------------------------

ChoiceFormat::ChoiceFormat(const double* limits, 
                           const UBool* closures,
                           const UnicodeString* formats, 
                           int32_t cnt )
: fChoiceLimits(0),
  fClosures(0),
  fChoiceFormats(0),
  fCount(0)
{
    setChoices(limits, closures, formats, cnt );
}

// -------------------------------------
// copy constructor

ChoiceFormat::ChoiceFormat(const    ChoiceFormat&   that) 
: NumberFormat(that),
  fChoiceLimits(0),
  fClosures(0),
  fChoiceFormats(0)
{
    *this = that;
}

// -------------------------------------
// Private constructor that creates a 
// ChoiceFormat instance based on the 
// pattern and populates UParseError

ChoiceFormat::ChoiceFormat(const UnicodeString& newPattern,
                           UParseError& parseError,
                           UErrorCode& status)
: fChoiceLimits(0),
  fClosures(0),
  fChoiceFormats(0),
  fCount(0)
{
    applyPattern(newPattern,parseError, status);
}
// -------------------------------------

UBool
ChoiceFormat::operator==(const Format& that) const
{
    if (this == &that) return TRUE;
    if (this->getDynamicClassID() != that.getDynamicClassID()) return FALSE;  // not the same class
    if (!NumberFormat::operator==(that)) return FALSE;
    ChoiceFormat& thatAlias = (ChoiceFormat&)that;
    if (fCount != thatAlias.fCount) return FALSE;
    // Checks the limits, the corresponding format string and LE or LT flags.
    // LE means less than and equal to, LT means less than.
    for (int32_t i = 0; i < fCount; i++) {
        if ((fChoiceLimits[i] != thatAlias.fChoiceLimits[i]) ||
            (fClosures[i] != thatAlias.fClosures[i]) ||
            (fChoiceFormats[i] != thatAlias.fChoiceFormats[i]))
            return FALSE;
    }
    return TRUE;
}

// -------------------------------------
// copy constructor

const ChoiceFormat&
ChoiceFormat::operator=(const   ChoiceFormat& that)
{
    if (this != &that) {
        NumberFormat::operator=(that);
        fCount = that.fCount;
        uprv_free(fChoiceLimits);
        fChoiceLimits = NULL;
        uprv_free(fClosures);
        fClosures = NULL;
        delete [] fChoiceFormats;
        fChoiceFormats = NULL;

        fChoiceLimits = (double*) uprv_malloc( sizeof(double) * fCount);
        fClosures = (UBool*) uprv_malloc( sizeof(UBool) * fCount);
        fChoiceFormats = new UnicodeString[fCount];

        uprv_arrayCopy(that.fChoiceLimits, fChoiceLimits, fCount);
        uprv_arrayCopy(that.fClosures, fClosures, fCount);
        uprv_arrayCopy(that.fChoiceFormats, fChoiceFormats, fCount);
    }
    return *this;
}

// -------------------------------------

ChoiceFormat::~ChoiceFormat()
{
    uprv_free(fChoiceLimits);
    fChoiceLimits = NULL;
    uprv_free(fClosures);
    fClosures = NULL;
    delete [] fChoiceFormats;
    fChoiceFormats = NULL;
    fCount = 0;
}

/**
 * Convert a string to a double value
 */
double
ChoiceFormat::stod(const UnicodeString& string)
{
    char source[256];
    char* end;

    string.extract(0, string.length(), source, sizeof(source), "");    /* invariant codepage */
    return uprv_strtod(source,&end);
}

// -------------------------------------

/**
 * Convert a double value to a string
 */
UnicodeString&
ChoiceFormat::dtos(double value,
                   UnicodeString& string)
{
    char temp[256];

    uprv_dtostr(value, temp, 3, TRUE);
    string = UnicodeString(temp, "");    /* invariant codepage */
    return string;
}

// -------------------------------------
// calls the overloaded applyPattern method.

void
ChoiceFormat::applyPattern(const UnicodeString& pattern,
                           UErrorCode& status)
{
    UParseError parseError;
    applyPattern(pattern, parseError, status);
}

// -------------------------------------
// Applies the pattern to this ChoiceFormat instance.

void
ChoiceFormat::applyPattern(const UnicodeString& pattern,
                           UParseError& parseError,
                           UErrorCode& status)
{
    if (U_FAILURE(status)) 
    {
        return;
    }

    // Clear error struct
    parseError.offset = -1;
    parseError.preContext[0] = parseError.postContext[0] = (UChar)0;

    // Perform 2 passes.  The first computes the number of limits in
    // this pattern (fCount), which is 1 more than the number of
    // literal VERTICAL_BAR characters.
    int32_t count = 1;
    int32_t i;
    for (i=0; i<pattern.length(); ++i) {
        UChar c = pattern[i];
        if (c == SINGLE_QUOTE) {
            // Skip over the entire quote, including embedded
            // contiguous pairs of SINGLE_QUOTE.
            for (;;) {
                do {
                    ++i;
                } while (i<pattern.length() &&
                         pattern[i] != SINGLE_QUOTE);
                if ((i+1)<pattern.length() &&
                    pattern[i+1] == SINGLE_QUOTE) {
                    // SINGLE_QUOTE pair; skip over it
                    ++i;
                } else {
                    break;
                }
            }
        } else if (c == VERTICAL_BAR) {
            ++count;
        }
    }

    // Allocate the required storage.
    double *newLimits = (double*) uprv_malloc( sizeof(double) * count);
    UBool *newClosures = (UBool*) uprv_malloc( sizeof(UBool) * count);
    UnicodeString *newFormats = new UnicodeString[count];

    // Perform the second pass
    int32_t k = 0; // index into newXxx[] arrays
    UnicodeString buf; // scratch buffer
    UBool inQuote = FALSE;
    UBool inNumber = TRUE; // TRUE before < or #, FALSE after
    for (i=0; i<pattern.length(); ++i) {
        UChar c = pattern[i];
        if (c == SINGLE_QUOTE) {
            // Check for SINGLE_QUOTE pair indicating a literal quote
            if ((i+1) < pattern.length() &&
                pattern[i+1] == SINGLE_QUOTE) {
                buf += SINGLE_QUOTE;
                ++i;
            } else {
                inQuote = !inQuote;
            }
        } else if (inQuote) {
            buf += c;
        } else if (c == LESS_THAN || c == LESS_EQUAL || c == LESS_EQUAL2) {
            if (!inNumber || buf.length() == 0) {
                goto error;
            }
            inNumber = FALSE;

            double limit;
            buf.trim();
            if (!buf.compare(fgPositiveInfinity, POSITIVE_INF_STRLEN)) {
                limit = uprv_getInfinity();
            } else if (!buf.compare(fgNegativeInfinity, NEGATIVE_INF_STRLEN)) {
                limit = -uprv_getInfinity();
            } else {
                limit = stod(buf);
            }

            if (k == count) {
                // This shouldn't happen.  If it does, it means that
                // the count determined in the first pass did not
                // match the number of elements found in the second
                // pass.
                goto error;
            }
            newLimits[k] = limit;
            newClosures[k] = (c == LESS_THAN);

            if (k > 0 && limit <= newLimits[k-1]) {
                // Each limit must be strictly > than the previous
                // limit.  One exception: Two subsequent limits may be
                // == if the first closure is FALSE and the second
                // closure is TRUE.  This places the limit value in
                // the second interval.
                if (!(limit == newLimits[k-1] &&
                      !newClosures[k-1] &&
                      newClosures[k])) {
                    goto error;
                }
            }

            buf.truncate(0);
        } else if (c == VERTICAL_BAR) {
            if (inNumber) {
                goto error;
            }
            inNumber = TRUE;

            newFormats[k] = buf;
            ++k;
            buf.truncate(0);
        } else {
            buf += c;
        }        
    }

    if (k != (count-1) || inNumber || inQuote) {
        goto error;
    }
    newFormats[k] = buf;

    // Don't modify this object until the parse succeeds
    uprv_free(fChoiceLimits);
    uprv_free(fClosures);
    delete[] fChoiceFormats;
    fCount = count;
    fChoiceLimits  = newLimits;
    fClosures      = newClosures;
    fChoiceFormats = newFormats;
    return;

error:
    status = U_ILLEGAL_ARGUMENT_ERROR;
    syntaxError(pattern,i,parseError);
    uprv_free(newLimits);
    uprv_free(newClosures);
    delete[] newFormats;
    return;

}
// -------------------------------------
// Reconstruct the original input pattern.

UnicodeString&
ChoiceFormat::toPattern(UnicodeString& result) const
{
    result.remove();
    for (int32_t i = 0; i < fCount; ++i) {
        if (i != 0) {
            result += VERTICAL_BAR;
        }
        UnicodeString buf;
        if (uprv_isPositiveInfinity(fChoiceLimits[i])) {
            result += INFINITY;
        } else if (uprv_isNegativeInfinity(fChoiceLimits[i])) {
            result += MINUS;
            result += INFINITY;
        } else {
            result += dtos(fChoiceLimits[i], buf);
        }
        if (fClosures[i]) {
            result += LESS_THAN;
        } else {
            result += LESS_EQUAL;
        }
        // Append fChoiceFormats[i], using quotes if there are special
        // characters.  Single quotes themselves must be escaped in
        // either case.
        const UnicodeString& text = fChoiceFormats[i];
        UBool needQuote = text.indexOf(LESS_THAN) >= 0
            || text.indexOf(LESS_EQUAL) >= 0
            || text.indexOf(LESS_EQUAL2) >= 0
            || text.indexOf(VERTICAL_BAR) >= 0;
        if (needQuote) {
            result += SINGLE_QUOTE;
        }
        if (text.indexOf(SINGLE_QUOTE) < 0) {
            result += text;
        }
        else {
            for (int32_t j = 0; j < text.length(); ++j) {
                UChar c = text[j];
                result += c;
                if (c == SINGLE_QUOTE) {
                    result += c;
                }
            }
        }
        if (needQuote) {
            result += SINGLE_QUOTE;
        }
    }

    return result;
}

// -------------------------------------
// Adopts the limit and format arrays.

void
ChoiceFormat::adoptChoices(double *limits, 
                           UnicodeString *formats, 
                           int32_t cnt )
{
    adoptChoices(limits, (UBool *)0, formats, cnt);
}

// -------------------------------------
// Adopts the limit and format arrays.

void
ChoiceFormat::adoptChoices(double *limits, 
                           UBool *closures,
                           UnicodeString *formats, 
                           int32_t cnt )
{
    if(limits == 0 || formats == 0)
        return;

    uprv_free(fChoiceLimits);
    uprv_free(fClosures);
    delete [] fChoiceFormats;
    fChoiceLimits = limits;
    fClosures = closures;
    fChoiceFormats = formats;
    fCount = cnt;

    if (fClosures == 0) {
        fClosures = (UBool*) uprv_malloc( sizeof(UBool) * fCount);
        int32_t i;
        for (i=0; i<fCount; ++i) {
            fClosures[i] = FALSE;
        }
    }
}

// -------------------------------------
// Sets the limit and format arrays. 
void
ChoiceFormat::setChoices(  const double* limits, 
                           const UnicodeString* formats, 
                           int32_t cnt )
{
    setChoices(limits, 0, formats, cnt);
}

// -------------------------------------
// Sets the limit and format arrays. 
void
ChoiceFormat::setChoices(  const double* limits, 
                           const UBool* closures,
                           const UnicodeString* formats, 
                           int32_t cnt )
{
    if(limits == 0 || formats == 0)
        return;

    uprv_free(fChoiceLimits);
    uprv_free(fClosures);
    delete [] fChoiceFormats;

    // Note that the old arrays are deleted and this owns
    // the created array.
    fCount = cnt;
    fChoiceLimits = (double*) uprv_malloc( sizeof(double) * fCount);
    fClosures = (UBool*) uprv_malloc( sizeof(UBool) * fCount);
    fChoiceFormats = new UnicodeString[fCount];

    uprv_arrayCopy(limits, fChoiceLimits, fCount);
    uprv_arrayCopy(formats, fChoiceFormats, fCount);

    if (closures != 0) {
        uprv_arrayCopy(closures, fClosures, fCount);
    } else {
        int32_t i;
        for (i=0; i<fCount; ++i) {
            fClosures[i] = FALSE;
        }
    }
}

// -------------------------------------
// Gets the limit array.

const double*
ChoiceFormat::getLimits(int32_t& cnt) const 
{
    cnt = fCount;
    return fChoiceLimits;
}

// -------------------------------------
// Gets the closures array.

const UBool*
ChoiceFormat::getClosures(int32_t& cnt) const 
{
    cnt = fCount;
    return fClosures;
}

// -------------------------------------
// Gets the format array.

const UnicodeString*
ChoiceFormat::getFormats(int32_t& cnt) const
{
    cnt = fCount;
    return fChoiceFormats;
}

// -------------------------------------
// Formats a long number, it's actually formatted as
// a double.  The returned format string may differ
// from the input number because of this.

UnicodeString&
ChoiceFormat::format(int32_t number, 
                     UnicodeString& toAppendTo, 
                     FieldPosition& status) const
{
    return format((double) number, toAppendTo, status);
}

// -------------------------------------
// Formats a double number.

UnicodeString&
ChoiceFormat::format(double number, 
                     UnicodeString& toAppendTo, 
                     FieldPosition& /*pos*/) const
{
    // find the number
    int32_t i;
    for (i = 0; i < fCount; ++i) {
        if (fClosures[i]) {
            if (!(number > fChoiceLimits[i])) {
                // same as number <= fChoiceLimits, except catches NaN
                break;
            }
        } else if (!(number >= fChoiceLimits[i])) {
            // same as number < fChoiceLimits, except catches NaN
            break;
        }
    }
    --i;
    if (i < 0) {
        i = 0;
    }
    // return either a formatted number, or a string
    toAppendTo += fChoiceFormats[i];
    return toAppendTo;
}

// -------------------------------------
// Formats an array of objects. Checks if the data type of the objects
// to get the right value for formatting.  

UnicodeString&
ChoiceFormat::format(const Formattable* objs,
                     int32_t cnt,
                     UnicodeString& toAppendTo,
                     FieldPosition& pos,
                     UErrorCode& status) const
{
    if(cnt < 0) {
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return toAppendTo;
    }

    UnicodeString buffer;
    for (int32_t i = 0; i < cnt; i++) {
        double objDouble = (objs[i].getType() == Formattable::kLong) ?
            ((double) objs[i].getLong()) : objs[i].getDouble();
        buffer.remove();
        toAppendTo += format(objDouble, buffer, pos);
    }

    return toAppendTo;
}

// -------------------------------------
// Formats an array of objects. Checks if the data type of the objects
// to get the right value for formatting.  

UnicodeString&
ChoiceFormat::format(const Formattable& obj, 
                     UnicodeString& toAppendTo, 
                     FieldPosition& pos,
                     UErrorCode& status) const
{
    return NumberFormat::format(obj, toAppendTo, pos, status);
}
// -------------------------------------

void
ChoiceFormat::parse(const UnicodeString& text, 
                    Formattable& result,
                    ParsePosition& status) const
{
    // find the best number (defined as the one with the longest parse)
    int32_t start = status.getIndex();
    int32_t furthest = start;
    double bestNumber = uprv_getNaN();
    double tempNumber = 0.0;
    for (int i = 0; i < fCount; ++i) {
        UnicodeString tempString = fChoiceFormats[i];
        if(text.compareBetween(start, tempString.length(), tempString, 0, tempString.length()) == 0) {
            status.setIndex(start + tempString.length());
            tempNumber = fChoiceLimits[i];
            if (status.getIndex() > furthest) {
                furthest = status.getIndex();
                bestNumber = tempNumber;
                if (furthest == text.length()) 
                    break;
            }
        }
    }
    status.setIndex(furthest);
    if (status.getIndex() == start) {
        status.setErrorIndex(furthest);
    }
    result.setDouble(bestNumber);
}

// -------------------------------------
// Parses the text and return the Formattable object.  

void
ChoiceFormat::parse(const UnicodeString& text, 
                    Formattable& result,
                    UErrorCode& status) const
{
    NumberFormat::parse(text, result, status);
}

// -------------------------------------

Format*
ChoiceFormat::clone() const
{
    ChoiceFormat *aCopy = new ChoiceFormat(*this);
    return aCopy;
}

// -------------------------------------

double 
ChoiceFormat::nextDouble( double d, UBool positive )
{
    return uprv_nextDouble( d, positive );
}

U_NAMESPACE_END

//eof
