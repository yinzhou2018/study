package cgo

/*
#include "lib.h"
*/
import "C"
import (
	"fmt"
	"unsafe"
)

//export goCallback
func goCallback(n C.int) {
	fmt.Printf("Go 回调函数被 C 调用，参数为: %d\n", int(n))
}

//export sumArray
func sumArray(arr *C.int, len C.int) C.int {
	slice := (*[1 << 30]C.int)(unsafe.Pointer(arr))[:len:len]
	var sum C.int
	for _, v := range slice {
		sum += v
	}
	return sum
}

// AdvancedCGoTest 展示高级 CGO 特性
func AdvancedCGoTest() {
	point := C.makePoint(C.int(10), C.int(20))
	fmt.Printf("C 结构体 Point: x=%d, y=%d\n", int(point.x), int(point.y))

	C.goCallback(C.int(42))

	numbers := []C.int{1, 2, 3, 4, 5}
	sum := C.sumArray(&numbers[0], C.int(len(numbers)))
	fmt.Printf("数组总和: %d\n", int(sum))
}
