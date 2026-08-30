package io.quic.stream.core.impl;

import io.quic.stream.api.StreamHandler;
import io.quic.stream.api.StreamServer;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.nio.NioDatagramChannel;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;
import io.netty.incubator.codec.quic.*;

import javax.net.ssl.KeyManagerFactory;
import java.io.InputStream;
import java.security.KeyStore;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.TimeUnit;

/**
 * QUIC server implementation based on Netty.
 */
public class NettyStreamServer implements StreamServer {

    private final EventLoopGroup group;
    private final ConcurrentMap<String, StreamHandler<?, ?>> handlers = new ConcurrentHashMap<>();
    private Channel serverChannel;
    private volatile boolean running;

    public NettyStreamServer() {
        this.group = new NioEventLoopGroup(1);
    }

    @Override
    @SuppressWarnings("unchecked")
    public <ReqT, RespT> void registerService(String path, StreamHandler<ReqT, RespT> handler) {
        handlers.put(path, handler);
    }

    @Override
    public void start(int port) throws Exception {
        Quic.ensureAvailability();

        // Load the self-signed certificate
        KeyStore keyStore = KeyStore.getInstance("PKCS12");
        try (InputStream in = getClass().getClassLoader().getResourceAsStream("certs/server.p12")) {
            if (in == null) {
                throw new RuntimeException("TLS certificate not found at certs/server.p12");
            }
            keyStore.load(in, "changeit".toCharArray());
        }

        KeyManagerFactory kmf = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        kmf.init(keyStore, "changeit".toCharArray());

        QuicSslContext sslContext = QuicSslContextBuilder.forServer(kmf, "changeit")
                .applicationProtocols("h3")
                .build();

        ChannelHandler codec = new QuicServerCodecBuilder()
                .sslContext(sslContext)
                .tokenHandler(InsecureQuicTokenHandler.INSTANCE)
                .initialMaxData(10000000)
                .initialMaxStreamDataBidirectionalLocal(1000000)
                .initialMaxStreamDataBidirectionalRemote(1000000)
                .initialMaxStreamsBidirectional(100)
                .initialMaxStreamsUnidirectional(100)
                .streamHandler(new ChannelInitializer<QuicStreamChannel>() {
                    @Override
                    protected void initChannel(QuicStreamChannel ch) {
                        ch.pipeline().addLast(new LoggingHandler(LogLevel.INFO));
                        ch.pipeline().addLast(new StreamDispatchHandler(handlers));
                    }
                })
                .build();

        Bootstrap bootstrap = new Bootstrap()
                .group(group)
                .channel(NioDatagramChannel.class)
                .handler(codec);

        this.serverChannel = bootstrap.bind(port).sync().channel();
        this.running = true;
    }

    @Override
    public void stop() throws Exception {
        running = false;
        if (serverChannel != null) {
            serverChannel.close().get(5, TimeUnit.SECONDS);
        }
        group.shutdownGracefully(0, 5, TimeUnit.SECONDS).get();
    }

    public boolean isRunning() {
        return running;
    }
}