interface ButtonProps {
  label: string
  onPress: () => void
  variant?: 'number' | 'operator' | 'function' | 'equals'
  active?: boolean
  className?: string
}

const variantClass: Record<NonNullable<ButtonProps['variant']>, string> = {
  number: 'btn number',
  operator: 'btn operator',
  function: 'btn function',
  equals: 'btn equals',
}

export function Button({ label, onPress, variant = 'number', active = false, className }: ButtonProps) {
  const classes = [
    variantClass[variant],
    active ? 'active' : '',
    className ?? '',
  ]
    .filter(Boolean)
    .join(' ')

  return (
    <button
      className={classes}
      onClick={onPress}
      aria-label={label}
      type="button"
    >
      {label}
    </button>
  )
}
