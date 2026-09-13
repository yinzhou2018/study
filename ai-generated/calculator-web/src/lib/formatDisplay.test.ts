import { describe, it, expect } from 'vitest'
import { formatDisplay } from './formatDisplay'

describe('formatDisplay', () => {
  it('零显示为 "0"', () => {
    expect(formatDisplay(0)).toBe('0')
  })

  it('整数直接显示', () => {
    expect(formatDisplay(123)).toBe('123')
  })

  it('小数显示', () => {
    expect(formatDisplay(2.5)).toBe('2.5')
  })

  it('消除浮点误差', () => {
    expect(formatDisplay(0.1 + 0.2)).toBe('0.3')
  })

  it('非有限数显示 "错误"', () => {
    expect(formatDisplay(Infinity)).toBe('错误')
    expect(formatDisplay(NaN)).toBe('错误')
  })

  it('超长数使用科学计数法', () => {
    const result = formatDisplay(123456789012345)
    expect(result.length).toBeLessThanOrEqual(12)
  })
})
