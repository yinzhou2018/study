interface DisplayProps {
  value: string
  hasError: boolean
}

export function Display({ value, hasError }: DisplayProps) {
  return (
    <div className="display" role="region" aria-label="计算结果">
      <span className={hasError ? 'display-value error' : 'display-value'}>{value}</span>
    </div>
  )
}
