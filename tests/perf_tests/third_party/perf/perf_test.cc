// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "perf_test.h"

#include "../dmg_fp/dmg_fp.h"

#include <stdio.h>
#include <stdarg.h>
#include <vector>

#ifndef DCHECK
#define DCHECK
#endif

namespace {

namespace base {

template <typename STR, typename INT, typename UINT, bool NEG>
struct IntToStringT {
  // This is to avoid a compiler warning about unary minus on unsigned type.
  // For example, say you had the following code:
  //   template <typename INT>
  //   INT abs(INT value) { return value < 0 ? -value : value; }
  // Even though if INT is unsigned, it's impossible for value < 0, so the
  // unary minus will never be taken, the compiler will still generate a
  // warning.  We do a little specialization dance...
  template <typename INT2, typename UINT2, bool NEG2>
  struct ToUnsignedT {};

  template <typename INT2, typename UINT2>
  struct ToUnsignedT<INT2, UINT2, false> {
    static UINT2 ToUnsigned(INT2 value) {
      return static_cast<UINT2>(value);
    }
  };

  template <typename INT2, typename UINT2>
  struct ToUnsignedT<INT2, UINT2, true> {
    static UINT2 ToUnsigned(INT2 value) {
      return static_cast<UINT2>(value < 0 ? -value : value);
    }
  };

  // This set of templates is very similar to the above templates, but
  // for testing whether an integer is negative.
  template <typename INT2, bool NEG2>
  struct TestNegT {};
  template <typename INT2>
  struct TestNegT<INT2, false> {
    static bool TestNeg(INT2 value) {
      // value is unsigned, and can never be negative.
      return false;
    }
  };
  template <typename INT2>
  struct TestNegT<INT2, true> {
    static bool TestNeg(INT2 value) {
      return value < 0;
    }
  };

  static STR IntToString(INT value) {
    // log10(2) ~= 0.3 bytes needed per bit or per byte log10(2**8) ~= 2.4.
    // So round up to allocate 3 output characters per byte, plus 1 for '-'.
    const int kOutputBufSize = 3 * sizeof(INT) + 1;

    // Allocate the whole string right away, we will right back to front, and
    // then return the substr of what we ended up using.
    STR outbuf(kOutputBufSize, 0);

    bool is_neg = TestNegT<INT, NEG>::TestNeg(value);
    // Even though is_neg will never be true when INT is parameterized as
    // unsigned, even the presence of the unary operation causes a warning.
    UINT res = ToUnsignedT<INT, UINT, NEG>::ToUnsigned(value);

    typename STR::iterator it(outbuf.end());
    do {
      --it;
      DCHECK(it != outbuf.begin());
      *it = static_cast<typename STR::value_type>((res % 10) + '0');
      res /= 10;
    } while (res != 0);
    if (is_neg) {
      --it;
      DCHECK(it != outbuf.begin());
      *it = static_cast<typename STR::value_type>('-');
    }
    return STR(it, outbuf.end());
  }
};

std::string UintToString(unsigned int value) {
  return IntToStringT<std::string, unsigned int, unsigned int, false>::
    IntToString(value);
}

std::string DoubleToString(double value) {
  // According to g_fmt.cc, it is sufficient to declare a buffer of size 32.
  char buffer[32];
  dmg_fp::g_fmt(buffer, value);
  return std::string(buffer);
}

std::string FormatString(const char *fmt, va_list vararg)
{
    static std::vector<char> buffer(512);

    // Attempt to just print to the current buffer
    int len = vsnprintf(&buffer[0], buffer.size(), fmt, vararg);
    if (len < 0 || static_cast<size_t>(len) >= buffer.size())
    {
        // Buffer was not large enough, calculate the required size and resize the buffer
        len = vsnprintf(NULL, 0, fmt, vararg);
        buffer.resize(len + 1);

        // Print again
        vsnprintf(&buffer[0], buffer.size(), fmt, vararg);
    }

    return std::string(buffer.data(), len);
}

std::string StringPrintf(const char *fmt, ...)
{
    va_list vararg;
    va_start(vararg, fmt);
    std::string result = FormatString(fmt, vararg);
    va_end(vararg);
    return result;
}

}

std::string ResultsToString(const std::string& measurement,
                            const std::string& modifier,
                            const std::string& trace,
                            const std::string& values,
                            const std::string& prefix,
                            const std::string& suffix,
                            const std::string& units,
                            bool important) {
  // <*>RESULT <graph_name>: <trace_name>= <value> <units>
  // <*>RESULT <graph_name>: <trace_name>= {<mean>, <std deviation>} <units>
  // <*>RESULT <graph_name>: <trace_name>= [<value>,value,value,...,] <units>
  return base::StringPrintf("%sRESULT %s%s: %s= %s%s%s %s\n",
         important ? "*" : "", measurement.c_str(), modifier.c_str(),
         trace.c_str(), prefix.c_str(), values.c_str(), suffix.c_str(),
         units.c_str());
}

void PrintResultsImpl(const std::string& measurement,
                      const std::string& modifier,
                      const std::string& trace,
                      const std::string& values,
                      const std::string& prefix,
                      const std::string& suffix,
                      const std::string& units,
                      bool important) {
  fflush(stdout);
  printf("%s", ResultsToString(measurement, modifier, trace, values,
                               prefix, suffix, units, important).c_str());
  fflush(stdout);
}

}  // namespace

namespace perf_test {

void PrintResult(const std::string& measurement,
                 const std::string& modifier,
                 const std::string& trace,
                 size_t value,
                 const std::string& units,
                 bool important) {
  PrintResultsImpl(measurement,
                   modifier,
                   trace,
                   base::UintToString(static_cast<unsigned int>(value)),
                   std::string(),
                   std::string(),
                   units,
                   important);
}

void PrintResult(const std::string& measurement,
                 const std::string& modifier,
                 const std::string& trace,
                 double value,
                 const std::string& units,
                 bool important) {
  PrintResultsImpl(measurement,
                   modifier,
                   trace,
                   base::DoubleToString(value),
                   std::string(),
                   std::string(),
                   units,
                   important);
}

void AppendResult(std::string& output,
                  const std::string& measurement,
                  const std::string& modifier,
                  const std::string& trace,
                  size_t value,
                  const std::string& units,
                  bool important) {
  output += ResultsToString(
      measurement,
      modifier,
      trace,
      base::UintToString(static_cast<unsigned int>(value)),
      std::string(),
      std::string(),
      units,
      important);
}

void PrintResult(const std::string& measurement,
                 const std::string& modifier,
                 const std::string& trace,
                 const std::string& value,
                 const std::string& units,
                 bool important) {
  PrintResultsImpl(measurement,
                   modifier,
                   trace,
                   value,
                   std::string(),
                   std::string(),
                   units,
                   important);
}

void AppendResult(std::string& output,
                  const std::string& measurement,
                  const std::string& modifier,
                  const std::string& trace,
                  const std::string& value,
                  const std::string& units,
                  bool important) {
  output += ResultsToString(measurement,
                            modifier,
                            trace,
                            value,
                            std::string(),
                            std::string(),
                            units,
                            important);
}

void PrintResultMeanAndError(const std::string& measurement,
                             const std::string& modifier,
                             const std::string& trace,
                             const std::string& mean_and_error,
                             const std::string& units,
                             bool important) {
  PrintResultsImpl(measurement, modifier, trace, mean_and_error,
                   "{", "}", units, important);
}

void AppendResultMeanAndError(std::string& output,
                              const std::string& measurement,
                              const std::string& modifier,
                              const std::string& trace,
                              const std::string& mean_and_error,
                              const std::string& units,
                              bool important) {
  output += ResultsToString(measurement, modifier, trace, mean_and_error,
                            "{", "}", units, important);
}

void PrintResultList(const std::string& measurement,
                     const std::string& modifier,
                     const std::string& trace,
                     const std::string& values,
                     const std::string& units,
                     bool important) {
  PrintResultsImpl(measurement, modifier, trace, values,
                   "[", "]", units, important);
}

void AppendResultList(std::string& output,
                      const std::string& measurement,
                      const std::string& modifier,
                      const std::string& trace,
                      const std::string& values,
                      const std::string& units,
                      bool important) {
  output += ResultsToString(measurement, modifier, trace, values,
                            "[", "]", units, important);
}

void PrintSystemCommitCharge(const std::string& test_name,
                             size_t charge,
                             bool important) {
  PrintSystemCommitCharge(stdout, test_name, charge, important);
}

void PrintSystemCommitCharge(FILE* target,
                             const std::string& test_name,
                             size_t charge,
                             bool important) {
  fprintf(target, "%s", SystemCommitChargeToString(test_name, charge,
                                                   important).c_str());
}

std::string SystemCommitChargeToString(const std::string& test_name,
                                       size_t charge,
                                       bool important) {
  std::string trace_name(test_name);
  std::string output;
  AppendResult(output,
               "commit_charge",
               std::string(),
               "cc" + trace_name,
               charge,
               "kb",
               important);
  return output;
}

}  // namespace perf_test
