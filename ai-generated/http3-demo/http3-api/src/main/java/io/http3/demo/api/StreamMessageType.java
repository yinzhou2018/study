package io.http3.demo.api;

/**
 * Type of a stream message.
 */
public enum StreamMessageType {
    DATA,
    COMPLETE,
    ERROR,
    PING
}