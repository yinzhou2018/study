package io.http3.demo.core.impl;

import io.http3.demo.api.StreamHandler;
import io.netty.buffer.ByteBuf;
import io.netty.channel.*;
import io.netty.incubator.codec.quic.QuicStreamChannel;
import io.netty.util.CharsetUtil;

import java.util.concurrent.ConcurrentMap;

/**
 * Dispatches incoming QUIC streams to the registered {@link StreamHandler} based on the path.
 * <p>
 * The first message on a new stream is expected to be the path (e.g., "/echo").
 * After the path is received, the stream is handed off to the appropriate handler.
 */
public class StreamDispatchHandler extends ChannelInboundHandlerAdapter {

    private final ConcurrentMap<String, StreamHandler<?, ?>> handlers;

    public StreamDispatchHandler(ConcurrentMap<String, StreamHandler<?, ?>> handlers) {
        this.handlers = handlers;
    }

    @Override
    public void channelActive(ChannelHandlerContext ctx) {
        // When a new QUIC stream is created on this connection
        QuicStreamChannel streamChannel = (QuicStreamChannel) ctx.channel();
        // Add a handler to read the path from the first message
        streamChannel.pipeline().addLast(new PathReadingHandler(handlers));
        ctx.fireChannelActive();
    }

    /**
     * Reads the first message (path) from the stream, then replaces itself
     * with the actual handler pipeline.
     */
    private static class PathReadingHandler extends ChannelInboundHandlerAdapter {

        private final ConcurrentMap<String, StreamHandler<?, ?>> handlers;
        private boolean pathRead;

        PathReadingHandler(ConcurrentMap<String, StreamHandler<?, ?>> handlers) {
            this.handlers = handlers;
        }

        @Override
        public void channelRead(ChannelHandlerContext ctx, Object msg) {
            if (!pathRead && msg instanceof ByteBuf) {
                pathRead = true;
                ByteBuf buf = (ByteBuf) msg;
                String path = buf.toString(CharsetUtil.UTF_8);
                buf.release();
                StreamHandler<?, ?> handler = handlers.get(path);

                if (handler == null) {
                    ctx.close();
                    return;
                }

                // Remove this handler and invoke the stream handler
                ctx.pipeline().remove(this);
                invokeHandler(ctx, handler);
            } else {
                ctx.fireChannelRead(msg);
            }
        }

        @SuppressWarnings({"unchecked", "rawtypes"})
        private void invokeHandler(ChannelHandlerContext ctx, StreamHandler handler) {
            QuicStreamChannel streamChannel = (QuicStreamChannel) ctx.channel();

            // Create a DefaultStreamCall to bridge the API and the channel
            DefaultStreamCall<?, ?> call = new DefaultStreamCall(
                    streamChannel, Object.class, Object.class);

            handler.handle(call.responseStream(), call.requestStream());
        }
    }
}