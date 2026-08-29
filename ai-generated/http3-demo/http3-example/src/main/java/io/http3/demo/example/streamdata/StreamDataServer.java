package io.http3.demo.example.streamdata;

import io.http3.demo.api.*;
import io.http3.demo.core.impl.NettyStreamServer;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Server that pushes streaming data (timestamps) to connected clients at regular intervals.
 * <p>
 * Usage:
 * <pre>
 *   StreamDataServer server = new StreamDataServer();
 *   server.start(9999);
 * </pre>
 */
public class StreamDataServer {

    private final NettyStreamServer server;
    private final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    public StreamDataServer() {
        this.server = new NettyStreamServer();
        server.registerService("/stream-data", (StreamHandler<String, String>) (request, response) -> {
            AtomicBoolean cancelled = new AtomicBoolean(false);
            AtomicInteger counter = new AtomicInteger(0);

            // Handle cancellation from client
            request.onMessage(msg -> {
                System.out.println("[StreamDataServer] Client message: " + msg.payload());
                if ("cancel".equalsIgnoreCase(msg.payload())) {
                    cancelled.set(true);
                }
            });

            request.onComplete(() -> {
                System.out.println("[StreamDataServer] Client completed stream.");
                cancelled.set(true);
            });

            request.onError(cause -> {
                System.err.println("[StreamDataServer] Error: " + cause.getMessage());
                cancelled.set(true);
            });

            // Push data every second
            scheduler.scheduleAtFixedRate(() -> {
                if (cancelled.get()) {
                    return;
                }

                int seq = counter.incrementAndGet();
                String timestamp = LocalDateTime.now().format(DateTimeFormatter.ISO_LOCAL_TIME);
                String data = "[" + seq + "] Server timestamp: " + timestamp;

                System.out.println("[StreamDataServer] Pushing: " + data);
                response.send(StreamMessage.data(data));

                // Stop after 10 pushes
                if (seq >= 10) {
                    System.out.println("[StreamDataServer] Push complete.");
                    response.complete();
                    cancelled.set(true);
                }
            }, 0, 1, TimeUnit.SECONDS);
        });
    }

    public void start(int port) throws Exception {
        server.start(port);
        System.out.println("[StreamDataServer] Streaming data server on UDP port " + port);
    }

    public void stop() throws Exception {
        scheduler.shutdown();
        server.stop();
    }

    public static void main(String[] args) throws Exception {
        StreamDataServer server = new StreamDataServer();
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