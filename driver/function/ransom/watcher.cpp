#include "watcher.h"

namespace ransom {

    long long EntropyAnalyzer::ChiSquareTest() {

        long long chi_square = 0;

        long long expected_freq[256] = { 0 };
        for (int i = 0; i < 256; ++i) {
            expected_freq[i] = (size_ >> 8) + (i >= size_ % 256 ? 0 : 1);
        }

        for (int i = 0; i < 256; ++i) {
            long long diff = freq_[i] - expected_freq[i];
            chi_square += (diff * diff);
        }

        return chi_square / (size_ >> 8);
    }

    long long EntropyAnalyzer::SerialByteCorrelationCoefficient() {
        if (size_ < 2) {
            return INT_MAX;
        }

        long long flat = max((long long)INT_MAX, max(sum_ui_ui_plus_1_, max(sum_ui_squared_, sum_ui_))) / INT_MAX;

        long long tmp_sum_ui_ui_plus_1 = sum_ui_ui_plus_1_ / flat;
        long long tmp_sum_ui_squared = sum_ui_squared_ / flat;
        long long tmp_sum_ui = sum_ui_ / flat;
        long long tmp_size = size_ / flat;

        long long numerator = tmp_size * tmp_sum_ui_ui_plus_1 - tmp_sum_ui * tmp_sum_ui;
        long long denominator = tmp_size * tmp_sum_ui_squared - tmp_sum_ui * tmp_sum_ui;

        if (denominator == 0) {
            return 0;  // Avoid division by zero
        }

        long long correlation = numerator * SERIAL_BYTE_CORRELATION_COEFFICIENT_MULTIPLE / denominator;

        return correlation;
    }

    void EntropyAnalyzer::UpdateChiSquareTest(const Vector<unsigned char>& data)
    {
        long long sz = data.Size();

        for (int i = 0; i < sz; ++i)
        {
            freq_[data[i]]++;
        }
    }

    void EntropyAnalyzer::UpdateSerialByteCorrelationCoefficient(const Vector<unsigned char>& data)
    {
        long long sz = data.Size();
        for (long long i = 0; i < sz - 1; ++i)
        {
            sum_ui_ui_plus_1_ += (long long)data[i] * data[i + 1];
        }
        sum_ui_ui_plus_1_ += (long long)data[0] * last_byte_;
        sum_ui_ui_plus_1_ += (long long)data[sz - 1] * first_byte_;
        sum_ui_ui_plus_1_ -= (long long)last_byte_ * (long long)first_byte_;

        for (int i = 0; i < sz; ++i)
        {
            long long byte = data[i];
            sum_ui_ += byte;
            sum_ui_squared_ += byte * byte;
        }
    }

    void EntropyAnalyzer::AddData(const Vector<unsigned char>& new_data) {

        if (is_random == true)
        {
            return;
        }
        if (size_ == 0)
        {
            first_byte_ = new_data[0];
        }

        size_ += new_data.Size();

        UpdateChiSquareTest(new_data);
        UpdateSerialByteCorrelationCoefficient(new_data);

        last_byte_ = new_data[new_data.Size() - 1];

    }


    bool EntropyAnalyzer::IsRandom() {

        if (size_ < MINIMUM_BYTES)
        {
            return false;
        }
        if (is_random == true)
        {
            return true;
        }

        long long chi_square_value = ChiSquareTest();
        long long correlation = SerialByteCorrelationCoefficient();

        bool is_chi_square_random = (chi_square_value < CHI_SQUARE_THRESHOLD);
        bool is_archive = (correlation > SERIAL_BYTE_CORRELATION_COEFFICIENT_THRESHOLD);

        Clean();

        //DebugMessage("Chi: %lld, corr: %lld", chi_square_value, correlation);

        if (is_archive)
        {
            return false;
        }
        else
        {
            if (is_chi_square_random == true)
            {
                is_random = true;
                Clean();
            }
            return is_random;
        }
    }

    long long EntropyAnalyzer::GetSize()
    {
        return size_;
    }

    void EntropyAnalyzer::Clean()
    {
        size_ = 0;
        for (int i = 0; i < 256; i++)
        {
            freq_[i] = 0;
        }
        sum_ui_ui_plus_1_ = 0;
        sum_ui_ = 0;
        sum_ui_squared_ = 0;
        last_byte_ = 0;
        first_byte_ = 0;
    }

    void HoneyAnalyzer::IncHoneyCnt(const String<WCHAR>& str)
    {
        unsigned long long hash = HashWstring(str);
        bool exist = false;
        for (int i = 0; i < file_list.Size(); i++)
        {
            if (file_list[i] == hash)
            {
                exist = true;
                break;
            }
        }
        if (exist == false)
        {
            file_list.PushBack(hash);
        }
    }

    bool HoneyAnalyzer::IsThresholdReached()
    {
        return file_list.Size() >= HONEY_CNT_THRESHOLD;
    }

}