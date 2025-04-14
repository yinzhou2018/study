package cgo

/*
#include "lib.h"
#include <stdlib.h>
*/
import "C"
import (
	"fmt"
	"unsafe"
)

// BasicCGoTest 展示基本的 CGO 调用
func BasicCGoTest() {
	// 从 Go 调用 C 函数
	message := C.CString("Hello from Go!")
	defer C.free(unsafe.Pointer(message))
	C.printMessage(message)

	// 调用 C 的加法函数
	result := C.add(C.int(42), C.int(58))
	fmt.Printf("C 函数计算结果: %d\n", int(result))
}
