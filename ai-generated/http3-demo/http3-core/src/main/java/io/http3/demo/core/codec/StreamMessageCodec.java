package io.http3.demo.core.codec;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.type.TypeFactory;
import io.http3.demo.api.StreamMessage;

import java.io.IOException;

/**
 * Low-level codec for serializing/deserializing {@link StreamMessage} to/from JSON bytes.
 * Used by the Netty encoders/decoders.
 */
public class StreamMessageCodec {

    private static final ObjectMapper MAPPER = new ObjectMapper();

    private StreamMessageCodec() {
    }

    /**
     * Encode a message to JSON bytes.
     */
    public static byte[] encode(StreamMessage<?> message) {
        try {
            return MAPPER.writeValueAsBytes(message);
        } catch (IOException e) {
            throw new RuntimeException("Failed to encode StreamMessage", e);
        }
    }

    /**
     * Decode JSON bytes to a StreamMessage with the given payload type.
     */
    @SuppressWarnings("unchecked")
    public static <T> StreamMessage<T> decode(byte[] bytes, Class<T> payloadType) {
        try {
            return MAPPER.readValue(bytes,
                    TypeFactory.defaultInstance().constructParametricType(StreamMessage.class, payloadType));
        } catch (IOException e) {
            throw new RuntimeException("Failed to decode StreamMessage", e);
        }
    }
}