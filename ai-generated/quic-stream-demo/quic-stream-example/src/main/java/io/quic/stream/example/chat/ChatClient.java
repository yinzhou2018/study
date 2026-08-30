package io.quic.stream.example.chat;

import io.quic.stream.api.*;
import io.quic.stream.core.impl.NettyStreamClient;

/**
 * Chat client that connects to the chat server and exchanges messages.
 * <p>
 * Usage:
 * <pre>
 *   ChatClient client = new ChatClient();
 *   client.run("localhost", 9999, "Hello everyone!", "How are you?");
 * </pre>
 */
public class ChatClient {

    public void run(String host, int port, String... messages) throws Exception {
        NettyStreamClient client = new NettyStreamClient();
        try {
            client.connect(host, port);

            StreamCall<String, String> call = client.newCall("/chat", String.class, String.class);

            // Register response handler
            call.responseStream()
                    .onMessage(msg -> System.out.println("[ChatClient] " + msg.payload()))
                    .onComplete(() -> System.out.println("[ChatClient] Chat ended"))
                    .onError(cause -> System.err.println("[ChatClient] Error: " + cause.getMessage()));

            // Send messages
            for (String msg : messages) {
                System.out.println("[ChatClient] Sending: " + msg);
                call.requestStream().send(StreamMessage.data(msg));
            }

            call.requestStream().complete();
            call.await();

        } finally {
            client.shutdown();
        }
    }

    public static void main(String[] args) throws Exception {
        String host = args.length > 0 ? args[0] : "localhost";
        int port = args.length > 1 ? Integer.parseInt(args[1]) : 9999;
        String[] messages = args.length > 2 ? new String[]{args[2]} : new String[]{"Hello!", "How are you?"};

        new ChatClient().run(host, port, messages);
    }
}