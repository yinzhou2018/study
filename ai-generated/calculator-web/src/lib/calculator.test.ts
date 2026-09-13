import { describe, it, expect } from 'vitest'
import { calculatorReducer, initialState, type CalcState, type Action } from './calculator'

/** 辅助：从初始状态依次执行一组 action，返回最终状态 */
function run(actions: Action[]): CalcState {
  return actions.reduce(calculatorReducer, initialState)
}

const digit = (d: string): Action => ({ type: 'DIGIT', digit: d })
const op = (o: '+' | '-' | 'x' | '/'): Action => ({ type: 'OPERATOR', op: o })
const DECIMAL: Action = { type: 'DECIMAL' }
const EQUALS: Action = { type: 'EQUALS' }
const CLEAR: Action = { type: 'CLEAR' }
const BACKSPACE: Action = { type: 'BACKSPACE' }
const TOGGLE_SIGN: Action = { type: 'TOGGLE_SIGN' }

describe('calculatorReducer — P0 核心功能', () => {
  it('初始状态显示 "0"', () => {
    expect(initialState.display).toBe('0')
  })

  it('数字输入正确显示', () => {
    expect(run([digit('1'), digit('2'), digit('3')]).display).toBe('123')
  })

  it('基本加法：1 + 2 = 3', () => {
    expect(run([digit('1'), op('+'), digit('2'), EQUALS]).display).toBe('3')
  })

  it('基本减法：5 - 3 = 2', () => {
    expect(run([digit('5'), op('-'), digit('3'), EQUALS]).display).toBe('2')
  })

  it('基本乘法：4 × 6 = 24', () => {
    expect(run([digit('4'), op('x'), digit('6'), EQUALS]).display).toBe('24')
  })

  it('基本除法：10 ÷ 4 = 2.5', () => {
    expect(run([digit('1'), digit('0'), op('/'), digit('4'), EQUALS]).display).toBe('2.5')
  })

  it('AC 清除归零', () => {
    const s = run([digit('1'), digit('2'), op('+'), digit('3'), CLEAR])
    expect(s.display).toBe('0')
    expect(s.operand).toBeNull()
    expect(s.operator).toBeNull()
    expect(s.hasError).toBe(false)
  })

  it('小数点不能重复输入', () => {
    const s = run([digit('3'), DECIMAL, digit('5'), DECIMAL, digit('2')])
    expect(s.display).toBe('3.52')
  })

  it('浮点精度：0.1 + 0.2 = 0.3', () => {
    const s = run([digit('0'), DECIMAL, digit('1'), op('+'), digit('0'), DECIMAL, digit('2'), EQUALS])
    expect(s.display).toBe('0.3')
  })
})

describe('calculatorReducer — 除零处理（P0）', () => {
  it('5 ÷ 0 显示 "错误"', () => {
    const s = run([digit('5'), op('/'), digit('0'), EQUALS])
    expect(s.display).toBe('错误')
    expect(s.hasError).toBe(true)
  })

  it('错误状态下仅 AC 可恢复', () => {
    const errState = run([digit('5'), op('/'), digit('0'), EQUALS])
    // 数字输入被忽略
    expect(calculatorReducer(errState, digit('1'))).toBe(errState)
    // 运算符被忽略
    expect(calculatorReducer(errState, op('+'))).toBe(errState)
    // 等号被忽略
    expect(calculatorReducer(errState, EQUALS)).toBe(errState)
    // AC 恢复
    const cleared = calculatorReducer(errState, CLEAR)
    expect(cleared.display).toBe('0')
    expect(cleared.hasError).toBe(false)
  })
})

describe('calculatorReducer — 链式求值（无优先级）', () => {
  it('1 + 2 × 3 = 9（非 7）', () => {
    expect(run([digit('1'), op('+'), digit('2'), op('x'), digit('3'), EQUALS]).display).toBe('9')
  })

  it('连续运算：2 + 3 + 4 = 9', () => {
    expect(run([digit('2'), op('+'), digit('3'), op('+'), digit('4'), EQUALS]).display).toBe('9')
  })

  it('等号后基于结果继续运算', () => {
    // 6 + 4 = 10, 再 + 5 = 15
    const s = run([digit('6'), op('+'), digit('4'), EQUALS])
    const s2 = run([digit('6'), op('+'), digit('4'), EQUALS, op('+'), digit('5'), EQUALS])
    expect(s.display).toBe('10')
    expect(s2.display).toBe('15')
  })
})

describe('calculatorReducer — P1 功能', () => {
  it('退格逐位删除', () => {
    expect(run([digit('1'), digit('2'), digit('3'), BACKSPACE]).display).toBe('12')
  })

  it('退格删空归零', () => {
    expect(run([digit('5'), BACKSPACE]).display).toBe('0')
  })

  it('退格删除小数点', () => {
    expect(run([digit('3'), DECIMAL, BACKSPACE]).display).toBe('3')
  })

  it('+/- 正负号切换', () => {
    expect(run([digit('5'), TOGGLE_SIGN]).display).toBe('-5')
    expect(run([digit('5'), TOGGLE_SIGN, TOGGLE_SIGN]).display).toBe('5')
  })

  it('+/- 对零无效', () => {
    expect(run([TOGGLE_SIGN]).display).toBe('0')
  })
})

describe('calculatorReducer — 边界情况', () => {
  it('前导零：00 → 0, 01 → 1', () => {
    expect(run([digit('0'), digit('0')]).display).toBe('0')
    expect(run([digit('0'), digit('1')]).display).toBe('1')
  })

  it('小数点前导：直接按 . → 0.', () => {
    const s = run([DECIMAL])
    expect(s.display).toBe('0.')
  })

  it('连续运算符输入：替换为最新运算符', () => {
    // 5 + × 3 = 15（+ 被 × 替换）
    expect(run([digit('5'), op('+'), op('x'), digit('3'), EQUALS]).display).toBe('15')
  })

  it('不完整表达式：5 + = → 10（重复最后操作数）', () => {
    expect(run([digit('5'), op('+'), EQUALS]).display).toBe('10')
  })

  it('不完整表达式：× 5 = → 0（运算符在前视为 0 × 5）', () => {
    expect(run([op('x'), digit('5'), EQUALS]).display).toBe('0')
  })

  it('最大输入长度：12 位后忽略后续输入', () => {
    const actions = [
      digit('1'), digit('2'), digit('3'), digit('4'), digit('5'), digit('6'),
      digit('7'), digit('8'), digit('9'), digit('0'), digit('1'), digit('2'),
      digit('3'), // 第 13 位，应被忽略
    ]
    expect(run(actions).display).toBe('123456789012')
  })

  it('显示溢出使用科学计数法', () => {
    // 999999999999 × 999999999999 = 很大的数
    const big = [
      digit('9'), digit('9'), digit('9'), digit('9'), digit('9'), digit('9'),
      digit('9'), digit('9'), digit('9'), digit('9'), digit('9'), digit('9'),
    ]
    const s = run([...big, op('x'), ...big, EQUALS])
    expect(s.display.length).toBeLessThanOrEqual(12)
    expect(s.display).not.toBe('溢出')
  })
})
