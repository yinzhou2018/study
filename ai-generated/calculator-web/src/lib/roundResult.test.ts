import { describe, it, expect } from 'vitest'
import { roundResult } from './roundResult'

describe('roundResult', () => {
  it('消除 0.1 + 0.2 的浮点误差', () => {
    expect(roundResult(0.1 + 0.2)).toBe(0.3)
  })

  it('整数保持不变', () => {
    expect(roundResult(5)).toBe(5)
    expect(roundResult(0)).toBe(0)
  })

  it('截断至 10 位小数', () => {
    expect(roundResult(1 / 3)).toBe(0.3333333333)
    expect(roundResult(2 / 3)).toBe(0.6666666667)
  })

  it('非有限数原样返回', () => {
    expect(roundResult(Infinity)).toBe(Infinity)
    expect(Number.isNaN(roundResult(NaN))).toBe(true)
  })
})
