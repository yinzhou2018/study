package io.http3.demo.example.echo;

import io.http3.demo.api.StreamCall;
import io.http3.demo.api.StreamHandler;
import io.http3.demo.api.StreamMessage;
import io.http3.demo.core.impl.NettyStreamClient;
import io.http3.demo.core.impl.NettyStreamServer;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

import static org.junit.jupiter.api.Assertions.assertEquals;

/**
 * Integration test for Echo example.
 * Starts a real server and client, sends a message, and verifies the echo response.
 */
class EchoIntegrationTest {

    private NettyStreamServer server;
    private int port;

    @BeforeEach
    void setUp() throws Exception {
        // Find a free port
        port = 19999;
        server = new NettyStreamServer();
        server.registerService("/echo", (StreamHandler<String, String>) (request, response) -> {
            request.onMessage(msg -> {
                response.send(StreamMessage.data(msg.payload()));
            });
            request.onComplete(() -> response.complete());
        });
        server.start(port);
    }

    @AfterEach
    void tearDown() throws Exception {
        server.stop();
    }

    @Test
    void testEchoMessage() throws Exception {
        NettyStreamClient client = new NettyStreamClient();
        try {
            client.connect("localhost", port);
            StreamCall<String, String> call = client.newCall("/echo", String.class, String.class);

            CompletableFuture<String> result = new CompletableFuture<>();
            call.responseStream()
                    .onMessage(msg -> result.complete(msg.payload()))
                    .onError(result::completeExceptionally);

            call.requestStream().send(StreamMessage.data("Hello Echo!"));
            call.requestStream().complete();

            String response = result.get(5, TimeUnit.SECONDS);
            assertEquals("Hello Echo!", response);

        } finally {
            client.shutdown();
        }
    }
}