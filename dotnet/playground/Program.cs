
Printer printer = new Printer2();
printer.Print();
var val = Lib.Add(10, 20);
Console.WriteLine(val);
Console.WriteLine(printer.Name);
Func<int, int> square = x => x + val;
Console.WriteLine(square(5));
val = 100;
Console.WriteLine(square(10));

class Printer
{
  public virtual void Print()
  {
    Console.WriteLine("Printer.");
  }

  public virtual string Name { get { return "Printer"; } }
}

class Printer2 : Printer
{
  public override void Print()
  {
    Console.WriteLine("Printer2.");
  }

  public override string Name { get { return "Printer2"; } }
}