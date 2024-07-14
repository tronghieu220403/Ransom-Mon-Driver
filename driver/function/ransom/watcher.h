#pragma once

#include "../../std/vector/vector.h"
#include "../../std/string/string.h"
#include "../../std/sync/mutex.h"
#include "../../std/algo/hash.h"

#include <wdm.h>
#include <fltKernel.h>

#define CHI_SQUARE_THRESHOLD 311*2
// real threshold is 0.0011
#define SERIAL_BYTE_CORRELATION_COEFFICIENT_THRESHOLD 11*2
#define SERIAL_BYTE_CORRELATION_COEFFICIENT_MULTIPLE 10000
#define MINIMUM_BYTES 10485760 // 10MB

#define HONEY_CNT_THRESHOLD 3
#define HONEY_NAME L"hieunt_important"

namespace ransom
{
    class EntropyAnalyzer {
    private:

        long long size_ = 0;

        long long freq_[256] = { 0 };

        long long sum_ui_ui_plus_1_ = 0;
        long long sum_ui_ = 0;
        long long sum_ui_squared_ = 0;
        unsigned char last_byte_ = 0;
        unsigned char first_byte_ = 0;

        bool is_random = false;

        long long ChiSquareTest();
        long long SerialByteCorrelationCoefficient();
        void UpdateChiSquareTest(const Vector<unsigned char>& data);
        void UpdateSerialByteCorrelationCoefficient(const Vector<unsigned char>& data);

    public:

        EntropyAnalyzer() = default;
        void AddData(const Vector<unsigned char>& new_data);
        bool IsRandom();
        long long GetSize();
        void Clean();
        ~EntropyAnalyzer() = default;
    };

    class HoneyAnalyzer
    {
    private:
        Vector<unsigned long long> file_list;
    public:
        void IncHoneyCnt(const String<WCHAR>& str);
        bool IsThresholdReached();
    };
}
