package io.quic.stream.api;

/**
 * Type of a stream message.
 */
public enum StreamMessageType {
    DATA,
    COMPLETE,
    ERROR,
    PING
}