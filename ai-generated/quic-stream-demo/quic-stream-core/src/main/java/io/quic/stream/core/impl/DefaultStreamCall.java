package io.quic.stream.core.impl;

import io.quic.stream.api.*;
import io.quic.stream.core.codec.StreamMessageDecoder;
import io.quic.stream.core.codec.StreamMessageEncoder;
import io.netty.channel.Channel;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.channel.ChannelInitializer;
import io.netty.incubator.codec.quic.QuicStreamChannel;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;

/**
 * Default implementation of {@link StreamCall} that binds a Netty QUIC stream channel
 * to the request/response stream interfaces.
 */
public class DefaultStreamCall<ReqT, RespT> implements StreamCall<ReqT, RespT> {

    private final DefaultStreamSender<ReqT> requestStream;
    private final DefaultStreamReceiver<RespT> responseStream;
    private final CompletableFuture<Void> completionFuture = new CompletableFuture<>();
    private final QuicStreamChannel channel;

    public DefaultStreamCall(QuicStreamChannel channel,
                             Class<ReqT> requestClass,
                             Class<RespT> responseClass) {
        this.channel = channel;
        this.requestStream = new DefaultStreamSender<>(channel, requestClass);
        this.responseStream = new DefaultStreamReceiver<>(channel, responseClass);

        // Add the inbound handler to receive response messages
        channel.pipeline().addLast(new ChannelInboundHandlerAdapter() {
            @SuppressWarnings("unchecked")
            @Override
            public void channelRead(ChannelHandlerContext ctx, Object msg) {
                if (msg instanceof StreamMessage) {
                    responseStream.handleMessage((StreamMessage<RespT>) msg);
                }
            }

            @Override
            public void channelInactive(ChannelHandlerContext ctx) {
                completionFuture.complete(null);
                responseStream.handleComplete();
            }

            @Override
            public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
                completionFuture.completeExceptionally(cause);
                responseStream.handleError(cause);
            }
        });
    }

    @Override
    public StreamSender<ReqT> requestStream() {
        return requestStream;
    }

    @Override
    public StreamReceiver<RespT> responseStream() {
        return responseStream;
    }

    @Override
    public void await() throws InterruptedException {
        try {
            completionFuture.get();
        } catch (java.util.concurrent.ExecutionException e) {
            throw new RuntimeException("Stream call failed", e.getCause());
        }
    }

    /**
     * Sender implementation that writes to the Netty channel.
     */
    private static class DefaultStreamSender<T> implements StreamSender<T> {

        private final Channel channel;
        private final AtomicBoolean closed = new AtomicBoolean(false);

        DefaultStreamSender(Channel channel, Class<T> payloadType) {
            this.channel = channel;
            this.channel.pipeline().addFirst(new StreamMessageEncoder(payloadType));
        }

        @Override
        public void send(StreamMessage<T> message) {
            if (closed.get()) {
                throw new IllegalStateException("Stream is already closed");
            }
            channel.writeAndFlush(message);
            if (message.isComplete() || message.isError()) {
                closed.set(true);
            }
        }

        @Override
        public void complete() {
            if (closed.compareAndSet(false, true)) {
                channel.writeAndFlush(StreamMessage.complete());
            }
        }

        @Override
        public void cancel(Throwable cause) {
            if (closed.compareAndSet(false, true)) {
                channel.writeAndFlush(StreamMessage.error(cause.getMessage()));
            }
        }
    }

    /**
     * Receiver implementation that collects messages from the Netty channel.
     */
    private static class DefaultStreamReceiver<T> implements StreamReceiver<T> {

        private Consumer<StreamMessage<T>> messageHandler;
        private Runnable completeHandler;
        private Consumer<Throwable> errorHandler;

        DefaultStreamReceiver(Channel channel, Class<T> payloadType) {
            channel.pipeline().addLast(new StreamMessageDecoder(payloadType));
        }

        @Override
        public StreamReceiver<T> onMessage(Consumer<StreamMessage<T>> handler) {
            this.messageHandler = handler;
            return this;
        }

        @Override
        public StreamReceiver<T> onComplete(Runnable handler) {
            this.completeHandler = handler;
            return this;
        }

        @Override
        public StreamReceiver<T> onError(Consumer<Throwable> handler) {
            this.errorHandler = handler;
            return this;
        }

        void handleMessage(StreamMessage<T> msg) {
            if (msg.isComplete()) {
                if (completeHandler != null) {
                    completeHandler.run();
                }
            } else if (msg.isError()) {
                if (errorHandler != null) {
                    errorHandler.accept(new RuntimeException(String.valueOf(msg.payload())));
                }
            } else {
                if (messageHandler != null) {
                    messageHandler.accept(msg);
                }
            }
        }

        void handleComplete() {
            if (completeHandler != null) {
                completeHandler.run();
            }
        }

        void handleError(Throwable cause) {
            if (errorHandler != null) {
                errorHandler.accept(cause);
            }
        }
    }
}