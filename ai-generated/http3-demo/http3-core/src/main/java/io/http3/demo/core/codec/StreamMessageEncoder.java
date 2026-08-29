package io.http3.demo.core.codec;

import io.http3.demo.api.StreamMessage;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;

/**
 * Encodes a {@link StreamMessage} to bytes using length-prefixed framing.
 * <p>
 * Wire format:
 * <pre>
 *   [4 bytes: payload length (big-endian)] [N bytes: JSON payload]
 * </pre>
 */
public class StreamMessageEncoder extends MessageToByteEncoder<StreamMessage<?>> {

    private final Class<?> payloadType;

    public StreamMessageEncoder(Class<?> payloadType) {
        this.payloadType = payloadType;
    }

    @Override
    protected void encode(ChannelHandlerContext ctx, StreamMessage<?> msg, ByteBuf out) {
        byte[] jsonBytes = StreamMessageCodec.encode(msg);
        // Write length prefix (4 bytes, big-endian)
        out.writeInt(jsonBytes.length);
        // Write payload
        out.writeBytes(jsonBytes);
    }
}