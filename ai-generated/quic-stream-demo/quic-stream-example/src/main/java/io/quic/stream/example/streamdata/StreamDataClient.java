package io.quic.stream.example.streamdata;

import io.quic.stream.api.*;
import io.quic.stream.core.impl.NettyStreamClient;

/**
 * Client that connects to the streaming data server and receives pushed data.
 * <p>
 * Usage:
 * <pre>
 *   StreamDataClient client = new StreamDataClient();
 *   client.run("localhost", 9999);
 * </pre>
 */
public class StreamDataClient {

    public void run(String host, int port) throws Exception {
        NettyStreamClient client = new NettyStreamClient();
        try {
            client.connect(host, port);

            StreamCall<String, String> call = client.newCall("/stream-data", String.class, String.class);

            // Register response handler for pushed data
            call.responseStream()
                    .onMessage(msg -> System.out.println("[StreamDataClient] Received: " + msg.payload()))
                    .onComplete(() -> System.out.println("[StreamDataClient] Data stream complete"))
                    .onError(cause -> System.err.println("[StreamDataClient] Error: " + cause.getMessage()));

            // Send initial request to start streaming
            call.requestStream().send(StreamMessage.data("start"));

            // Wait for the stream to complete
            call.await();

        } finally {
            client.shutdown();
        }
    }

    public static void main(String[] args) throws Exception {
        String host = args.length > 0 ? args[0] : "localhost";
        int port = args.length > 1 ? Integer.parseInt(args[1]) : 9999;

        new StreamDataClient().run(host, port);
    }
}