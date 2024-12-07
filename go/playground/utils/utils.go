package utils1

import "fmt"
import utilsimpl "playground/utils/utils_impl"

func Div(a, b int) int {
	fmt.Println("utils1.Div")
	return utilsimpl.Div(a, b)
}
