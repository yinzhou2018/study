package main

import (
	"fmt"
	utils1 "playground/utils"
)

type Clone interface {
	clone() Clone
}

type Person struct {
	name string
	age  int32
}

func (self *Person) clone() Clone {
	return &Person{name: self.name, age: self.age}
}

func main() {
	fmt.Println("Hello World: ", add(10, 20), utils1.Div(20, 10))
	p1 := Person{name: "yin", age: 30}
	p2 := p1.clone()
	fmt.Println(p1, p2)
}
