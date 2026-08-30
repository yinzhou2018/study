package io.quic.stream.api;

/**
 * A client that establishes QUIC connections and creates streaming calls.
 */
public interface StreamClient {

    /**
     * Connect to a remote server.
     *
     * @param host the server hostname
     * @param port the server port
     */
    void connect(String host, int port) throws Exception;

    /**
     * Create a new streaming call to the given path.
     *
     * @param path          the service path (e.g., "/echo")
     * @param requestClass  the request payload type
     * @param responseClass the response payload type
     * @param <ReqT>        the request payload type
     * @param <RespT>       the response payload type
     * @return a new streaming call
     */
    <ReqT, RespT> StreamCall<ReqT, RespT> newCall(String path, Class<ReqT> requestClass, Class<RespT> responseClass);

    /**
     * Gracefully shut down the client, closing all connections.
     */
    void shutdown() throws Exception;
}