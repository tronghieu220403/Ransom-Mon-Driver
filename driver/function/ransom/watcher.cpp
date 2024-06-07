#include "watcher.h"

namespace ransom {

    long long DataAnalyzer::ChiSquareTest() {

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

    long long DataAnalyzer::SerialByteCorrelationCoefficient() {
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

    void DataAnalyzer::UpdateChiSquareTest(const Vector<unsigned char>& data)
    {
        long long sz = data.Size();

        for (int i = 0; i < sz; ++i)
        {
            freq_[data[i]]++;
        }
        size_ += sz;
    }

    void DataAnalyzer::UpdateSerialByteCorrelationCoefficient(const Vector<unsigned char>& data)
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

    DataAnalyzer::DataAnalyzer() {
        for (int i = 0; i < 256; ++i) {
            freq_[i] = 0;
        }
    }

    void DataAnalyzer::AddData(const Vector<unsigned char>& new_data) {

        if (size_ == 0)
        {
            first_byte_ = new_data[0];
        }

        UpdateChiSquareTest(new_data);
        UpdateSerialByteCorrelationCoefficient(new_data);

        last_byte_ = new_data[new_data.Size() - 1];

    }


    bool DataAnalyzer::IsRandom() {

        if (size_ == 0) {
            return false;
        }

        long long chiSquareValue = ChiSquareTest();
        long long correlation = SerialByteCorrelationCoefficient();

        bool isChiSquareRandom = (chiSquareValue < CHI_SQUARE_THRESHOLD);
        bool isArchive = (correlation > SERIAL_BYTE_CORRELATION_COEFFICIENT_THRESHOLD);

        if (isArchive)
        {
            return false;
        }
        else
        {
            return isChiSquareRandom;
        }
    }

    long long DataAnalyzer::GetSize()
    {
        return size_;
    }

}