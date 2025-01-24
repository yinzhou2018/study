package cocurrency

import (
	"fmt"
	"sync"
	"sync/atomic"
	"time"
)

func NativeChannelTest(epoch int) {
	var readFlags [WRITE_TOTAL_COUNT]uint
	hasReadCount := atomic.Uint64{}
	ch := make(chan uint64, QUEUE_SIZE)
	defer close(ch)
	var wg sync.WaitGroup

	start := time.Now()

	for i := 0; i < WRITE_THREAD_COUNT; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			index := uint64(i * WRITE_COUNT_PER_THREAD)
			end := index + WRITE_COUNT_PER_THREAD
			for index < end {
				select {
				case ch <- index:
					index++
				default:
				}
			}
		}(i)
	}

	for i := 0; i < READ_THREAD_COUNT; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for hasReadCount.Load() < WRITE_TOTAL_COUNT {
				select {
				case val := <-ch:
					readFlags[val] = 1
					hasReadCount.Add(1)
				default:
				}
			}
		}()
	}

	wg.Wait()

	end := time.Now()
	spend := end.Sub(start).Milliseconds()
	result := uint(0)
	for _, x := range readFlags {
		result += x
	}
	fmt.Printf("%d. [NativeChannel][%s result: %d, total written count: %d, write threads: %d, read threads: %d] spends: %dms\n",
		epoch,
		func() string {
			if result == WRITE_TOTAL_COUNT {
				return "OK"
			}
			return "FAILED"
		}(),
		result,
		WRITE_TOTAL_COUNT,
		WRITE_THREAD_COUNT,
		READ_THREAD_COUNT,
		spend,
	)
}
