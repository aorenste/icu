/*
**********************************************************************
*   Copyright (C) 1999 IBM and others. All rights reserved.
**********************************************************************
*   Date        Name        Description
*   12/1/99     rtg         Ported from Java
**********************************************************************
*/

#include "brkdict.h"

//=================================================================================
// deserialization
//=================================================================================

BreakDictionary::BreakDictionary(char* dictionaryFilename)
{
    FileStream* dictionaryStream = T_FileStream_open(dictionaryFilename, "rb");
    readDictionaryFile(dictionaryStream);
    T_FileStream_close(dictionaryStream);
}

BreakDictionary::~BreakDictionary()
{
    ucmp8_close(columnMap);
    delete [] table;
    delete [] rowIndex;
    delete [] rowIndexFlags;
    delete [] rowIndexFlagsIndex;
    delete [] rowIndexShifts;
}

// macros to support readDictionaryFile.  The data files originated from a Java
// program, and Java always writes data out in big-endian format.  These macros will
// byte-swap the data for appropriate use on Windows.
#define SWAP32(x) x = (x >> 24 & 0xff) | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) | (x << 24 & 0xff000000)
#define SWAP16(x) x = (x << 8 & 0xff00) | (x >> 8 & 0xff)

#include <stdio.h>

void
BreakDictionary::readDictionaryFile(FileStream* in)
{
    int32_t l;
    int32_t version;

	int i;

    // read in the version number (right now we just ignore it)
    T_FileStream_read(in, &version, 4);

    // read in the column map (this is serialized in its internal form:
    // an index array followed by a data array)
    T_FileStream_read(in, &l, 4);
    SWAP32(l);
    uint16_t* temp = new uint16_t[l];
    T_FileStream_read(in, temp, l * sizeof (int16_t) );
    for (i = 0; i < l; i++) {
        SWAP16(temp[i]);
    }
    T_FileStream_read(in, &l, 4);
    SWAP32(l);
    int8_t* temp2 = new int8_t[l];
    T_FileStream_read(in, temp2, l);
    columnMap = ucmp8_openAdopt(temp, temp2, l);

    // read in numCols and numColGroups
    T_FileStream_read(in, &numCols, 4);
    SWAP32(numCols);
    T_FileStream_read(in, &numColGroups, 4);
    SWAP32(numColGroups);

    // read in the row-number index
    T_FileStream_read(in, &l, 4);
    SWAP32(l);
    rowIndex = new int16_t[l];
    T_FileStream_read(in, rowIndex, l * sizeof (int16_t) );
    for (i = 0; i < l; i++) {
        SWAP16(rowIndex[i]);
    }

    // load in the populated-cells bitmap: index first, then bitmap list
    T_FileStream_read(in, &l, 4);
    SWAP32(l);
    rowIndexFlagsIndex = new int16_t[l];
    T_FileStream_read(in, rowIndexFlagsIndex, l * sizeof(int16_t) );
    for (i = 0; i < l; i++) {
        SWAP16(rowIndexFlagsIndex[i]);
    }
    T_FileStream_read(in, &l, 4);
    SWAP32(l);
    rowIndexFlags = new int32_t[l];
    T_FileStream_read(in, rowIndexFlags, l * sizeof(int32_t));
    for (i = 0; i < l; i++) {
        SWAP32(rowIndexFlags[i]);
    }

    // load in the row-shift index
    T_FileStream_read(in, &l, 4);
    SWAP32(l);
    rowIndexShifts = new int8_t[l];
    T_FileStream_read(in, rowIndexShifts, l);

    // finally, load in the actual state table
    T_FileStream_read(in, &l, 4);
    SWAP32(l);
    table = new int16_t[l];
    T_FileStream_read(in, table, l * sizeof(int16_t) );
    for (i = 0; i < l; i++) {
        SWAP16(table[i]);
    }

    // the reverse column map occurs next in the file.  In the C/C++ code, for the
    // time being, we're not going to worry about that.
}

//=================================================================================
// access to the words
//=================================================================================

/**
 * Uses the column map to map the character to a column number, then
 * passes the row and column number to the other version of at()
 * @param row The current state
 * @param ch The character whose column we're interested in
 * @return The new state to transition to
 */
int16_t
BreakDictionary::at(int32_t row, UChar ch) const
{
    int16_t col = ucmp8_get(columnMap, ch);
    return at(row, (int32_t)col);
}

/**
 * Returns the value in the cell with the specified (logical) row and
 * column numbers.  In DictionaryBasedBreakIterator, the row number is
 * a state number, the column number is an input, and the return value
 * is the row number of the new state to transition to.  (0 is the
 * "error" state, and -1 is the "end of word" state in a dictionary)
 * @param row The row number of the current state
 * @param col The column number of the input character (0 means "not a
 * dictionary character")
 * @return The row number of the new state to transition to
 */
int16_t
BreakDictionary::at(int32_t row, int32_t col) const
{
    if (cellIsPopulated(row, col)) {
        // we map from logical to physical row number by looking up the
        // mapping in rowIndex; we map from logical column number to
        // physical column number by looking up a shift value for this
        // logical row and offsetting the logical column number by
        // the shift amount.  Then we can use internalAt() to actually
        // get the value out of the table.
        return internalAt(rowIndex[row], col + rowIndexShifts[row]);
    }
    else {
        return 0;
    }
}

//=================================================================================
// implementation
//=================================================================================
/**
 * Given (logical) row and column numbers, returns true if the
 * cell in that position is populated
 */
bool_t
BreakDictionary::cellIsPopulated(int32_t row, int32_t col) const
{
    // look up the entry in the bitmap index for the specified row.
    // If it's a negative number, it's the column number of the only
    // populated cell in the row
    if (rowIndexFlagsIndex[row] < 0) {
        return col == -rowIndexFlagsIndex[row];
    }

    // if it's a positive number, it's the offset of an entry in the bitmap
    // list.  If the table is more than 32 columns wide, the bitmap is stored
    // successive entries in the bitmap list, so we have to divide the column
    // number by 32 and offset the number we got out of the index by the result.
    // Once we have the appropriate piece of the bitmap, test the appropriate
    // bit and return the result.
    else {
        int32_t flags = rowIndexFlags[rowIndexFlagsIndex[row] + (col >> 5)];
        return (flags & (1 << (col & 0x1f))) != 0;
    }
}

/**
 * Implementation of at() when we know the specified cell is populated.
 * @param row The PHYSICAL row number of the cell
 * @param col The PHYSICAL column number of the cell
 * @return The value stored in the cell
 */
int16_t
BreakDictionary::internalAt(int32_t row, int32_t col) const
{
    // the table is a one-dimensional array, so this just does the math necessary
    // to treat it as a two-dimensional array (we don't just use a two-dimensional
    // array because two-dimensional arrays are inefficient in Java)
    return table[row * numCols + col];
}

