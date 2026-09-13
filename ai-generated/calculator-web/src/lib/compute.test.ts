import { describe, it, expect } from 'vitest'
import { compute } from './compute'

describe('compute', () => {
  it('加法', () => {
    expect(compute(1, '+', 2)).toBe(3)
  })

  it('减法', () => {
    expect(compute(5, '-', 3)).toBe(2)
  })

  it('乘法', () => {
    expect(compute(4, 'x', 6)).toBe(24)
  })

  it('除法', () => {
    expect(compute(10, '/', 4)).toBe(2.5)
  })

  it('除零返回 null', () => {
    expect(compute(5, '/', 0)).toBeNull()
  })

  it('浮点精度处理', () => {
    expect(compute(0.1, '+', 0.2)).toBe(0.3)
  })
})
