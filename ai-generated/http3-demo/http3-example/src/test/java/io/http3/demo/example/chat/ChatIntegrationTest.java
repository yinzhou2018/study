package io.http3.demo.example.chat;

import io.http3.demo.api.StreamCall;
import io.http3.demo.api.StreamHandler;
import io.http3.demo.api.StreamMessage;
import io.http3.demo.core.impl.NettyStreamClient;
import io.http3.demo.core.impl.NettyStreamServer;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Integration test for Chat example.
 */
class ChatIntegrationTest {

    private NettyStreamServer server;

    @BeforeEach
    void setUp() throws Exception {
        server = new NettyStreamServer();
        server.registerService("/chat", (StreamHandler<String, String>) (request, response) -> {
            request.onMessage(msg -> {
                response.send(StreamMessage.data("[Server] Received: " + msg.payload()));
            });
            request.onComplete(() -> {
                response.send(StreamMessage.data("[Server] Goodbye!"));
                response.complete();
            });
        });
        server.start(19998);
    }

    @AfterEach
    void tearDown() throws Exception {
        server.stop();
    }

    @Test
    void testChatMessages() throws Exception {
        NettyStreamClient client = new NettyStreamClient();
        try {
            client.connect("localhost", 19998);
            StreamCall<String, String> call = client.newCall("/chat", String.class, String.class);

            List<String> received = new ArrayList<>();
            CompletableFuture<Void> done = new CompletableFuture<>();
            call.responseStream()
                    .onMessage(msg -> received.add(msg.payload()))
                    .onComplete(() -> done.complete(null))
                    .onError(done::completeExceptionally);

            call.requestStream().send(StreamMessage.data("Hello"));
            call.requestStream().send(StreamMessage.data("How are you?"));
            call.requestStream().complete();

            done.get(5, TimeUnit.SECONDS);
            assertEquals(3, received.size());
            assertTrue(received.get(0).contains("Hello"));
            assertTrue(received.get(1).contains("How are you?"));
            assertTrue(received.get(2).contains("Goodbye"));

        } finally {
            client.shutdown();
        }
    }

    @Test
    void testChatMultipleClients() throws Exception {
        // Client 1
        NettyStreamClient client1 = new NettyStreamClient();
        // Client 2
        NettyStreamClient client2 = new NettyStreamClient();
        try {
            client1.connect("localhost", 19998);
            client2.connect("localhost", 19998);

            StreamCall<String, String> call1 = client1.newCall("/chat", String.class, String.class);
            StreamCall<String, String> call2 = client2.newCall("/chat", String.class, String.class);

            CompletableFuture<String> result1 = new CompletableFuture<>();
            CompletableFuture<String> result2 = new CompletableFuture<>();

            call1.responseStream()
                    .onMessage(msg -> result1.complete(msg.payload()))
                    .onError(result1::completeExceptionally);
            call2.responseStream()
                    .onMessage(msg -> result2.complete(msg.payload()))
                    .onError(result2::completeExceptionally);

            call1.requestStream().send(StreamMessage.data("from client1"));
            call1.requestStream().complete();
            call2.requestStream().send(StreamMessage.data("from client2"));
            call2.requestStream().complete();

            assertEquals("[Server] Received: from client1", result1.get(5, TimeUnit.SECONDS));
            assertEquals("[Server] Received: from client2", result2.get(5, TimeUnit.SECONDS));

        } finally {
            client1.shutdown();
            client2.shutdown();
        }
    }
}