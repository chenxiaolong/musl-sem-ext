#include <gtest/gtest.h>

#include <thread>

#include "musl-sem.h"

TEST(MuslSemTest, CheckNamedSemaphoresWork)
{
    constexpr int thread_count = 10;
    constexpr int rounds = 10000;

    std::array<std::thread, thread_count> threads;
    musl_sem_t *sem;
    int counter = 0;

    std::string name{"/musl-sem-test-"};
    name += std::to_string(getpid());

    sem = musl_sem_open(name.c_str(), O_CREAT | O_EXCL, 0600, 1);
    ASSERT_NE(sem, MUSL_SEM_FAILED);

    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i] = std::thread([&sem, &counter, i]{
            for (int j = 0; j < rounds; ++j) {
                musl_sem_wait(sem);
                if (i & 1) {
                    --counter;
                } else {
                    ++counter;
                }
                musl_sem_post(sem);
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    ASSERT_EQ(musl_sem_close(sem), 0);
    ASSERT_EQ(musl_sem_unlink(name.c_str()), 0);

    ASSERT_EQ(counter, threads.size() & 1 ? rounds : 0);
}

TEST(MuslSemTest, CheckUnnamedSemaphoresWork)
{
    constexpr int thread_count = 10;
    constexpr int rounds = 10000;

    std::array<std::thread, thread_count> threads;
    musl_sem_t sem;
    int counter = 0;

    ASSERT_EQ(musl_sem_init(&sem, 0, 1), 0);

    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i] = std::thread([&sem, &counter, i]{
            for (int j = 0; j < rounds; ++j) {
                musl_sem_wait(&sem);
                if (i & 1) {
                    --counter;
                } else {
                    ++counter;
                }
                musl_sem_post(&sem);
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    ASSERT_EQ(musl_sem_destroy(&sem), 0);

    ASSERT_EQ(counter, threads.size() & 1 ? rounds : 0);
}

TEST(MuslSemTest, CheckSemaphoreLimitGreaterThan256)
{
    constexpr int sem_count = 1000;

    std::array<musl_sem_t *, sem_count> sems;
    std::vector<std::string> names;
    names.reserve(sem_count);

    for (size_t i = 0; i < sems.size(); ++i) {
        std::string name{"/musl-sem-test-"};
        name += std::to_string(getpid());
        name += "-";
        name += std::to_string(i);

        sems[i] = musl_sem_open(name.c_str(), O_CREAT | O_EXCL, 0600, 1);
        ASSERT_NE(sems[i], MUSL_SEM_FAILED);

        names.push_back(std::move(name));
    }

    for (auto &sem : sems) {
        ASSERT_EQ(musl_sem_close(sem), 0);
    }

    for (auto const &name : names) {
        ASSERT_EQ(musl_sem_unlink(name.c_str()), 0);
    }
}
