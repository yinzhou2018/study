package io.quic.stream.core.codec;

import io.quic.stream.api.StreamMessage;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;

import java.util.List;

/**
 * Decodes bytes into {@link StreamMessage} using length-prefixed framing.
 * <p>
 * Wire format:
 * <pre>
 *   [4 bytes: payload length (big-endian)] [N bytes: JSON payload]
 * </pre>
 * Handles partial frames (fragmentation) and multiple frames in one buffer.
 */
public class StreamMessageDecoder extends ByteToMessageDecoder {

    private final Class<?> payloadType;

    public StreamMessageDecoder(Class<?> payloadType) {
        this.payloadType = payloadType;
    }

    @Override
    protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
        // Need at least 4 bytes to read the length prefix
        if (in.readableBytes() < 4) {
            return;
        }

        in.markReaderIndex();
        int length = in.readInt();

        // Not enough data for the full frame
        if (in.readableBytes() < length) {
            in.resetReaderIndex();
            return;
        }

        // Read the JSON payload bytes
        byte[] jsonBytes = new byte[length];
        in.readBytes(jsonBytes);

        StreamMessage<?> message = StreamMessageCodec.decode(jsonBytes, payloadType);
        out.add(message);
    }
}