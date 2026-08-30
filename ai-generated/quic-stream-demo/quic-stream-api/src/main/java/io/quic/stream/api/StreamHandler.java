package io.quic.stream.api;

/**
 * A handler for a streaming service.
 *
 * @param <ReqT>  the request payload type
 * @param <RespT> the response payload type
 */
@FunctionalInterface
public interface StreamHandler<ReqT, RespT> {

    /**
     * Handle a streaming call.
     *
     * @param requestStream  the stream of incoming request messages
     * @param responseStream the stream for sending response messages
     */
    void handle(StreamReceiver<ReqT> requestStream, StreamSender<RespT> responseStream);
}