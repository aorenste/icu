/*
 * @(#)ContextualSubstitutionSubtables.h	1.7 00/03/15
 *
 * (C) Copyright IBM Corp. 1998, 1999, 2000 - All Rights Reserved
 *
 */

#ifndef __CONTEXTUALSUBSTITUTIONSUBTABLES_H
#define __CONTEXTUALSUBSTITUTIONSUBTABLES_H

#include "LETypes.h"
#include "LEFontInstance.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "GlyphIterator.h"
#include "LookupProcessor.h"

struct SubstitutionLookupRecord
{
    le_uint16  sequenceIndex;
    le_uint16  lookupListIndex;
};

struct ContextualSubstitutionBase : GlyphSubstitutionSubtable
{
    static le_bool matchGlyphIDs(
        LEGlyphID *glyphArray, le_uint16 glyphCount, GlyphIterator *glyphIterator);

    static le_bool matchGlyphClasses(
        le_uint16 *classArray, le_uint16 glyphCount, GlyphIterator *glyphIterator,
        ClassDefinitionTable *classDefinitionTable);

    static le_bool matchGlyphCoverages(
        Offset *coverageTableOffsetArray, le_uint16 glyphCount,
        GlyphIterator *glyphIterator, char *offsetBase);

    static void applySubstitutionLookups(
        LookupProcessor *lookupProcessor, 
        SubstitutionLookupRecord *substLookupRecordArray,
        le_uint16 substCount,
        GlyphIterator *glyphIterator,
        const LEFontInstance *fontInstance,
        le_int32 position);
};

struct ContextualSubstitutionSubtable : ContextualSubstitutionBase
{
    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

struct ContextualSubstitutionFormat1Subtable : ContextualSubstitutionSubtable
{
    le_uint16  subRuleSetCount;
    Offset  subRuleSetTableOffsetArray[ANY_NUMBER];

    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

struct SubRuleSetTable
{
    le_uint16  subRuleCount;
    Offset  subRuleTableOffsetArray[ANY_NUMBER];

};

// NOTE: Multiple variable size arrays!!
struct SubRuleTable
{
    le_uint16  glyphCount;
    le_uint16  substCount;
    LEGlyphID inputGlyphArray[ANY_NUMBER];
  //SubstitutionLookupRecord substLookupRecordArray[ANY_NUMBER];
};

struct ContextualSubstitutionFormat2Subtable : ContextualSubstitutionSubtable
{
    Offset  classDefTableOffset;
    le_uint16  subClassSetCount;
    Offset  subClassSetTableOffsetArray[ANY_NUMBER];

    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

struct SubClassSetTable
{
    le_uint16  subClassRuleCount;
    Offset  subClassRuleTableOffsetArray[ANY_NUMBER];
};

// NOTE: Multiple variable size arrays!!
struct SubClassRuleTable
{
    le_uint16  glyphCount;
    le_uint16  substCount;
    le_uint16  classArray[ANY_NUMBER];
  //SubstitutionLookupRecord substLookupRecordArray[ANY_NUMBER];
};

// NOTE: This isn't a subclass of GlyphSubstitutionSubtable 'cause
// it has an array of coverage tables instead of a single coverage table...
//
// NOTE: Multiple variable size arrays!!
struct ContextualSubstitutionFormat3Subtable
{
    le_uint16  substFormat;
    le_uint16  glyphCount;
    le_uint16  substCount;
    Offset  coverageTableOffsetArray[ANY_NUMBER];
  //SubstitutionLookupRecord substLookupRecord[ANY_NUMBER];

    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

struct ChainingContextualSubstitutionSubtable : ContextualSubstitutionBase
{
    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

struct ChainingContextualSubstitutionFormat1Subtable : ChainingContextualSubstitutionSubtable
{
    le_uint16  chainSubRuleSetCount;
    Offset  chainSubRuleSetTableOffsetArray[ANY_NUMBER];

    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

struct ChainSubRuleSetTable
{
    le_uint16  chainSubRuleCount;
    Offset  chainSubRuleTableOffsetArray[ANY_NUMBER];

};

// NOTE: Multiple variable size arrays!!
struct ChainSubRuleTable
{
    le_uint16  backtrackGlyphCount;
    LEGlyphID backtrackGlyphArray[ANY_NUMBER];
  //le_uint16  inputGlyphCount;
  //LEGlyphID inputGlyphArray[ANY_NUMBER];
  //le_uint16  lookaheadGlyphCount;
  //LEGlyphID lookaheadGlyphArray[ANY_NUMBER];
  //le_uint16  substCount;
  //SubstitutionLookupRecord substLookupRecordArray[ANY_NUMBER];
};

struct ChainingContextualSubstitutionFormat2Subtable : ChainingContextualSubstitutionSubtable
{
    Offset  backtrackClassDefTableOffset;
    Offset  inputClassDefTableOffset;
    Offset  lookaheadClassDefTableOffset;
    le_uint16  chainSubClassSetCount;
    Offset  chainSubClassSetTableOffsetArray[ANY_NUMBER];

    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

struct ChainSubClassSetTable
{
    le_uint16  chainSubClassRuleCount;
    Offset  chainSubClassRuleTableOffsetArray[ANY_NUMBER];
};

// NOTE: Multiple variable size arrays!!
struct ChainSubClassRuleTable
{
    le_uint16  backtrackGlyphCount;
    le_uint16  backtrackClassArray[ANY_NUMBER];
  //le_uint16  inputGlyphCount;
  //le_uint16  inputClassArray[ANY_NUMBER];
  //le_uint16  lookaheadGlyphCount;
  //le_uint16  lookaheadClassArray[ANY_NUMBER];
  //le_uint16  substCount;
  //SubstitutionLookupRecord substLookupRecordArray[ANY_NUMBER];
};

// NOTE: This isn't a subclass of GlyphSubstitutionSubtable 'cause
// it has arrays of coverage tables instead of a single coverage table...
//
// NOTE: Multiple variable size arrays!!
struct ChainingContextualSubstitutionFormat3Subtable
{
    le_uint16  substFormat;
    le_uint16  backtrackGlyphCount;
    Offset  backtrackCoverageTableOffsetArray[ANY_NUMBER];
  //le_uint16  inputGlyphCount;
  //Offset  inputCoverageTableOffsetArray[ANY_NUMBER];
  //le_uint16  lookaheadGlyphCount;
  //le_uint16  lookaheadCoverageTableOffsetArray[ANY_NUMBER];
  //le_uint16  substCount;
  //SubstitutionLookupRecord substLookupRecord[ANY_NUMBER];

    le_uint32  process(LookupProcessor *lookupProcessor, GlyphIterator *glyphIterator, const LEFontInstance *fontInstance);
};

#endif
