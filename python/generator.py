def fac(n):
  first, second = 1, 1
  for i in range(n):
    if i <= 1:
      yield 1
    else:
      value = first + second
      yield value
      first, second = second, value

def subgen():
  v = 0
  for _ in range(10):
    n = yield v
    v += n
  return v

def gen():
  v = yield from subgen()
  v += 10
  return v


for v in fac(10):
  print(v)



