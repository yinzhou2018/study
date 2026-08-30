package io.quic.stream.example.streamdata;

import io.quic.stream.api.StreamCall;
import io.quic.stream.api.StreamHandler;
import io.quic.stream.api.StreamMessage;
import io.quic.stream.core.impl.NettyStreamClient;
import io.quic.stream.core.impl.NettyStreamServer;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Integration test for StreamData example.
 */
class StreamDataIntegrationTest {

    private NettyStreamServer server;

    @BeforeEach
    void setUp() throws Exception {
        server = new NettyStreamServer();
        server.registerService("/stream-data", (StreamHandler<String, String>) (request, response) -> {
            AtomicInteger counter = new AtomicInteger(0);
            request.onMessage(msg -> {
                int seq = counter.incrementAndGet();
                response.send(StreamMessage.data("data-" + seq));
                if (seq >= 3) {
                    response.complete();
                }
            });
            request.onComplete(() -> {
                if (counter.get() == 0) {
                    response.complete();
                }
            });
        });
        server.start(19997);
    }

    @AfterEach
    void tearDown() throws Exception {
        server.stop();
    }

    @Test
    void testStreamDataReceivesMultipleMessages() throws Exception {
        NettyStreamClient client = new NettyStreamClient();
        try {
            client.connect("localhost", 19997);
            StreamCall<String, String> call = client.newCall("/stream-data", String.class, String.class);

            List<String> received = new ArrayList<>();
            CompletableFuture<Void> done = new CompletableFuture<>();
            call.responseStream()
                    .onMessage(msg -> received.add(msg.payload()))
                    .onComplete(() -> done.complete(null))
                    .onError(done::completeExceptionally);

            call.requestStream().send(StreamMessage.data("start"));
            call.requestStream().send(StreamMessage.data("go"));
            call.requestStream().send(StreamMessage.data("done"));

            done.get(5, TimeUnit.SECONDS);
            assertEquals(3, received.size());
            assertEquals("data-1", received.get(0));
            assertEquals("data-2", received.get(1));
            assertEquals("data-3", received.get(2));

        } finally {
            client.shutdown();
        }
    }

    @Test
    void testStreamDataCancel() throws Exception {
        NettyStreamClient client = new NettyStreamClient();
        try {
            client.connect("localhost", 19997);
            StreamCall<String, String> call = client.newCall("/stream-data", String.class, String.class);

            List<String> received = new ArrayList<>();
            CompletableFuture<Void> done = new CompletableFuture<>();
            call.responseStream()
                    .onMessage(msg -> {
                        received.add(msg.payload());
                        if (received.size() >= 1) {
                            call.requestStream().send(StreamMessage.data("cancel"));
                        }
                    })
                    .onComplete(() -> done.complete(null))
                    .onError(done::completeExceptionally);

            call.requestStream().send(StreamMessage.data("start"));
            call.requestStream().send(StreamMessage.data("cancel"));

            done.get(5, TimeUnit.SECONDS);
            assertTrue(received.size() >= 1);

        } finally {
            client.shutdown();
        }
    }
}