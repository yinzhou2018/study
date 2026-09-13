import { Button } from './Button'
import type { Action } from '../lib/calculator'
import type { Operator } from '../lib/compute'

interface ButtonPadProps {
  onAction: (action: Action) => void
  activeOperator: Operator | null
}

export function ButtonPad({ onAction, activeOperator }: ButtonPadProps) {
  return (
    <div className="button-pad">
      <Button label="AC" variant="function" onPress={() => onAction({ type: 'CLEAR' })} />
      <Button label="+/-" variant="function" onPress={() => onAction({ type: 'TOGGLE_SIGN' })} />
      <Button label="⌫" variant="function" onPress={() => onAction({ type: 'BACKSPACE' })} />
      <Button label="÷" variant="operator" active={activeOperator === '/'} onPress={() => onAction({ type: 'OPERATOR', op: '/' as Operator })} />

      <Button label="7" onPress={() => onAction({ type: 'DIGIT', digit: '7' })} />
      <Button label="8" onPress={() => onAction({ type: 'DIGIT', digit: '8' })} />
      <Button label="9" onPress={() => onAction({ type: 'DIGIT', digit: '9' })} />
      <Button label="×" variant="operator" active={activeOperator === 'x'} onPress={() => onAction({ type: 'OPERATOR', op: 'x' as Operator })} />

      <Button label="4" onPress={() => onAction({ type: 'DIGIT', digit: '4' })} />
      <Button label="5" onPress={() => onAction({ type: 'DIGIT', digit: '5' })} />
      <Button label="6" onPress={() => onAction({ type: 'DIGIT', digit: '6' })} />
      <Button label="−" variant="operator" active={activeOperator === '-'} onPress={() => onAction({ type: 'OPERATOR', op: '-' as Operator })} />

      <Button label="1" onPress={() => onAction({ type: 'DIGIT', digit: '1' })} />
      <Button label="2" onPress={() => onAction({ type: 'DIGIT', digit: '2' })} />
      <Button label="3" onPress={() => onAction({ type: 'DIGIT', digit: '3' })} />
      <Button label="+" variant="operator" active={activeOperator === '+'} onPress={() => onAction({ type: 'OPERATOR', op: '+' as Operator })} />

      <Button label="0" className="btn-zero" onPress={() => onAction({ type: 'DIGIT', digit: '0' })} />
      <Button label="." onPress={() => onAction({ type: 'DECIMAL' })} />
      <Button label="=" variant="equals" onPress={() => onAction({ type: 'EQUALS' })} />
    </div>
  )
}
