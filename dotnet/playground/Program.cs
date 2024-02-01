static async Task Main(string[] args)
{
  Console.WriteLine("Let's start ...");
  var done = DoneAsync();
  Console.WriteLine("Done is running ...");
  var result = await done;
  Console.WriteLine($"Main Thread 1: {Thread.CurrentThread.ManagedThreadId.ToString()}");
  Console.WriteLine(result);
}

static async Task<int> DoneAsync()
{
  var i = 0;
  var j = 10 / i;
  Console.WriteLine(j);
  Console.WriteLine("Warming up ...");
  await Task.Delay(3000);
  Console.WriteLine($"DoneAsync Thread 1: {Thread.CurrentThread.ManagedThreadId.ToString()}");
  Console.WriteLine("Done ...");
  return 1;
}

Console.WriteLine($"Global Thread 1: {Thread.CurrentThread.ManagedThreadId.ToString()}");
await Main(["hello"]);
Console.WriteLine($"Global Thread 2: {Thread.CurrentThread.ManagedThreadId.ToString()}");
