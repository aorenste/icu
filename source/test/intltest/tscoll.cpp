/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1997-2001, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

/**
 * IntlTestCollator is the medium level test class for everything in the directory "collate".
 */

/***********************************************************************
* Modification history
* Date        Name        Description
* 02/14/2001  synwee      Compare with cintltst and commented away tests 
*                         that are not run.
***********************************************************************/

#include "unicode/utypes.h"
#include "tscoll.h"

#include "encoll.h"
#include "frcoll.h"
#include "decoll.h"
#include "dacoll.h"
#include "escoll.h"
#include "ficoll.h"
#include "jacoll.h"
#include "trcoll.h"
#include "allcoll.h"
#include "g7coll.h"
#include "mnkytst.h"
#include "apicoll.h"
#include "regcoll.h"
#include "currcoll.h"
#include "itercoll.h"
//#include "capicoll.h"   // CollationCAPITest
#include "tstnorm.h"
#include "normconf.h"
#include "thcoll.h"

#include "lcukocol.h"

void IntlTestCollator::runIndexedTest( int32_t index, UBool exec, const char* &name, char* par )
{
    if (exec)
    {
        logln("TestSuite Collator: ");
    }

    switch (index)
    {
    case 0:
        name = "CollationEnglishTest";

        if (exec)
        {
            logln("CollationEnglishtest---");
            logln("");
            CollationEnglishTest test;
            callTest( test, par );
        }
        break;

    case 1:
        name = "CollationFrenchTest";
        
        if (exec)
        {
            logln("CollationFrenchtest---");
            logln("");
            CollationFrenchTest test;
            callTest( test, par );
        }
        break;

    case 2:
        name = "CollationGermanTest";
        
        if (exec)
        {
            logln("CollationGermantest---");
            logln("");

            CollationGermanTest test;
            callTest( test, par );
        }
        break;

    case 3:
        name = "CollationDanishTest";

        if (exec)
        {
            logln("CollationDanishtest---");
            logln("");
            
            CollationDanishTest test;
            callTest( test, par );
        }
        break;

    case 4:
        name = "CollationSpanishTest";
        
        if (exec)
        {
            logln("CollationSpanishtest---");
            logln("");

            CollationSpanishTest test;
            callTest( test, par );
        }
        break;

    case 5:
        name = "CollationFinnishTest"; 

        if (exec)
        {
            logln("CollationFinnishtest---");
            
            CollationFinnishTest test;
            callTest( test, par ); 
        }
        break;

    case 6:
        name = "CollationKanaTest"; 

        if (exec)
        {
            logln("CollationKanatest---");
            
            CollationKanaTest test;
            callTest( test, par );
        }
        break;

    case 7:
        name = "CollationTurkishTest";
        
        if (exec)
        {
            logln("CollationTurkishtest---");
            logln("");

            CollationTurkishTest test;
            callTest( test, par );
        }
        break;

    case 8:
        name = "CollationDummyTest"; 

        if (exec)
        {
            logln("CollationDummyTest---");
            
            CollationDummyTest test;
            callTest( test, par );
        }
        break;

    case 9:
        name = "G7CollationTest";
        
        if (exec)
        {
            logln("G7CollationTest---");
            
            G7CollationTest test;
            callTest( test, par );
        }
        break;

    case 10:
        name = "CollationMonkeyTest";
        
        if (exec)
        {
            logln("CollationMonkeyTest---");
            
            CollationMonkeyTest test;
            callTest( test, par );
        }
        break;

    case 11:
        name = "CollationAPITest";
        
        if (exec)
        {
            logln("CollationAPITest---");
            logln("");
            
            CollationAPITest test;
            callTest( test, par );
        }
        break;

    case 12:
        name = "CollationRegressionTest"; 

        if (exec)
        {
            logln("CollationRegressionTest---");
            
            CollationRegressionTest test;
            callTest( test, par );
        }
        break;

    case 13:
        name = "CollationCurrencyTest"; 

        if (exec)
        {
            logln("CollationCurrencyTest---");
            logln("");
            
            CollationCurrencyTest test;
            callTest( test, par );
        }
        break;

    case 14:
        name = "CollationIteratorTest"; 

        if (exec)
        {
            logln("CollationIteratorTest---");
            
            CollationIteratorTest test;
            callTest( test, par );
        }
        break;

    case 15: 
      /*        name = "CollationCAPITest"; 
        if (exec) {
            logln("Collation C API test---"); logln("");
            CollationCAPITest test;
            callTest( test, par );
        }
        break;

    case 16: */
        name = "BasicNormalizerTest"; 
        if (exec) {
            logln("BasicNormalizerTest---"); 
            logln("");
            BasicNormalizerTest test;
            callTest( test, par );
        }
        break;

    case 16:
        name = "NormalizerConformanceTest"; 
        if (exec) {
            logln("NormalizerConformanceTest---"); logln("");
            NormalizerConformanceTest test;
            callTest( test, par );
        }
        break;

    case 17:
        name = "CollationThaiTest"; 
        if (exec) {
            logln("CollationThaiTest---"); 
            
            CollationThaiTest test;
            callTest( test, par );
        }
        break;

		case 18: //all
		name = "LotusCollationTest";

		name = "LotusCollationKoreanTest"; 
        if (exec) {
            logln("LotusCollationKoreanTest---"); logln("");
            LotusCollationKoreanTest test;
            callTest( test, par );
        }
        break;

    default: name = ""; break;
    }
}

