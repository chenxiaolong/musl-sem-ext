#include <gtest/gtest.h>

#include <semaphore.h>

#include "musl-sem.h"

TEST(MuslSemEnvTest, CheckSemTSizeMatchesLibc)
{
    // The sizes must match if we want to use LD_PRELOAD. Otherwise, we'll
    // write to invalid memory
    ASSERT_EQ(sizeof(sem_t), sizeof(musl_sem_t));
}
