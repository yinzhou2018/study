import { roundResult } from './roundResult'

export const MAX_DISPLAY_LENGTH = 12

/**
 * 显示格式化：处理溢出、科学计数法、错误提示。
 * - 非有限数（NaN/Infinity）→ "错误"
 * - 整数位长度 <= 12 → 直接字符串
 * - 超长 → 尝试科学计数法（6 位有效数字）
 * - 仍超长 → "溢出"
 */
export function formatDisplay(value: number): string {
  if (!Number.isFinite(value)) return '错误'
  if (value === 0) return '0'

  const rounded = roundResult(value)
  const str = rounded.toString()

  if (str.length <= MAX_DISPLAY_LENGTH) return str

  // 尝试科学计数法（6 位有效数字）
  const exp = rounded.toExponential(6)
  if (exp.length <= MAX_DISPLAY_LENGTH) return exp

  return '溢出'
}
