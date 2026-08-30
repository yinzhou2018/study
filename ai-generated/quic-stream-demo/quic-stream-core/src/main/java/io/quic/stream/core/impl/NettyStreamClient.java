package io.quic.stream.core.impl;

import io.quic.stream.api.StreamCall;
import io.quic.stream.api.StreamClient;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.util.CharsetUtil;
import io.netty.channel.socket.nio.NioDatagramChannel;
import io.netty.handler.ssl.util.InsecureTrustManagerFactory;
import io.netty.incubator.codec.quic.*;
import io.netty.util.concurrent.Future;

import java.net.InetSocketAddress;
import java.util.concurrent.TimeUnit;

/**
 * QUIC client implementation based on Netty.
 */
public class NettyStreamClient implements StreamClient {

    private final EventLoopGroup group;
    private QuicChannel quicChannel;
    private Channel udpChannel;
    private volatile boolean connected;

    public NettyStreamClient() {
        this.group = new NioEventLoopGroup(1);
    }

    @Override
    public void connect(String host, int port) throws Exception {
        Quic.ensureAvailability();

        QuicSslContext sslContext = QuicSslContextBuilder.forClient()
                .applicationProtocols("h3")
                .trustManager(InsecureTrustManagerFactory.INSTANCE)
                .build();

        ChannelHandler codec = new QuicClientCodecBuilder()
                .sslContext(sslContext)
                .initialMaxData(10000000)
                .initialMaxStreamDataBidirectionalLocal(1000000)
                .initialMaxStreamDataBidirectionalRemote(1000000)
                .initialMaxStreamsBidirectional(100)
                .initialMaxStreamsUnidirectional(100)
                .build();

        Bootstrap bootstrap = new Bootstrap()
                .group(group)
                .channel(NioDatagramChannel.class)
                .handler(codec);

        this.udpChannel = bootstrap.bind(0).sync().channel();

        QuicChannelBootstrap quicBootstrap = QuicChannel.newBootstrap(udpChannel)
                .remoteAddress(new InetSocketAddress(host, port))
                .streamHandler(new ChannelInitializer<QuicStreamChannel>() {
                    @Override
                    protected void initChannel(QuicStreamChannel ch) {
                        // Stream-level handler will be added by DefaultStreamCall
                    }
                });

        Future<QuicChannel> connectFuture = quicBootstrap.connect();
        this.quicChannel = connectFuture.get(10, TimeUnit.SECONDS);
        this.connected = true;
    }

    @Override
    public <ReqT, RespT> StreamCall<ReqT, RespT> newCall(String path, Class<ReqT> requestClass, Class<RespT> responseClass) {
        if (!connected || quicChannel == null) {
            throw new IllegalStateException("Not connected. Call connect() first.");
        }

        try {
            Future<QuicStreamChannel> streamFuture = quicChannel.createStream(
                    QuicStreamType.BIDIRECTIONAL,
                    new ChannelInitializer<QuicStreamChannel>() {
                        @Override
                        protected void initChannel(QuicStreamChannel ch) {
                            // codec handlers will be added by DefaultStreamCall
                        }
                    });

            QuicStreamChannel streamChannel = streamFuture.get(10, TimeUnit.SECONDS);

            // Send path as first message (ByteBuf for QUIC transport)
            streamChannel.writeAndFlush(Unpooled.copiedBuffer(path, CharsetUtil.UTF_8)).get(5, TimeUnit.SECONDS);

            return new DefaultStreamCall<>(streamChannel, requestClass, responseClass);
        } catch (Exception e) {
            throw new RuntimeException("Failed to create stream call", e);
        }
    }

    @Override
    public void shutdown() throws Exception {
        connected = false;
        if (quicChannel != null) {
            quicChannel.close().get(5, TimeUnit.SECONDS);
        }
        if (udpChannel != null) {
            udpChannel.close().get(5, TimeUnit.SECONDS);
        }
        group.shutdownGracefully(0, 5, TimeUnit.SECONDS).get();
    }
}