#include <benchmark/benchmark.h>
#include "MPMCQueue.h"

static int maxRunsPerThread = 15000;
static int queueCapacity = 100;
static int producersNum = 8;
static int consumersNum = 8;

void addRandomToQueue(MPMCQueue<int>& q, int num)
{
    for (int i = 0; i < maxRunsPerThread; ++i)
    {
        q.push(num);
    }
}

void removeItemFromQueue(MPMCQueue<int>& q)
{
    for (int i = 0; i < maxRunsPerThread; ++i)
    {
        int popedElement = -1;
        q.pop(popedElement);
    }
}

static void BM_SimulateQueue(benchmark::State& state)
{
	for (auto _ : state)
	{
        state.PauseTiming();
        MPMCQueue<int> q(queueCapacity);

        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        state.ResumeTiming();

        //Create producers
        for (size_t i = 0; i < producersNum; ++i)
        {
            producers.emplace_back(std::thread(addRandomToQueue, std::ref(q), static_cast<int>(i)));
        }

        //Create consumers
        for (size_t i = 0; i < consumersNum; ++i)
        {
            consumers.emplace_back(std::thread(removeItemFromQueue, std::ref(q)));
        }

        for (auto& p : producers)
        {
            p.join();
        }
        for (auto& c : consumers)
        {
            c.join();
        }
	}
}
BENCHMARK(BM_SimulateQueue)
    ->UseRealTime()
    ->Unit(benchmark::kMillisecond)
    ->MeasureProcessCPUTime();
BENCHMARK_MAIN();