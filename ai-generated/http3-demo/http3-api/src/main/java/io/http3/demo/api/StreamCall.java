package io.http3.demo.api;

/**
 * A bidirectional streaming call that binds a request stream and a response stream
 * into a single call context.
 *
 * @param <ReqT> the type of the request payload
 * @param <RespT> the type of the response payload
 */
public interface StreamCall<ReqT, RespT> {

    /**
     * Returns the sender for the request (upstream) stream.
     */
    StreamSender<ReqT> requestStream();

    /**
     * Returns the receiver for the response (downstream) stream.
     */
    StreamReceiver<RespT> responseStream();

    /**
     * Block until the call is complete.
     */
    void await() throws InterruptedException;
}