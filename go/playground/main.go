package main

import (
	"playground/cocurrency"
)

func main() {
	for i := 0; ; i++ {
		cocurrency.LockFreeQueueTest(i)
		cocurrency.NativeChannelTest(i)
	}
}
