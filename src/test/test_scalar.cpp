#include <benchmark/benchmark.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdint>
#include "../scalar_impl.hpp"

using namespace httpvo::Implementation;


struct loadRequestLineFile : public benchmark::Fixture
{
    static constexpr uint32_t size = 1 << 25;

    loadRequestLineFile() : buf(size) {}

    void SetUp(const benchmark::State&) override
    {
        std::ifstream data("data/http_request_lines.txt", std::ios::binary);
        if (data.fail())
        {
            std::cout << "UNABLE TO OPEN FILE!" << std::endl;
            exit(-1);
        }

        while (std::getline(data, buf[0]))
        {
            buf.push_back(0);
        }
    }

    void TearDown(const benchmark::State&) override
    {
       
    }
    std::vector<std::string> buf;
    int n = 0;

};

//BENCHMARK_F(loadRequestLineFile, BM_ScalarVectorizedLoop)(benchmark::State& state)
//{
//}

static void BM_test_1(benchmark::State& state)
{
     http test_parse;
     static char str[] = "GET https://g8234293087097092307957023975029375029375029375029375023975023951e1we1we1we1we1e1e1w7023952746927346927346923469238469283469283469283469283649283642983642342oogle.com/index HTTP1.1\r\n";
     std::size_t len = strlen(str);

     for (auto _ : state)
     {
        int res = test_parse.parse_header_line_sc(str, len, len);
        benchmark::DoNotOptimize(res);
     }
}

BENCHMARK(BM_test_1)->Repetitions(2);

int main(int argc, char **argv)
{
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}