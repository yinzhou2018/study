package io.http3.demo.api;

/**
 * Sends messages into a stream (the upstream / request direction).
 *
 * @param <T> the type of the message payload
 */
@FunctionalInterface
public interface StreamSender<T> {

    /**
     * Send a message.
     *
     * @param message the message to send
     */
    void send(StreamMessage<T> message);

    /**
     * Notify that the sender has finished sending messages.
     * No further messages may be sent after this call.
     */
    default void complete() {
        send(StreamMessage.complete());
    }

    /**
     * Cancel the stream with an error.
     *
     * @param cause the error cause
     */
    default void cancel(Throwable cause) {
        send(StreamMessage.error(cause.getMessage()));
    }
}