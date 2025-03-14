// The Swift Programming Language
// https://docs.swift.org/swift-book

import Foundation

func fetchData(from url: String) async throws -> Data {
  guard let url = URL(string: url) else {
    throw URLError(.badURL)
  }
  let data: Data
  if #available(macOS 12.0, *) {
    (data, _) = try await URLSession.shared.data(from: url)
  } else {
    data = try Data(contentsOf: url)
  }
  return data
}

func processData() async {
  do {
    let data = try await fetchData(from: "https://jsonplaceholder.typicode.com/todos/1")
    if let json = try? JSONSerialization.jsonObject(with: data, options: []) {
      print("Fetched and processed data: \(json)")
    }
  } catch {
    print("Failed to fetch data: \(error)")
  }
}

print("Welcome to playground...")
if #available(macOS 10.15, *) {
  print("Main thread ID: \(pthread_mach_thread_np(pthread_self()))")
  print("Ready to create async task...")
  Task { @MainActor in
    print("Before await, thread ID: \(pthread_mach_thread_np(pthread_self()))")
    await processData()
    print("After await, thread ID: \(pthread_mach_thread_np(pthread_self()))")
  }
  print("Already to create async task...")
  RunLoop.main.run()
} else {
  print("macOS 10.15 or newer is required to run this code.")
}
<<<<<<< HEAD
=======
print(p)
 p = 20
print(p)

print("10 + 10 = \(add(a: 10, b: 10))")

>>>>>>> ce865c0afe70976713ea5e85f614556f4a1d2cdf
