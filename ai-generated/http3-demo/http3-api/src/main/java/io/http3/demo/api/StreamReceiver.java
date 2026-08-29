package io.http3.demo.api;

import java.util.function.Consumer;

/**
 * Receives messages from a stream (the downstream / response direction).
 *
 * @param <T> the type of the message payload
 */
public interface StreamReceiver<T> {

    /**
     * Register a callback for incoming messages.
     *
     * @param handler handler for received messages
     * @return this receiver for chaining
     */
    StreamReceiver<T> onMessage(Consumer<StreamMessage<T>> handler);

    /**
     * Register a callback for stream completion.
     *
     * @param handler handler invoked when the stream is complete
     * @return this receiver for chaining
     */
    StreamReceiver<T> onComplete(Runnable handler);

    /**
     * Register a callback for stream errors.
     *
     * @param handler handler invoked when an error occurs
     * @return this receiver for chaining
     */
    StreamReceiver<T> onError(Consumer<Throwable> handler);
}