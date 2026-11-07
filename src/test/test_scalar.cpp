#include <benchmark/benchmark.h>
#include "../implementation.cpp"

using namespace httpvo;
alignas(8) static char request[] = "GET https://g232332o279627368472368472364872348792734627346927346928346923846923846932323232332ogle.com/index HTTP/1.1\r\n";

static void BM_speed_test_1(benchmark::State& state)
{
    std::size_t len = std::strlen(request);
    httpvo::Implementation::Parse parser;
    httpvo::ReqLine req{0};

    for (auto _ : state)
    {
        auto status = parser.request_line<8>(reinterpret_cast<uint8_t *>(request), len, req);
        if (status != 0)
        {
            std::cout << status.stat << ' ' << status.offset << std::endl;
            exit(-1);
        }
        benchmark::DoNotOptimize(status);
        req.reset();
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