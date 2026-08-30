package io.quic.stream.example.echo;

import io.quic.stream.api.*;
import io.quic.stream.core.impl.NettyStreamServer;

/**
 * Echo server that replies with the same message it receives.
 * <p>
 * Usage:
 * <pre>
 *   EchoServer server = new EchoServer();
 *   server.start(9999);
 * </pre>
 */
public class EchoServer {

    private final NettyStreamServer server;

    public EchoServer() {
        this.server = new NettyStreamServer();
        server.registerService("/echo", (StreamHandler<String, String>) (request, response) -> {
            request.onMessage(msg -> {
                System.out.println("[EchoServer] Received: " + msg.payload());
                response.send(StreamMessage.data(msg.payload()));
            });
            request.onComplete(() -> {
                System.out.println("[EchoServer] Client finished, closing.");
                response.complete();
            });
            request.onError(cause -> {
                System.err.println("[EchoServer] Error: " + cause.getMessage());
                response.cancel(cause);
            });
        });
    }

    public void start(int port) throws Exception {
        server.start(port);
        System.out.println("[EchoServer] Listening on UDP port " + port);
    }

    public void stop() throws Exception {
        server.stop();
    }

    public static void main(String[] args) throws Exception {
        EchoServer server = new EchoServer();
        server.start(9999);
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            try {
                server.stop();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }));
        // Keep the main thread alive
        Thread.currentThread().join();
    }
}