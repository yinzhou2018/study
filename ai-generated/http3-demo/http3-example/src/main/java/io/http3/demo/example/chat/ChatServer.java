package io.http3.demo.example.chat;

import io.http3.demo.api.*;
import io.http3.demo.core.impl.NettyStreamServer;

/**
 * Simple chat server that handles JOIN/MESSAGE/LEAVE chat protocol.
 * <p>
 * Usage:
 * <pre>
 *   ChatServer server = new ChatServer();
 *   server.start(9999);
 * </pre>
 */
public class ChatServer {

    private final NettyStreamServer server;

    public ChatServer() {
        this.server = new NettyStreamServer();
        server.registerService("/chat", (StreamHandler<String, String>) (request, response) -> {
            request.onMessage(msg -> {
                String payload = msg.payload();
                System.out.println("[ChatServer] " + payload);

                // Echo the chat message back with a server prefix
                response.send(StreamMessage.data("[Server] Received: " + payload));
            });
            request.onComplete(() -> {
                System.out.println("[ChatServer] Client left the chat.");
                response.send(StreamMessage.data("[Server] Goodbye!"));
                response.complete();
            });
            request.onError(cause -> {
                System.err.println("[ChatServer] Error: " + cause.getMessage());
                response.cancel(cause);
            });
        });
    }

    public void start(int port) throws Exception {
        server.start(port);
        System.out.println("[ChatServer] Chat server listening on UDP port " + port);
    }

    public void stop() throws Exception {
        server.stop();
    }

    public static void main(String[] args) throws Exception {
        ChatServer server = new ChatServer();
        server.start(9999);
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            try {
                server.stop();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }));
        Thread.currentThread().join();
    }
}