package io.http3.demo.api;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.type.TypeFactory;

import java.io.IOException;

/**
 * Utility for JSON serialization/deserialization of {@link StreamMessage}.
 */
public class StreamMessageCodec {

    private static final ObjectMapper MAPPER = new ObjectMapper();

    private StreamMessageCodec() {
    }

    /**
     * Serialize a {@link StreamMessage} to JSON bytes.
     */
    public static byte[] encode(StreamMessage<?> message) {
        try {
            return MAPPER.writeValueAsBytes(message);
        } catch (IOException e) {
            throw new RuntimeException("Failed to encode StreamMessage", e);
        }
    }

    /**
     * Deserialize JSON bytes to a {@link StreamMessage} with the given payload type.
     */
    public static <T> StreamMessage<T> decode(byte[] bytes, Class<T> payloadType) {
        try {
            return MAPPER.readValue(bytes,
                    TypeFactory.defaultInstance().constructParametricType(StreamMessage.class, payloadType));
        } catch (IOException e) {
            throw new RuntimeException("Failed to decode StreamMessage", e);
        }
    }
}