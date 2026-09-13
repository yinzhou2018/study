import { useEffect } from 'react'
import { Calculator } from './components/Calculator'
import { useCalculator } from './hooks/useCalculator'
import type { Operator } from './lib/compute'
import './App.css'

export default function App() {
  const calc = useCalculator()

  // 键盘输入支持（P1）：全局监听 + preventDefault 阻止浏览器默认行为
  // useCalculator 返回值经 useMemo 稳定，仅在相关状态变化时更新引用
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      const { key } = e

      // 数字键 0-9
      if (/^[0-9]$/.test(key)) {
        e.preventDefault()
        calc.pressDigit(key)
        return
      }

      // 小数点
      if (key === '.') {
        e.preventDefault()
        calc.pressDecimal()
        return
      }

      // 运算符
      if (key === '+' || key === '-') {
        e.preventDefault()
        calc.pressOperator(key as Operator)
        return
      }
      if (key === '*') {
        e.preventDefault()
        calc.pressOperator('x')
        return
      }
      if (key === '/') {
        e.preventDefault()
        calc.pressOperator('/')
        return
      }

      // 等号
      if (key === 'Enter' || key === '=') {
        e.preventDefault()
        calc.pressEquals()
        return
      }

      // 清除
      if (key === 'Escape') {
        e.preventDefault()
        calc.pressClear()
        return
      }

      // 退格
      if (key === 'Backspace') {
        e.preventDefault()
        calc.pressBackspace()
        return
      }
    }

    window.addEventListener('keydown', handleKeyDown)
    return () => window.removeEventListener('keydown', handleKeyDown)
  }, [calc])

  return (
    <div className="app">
      <div className="calculator-shell">
        <Calculator
          displayValue={calc.displayValue}
          hasError={calc.hasError}
          activeOperator={calc.activeOperator}
          onAction={calc.onAction}
        />
      </div>
    </div>
  )
}
