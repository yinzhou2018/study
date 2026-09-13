import { compute, type Operator } from './compute'
import { formatDisplay } from './formatDisplay'

/** 计算器状态机状态 */
export type CalcState = {
  display: string // 当前显示值（输入中的原始字符串，或计算结果格式化字符串）
  operand: number | null // 已暂存的操作数
  operator: Operator | null // 已暂存的运算符
  waitingForNext: boolean // 是否等待输入下一个操作数
  hasError: boolean // 错误状态（仅 AC 可恢复）
}

/** 输入事件 */
export type Action =
  | { type: 'DIGIT'; digit: string }
  | { type: 'DECIMAL' }
  | { type: 'OPERATOR'; op: Operator }
  | { type: 'EQUALS' }
  | { type: 'CLEAR' }
  | { type: 'BACKSPACE' }
  | { type: 'TOGGLE_SIGN' }

/** 最大输入长度（含小数点，不含负号） */
export const MAX_INPUT_LENGTH = 12

export const initialState: CalcState = {
  display: '0',
  operand: null,
  operator: null,
  waitingForNext: false,
  hasError: false,
}

/** 计算显示字符串的有效长度（不含负号） */
function significantLength(display: string): number {
  return display.startsWith('-') ? display.length - 1 : display.length
}

/** 将显示字符串解析为数值 */
function parseDisplay(display: string): number {
  return parseFloat(display)
}

/**
 * 计算器状态机 reducer —— 纯函数，无副作用。
 * 链式按输入顺序求值（无运算符优先级）。
 */
export function calculatorReducer(state: CalcState, action: Action): CalcState {
  // 错误状态下仅响应 CLEAR
  if (state.hasError && action.type !== 'CLEAR') {
    return state
  }

  switch (action.type) {
    case 'CLEAR':
      return initialState

    case 'DIGIT': {
      const d = action.digit

      if (state.waitingForNext) {
        // 开始输入新操作数
        return { ...state, display: d, waitingForNext: false }
      }

      // 前导零处理：当前为 "0" 或 "-0" 时，输入数字替换零
      if (state.display === '0') {
        return { ...state, display: d }
      }
      if (state.display === '-0') {
        return { ...state, display: '-' + d }
      }

      // 最大输入长度限制
      if (significantLength(state.display) >= MAX_INPUT_LENGTH) {
        return state
      }

      return { ...state, display: state.display + d }
    }

    case 'DECIMAL': {
      if (state.waitingForNext) {
        // 小数点前导：直接按 "." → "0."
        return { ...state, display: '0.', waitingForNext: false }
      }

      // 已有小数点则忽略
      if (state.display.includes('.')) {
        return state
      }

      // 最大输入长度限制
      if (significantLength(state.display) >= MAX_INPUT_LENGTH) {
        return state
      }

      return { ...state, display: state.display + '.' }
    }

    case 'OPERATOR': {
      const op = action.op

      if (state.operand === null) {
        // 首次选择运算符：暂存当前显示值
        return {
          ...state,
          operand: parseDisplay(state.display),
          operator: op,
          waitingForNext: true,
        }
      }

      // 已有暂存运算符
      if (state.waitingForNext) {
        // 连续运算符输入：替换为最新运算符，不触发计算
        return { ...state, operator: op }
      }

      // 链式求值：执行前一步计算，暂存结果，记录新运算符
      const result = compute(state.operand, state.operator!, parseDisplay(state.display))
      if (result === null) {
        return { ...state, display: '错误', hasError: true }
      }
      return {
        ...state,
        display: formatDisplay(result),
        operand: result,
        operator: op,
        waitingForNext: true,
      }
    }

    case 'EQUALS': {
      if (state.operand === null || state.operator === null) {
        // 无可执行的运算，保持当前显示
        return state
      }

      // 不完整表达式：waitingForNext 时重复最后操作数（即当前显示值）
      const secondOperand = parseDisplay(state.display)
      const result = compute(state.operand, state.operator, secondOperand)
      if (result === null) {
        return { ...state, display: '错误', hasError: true }
      }
      return {
        ...state,
        display: formatDisplay(result),
        operand: null,
        operator: null,
        waitingForNext: true,
      }
    }

    case 'BACKSPACE': {
      if (state.waitingForNext) {
        // 等待下一操作数时不可退格
        return state
      }

      let next = state.display.slice(0, -1)
      // 删空或仅剩负号 → 归零
      if (next === '' || next === '-') {
        next = '0'
      }
      return { ...state, display: next }
    }

    case 'TOGGLE_SIGN': {
      if (state.display === '0' || state.display === '错误') {
        return state
      }
      if (state.display.startsWith('-')) {
        return { ...state, display: state.display.slice(1) }
      }
      return { ...state, display: '-' + state.display }
    }

    default:
      return state
  }
}
