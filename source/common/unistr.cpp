/*
*******************************************************************************
*                                                                             *
* COPYRIGHT:                                                                  *
*   (C) Copyright International Business Machines Corporation, 1998-1999      *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.        *
*   US Government Users Restricted Rights - Use, duplication, or disclosure   *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                    *
*                                                                             *
*******************************************************************************
*
* File unistr.cpp
*
* Modification History:
*
*   Date        Name        Description
*   09/25/98    stephen     Creation.
*   04/20/99    stephen     Overhauled per 4/16 code review.
*   07/09/99    stephen     Renamed {hi,lo},{byte,word} to icu_X for HP/UX
*******************************************************************************
*/

#include "unistr.h"

#include "locid.h"
#include "cstring.h"
#include "cmemory.h"
#include "ustring.h"
#include "mutex.h"

#if 0
//DEBUGGING
#include <iostream.h>

void
print(const UnicodeString& s,
      const char *name)
{
  UChar c;
  cout << name << ":|";
  for(int i = 0; i < s.length(); ++i) {
    c = s[i];
    if(c>= 0x007E || c < 0x0020)
      cout << "[0x" << hex << s[i] << "]";
    else
      cout << (char) s[i];
  }
  cout << '|' << endl;
}

void
print(const UChar *s,
      int32_t len,
      const char *name)
{
  UChar c;
  cout << name << ":|";
  for(int i = 0; i < len; ++i) {
    c = s[i];
    if(c>= 0x007E || c < 0x0020)
      cout << "[0x" << hex << s[i] << "]";
    else
      cout << (char) s[i];
  }
  cout << '|' << endl;
}
// END DEBUGGING
#endif

// Local function definitions for now

// move u_arrayCompare to utypes.h ??
inline int8_t
u_arrayCompare(const UChar *src, int32_t srcStart,
         const UChar *dst, int32_t dstStart, int32_t count)
{return icu_memcmp(src+srcStart, dst+dstStart, (size_t)(count*sizeof(*src)));}

// need to copy areas that may overlap
inline void
us_arrayCopy(const UChar *src, int32_t srcStart,
         UChar *dst, int32_t dstStart, int32_t count)
{icu_memmove(dst+dstStart, src+srcStart, (size_t)(count*sizeof(*src)));}

// static initialization
const UChar UnicodeString::fgInvalidUChar      = 0xFFFF;
const int32_t UnicodeString::kGrowSize         = 0x80;
const int32_t UnicodeString::kInvalidHashCode  = 0;
const int32_t UnicodeString::kEmptyHashCode    = 1;
UConverter* UnicodeString::fgDefaultConverter  = 0;

//========================================
// Constructors
//========================================
UnicodeString::UnicodeString()
  : fArray(fStackBuffer),
    fLength(0),
    fCapacity(US_STACKBUF_SIZE),
    fRefCounted(FALSE),
    fHashCode(kEmptyHashCode),
    fBogus(FALSE)
{}

UnicodeString::UnicodeString(int32_t capacity)
  : fArray(0),
    fLength(0),
    fCapacity(0),
    fRefCounted(FALSE),
    fHashCode(kEmptyHashCode),
    fBogus(FALSE)
{
  fArray = allocate(capacity, fCapacity);
  if(! fArray) {
    setToBogus();
    return;
  }

  setRefCount(1);
}

UnicodeString::UnicodeString(UChar ch)
  : fArray(fStackBuffer),
    fLength(0),
    fCapacity(US_STACKBUF_SIZE),
    fRefCounted(FALSE),
    fHashCode(kEmptyHashCode),
    fBogus(FALSE)
{
  doReplace(0, 0, &ch, 0, 1);
}

UnicodeString::UnicodeString(const UChar *text)
  : fArray(fStackBuffer),
    fLength(0),
    fCapacity(US_STACKBUF_SIZE),
    fRefCounted(FALSE),
    fHashCode(kEmptyHashCode),
    fBogus(FALSE)
{
  doReplace(0, 0, text, 0, u_strlen(text));
}

UnicodeString::UnicodeString( const UChar *text,
                  int32_t textLength)
  : fArray(fStackBuffer),
    fLength(0),
    fCapacity(US_STACKBUF_SIZE),
    fRefCounted(FALSE),
    fHashCode(kEmptyHashCode),
    fBogus(FALSE)
{
  doReplace(0, 0, text, 0, textLength);
}

UnicodeString::UnicodeString(const char *codepageData,
                 const char *codepage)
  : fArray(fStackBuffer),
    fLength(0),
    fCapacity(US_STACKBUF_SIZE),
    fRefCounted(FALSE),
    fHashCode(kEmptyHashCode),
    fBogus(FALSE)
{
  if(codepageData != 0)
    doCodepageCreate(codepageData, icu_strlen(codepageData), codepage);
}


UnicodeString::UnicodeString(const char *codepageData,
                 int32_t dataLength,
                 const char *codepage)
  : fArray(fStackBuffer),
    fLength(0),
    fCapacity(US_STACKBUF_SIZE),
    fRefCounted(FALSE),
    fHashCode(kEmptyHashCode),
    fBogus(FALSE)
{
  doCodepageCreate(codepageData, dataLength, codepage);
}

//========================================
// Destructor
//========================================
UnicodeString::~UnicodeString()
{
  // decrement ref count and reclaim storage, if owned
  if(fRefCounted && removeRef() == 0)
    delete [] fArray;
}

//========================================
// Assignment
//========================================
UnicodeString&
UnicodeString::operator= (const UnicodeString& src)
{
  // if src is bogus, or we're bogus, or assigning to ourselves, do nothing
  if(fBogus || src.isBogus() || this == &src)
  return *this;

  // if src is ref counted, point ourselves at its array
  if(src.fRefCounted) {

    // if we're ref counted, decrement our current ref count
    if(fRefCounted && removeRef() == 0)
    delete [] fArray;

    fArray      = src.fArray;
    fLength     = src.fLength;
    fCapacity   = src.fCapacity;
    fHashCode   = src.fHashCode;
    fRefCounted = TRUE;
    addRef();
  }
  // if src isn't ref counted, just do a replace
  else {
    doReplace(0, fLength, src.fArray, 0, src.fLength);
    fHashCode = src.fHashCode;
  }

  return *this;
}

//========================================
// Miscellaneous operations
//========================================
int32_t
UnicodeString::numDisplayCells( UTextOffset start,
                int32_t length,
                bool_t asian) const
{
  // pin indices to legal values
  pinIndices(start, length);

  UChar c;
  int32_t result = 0;
  UTextOffset limit = start + length;

  while(start < limit) {
    c = getArrayStart()[start];
    switch(Unicode::getCellWidth(c)) {
    case Unicode::ZERO_WIDTH:
      break;;

    case Unicode::HALF_WIDTH:
      result += 1;
      break;

    case Unicode::FULL_WIDTH:
      result += 2;
      break;

    case Unicode::NEUTRAL:
      result += (asian ? 2 : 1);
      break;
    }
    ++start;
  }

  return result;
}

UCharReference
UnicodeString::operator[] (UTextOffset pos)
{
  return UCharReference(this, pos);
}

//========================================
// Read-only implementation
//========================================
int8_t
UnicodeString::doCompare( UTextOffset start,
              int32_t length,
              const UnicodeString& src,
              UTextOffset srcStart,
              int32_t srcLength) const
{
  // pin indices to legal values
  pinIndices(start, length);

  // get the correct pointer
  const UChar *chars = getArrayStart();

  // compare the characters
  return (src.compare(srcStart, srcLength, chars, start, length) * -1);
}

int8_t
UnicodeString::doCompare( UTextOffset start,
              int32_t length,
              const UChar *srcChars,
              UTextOffset srcStart,
              int32_t srcLength) const
{
  // pin indices to legal values
  pinIndices(start, length);

  // get the correct pointer
  const UChar *chars = getArrayStart();

  // we're comparing different lengths
  if(length != srcLength) {

    // compare the minimum # of characters
    int32_t minLength     = (length < srcLength ? length : srcLength);
    const UChar *minLimit = chars + minLength;
    const UChar *limit    = chars + length;
    int8_t result;

    // adjust for starting offsets
    chars += start;
    srcChars += srcStart;

    while(chars < minLimit) {
      result = (*chars - *srcChars);

      if(result != 0)
    return result;

      ++chars;
      ++srcChars;
    }

    // if we got here, the leading portions are identical
    return (chars < limit ? 1 : -1);
  }
  // compare two identical lengths, use u_arrayCompare
  else
    return u_arrayCompare(chars, start, srcChars, srcStart, length);
}

void
UnicodeString::doExtract(UTextOffset start,
             int32_t length,
             UChar *dst,
             UTextOffset dstStart) const
{
  // pin indices to legal values
  pinIndices(start, length);
  us_arrayCopy(getArrayStart(), start, dst, dstStart, length);
}


UTextOffset
UnicodeString::doIndexOf(UChar c,
             UTextOffset start,
             int32_t length) const
{
  // pin indices
  pinIndices(start, length);

  // find the first occurrence of c
  const UChar *begin = getArrayStart() + start;
  const UChar *limit = begin + length;

  while(begin < limit && *begin != c)
    ++begin;

  return (begin == limit ? -1 : begin - getArrayStart());
}

UTextOffset
UnicodeString::doLastIndexOf(UChar c,
                 UTextOffset start,
                 int32_t length) const
{
  // pin indices
  pinIndices(start, length);

  const UChar *begin = getArrayStart() + start + length;
  const UChar *limit = begin - length;

  while(begin > limit && *begin != c)
    --begin;

  return (begin == limit ? -1 : begin - getArrayStart());
}


//========================================
// Write implementation
//========================================

UnicodeString&
UnicodeString::setCharAt(UTextOffset offset,
             UChar c)
{
  if(offset < 0)
    offset = 0;
  else if(offset >= fLength)
    offset = fLength - 1;

  doSetCharAt(offset, c);
  fHashCode = kInvalidHashCode;
  return *this;
}

UnicodeString&
UnicodeString::toUpper()
{ return toUpper(Locale::getDefault()); }

UnicodeString&
UnicodeString::toLower()
{ return toLower(Locale::getDefault()); }

UnicodeString&
UnicodeString::toUpper(const Locale& locale)
{
  UTextOffset start = 0;
  UTextOffset limit = fLength;
  UChar c;
  UnicodeString lang;

  locale.getLanguage(lang);

  // The German sharp S character (U+00DF)'s uppercase equivalent is
  // "SS", making it the only character that expands to two characters
  // when its case is changed (we don't automatically convert "SS" to
  // U+00DF going to lowercase because it can only be determined from
  // knowing the language whether a particular "SS" should map to
  // U+00DF or "ss").  So we make a preliminary pass through the
  // string looking for sharp S characters and then go back and make
  // room for the extra capital Ses if we find any.  [For performance,
  // we only do this extra work if the language is actually German]
  if(lang == "de") {
    UChar SS [] = { 0x0053, 0x0053 };
    while(start < limit) {

      c = getArrayStart()[start];

      // A sharp s needs to be replaced with two capital S's.
      if(c == 0x00DF) {
    doReplace(start, 1, SS, 0, 2);
    start++;
    limit++;
      }

      // Otherwise, the case conversion can be handled by the Unicode unit.
      else if(Unicode::isLowerCase(c))
    doSetCharAt(start, Unicode::toUpperCase(c));

      // If no conversion is necessary, do nothing
      ++start;
    }
  }

  // If the specfied language is Turkish, then we have to special-case
  // for the Turkish dotted and dotless Is.  The regular lowercase i
  // maps to the capital I with a dot (U+0130), and the lowercase i
  // without the dot (U+0131) maps to the regular capital I
  else if(lang == "tr") {
    while(start < limit) {
      c = getArrayStart()[start];

      if(c == 0x0069/*'i'*/)
    doSetCharAt(start, 0x0130);
      else if(c == 0x0131)
    doSetCharAt(start, 0x0049/*'I'*/);
      else if(Unicode::isLowerCase(c))
    doSetCharAt(start, Unicode::toUpperCase(c));
      ++start;
    }
  }

  else {
    while(start < limit) {
      c = getArrayStart()[start];
      if(Unicode::isLowerCase(c))
    doSetCharAt(start, Unicode::toUpperCase(c));
      ++start;
    }
  }

  fHashCode = kInvalidHashCode;

  return *this;
}

UnicodeString&
UnicodeString::toLower(const Locale& locale)
{
  UTextOffset start = 0;
  UTextOffset limit = fLength;
  UChar c;
  UnicodeString lang;

  locale.getLanguage(lang);

  // if the specfied language is Turkish, then we have to special-case
  // for the Turkish dotted and dotless Is.  The capital I with a dot
  // (U+0130) maps to the regular lowercase i, and the regular capital
  // I maps to the lowercase i without the dot (U+0131)
  if(lang == "tr") {
    while(start < limit) {
      c = getArrayStart()[start];
      if(c == 0x0049) // 'I'
    doSetCharAt(start, 0x0131);
      else if(c == 0x0130)
    doSetCharAt(start, 0x0069); // 'i'
      else if(Unicode::isUpperCase(c) || Unicode::isTitleCase(c))
    doSetCharAt(start, Unicode::toLowerCase(c));
      ++start;
    }
  }

  // if the specfied language is Greek, then we have to special-case
  // for the capital letter sigma (U+3A3), which has two lower-case
  // forms.  If the character following the capital sigma is a letter,
  // we use the medial form (U+3C3); otherwise, we use the final form
  // (U+3C2).
  else if(lang == "el") {
    while(start < limit) {
      c = getArrayStart()[start];
      if(c == 0x3a3) {
    if(start + 1 < limit && Unicode::isLetter(getArrayStart()[start + 1]))
      doSetCharAt(start, 0x3C3);
    else
      doSetCharAt(start, 0x3C2);
      }
      else if(Unicode::isUpperCase(c) || Unicode::isTitleCase(c))
    doSetCharAt(start, Unicode::toLowerCase(c));
      ++start;
    }
  }

  // if the specified language is anything other than Turkish or
  // Greek, we rely on the Unicode class to do all our case mapping--
  // there are no other special cases
  else {
    while(start < limit) {
      c = getArrayStart()[start];
      if(Unicode::isUpperCase(c) || Unicode::isTitleCase(c))
    doSetCharAt(start, Unicode::toLowerCase(c));
      ++start;
    }
  }

  fHashCode = kInvalidHashCode;

  return *this;
}

// for speed, no bounds checking is performed and the hash code isn't changed
UnicodeString&
UnicodeString::doSetCharAt(UTextOffset offset,
               UChar c)
{
  // clone our array, if necessary
  cloneArrayIfNeeded();

  // set the character
  fArray[ (fRefCounted ? offset + 1 : offset) ] = c;
  return *this;
}

UnicodeString&
UnicodeString::doReplace( UTextOffset start,
              int32_t length,
              const UnicodeString& src,
              UTextOffset srcStart,
              int32_t srcLength)
{
  // pin the indices to legal values
  src.pinIndices(srcStart, srcLength);

  // get the characters from src
  const UChar *chars = src.getArrayStart();

  // and replace the range in ourselves with them
  doReplace(start, length, chars, srcStart, srcLength);

  return *this;
}

UnicodeString&
UnicodeString::doReplace(UTextOffset start,
             int32_t length,
             const UChar *srcChars,
             UTextOffset srcStart,
             int32_t srcLength)
{
  // if we're bogus, do nothing
  if(fBogus)
    return *this;

  bool_t deleteWhenDone = FALSE;
  UChar *bufferToDelete = 0;

  // clone our array, if necessary
  cloneArrayIfNeeded();

  // pin the indices to legal values
  pinIndices(start, length);

  // calculate the size of the string after the replace
  int32_t newSize = fLength - length + srcLength;

  // allocate a bigger array if needed
  if( newSize > getCapacity() ) {

    // allocate at minimum the current capacity + needed space
    int32_t tempLength;
    UChar *temp = allocate(fCapacity + srcLength, tempLength);
    if(! temp) {
      setToBogus();
      return *this;
    }

    // if we're not currently ref counted, shift the array right by one
    if(fRefCounted == FALSE)
      us_arrayCopy(fArray, 0, temp, 1, fLength);
    // otherwise, copy the old array into temp, including the ref count
    else
      us_arrayCopy(fArray, 0, temp, 0, fLength + 1);

    // delete the old array if we were ref counted
    if(fRefCounted && removeRef() == 0) {
      // if the srcChars array is the same as this object's array,
      // don't delete it until the end of the method.  this can happen
      // in code like UnicodeString s = "foo"; s += s;
      if(srcChars != getArrayStart())
    delete [] fArray;
      else {
    deleteWhenDone = TRUE;
    bufferToDelete = fArray;
      }
    }

    // use the new array
    fCapacity = tempLength;
    fArray = temp;
    setRefCount(1);
  }

  // now do the replace

  // first copy the portion that isn't changing, leaving a hole
  us_arrayCopy(getArrayStart(), start + length,
          getArrayStart(), start + srcLength,
          fLength - (start + length));

  // now fill in the hole with the new string
  us_arrayCopy(srcChars, srcStart, getArrayStart(), start, srcLength);

  fLength = newSize;
  fHashCode = kInvalidHashCode;

  if(deleteWhenDone)
    delete [] bufferToDelete;

  return *this;
}

UnicodeString&
UnicodeString::doReverse(UTextOffset start,
             int32_t length)
{
  // if we're bogus, do nothing
  if(fBogus)
    return *this;

  // clone our array, if necessary
  cloneArrayIfNeeded();

  // pin the indices to legal values
  pinIndices(start, length);

  UChar *left = getArrayStart() + start;
  UChar *right = getArrayStart() + start + length;
  UChar swap;

  while(left < --right) {
    swap = *left;
    *left++ = *right;
    *right = swap;
  }

  fHashCode = kInvalidHashCode;

  return *this;
}

//========================================
// Hashing
//========================================
int32_t
UnicodeString::doHashCode()
{
  const UChar *key     = getArrayStart();
  int32_t len         = fLength;
  int32_t hash         = kInvalidHashCode;
  const UChar *limit     = key + len;
  int32_t inc         = (len >= 128 ? len/64 : 1);

  /*
    We compute the hash by iterating sparsely over 64 (at most)
    characters spaced evenly through the string.  For each character,
    we multiply the previous hash value by a prime number and add the
    new character in, in the manner of an additive linear congruential
    random number generator, thus producing a pseudorandom
    deterministic value which should be well distributed over the
    output range. [LIU] */

  while(key < limit) {
    hash = (hash * 37) + *key;
    key += inc;
  }

  if(hash == kInvalidHashCode)
    hash = kEmptyHashCode;

  fHashCode = hash;
  return fHashCode;
}

//========================================
// Bogusify?
//========================================
void
UnicodeString::setToBogus()
{
  fBogus = TRUE;
  if(fRefCounted) {
    if(removeRef() == 0)
      delete [] fArray;

    fArray = 0;
    fCapacity = fLength = 0;
  }

  fHashCode = kInvalidHashCode;
}

//========================================
// Codeset conversion
//========================================
int32_t
UnicodeString::extract(UTextOffset start,
               int32_t length,
               char *dst,
               const char *codepage) const
{
  // if we're bogus or there's nothing to convert, do nothing
  if(fBogus || length == 0)
    return 0;

  // pin the indices to legal values
  pinIndices(start, length);

  int32_t convertedLen = 0;

  // set up the conversion parameters
  int32_t sourceLen        = length;
  const UChar *mySource    = getArrayStart() + start;
  const UChar *mySourceEnd = mySource + length;
  char *myTarget           = dst;
  UErrorCode status        = ZERO_ERROR;
  int32_t arraySize        = 0x0FFFFFFF;

  // create the converter
  UConverter *converter = 0;

  // if the codepage is the default, use our cache
  if(codepage == 0)
    converter = getDefaultConverter(status);
  else
    converter = ucnv_open(codepage, &status);

  // if we failed, set the appropriate flags and return
  if(FAILURE(status)) {
    // close the converter
    if(codepage == 0)
      releaseDefaultConverter(converter);
    else
      ucnv_close(converter);
    return 0;
  }

  // perform the conversion
  // there is no loop here since we assume the buffer is large enough

  ucnv_fromUnicode(converter, &myTarget,  myTarget + arraySize,
           &mySource, mySourceEnd, NULL, TRUE, &status);

  // close the converter
  if(codepage == 0)
    releaseDefaultConverter(converter);
  else
    ucnv_close(converter);

  return (myTarget - dst);
}

void
UnicodeString::doCodepageCreate(const char *codepageData,
                int32_t dataLength,
                const char *codepage)
{
  // if there's nothing to convert, do nothing
  if(codepageData == 0 || dataLength == 0)
    return;

  // set up the conversion parameters
  int32_t sourceLen        = dataLength;
  const char *mySource     = codepageData;
  const char *mySourceEnd  = mySource + sourceLen;
  UChar *myTarget          = getArrayStart();
  UErrorCode status        = ZERO_ERROR;
  int32_t arraySize        = getCapacity();

  // create the converter
  UConverter *converter = 0;

  // if the codepage is the default, use our cache
  converter = (codepage == 0
           ? getDefaultConverter(status)
           : ucnv_open(codepage, &status));

  // if we failed, set the appropriate flags and return
  if(FAILURE(status)) {
    // close the converter
    if(codepage == 0)
      releaseDefaultConverter(converter);
    else
      ucnv_close(converter);
    setToBogus();
    return;
  }

  // perform the conversion
  do {
    // reset the error code
    status = ZERO_ERROR;

    // perform the conversion
    ucnv_toUnicode(converter, &myTarget,  myTarget + arraySize,
           &mySource, mySourceEnd, NULL, TRUE, &status);

    // update the conversion parameters
    fLength      = myTarget - getArrayStart();
    arraySize    = getCapacity() - fLength;

    // allocate more space and copy data, if needed
    if(fLength < dataLength) {
      int32_t tempCapacity;
      UChar *temp = allocate(fCapacity, tempCapacity);

      if(! temp) {
    // close the converter
    if(codepage == 0)
      releaseDefaultConverter(converter);
    else
      ucnv_close(converter);
    // set flags and return
    setToBogus();
    return;
      }

      // if we're not currently ref counted, shift the array right by one
      if(fRefCounted == FALSE)
    us_arrayCopy(fArray, 0, temp, 1, fLength);
      // otherwise, copy the old array into temp, including the ref count
      else
          us_arrayCopy(fArray, 0, temp, 0, fLength + 1);

      if(fRefCounted && removeRef() == 0)
    delete [] fArray;

      fArray      = temp;
      fCapacity   = tempCapacity;

      setRefCount(1);

      myTarget    = getArrayStart() + fLength;
      arraySize   = getCapacity() - fLength;
    }
  }
  while(status == INDEX_OUTOFBOUNDS_ERROR);

  fHashCode = kInvalidHashCode;

  // close the converter
  if(codepage == 0)
    releaseDefaultConverter(converter);
  else
    ucnv_close(converter);
}

//========================================
// External Buffer
//========================================
UnicodeString::UnicodeString(UChar *buff,
                 int32_t bufLength,
                 int32_t buffCapacity)
  : fArray(buff),
    fLength(bufLength),
    fCapacity(buffCapacity),
    fRefCounted(FALSE),
    fHashCode(kInvalidHashCode),
    fBogus(FALSE)
{}

const UChar*
UnicodeString::getUChars() const
{
  // if we're bogus, do nothing
  if(fBogus)
    return 0;

  // clone our array, if necessary
  ((UnicodeString*)this)->cloneArrayIfNeeded();

  // no room for null, resize
  if(getCapacity() <= fLength) {
    // allocate at minimum the current capacity + needed space
    int32_t tempLength;
    UChar *temp = allocate(fCapacity + 1, tempLength);
    if(! temp) {
      ((UnicodeString*)this)->setToBogus();
      return 0;
    }

    // if we're not currently ref counted, shift the array right by one
    if(fRefCounted == FALSE)
      us_arrayCopy(fArray, 0, temp, 1, fLength);
    // otherwise, copy the old array into temp, including the ref count
    else
      us_arrayCopy(fArray, 0, temp, 0, fLength + 1);

    // delete the old array
    if(fRefCounted && ((UnicodeString*)this)->removeRef() == 0)
      delete [] ((UnicodeString*)this)->fArray;

    // use the new array
    ((UnicodeString*)this)->fCapacity = tempLength;
    ((UnicodeString*)this)->fArray    = temp;
    ((UnicodeString*)this)->setRefCount(1);
  }

  // tack on a trailing null
  fArray[(fRefCounted ? 1 : 0) + fLength] = 0;

  return getArrayStart();
}

UChar*
UnicodeString::orphanStorage()
{
  // if we're bogus, do nothing
  if(fBogus)
    return 0;

  // clone our array, if necessary
  ((UnicodeString*)this)->cloneArrayIfNeeded();

  // if we're ref counted, get rid of the leading ref count
  if(fRefCounted) {
    us_arrayCopy(getArrayStart(), 0, fArray, 0, fLength);
  }

  UChar *retVal = fArray;

  fArray = fStackBuffer;
  fLength = 0;
  fCapacity = US_STACKBUF_SIZE;
  fHashCode = kEmptyHashCode;

  return retVal;
}

//========================================
// Miscellaneous
//========================================
void
UnicodeString::pinIndices(UTextOffset& start,
              int32_t& length) const
{
  // pin indices
  if(length < 0 || start < 0)
    start = length = 0;
  else {
    if(start < 0)
      start = 0;
    if(length > (fLength - start))
      length = (fLength - start);
  }
}

void
UnicodeString::cloneArrayIfNeeded()
{
  // if we're ref counted, make a copy of the buffer if necessary
  if(fRefCounted && refCount() > 1) {
    UChar *copy = new UChar [ fCapacity ];
    if( ! copy ) {
      setToBogus();
      return;
    }

    // copy the current shared array into our new array
    us_arrayCopy(fArray, 0, copy, 0, fLength + 1);

    // remove a reference from the current shared array
    // if there are no more references to the current shared array,
    // after we remove the reference, delete the array
    if(removeRef() == 0)
      delete [] fArray;

    // make our array point to the new copy and set the ref count to one
    fArray = copy;
    setRefCount(1);
  }
}

// private function for C API
C_FUNC const UChar*
T_UnicodeString_getUChars(const UnicodeString *s)
{
  return s->getUChars();
}


//========================================
// Default converter caching
//========================================

UConverter*
UnicodeString::getDefaultConverter(UErrorCode &status)
{
  UConverter *converter = 0;

  if(fgDefaultConverter != 0) {
    Mutex lock;

    // need to check to make sure it wasn't taken out from under us
    if(fgDefaultConverter != 0) {
      converter = fgDefaultConverter;
      fgDefaultConverter = 0;
    }
  }

  // if the cache was empty, create a converter
  if(converter == 0) {
    converter = ucnv_open(0, &status);
    if(FAILURE(status))
      return 0;
  }

  return converter;
}

void
UnicodeString::releaseDefaultConverter(UConverter *converter)
{
  if(fgDefaultConverter == 0) {
    Mutex lock;

    if(fgDefaultConverter == 0) {
      fgDefaultConverter = converter;
      converter = 0;
    }
  }

  // it's safe to close a NULL converter
  ucnv_close(converter);
}

//========================================
// Streaming (to be removed)
//========================================

#include <iostream.h>
#include "unistrm.h"
#include "filestrm.h"


inline uint8_t
icu_hibyte(uint16_t x)
{ return (uint8_t)(x >> 8); }

inline uint8_t
icu_lobyte(uint16_t x)
{ return (uint8_t)(x & 0xff); }

inline uint16_t
icu_hiword(uint32_t x)
{ return (uint16_t)(x >> 16); }

inline uint16_t
icu_loword(uint32_t x)
{ return (uint16_t)(x & 0xffff); }

inline void
writeLong(FileStream *os,
      int32_t x)
{
  uint16_t word = icu_hiword((uint32_t)x);
  T_FileStream_putc(os, icu_hibyte(word));
  T_FileStream_putc(os, icu_lobyte(word));
  word = icu_loword((uint32_t)x);
  T_FileStream_putc(os, icu_hibyte(word));
  T_FileStream_putc(os, icu_lobyte(word));
}

inline int32_t
readLong(FileStream *is)
{
  int32_t x = 0;
  uint16_t byte;

  byte = T_FileStream_getc(is);
  x |= byte;
  byte = T_FileStream_getc(is);
  x = (x << 8) | byte;
  byte = T_FileStream_getc(is);
  x = (x << 8) | byte;
  byte = T_FileStream_getc(is);
  x = (x << 8) | byte;

  return x;
}

inline void
writeUChar(FileStream *os,
       UChar c)
{
  T_FileStream_putc(os, icu_hibyte(c));
  T_FileStream_putc(os, icu_lobyte(c));
}

inline UChar
readUChar(FileStream *is)
{
  UChar c = 0;
  uint16_t byte;

  byte = T_FileStream_getc(is);
  c |= byte;
  byte = T_FileStream_getc(is);
  c = (c << 8) | byte;

  return c;
}

void
UnicodeStringStreamer::streamOut(const UnicodeString *s,
                 FileStream *os)
{
  if(!T_FileStream_error(os))
    writeLong(os, s->fLength);

  const UChar *c   = s->getArrayStart();
  const UChar *end = c + s->fLength;

  while(c != end && ! T_FileStream_error(os))
    writeUChar(os, *c++);
}

void
UnicodeStringStreamer::streamIn(UnicodeString *s,
                FileStream *is)
{
  int32_t newSize;

  // handle error conditions
  if(T_FileStream_error(is) || T_FileStream_eof(is)) {
    s->setToBogus();
    return;
  }
  newSize = readLong(is);
  if((newSize < 0) || T_FileStream_error(is)
     || ((newSize > 0) && T_FileStream_eof(is))) {
    s->setToBogus(); //error condition
    return;
  }

  // clone s's array, if needed
  s->cloneArrayIfNeeded();

  // if the string isn't big enough to hold the data, enlarge it
  if(s->getCapacity() < newSize) {

    int32_t tempLength;
    UChar *temp = s->allocate(newSize, tempLength);
    if(! temp) {
      s->setToBogus();
      return;
    }

    // if s is not currently ref counted, shift the array right by one
    if(s->fRefCounted == FALSE)
      us_arrayCopy(s->fArray, 0, temp, 1, s->fLength);
    // otherwise, copy the old array into temp, including the ref count
    else
      us_arrayCopy(s->fArray, 0, temp, 0, s->fLength + 1);

    // delete the old array if s is ref counted
    if(s->fRefCounted && s->removeRef() == 0)
      delete [] s->fArray;

    // use the new array
    s->fCapacity = tempLength;
    s->fArray    = temp;
    s->setRefCount(1);
  }

  UChar *c = s->getArrayStart();
  UChar *end = c + newSize;

  while(c < end && ! (T_FileStream_error(is) || T_FileStream_eof(is)))
    *c++ = readUChar(is);

  // couldn't read all chars
  if(c < end) {
    s->setToBogus();
    return;
  }

  s->fLength = newSize;
}

// console IO

ostream&
operator<<(ostream& stream,
       const UnicodeString& s)
{
  UTextOffset i;
  UChar c;
  int32_t saveFlags = stream.flags();

  stream << hex;

  for(i = 0; i < s.length(); i++) {
    c = s.charAt(i);
    if((c >= ' ' && c <= '~') || c == '\n')
      stream << (char)c;
    else
      stream << "[0x" << c << "]";
  }
  stream.flush();
  stream.setf(saveFlags & ios::basefield, ios::basefield);
  return stream;
}


