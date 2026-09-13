interface ButtonProps {
  label: string
  onPress: () => void
  variant?: 'number' | 'operator' | 'function' | 'equals'
}

const variantClass: Record<NonNullable<ButtonProps['variant']>, string> = {
  number: 'btn number',
  operator: 'btn operator',
  function: 'btn function',
  equals: 'btn equals',
}

export function Button({ label, onPress, variant = 'number' }: ButtonProps) {
  return (
    <button
      className={variantClass[variant]}
      onClick={onPress}
      aria-label={label}
      type="button"
    >
      {label}
    </button>
  )
}
