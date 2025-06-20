#include "terminal.h"

#include <Windows.h>

#include <iostream>
#include <thread>

void start_cui_app(const char* path) {
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;  // 允许句柄继承
  saAttr.lpSecurityDescriptor = NULL;

  // 创建管道，用于子进程的标准输出
  HANDLE hChildStd_OUT_Rd = NULL;
  HANDLE hChildStd_OUT_Wr = NULL;
  if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
    std::cerr << "Stdout pipe creation failed\n";
    return;
  }
  // 确保读端不被子进程继承
  if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
    std::cerr << "Stdout SetHandleInformation failed\n";
    return;
  }

  // 创建管道，用于子进程的标准输入
  HANDLE hChildStd_IN_Rd = NULL;
  HANDLE hChildStd_IN_Wr = NULL;
  if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
    std::cerr << "Stdin pipe creation failed\n";
    return;
  }
  // 确保写端不被子进程继承
  if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
    std::cerr << "Stdin SetHandleInformation failed\n";
    return;
  }

  // 设置子进程启动信息
  PROCESS_INFORMATION piProcInfo;
  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

  STARTUPINFOA siStartInfo;
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
  siStartInfo.cb = sizeof(STARTUPINFOA);
  siStartInfo.hStdError = hChildStd_OUT_Wr;
  siStartInfo.hStdOutput = hChildStd_OUT_Wr;
  siStartInfo.hStdInput = hChildStd_IN_Rd;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  BOOL bSuccess = CreateProcessA(NULL, (LPSTR)(path ? path : "cmd.exe"), NULL, NULL,
                                 TRUE,  // 继承句柄
                                 0, NULL, NULL, &siStartInfo, &piProcInfo);

  if (!bSuccess) {
    std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
    return;
  }

  // 关闭不需要的句柄
  CloseHandle(hChildStd_OUT_Wr);
  CloseHandle(hChildStd_IN_Rd);

  // 现在可以通过 hChildStd_OUT_Rd 读取子进程输出
  // 通过 hChildStd_IN_Wr 向子进程写入输入

  // 示例：读取子进程输出并打印
  DWORD dwRead;
  CHAR chBuf[4096];
  BOOL bReadSuccess;

  // 启动一个线程来读取标准输入并写入子进程的标准输入
  auto readThread = std::thread([&]() {
    DWORD dwRead;
    CHAR chBuf[4096];
    for (;;) {
      if (!std::cin.getline(chBuf, sizeof(chBuf))) {
        break;
      }
      dwRead = static_cast<DWORD>(strlen(chBuf));
      chBuf[dwRead++] = '\n';  // 添加换行符
      WriteFile(hChildStd_IN_Wr, chBuf, dwRead, &dwRead, NULL);
    }
  });
  readThread.detach();  // 分离线程，允许它在后台运行

  // 读取输出直到管道关闭
  for (;;) {
    bReadSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, sizeof(chBuf) - 1, &dwRead, NULL);
    if (!bReadSuccess || dwRead == 0)
      break;

    chBuf[dwRead] = '\0';  // null terminate
    std::cout << "Redirect child output: " << std::endl;
    std::cout << chBuf;
  }

  // 等待子进程结束
  WaitForSingleObject(piProcInfo.hProcess, INFINITE);

  // 关闭句柄
  CloseHandle(piProcInfo.hProcess);
  CloseHandle(piProcInfo.hThread);
  CloseHandle(hChildStd_OUT_Rd);
  CloseHandle(hChildStd_IN_Wr);
}