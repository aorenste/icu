/*
*****************************************************************************************
*                                                                                       *
* COPYRIGHT:                                                                            *
*   (C) Copyright International Business Machines Corporation,  1998                    *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.                  *
*   US Government Users Restricted Rights - Use, duplication, or disclosure             *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                              *
*                                                                                       *
*****************************************************************************************
*/

#include "tsputil.h"

#include <float.h> // DBL_MAX, DBL_MIN

#define CASE(id,test) case id: name = #test; if (exec) { logln(#test "---"); logln((UnicodeString)""); test(); } break;

void 
PUtilTest::runIndexedTest( int32_t index, bool_t exec, char* &name, char* par )
{
    //if (exec) logln("TestSuite PUtilTest: ");
    switch (index) {

        CASE(0, testIEEEremainder)
        CASE(1, testMaxMin)

        default: name = ""; break; //needed to end loop
    }
}

void
PUtilTest::testIEEEremainder()
{
    double    pinf        = icu_getInfinity();
    double    ninf        = -icu_getInfinity();
    double    nan            = icu_getNaN();
    double    pzero        = 0.0;
    double    nzero        = 0.0;

    nzero *= -1;

    // simple remainder checks
    remainderTest(7.0, 2.5, -0.5);
    remainderTest(7.0, -2.5, -0.5);
    remainderTest(-7.0, 2.5, 0.5);
    remainderTest(-7.0, -2.5, 0.5);
    remainderTest(5.0, 3.0, -1.0);
    
    // this should work
    //remainderTest(43.7, 2.5, 1.25);

    /*

    // infinity and real
    remainderTest(pinf, 1.0, 1.25);
    remainderTest(1.0, pinf, 1.0);
    remainderTest(ninf, 1.0, 1.25);
    remainderTest(1.0, ninf, 1.0);

    // test infinity and nan
    remainderTest(ninf, pinf, 1.25);
    remainderTest(ninf, nan, 1.25);
    remainderTest(pinf, nan, 1.25);

    // test infinity and zero
    remainderTest(pinf, pzero, 1.25);
    remainderTest(pinf, nzero, 1.25);
    remainderTest(ninf, pzero, 1.25);
    remainderTest(ninf, nzero, 1.25);
*/
}

void
PUtilTest::remainderTest(double x, double y, double exp)
{
    double result = icu_IEEEremainder(x,y);

    if(        icu_isNaN(result) && 
        ! ( icu_isNaN(x) || icu_isNaN(y))) {
        errln(UnicodeString("FAIL: got NaN as result without NaN as argument"));
        errln(UnicodeString("      IEEEremainder(") + x + ", " + y + ") is " + result + ", expected " + exp);
    }
    else if(result != exp)
        errln(UnicodeString("FAIL: IEEEremainder(") + x + ", " + y + ") is " + result + ", expected " + exp);
    else
        logln(UnicodeString("OK: IEEEremainder(") + x + ", " + y + ") is " + result);

}

void
PUtilTest::testMaxMin()
{
    double    pinf        = icu_getInfinity();
    double    ninf        = -icu_getInfinity();
    double    nan        = icu_getNaN();
    double    pzero        = 0.0;
    double    nzero        = 0.0;

    nzero *= -1;

    // +Inf with -Inf
    maxMinTest(pinf, ninf, pinf, TRUE);
    maxMinTest(pinf, ninf, ninf, FALSE);

    // +Inf with +0 and -0
    maxMinTest(pinf, pzero, pinf, TRUE);
    maxMinTest(pinf, pzero, pzero, FALSE);
    maxMinTest(pinf, nzero, pinf, TRUE);
    maxMinTest(pinf, nzero, nzero, FALSE);

    // -Inf with +0 and -0
    maxMinTest(ninf, pzero, pzero, TRUE);
    maxMinTest(ninf, pzero, ninf, FALSE);
    maxMinTest(ninf, nzero, nzero, TRUE);
    maxMinTest(ninf, nzero, ninf, FALSE);

    // NaN with +Inf and -Inf
    maxMinTest(pinf, nan, nan, TRUE);
    maxMinTest(pinf, nan, nan, FALSE);
    maxMinTest(ninf, nan, nan, TRUE);
    maxMinTest(ninf, nan, nan, FALSE);

    // NaN with NaN
    maxMinTest(nan, nan, nan, TRUE);
    maxMinTest(nan, nan, nan, FALSE);

    // NaN with +0 and -0
    maxMinTest(nan, pzero, nan, TRUE);
    maxMinTest(nan, pzero, nan, FALSE);
    maxMinTest(nan, nzero, nan, TRUE);
    maxMinTest(nan, nzero, nan, FALSE);

    // +Inf with DBL_MAX and DBL_MIN
    maxMinTest(pinf, DBL_MAX, pinf, TRUE);
    maxMinTest(pinf, -DBL_MAX, pinf, TRUE);
    maxMinTest(pinf, DBL_MIN, pinf, TRUE);
    maxMinTest(pinf, -DBL_MIN, pinf, TRUE);
    maxMinTest(pinf, DBL_MIN, DBL_MIN, FALSE);
    maxMinTest(pinf, -DBL_MIN, -DBL_MIN, FALSE);
    maxMinTest(pinf, DBL_MAX, DBL_MAX, FALSE);
    maxMinTest(pinf, -DBL_MAX, -DBL_MAX, FALSE);

    // -Inf with DBL_MAX and DBL_MIN
    maxMinTest(ninf, DBL_MAX, DBL_MAX, TRUE);
    maxMinTest(ninf, -DBL_MAX, -DBL_MAX, TRUE);
    maxMinTest(ninf, DBL_MIN, DBL_MIN, TRUE);
    maxMinTest(ninf, -DBL_MIN, -DBL_MIN, TRUE);
    maxMinTest(ninf, DBL_MIN, ninf, FALSE);
    maxMinTest(ninf, -DBL_MIN, ninf, FALSE);
    maxMinTest(ninf, DBL_MAX, ninf, FALSE);
    maxMinTest(ninf, -DBL_MAX, ninf, FALSE);

    // +0 with DBL_MAX and DBL_MIN
    maxMinTest(pzero, DBL_MAX, DBL_MAX, TRUE);
    maxMinTest(pzero, -DBL_MAX, pzero, TRUE);
    maxMinTest(pzero, DBL_MIN, DBL_MIN, TRUE);
    maxMinTest(pzero, -DBL_MIN, pzero, TRUE);
    maxMinTest(pzero, DBL_MIN, pzero, FALSE);
    maxMinTest(pzero, -DBL_MIN, -DBL_MIN, FALSE);
    maxMinTest(pzero, DBL_MAX, pzero, FALSE);
    maxMinTest(pzero, -DBL_MAX, -DBL_MAX, FALSE);

    // -0 with DBL_MAX and DBL_MIN
    maxMinTest(nzero, DBL_MAX, DBL_MAX, TRUE);
    maxMinTest(nzero, -DBL_MAX, nzero, TRUE);
    maxMinTest(nzero, DBL_MIN, DBL_MIN, TRUE);
    maxMinTest(nzero, -DBL_MIN, nzero, TRUE);
    maxMinTest(nzero, DBL_MIN, nzero, FALSE);
    maxMinTest(nzero, -DBL_MIN, -DBL_MIN, FALSE);
    maxMinTest(nzero, DBL_MAX, nzero, FALSE);
    maxMinTest(nzero, -DBL_MAX, -DBL_MAX, FALSE);
}

void
PUtilTest::maxMinTest(double a, double b, double exp, bool_t max)
{
  double result = 0.0;
  
  if(max)
    result = icu_fmax(a, b);
  else
    result = icu_fmin(a, b);
  
  bool_t nanResultOK = (icu_isNaN(a) || icu_isNaN(b));
  
  if(icu_isNaN(result) && ! nanResultOK) {
    errln(UnicodeString("FAIL: got NaN as result without NaN as argument"));
    if(max)
      errln(UnicodeString("      max(") + a + ", " + b + ") is " + result + ", expected " + exp);
    else
      errln(UnicodeString("      min(") + a + ", " + b + ") is " + result + ", expected " + exp);
  }
  else if(result != exp && ! (icu_isNaN(result) || icu_isNaN(exp)))
    if(max)
      errln(UnicodeString("FAIL: max(") + a + ", " + b + ") is " + result + ", expected " + exp);
    else
      errln(UnicodeString("FAIL: min(") + a + ", " + b + ") is " + result + ", expected " + exp);
  else
    if(max)
      logln(UnicodeString("OK: max(") + a + ", " + b + ") is " + result);
    else
      logln(UnicodeString("OK: min(") + a + ", " + b + ") is " + result);
}
