#include <benchmark/benchmark.h>
#include "../scalar_impl.hpp"

using namespace httpvo::Implementation;

int main(void)
{
    http request;

    return 0;
}

static void BM_test_1(benchmark::State& state)
{
     http test_parse;
     alignas(8) static char str[] = "GET https://google.com/index HTTP1.1\r\n";
     std::size_t len = strlen(str);

     for (auto _ : state)
     {
        int res = test_parse.parse_header_line_sc(str, len, len);
        benchmark::DoNotOptimize(res);
     }
}

BENCHMARK(BM_test_1);

int main(int argc, char **argv)
{
    benchmark::MaybeReenterWithoutASLR(argc, argv);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}