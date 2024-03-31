use async_std::io::ReadExt;
use async_std::io::WriteExt;
use async_std::net::TcpListener as AsyncTcpListener;
use async_std::net::TcpStream as AsyncTcpStream;
use async_std::task;
use futures::stream::StreamExt;
use std::io::prelude::*;
use std::net::TcpListener;
use std::net::TcpStream;
use std::time::Duration;

const HELLO_HTML: &str = r#"
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>Hello!</title>
  </head>
  <body>
    <h1>Hello!</h1>
    <p>Hi from Rust</p>
  </body>
</html>"#;

const NOT_FOUND_HTML: &str = r#"
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>Hello!</title>
  </head>
  <body>
    <h1>Oops!</h1>
    <p>Sorry, I don't know what you're asking for.</p>
  </body>
</html>
"#;

#[allow(dead_code)]
pub fn start_st_server() {
  // 监听本地端口 7878 ，等待 TCP 连接的建立
  let listener = TcpListener::bind("127.0.0.1:7878").unwrap();

  // 阻塞等待请求的进入
  for stream in listener.incoming() {
    let stream = stream.unwrap();
    handle_connection(stream);
  }
}

#[async_std::main]
pub async fn main() {
  println!("Async server started...");
  // let listener = TcpListener::bind("127.0.0.1:8787").unwrap();
  // for stream in listener.incoming() {
  //   let stream = stream.unwrap();
  //   // 警告，这里无法并发
  //   async_handle_connection(stream).await;
  // }
  let listener = AsyncTcpListener::bind("127.0.0.1:7878").await.unwrap();
  listener
    .incoming()
    .for_each_concurrent(/* limit */ None, |tcpstream| async move {
      let tcpstream = tcpstream.unwrap();
      // async_handle_connection(tcpstream).await;
      task::spawn(async_handle_connection(tcpstream));
    })
    .await;
}

fn handle_connection(mut stream: TcpStream) {
  // 从连接中顺序读取 1024 字节数据
  let mut buffer = [0; 1024];
  stream.read(&mut buffer).unwrap();

  let get = b"GET / HTTP/1.1\r\n";

  // 处理HTTP协议头，若不符合则返回404和对应的 `html` 文件
  let (status_line, contents) = if buffer.starts_with(get) {
    ("HTTP/1.1 200 OK\r\n\r\n", HELLO_HTML)
  } else {
    ("HTTP/1.1 404 NOT FOUND\r\n\r\n", NOT_FOUND_HTML)
  };

  // 将回复内容写入连接缓存中
  let response = format!("{status_line}{contents}");
  stream.write_all(response.as_bytes()).unwrap();
  // 使用 flush 将缓存中的内容发送到客户端
  stream.flush().unwrap();
}

#[allow(dead_code)]
async fn async_handle_connection(mut stream: AsyncTcpStream) {
  let mut buffer = [0; 1024];
  stream.read(&mut buffer).await.unwrap();

  let get = b"GET / HTTP/1.1\r\n";
  let sleep = b"GET /sleep HTTP/1.1\r\n";

  let (status_line, contents) = if buffer.starts_with(get) {
    ("HTTP/1.1 200 OK\r\n\r\n", HELLO_HTML)
  } else if buffer.starts_with(sleep) {
    task::sleep(Duration::from_secs(10)).await;
    ("HTTP/1.1 200 OK\r\n\r\n", HELLO_HTML)
  } else {
    ("HTTP/1.1 404 NOT FOUND\r\n\r\n", NOT_FOUND_HTML)
  };

  let response = format!("{status_line}{contents}");
  stream.write(response.as_bytes()).await.unwrap();
  stream.flush().await.unwrap();
}
