import { useReducer, useCallback } from 'react'
import { calculatorReducer, initialState, type Action } from '../lib/calculator'
import type { Operator } from '../lib/compute'

export function useCalculator() {
  const [state, dispatch] = useReducer(calculatorReducer, initialState)

  const onAction = useCallback((action: Action) => {
    dispatch(action)
  }, [])

  const pressDigit = useCallback((d: string) => dispatch({ type: 'DIGIT', digit: d }), [])
  const pressDecimal = useCallback(() => dispatch({ type: 'DECIMAL' }), [])
  const pressOperator = useCallback((op: Operator) => dispatch({ type: 'OPERATOR', op }), [])
  const pressEquals = useCallback(() => dispatch({ type: 'EQUALS' }), [])
  const pressClear = useCallback(() => dispatch({ type: 'CLEAR' }), [])
  const pressBackspace = useCallback(() => dispatch({ type: 'BACKSPACE' }), [])
  const toggleSign = useCallback(() => dispatch({ type: 'TOGGLE_SIGN' }), [])

  return {
    displayValue: state.display,
    hasError: state.hasError,
    onAction,
    pressDigit,
    pressDecimal,
    pressOperator,
    pressEquals,
    pressClear,
    pressBackspace,
    toggleSign,
  }
}
