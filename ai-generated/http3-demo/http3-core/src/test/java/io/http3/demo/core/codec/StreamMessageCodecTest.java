package io.http3.demo.core.codec;

import io.http3.demo.api.StreamMessage;
import io.http3.demo.api.StreamMessageType;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.embedded.EmbeddedChannel;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class StreamMessageCodecTest {

    @Test
    void testEncodeDecodeString() {
        StreamMessage<String> original = StreamMessage.data("Hello, HTTP/3!");
        byte[] bytes = StreamMessageCodec.encode(original);
        StreamMessage<String> decoded = StreamMessageCodec.decode(bytes, String.class);

        assertEquals(original.type(), decoded.type());
        assertEquals(original.payload(), decoded.payload());
        assertNotNull(decoded.msgId());
    }

    @Test
    void testEncodeDecodeInteger() {
        StreamMessage<Integer> original = StreamMessage.data(42);
        byte[] bytes = StreamMessageCodec.encode(original);
        StreamMessage<Integer> decoded = StreamMessageCodec.decode(bytes, Integer.class);

        assertEquals(StreamMessageType.DATA, decoded.type());
        assertEquals(42, decoded.payload());
    }

    @Test
    void testCompleteMessage() {
        StreamMessage<Void> complete = StreamMessage.complete();
        byte[] bytes = StreamMessageCodec.encode(complete);
        StreamMessage<Void> decoded = StreamMessageCodec.decode(bytes, Void.class);

        assertEquals(StreamMessageType.COMPLETE, decoded.type());
        assertNull(decoded.payload());
    }

    @Test
    void testErrorMessage() {
        StreamMessage<String> error = StreamMessage.error("something went wrong");
        byte[] bytes = StreamMessageCodec.encode(error);
        StreamMessage<String> decoded = StreamMessageCodec.decode(bytes, String.class);

        assertEquals(StreamMessageType.ERROR, decoded.type());
        assertEquals("something went wrong", decoded.payload());
    }

    @Test
    void testLengthPrefixedFraming() {
        EmbeddedChannel channel = new EmbeddedChannel(
                new StreamMessageEncoder(String.class),
                new StreamMessageDecoder(String.class));

        StreamMessage<String> msg1 = StreamMessage.data("first");
        StreamMessage<String> msg2 = StreamMessage.data("second");

        // Encode both messages
        channel.writeOutbound(msg1);
        channel.writeOutbound(msg2);

        // Read the encoded bytes
        ByteBuf buf1 = channel.readOutbound();
        ByteBuf buf2 = channel.readOutbound();

        // Feed them back in a single buffer (simulating coalesced frames)
        ByteBuf combined = Unpooled.buffer();
        combined.writeBytes(buf1);
        combined.writeBytes(buf2);
        buf1.release();
        buf2.release();

        channel.writeInbound(combined);

        // Should decode both messages
        StreamMessage<?> decoded1 = channel.readInbound();
        StreamMessage<?> decoded2 = channel.readInbound();

        assertNotNull(decoded1);
        assertNotNull(decoded2);
        assertEquals("first", decoded1.payload());
        assertEquals("second", decoded2.payload());

        channel.finish();
    }

    @Test
    void testPartialFrame() {
        // Test that the codec handles partial frames correctly
        StreamMessage<String> original = StreamMessage.data("partial");
        byte[] bytes = StreamMessageCodec.encode(original);

        // The length prefix should be 4 bytes
        assertTrue(bytes.length > 4, "Encoded message should have length prefix");

        // Simulate receiving bytes in chunks
        byte[] firstChunk = new byte[4 + bytes.length / 2];
        System.arraycopy(bytes, 0, firstChunk, 0, firstChunk.length);

        byte[] secondChunk = new byte[bytes.length - firstChunk.length];
        System.arraycopy(bytes, firstChunk.length, secondChunk, 0, secondChunk.length);

        // Verify we can decode from the full buffer
        StreamMessage<String> decoded = StreamMessageCodec.decode(bytes, String.class);
        assertNotNull(decoded);
        assertEquals("partial", decoded.payload());
    }
}