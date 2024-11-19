(module
  (func $add (param $a i32) (param $b i32) (result i32 i32)
    local.get 0
    local.get 1
  )
  
  (func $call_add (result i32 i32)
    (call $add (i32.const 1) (i32.const 2))
  )

  (export "call_add" (func $call_add))
)