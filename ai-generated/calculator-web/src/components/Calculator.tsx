import { Display } from './Display'
import { ButtonPad } from './ButtonPad'
import type { Operator } from '../lib/compute'

interface CalculatorProps {
  displayValue: string
  hasError: boolean
  activeOperator: Operator | null
  onAction: (action: import('../lib/calculator').Action) => void
}

export function Calculator({ displayValue, hasError, activeOperator, onAction }: CalculatorProps) {
  return (
    <div className="calculator">
      <Display value={displayValue} hasError={hasError} />
      <ButtonPad onAction={onAction} activeOperator={activeOperator} />
    </div>
  )
}
