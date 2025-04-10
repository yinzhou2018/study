// The Swift Programming Language
// https://docs.swift.org/swift-book

import Foundation

@available(macOS 10.15.0, *)
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

@available(macOS 10.15.0, *)
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
