#pragma once
#include <stdio.h>
#include <semaphore>
#include <thread>

//-------------------------------------
//  MersenneTwister
//  A thread-safe random number generator with good randomness
//  in a small number of instructions. We'll use it to introduce
//  random timing delays.
//-------------------------------------
#define MT_IA 397
#define MT_LEN 624

class MersenneTwister {
  unsigned int m_buffer[MT_LEN];
  int m_index;

 public:
  MersenneTwister(unsigned int seed);
  // Declare noinline so that the function call acts as a compiler barrier:
  unsigned int integer();
};

MersenneTwister::MersenneTwister(unsigned int seed) {
  // Initialize by filling with the seed, then iterating
  // the algorithm a bunch of times to shuffle things up.
  for (int i = 0; i < MT_LEN; i++)
    m_buffer[i] = seed;
  m_index = 0;
  for (int i = 0; i < MT_LEN * 100; i++)
    integer();
}

unsigned int MersenneTwister::integer() {
  // Indices
  int i = m_index;
  int i2 = m_index + 1;
  if (i2 >= MT_LEN)
    i2 = 0;  // wrap-around
  int j = m_index + MT_IA;
  if (j >= MT_LEN)
    j -= MT_LEN;  // wrap-around

  // Twist
  unsigned int s = (m_buffer[i] & 0x80000000) | (m_buffer[i2] & 0x7fffffff);
  unsigned int r = m_buffer[j] ^ (s >> 1) ^ ((s & 1) * 0x9908B0DF);
  m_buffer[m_index] = r;
  m_index = i2;

  // Swizzle
  r ^= (r >> 11);
  r ^= (r << 7) & 0x9d2c5680UL;
  r ^= (r << 15) & 0xefc60000UL;
  r ^= (r >> 18);
  return r;
}

//-------------------------------------
//  Main program, as decribed in the post
//-------------------------------------
std::binary_semaphore beginSema1{0};
std::binary_semaphore beginSema2{0};
std::binary_semaphore endSema{0};

int X, Y;
int r1, r2;

void thread1Func() {
  MersenneTwister random(1);
  for (;;) {
    beginSema1.acquire();  // Wait for signal
    while (random.integer() % 8 != 0) {
    }  // Random delay

    // ----- THE TRANSACTION! -----
    X = 1;
    std::atomic_thread_fence(std::memory_order_release);
    Y = 1;

    endSema.release();  // Notify transaction complete
  }
};

void thread2Func() {
  MersenneTwister random(2);
  for (;;) {
    beginSema2.acquire();  // Wait for signal
    while (random.integer() % 8 != 0) {
    }  // Random delay

    while (Y != 1)
      ;
    std::atomic_thread_fence(std::memory_order_acquire);

    if (X == 0) {
      printf("X is 0!\n");
    }

    endSema.release();  // Notify transaction complete
  }
};

int multithread_study() {
  // Spawn the threads
  auto thread1 = std::thread(thread1Func);
  auto thread2 = std::thread(thread2Func);

  // Repeat the experiment ad infinitum
  int detected = 0;
  for (int iterations = 1;; iterations++) {
    // Reset X and Y
    X = 0;
    Y = 0;
    // Signal both threads
    beginSema1.release();
    beginSema2.release();
    // Wait for both threads
    endSema.acquire();
    endSema.acquire();
    // // Check if there was a simultaneous reorder
    // if (r1 == 0 && r2 == 0) {
    //   detected++;
    //   printf("%d reorders detected after %d iterations\n", detected, iterations);
    // }
  }
  return 0;  // Never returns
}
