/**
 * 浮点精度处理 —— 四舍五入截断至 10 位小数。
 * 用于消除 JavaScript 浮点运算误差（如 0.1 + 0.2 = 0.30000000000000004）。
 */
export const MAX_DECIMAL_PLACES = 10

export function roundResult(value: number): number {
  if (!Number.isFinite(value)) return value
  const factor = 10 ** MAX_DECIMAL_PLACES
  return Math.round(value * factor) / factor
}
