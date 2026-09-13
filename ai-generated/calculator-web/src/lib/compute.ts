import { roundResult } from './roundResult'

export type Operator = '+' | '-' | 'x' | '/'

/**
 * 四则运算执行 + 除零拦截。
 * @returns 计算结果；除零时返回 null（调用方进入 Error 状态）。
 */
export function compute(a: number, op: Operator, b: number): number | null {
  switch (op) {
    case '+':
      return roundResult(a + b)
    case '-':
      return roundResult(a - b)
    case 'x':
      return roundResult(a * b)
    case '/':
      if (b === 0) return null
      return roundResult(a / b)
  }
}
