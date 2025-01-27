#include "load_csv.hpp"
#include "model_test.hpp"
#include "util_tests.hpp"
#include <iostream>
#include <map>
#include <string>

// @TODO: make tests for both float and double precision

struct TestConfig
{
    std::string name;
    std::string model_file;
    std::string x_data_file;
    std::string y_data_file;
    const double threshold;
};

static std::map<std::string, TestConfig> tests {
    { "conv1d",
        TestConfig { "CONV1D", "models/conv.json", "test_data/conv_x_python.csv",
            "test_data/conv_y_python.csv", 1.0e-6 } },
    { "dense",
        TestConfig { "DENSE", "models/dense.json", "test_data/dense_x_python.csv",
            "test_data/dense_y_python.csv", 2.0e-8 } },
    { "gru", TestConfig { "GRU", "models/gru.json", "test_data/gru_x_python.csv", "test_data/gru_y_python.csv", 5.0e-6 } },
    { "lstm",
        TestConfig { "LSTM", "models/lstm.json", "test_data/lstm_x_python.csv",
            "test_data/lstm_y_python.csv", 1.0e-6 } },
};

void help()
{
    std::cout << "RTNeural test suite:" << std::endl;
    std::cout << "Usage: rtneural_tests <test_type>" << std::endl;
    std::cout << std::endl;
    std::cout << "Available test types are:" << std::endl;

    std::cout << "    all" << std::endl;
    std::cout << "    util" << std::endl;
    std::cout << "    model" << std::endl;
    for(auto& testConfig : tests)
        std::cout << "    " << testConfig.first << std::endl;
}

template <typename T>
int runTest(const TestConfig& test)
{
    std::cout << "TESTING " << test.name << " IMPLEMENTATION..." << std::endl;

    std::ifstream jsonStream(test.model_file, std::ifstream::binary);
    auto model = RTNeural::json_parser::parseJson<T>(jsonStream, true);
    model->reset();

    std::ifstream pythonX(test.x_data_file);
    auto xData = load_csv::loadFile<T>(pythonX);

    std::ifstream pythonY(test.y_data_file);
    const auto yRefData = load_csv::loadFile<T>(pythonY);

    std::vector<T> yData(xData.size(), (T)0);
    for(size_t n = 0; n < xData.size(); ++n)
    {
        T input[] = { xData[n] };
        yData[n] = model->forward(input);
    }

    size_t nErrs = 0;
    T max_error = (T)0;
    for(size_t n = 0; n < xData.size(); ++n)
    {
        auto err = std::abs(yData[n] - yRefData[n]);
        if(err > test.threshold)
        {
            max_error = std::max(err, max_error);
            nErrs++;

            // For debugging purposes
            // std::cout << "ERR: " << err << ", idx: " << n << std::endl;
            // std::cout << yData[n] << std::endl;
            // std::cout << yRefData[n] << std::endl;
            // break;
        }
    }

    if(nErrs > 0)
    {
        std::cout << "FAIL: " << nErrs << " errors!" << std::endl;
        std::cout << "Maximum error: " << max_error << std::endl;
        return 1;
    }

    std::cout << "SUCCESS" << std::endl;
    return 0;
}

using TestType = double;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        help();
        return 1;
    }

    std::string arg = argv[1];
    if(arg == "--help")
    {
        help();
        return 1;
    }

    if(arg == "all")
    {
        util_test();

        int result = 0;
        result |= model_test::model_test();

        for(auto& testConfig : tests)
            result |= runTest<TestType>(testConfig.second);

        return result;
    }

    if(arg == "util")
    {
        util_test();
        return 0;
    }

    if(arg == "model")
    {
        return model_test::model_test();
    }

    if(tests.find(arg) != tests.end())
    {
        return runTest<TestType>(tests.at(arg));
    }

    std::cout << "Test: " << arg << " not found!" << std::endl;
    return 1;
}
