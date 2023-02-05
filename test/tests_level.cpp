/*
MIT License

Copyright (c) 2022 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include <AUnit.h>
#include <levelraw.hpp>
#include <log.hpp>
#include <main.hpp>
#include <kegconfig.hpp>

RawLevelDetection raw(UnitIndex::U1, 1, 1, 1);
KegConfig myConfig("TEST", "TEST");

test(level_raw) {
  float data[10] = { 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9 };
  float sum;

  // Check that we have the default values
  assertEqual(raw.hasRawValue(), false);
  assertEqual(raw.hasAverageValue(), false);

  raw.add( data[0], 0 );
  assertEqual(raw.getRawValue(), data[0]);
  assertEqual(raw.average(), data[0]);
  assertEqual(raw.sum(), data[0]);

  raw.add( data[1], 0 );
  sum = data[0] + data[1];
  assertEqual(raw.getRawValue(), data[1]);
  assertEqual(raw.average(), sum/2);
  assertEqual(raw.sum(), sum);

  raw.add( data[2], 0 );
  raw.add( data[3], 0 );
  raw.add( data[4], 0 );
  raw.add( data[5], 0 );
  raw.add( data[6], 0 );
  raw.add( data[7], 0 );
  sum = data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7];
  assertEqual(raw.getRawValue(), data[7]);
  assertEqual(raw.sum(), sum);

  // Store last 8 values, so data[0] should be dropped.
  raw.add( data[8], 0 );
  sum = data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7] + data[8];
  assertEqual(raw.getRawValue(), data[8]);
  assertEqual(raw.sum(), sum);
}

// EOF
