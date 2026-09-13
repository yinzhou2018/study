import { compute, type Operator } from './compute'
import { formatDisplay } from './formatDisplay'

/** 计算器状态机状态 */
export type CalcState = {
  display: string // 当前显示值（输入中的原始字符串，或计算结果格式化字符串）
  operand: number | null // 已暂存的操作数
  operator: Operator | null // 已暂存的运算符
  lastOperand: number | null // 等号后保留的第二操作数（支持连续按等号重复运算）
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
  lastOperand: null,
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
        // 等号后开始新输入：清除上一轮运算状态
        if (state.lastOperand !== null) {
          return {
            ...state,
            display: d,
            operand: null,
            operator: null,
            lastOperand: null,
            waitingForNext: false,
          }
        }
        // 运算符后开始输入新操作数
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
        // 等号后开始新输入：清除上一轮运算状态
        if (state.lastOperand !== null) {
          return {
            ...state,
            display: '0.',
            operand: null,
            operator: null,
            lastOperand: null,
            waitingForNext: false,
          }
        }
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
      const currentNum = parseDisplay(state.display)

      // 溢出后按运算符存入 NaN 的防护：直接进入 Error 状态
      if (Number.isNaN(currentNum)) {
        return { ...state, display: '错误', hasError: true }
      }

      if (state.operand === null) {
        // 首次选择运算符：暂存当前显示值
        return {
          ...state,
          operand: currentNum,
          operator: op,
          lastOperand: null,
          waitingForNext: true,
        }
      }

      // 已有暂存运算符
      if (state.waitingForNext) {
        // 连续运算符输入：替换为最新运算符，不触发计算
        // 等号后按运算符：清除 lastOperand，进入"运算符已选"状态
        return { ...state, operator: op, lastOperand: null }
      }

      // 链式求值：执行前一步计算，暂存结果，记录新运算符
      const result = compute(state.operand, state.operator!, currentNum)
      if (result === null) {
        return { ...state, display: '错误', hasError: true }
      }
      return {
        ...state,
        display: formatDisplay(result),
        operand: result,
        operator: op,
        lastOperand: null,
        waitingForNext: true,
      }
    }

    case 'EQUALS': {
      if (state.operand === null || state.operator === null) {
        // 无可执行的运算，保持当前显示
        return state
      }

      const currentNum = parseDisplay(state.display)

      // 溢出后按等号存入 NaN 的防护：直接进入 Error 状态
      if (Number.isNaN(currentNum)) {
        return { ...state, display: '错误', hasError: true }
      }

      // 连续按等号：用上次结果作为第一操作数，保留的 lastOperand 作为第二操作数重复运算
      // 首次按等号：使用当前显示值作为第二操作数并保留
      const isFirstEquals = !state.waitingForNext || state.lastOperand === null
      const secondOperand = isFirstEquals ? currentNum : state.lastOperand!

      const result = compute(state.operand, state.operator, secondOperand)
      if (result === null) {
        return { ...state, display: '错误', hasError: true }
      }
      return {
        ...state,
        display: formatDisplay(result),
        operand: result,
        lastOperand: secondOperand,
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

      const nextDisplay = state.display.startsWith('-')
        ? state.display.slice(1)
        : '-' + state.display

      // 等号后切换正负号：视为新计算的起始值
      if (state.waitingForNext && state.lastOperand !== null) {
        return {
          ...state,
          display: nextDisplay,
          operand: null,
          operator: null,
          lastOperand: null,
          waitingForNext: false,
        }
      }

      return { ...state, display: nextDisplay }
    }

    default:
      return state
  }
}
