package io.quic.stream.example.echo;

import io.quic.stream.api.*;
import io.quic.stream.core.impl.NettyStreamClient;

/**
 * Echo client that sends messages and prints responses.
 * <p>
 * Usage:
 * <pre>
 *   EchoClient client = new EchoClient();
 *   client.run("localhost", 9999, "Hello, QUIC!");
 * </pre>
 */
public class EchoClient {

    public void run(String host, int port, String... messages) throws Exception {
        NettyStreamClient client = new NettyStreamClient();
        try {
            client.connect(host, port);

            StreamCall<String, String> call = client.newCall("/echo", String.class, String.class);

            // Register response handler
            call.responseStream()
                    .onMessage(msg -> System.out.println("[EchoClient] Received: " + msg.payload()))
                    .onComplete(() -> System.out.println("[EchoClient] Stream complete"))
                    .onError(cause -> System.err.println("[EchoClient] Error: " + cause.getMessage()));

            // Send messages
            for (String msg : messages) {
                System.out.println("[EchoClient] Sending: " + msg);
                call.requestStream().send(StreamMessage.data(msg));
            }

            // Signal completion
            call.requestStream().complete();

            // Wait for the call to finish
            call.await();

        } finally {
            client.shutdown();
        }
    }

    public static void main(String[] args) throws Exception {
        String host = args.length > 0 ? args[0] : "localhost";
        int port = args.length > 1 ? Integer.parseInt(args[1]) : 9999;
        String[] messages = args.length > 2 ? new String[]{args[2]} : new String[]{"Hello, QUIC!"};

        new EchoClient().run(host, port, messages);
    }
}