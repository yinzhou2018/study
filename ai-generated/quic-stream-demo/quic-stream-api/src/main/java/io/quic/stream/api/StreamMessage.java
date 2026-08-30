package io.quic.stream.api;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.type.TypeFactory;

import java.util.Objects;
import java.util.UUID;

/**
 * A generic stream message with a message ID, type, and typed payload.
 *
 * @param <T> the type of the payload
 */
public class StreamMessage<T> {

    private static final ObjectMapper MAPPER = new ObjectMapper();

    @JsonProperty("msgId")
    private String msgId;

    @JsonProperty("type")
    private StreamMessageType type;

    @JsonProperty("payload")
    private T payload;

    /** For Jackson deserialization only. */
    StreamMessage() {
    }

    /**
     * Create a new StreamMessage with a generated UUID.
     *
     * @param type    the message type
     * @param payload the message payload
     */
    public StreamMessage(StreamMessageType type, T payload) {
        this(UUID.randomUUID().toString(), type, payload);
    }

    /**
     * Create a new StreamMessage with a specific message ID.
     *
     * @param msgId   the message ID
     * @param type    the message type
     * @param payload the message payload
     */
    public StreamMessage(String msgId, StreamMessageType type, T payload) {
        this.msgId = Objects.requireNonNull(msgId, "msgId must not be null");
        this.type = Objects.requireNonNull(type, "type must not be null");
        this.payload = payload;
    }

    public String msgId() {
        return msgId;
    }

    public StreamMessageType type() {
        return type;
    }

    public T payload() {
        return payload;
    }

    @JsonIgnore
    public boolean isData() {
        return type == StreamMessageType.DATA;
    }

    @JsonIgnore
    public boolean isComplete() {
        return type == StreamMessageType.COMPLETE;
    }

    @JsonIgnore
    public boolean isError() {
        return type == StreamMessageType.ERROR;
    }

    @JsonIgnore
    public boolean isPing() {
        return type == StreamMessageType.PING;
    }

    /**
     * Serialize this message to JSON bytes.
     */
    public byte[] toBytes() {
        try {
            return MAPPER.writeValueAsBytes(this);
        } catch (JsonProcessingException e) {
            throw new RuntimeException("Failed to serialize StreamMessage", e);
        }
    }

    /**
     * Create a DATA message.
     */
    public static <T> StreamMessage<T> data(T payload) {
        return new StreamMessage<>(StreamMessageType.DATA, payload);
    }

    /**
     * Create a COMPLETE message.
     */
    public static <T> StreamMessage<T> complete() {
        return new StreamMessage<>(StreamMessageType.COMPLETE, null);
    }

    /**
     * Create an ERROR message.
     */
    public static <T> StreamMessage<T> error(String errorMessage) {
        @SuppressWarnings("unchecked")
        T payload = (T) errorMessage;
        return new StreamMessage<>(StreamMessageType.ERROR, payload);
    }

    /**
     * Create a PING message.
     */
    public static <T> StreamMessage<T> ping() {
        return new StreamMessage<>(StreamMessageType.PING, null);
    }

    @Override
    public String toString() {
        return "StreamMessage{" +
                "msgId='" + msgId + '\'' +
                ", type=" + type +
                ", payload=" + payload +
                '}';
    }
}