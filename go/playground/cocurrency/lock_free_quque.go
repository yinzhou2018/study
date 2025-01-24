package cocurrency

import (
	"fmt"
	"sync"
	"sync/atomic"
	"time"
)

type QueueResult int

const (
	Ok QueueResult = iota
	Full
	Empty
)

type Element[T any] struct {
	sequence atomic.Uint64
	data     T
}

type LockFreeQueue[T any] struct {
  capacity   uint64
  mask       uint64
  elements   []Element[T]
  count      atomic.Int64
  enqueuePos atomic.Uint64
  dequeuePos atomic.Uint64
}

func NewLockFreeQueue[T any](size uint64) *LockFreeQueue[T] {
	if size%2 != 0 {
		tmp := uint64(1)
		for tmp <= size {
			tmp <<= 1
		}
		size = tmp
	}

	elements := make([]Element[T], size)
	for i := range elements {
		elements[i].sequence.Store(uint64(i))
	}

	return &LockFreeQueue[T]{
		capacity: size,
		mask:     size - 1,
		elements: elements,
	}
}

func (q *LockFreeQueue[T]) Enqueue(val T) QueueResult {
	pos := q.enqueuePos.Load()

	for {
		elem := &q.elements[pos&q.mask]
		seq := elem.sequence.Load()
		dif := int64(seq) - int64(pos)

		if dif == 0 {
			if q.enqueuePos.CompareAndSwap(pos, pos+1) {
				break
			}
			pos = q.enqueuePos.Load()
		} else if dif < 0 {
			if pos == q.capacity+q.dequeuePos.Load() {
				return Full
			}
			pos = q.enqueuePos.Load()
		} else {
			pos = q.enqueuePos.Load()
		}
	}

	elem := &q.elements[pos&q.mask]
	elem.data = val
	elem.sequence.Store(pos + 1)
	q.count.Add(1)

	return Ok
}

func (q *LockFreeQueue[T]) Dequeue(val *T) QueueResult {
	pos := q.dequeuePos.Load()

	for {
		elem := &q.elements[pos&q.mask]
		seq := elem.sequence.Load()
		dif := int64(seq) - int64(pos+1)

		if dif == 0 {
			if q.dequeuePos.CompareAndSwap(pos, pos+1) {
				break
			}
			pos = q.dequeuePos.Load()
		} else if dif < 0 {
			if pos == q.enqueuePos.Load() {
				return Empty
			}
			pos = q.dequeuePos.Load()
		} else {
			pos = q.dequeuePos.Load()
		}
	}

	elem := &q.elements[pos&q.mask]
	*val = elem.data
	elem.sequence.Store(pos + q.mask + 1)
	q.count.Add(-1)

	return Ok
}

func (q *LockFreeQueue[T]) Size() int64 {
	return q.count.Load()
}

func (q *LockFreeQueue[T]) Capacity() uint64 {
	return q.capacity
}

func LockFreeQueueTest(epoch int) {
	var readFlags [WRITE_TOTAL_COUNT]uint
	hasReadCount := atomic.Uint64{}
	lfQueue := NewLockFreeQueue[uint64](QUEUE_SIZE)

	var wg sync.WaitGroup

	start := time.Now()

	for i := 0; i < WRITE_THREAD_COUNT; i++ {
		wg.Add(1)
		go func(i int) {
			defer wg.Done()
			index := uint64(i * WRITE_COUNT_PER_THREAD)
			end := index + WRITE_COUNT_PER_THREAD
			for index < end {
				if lfQueue.Enqueue(index) == Ok {
					index++
				}
			}
		}(i)
	}

	for i := 0; i < READ_THREAD_COUNT; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for hasReadCount.Load() < WRITE_TOTAL_COUNT {
				var val uint64
				if lfQueue.Dequeue(&val) == Ok {
					readFlags[val] = 1
					hasReadCount.Add(1)
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
	fmt.Printf("%d. [LockFreeQueue][%s result: %d, total written count: %d, write threads: %d, read threads: %d] spends: %dms\n",
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
