package io.quic.stream.api;

/**
 * A server that accepts QUIC connections and dispatches streaming calls
 * to registered handlers.
 */
public interface StreamServer {

    /**
     * Register a handler for the given path.
     *
     * @param path    the service path (e.g., "/echo")
     * @param handler the handler that processes streaming calls
     * @param <ReqT>  the request payload type
     * @param <RespT> the response payload type
     */
    <ReqT, RespT> void registerService(String path, StreamHandler<ReqT, RespT> handler);

    /**
     * Start the server and listen on the given port.
     *
     * @param port the UDP port to listen on
     */
    void start(int port) throws Exception;

    /**
     * Gracefully stop the server.
     */
    void stop() throws Exception;
}