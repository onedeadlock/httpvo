#include <benchmark/benchmark.h>
#include "../scalar_impl.hpp"

using namespace httpvo;
alignas(8) static char str[] = "GET https://g232332o279627368472368472364872348792734627346927346928346923846923846932323232332ogle.com/index HTTP1.1\r\n";

void correct_request_line_index(void)
{
     Implementation::http test_parse;
     std::size_t len = strlen(str);
     (void)len;
}

static void BM_speed_test_1(benchmark::State& state)
{
     Implementation::http test_parse;
     ReqLine req;
     req.request();

     std::size_t len = strlen(str);

     for (auto _ : state)
     {
        auto res = test_parse.scparse_header_line((u8_t *)str, req, len, 0, constant::cff, 0);
        benchmark::DoNotOptimize(res);
        req.request();
     }
}

BENCHMARK(BM_speed_test_1)->Repetitions(10)->Iterations(64900)->ReportAggregatesOnly()->MinWarmUpTime(5);

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