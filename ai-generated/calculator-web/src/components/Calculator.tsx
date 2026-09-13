import { Display } from './Display'
import { ButtonPad } from './ButtonPad'
import { useCalculator } from '../hooks/useCalculator'

export function Calculator() {
  const { displayValue, hasError, onAction } = useCalculator()

  return (
    <div className="calculator">
      <Display value={displayValue} hasError={hasError} />
      <ButtonPad onAction={onAction} />
    </div>
  )
}
